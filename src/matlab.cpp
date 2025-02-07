#include "matlab.hpp"
#include <unordered_map>
#include <string_view>
#include "converter_func.hpp"

using sv = std::string_view;

static void skip(const pugi::xml_node& node, std::ostream& os)
{}
static void traverse(const pugi::xml_node& node, std::ostream& os)
{
    auto child = node.first_child();
	while (child)
    {
		matlab::convert(child, os);
        child = child.next_sibling();
    }
}
static void multi(const pugi::xml_node& node, std::ostream& os, sv between)
{
    auto child = node.first_child();
	while (child)
    {
		matlab::convert(child, os);
        child = child.next_sibling();
        if (child)
            os << between;
    }
}
static void multimul(const pugi::xml_node& node, std::ostream& os)
{
    multi(node, os, " * ");
}
static void sequence(const pugi::xml_node& node, std::ostream& os)
{
    multi(node, os, ", ");
}
static void echo(const pugi::xml_node& node, std::ostream& os)
{
	os << node.text().get();
}
static void id(const pugi::xml_node& node, std::ostream& os)
{
	os << node.text().get();
	const auto subscript = node.attribute("subscript");
	if (subscript)
		os << '_' << subscript.value();
}
static void unitReference(const pugi::xml_node& node, std::ostream& os)
{
	const auto unit = node.attribute("unit");
    if (unit)
        os << unit.value();
	const auto pow_num = node.attribute("power-numerator");
	if (pow_num)
		os << "^" << pow_num.value();
}
static void parens(const pugi::xml_node& node, std::ostream& os)
{
	os << '(';
    matlab::convert(node.first_child(), os);
	os << ')';
}
static converter_func operator_(std::string_view op)
{
    return [op](const pugi::xml_node& node, std::ostream& os)
    {
        os << ' ' << op << ' ';
    };
}
static converter_func prefix_(std::string_view op)
{
    return [op](const pugi::xml_node& node, std::ostream& os)
    {
        os << ' ' << op;
    };
}
static void apply_op(const pugi::xml_node& a, sv op, const pugi::xml_node& b, std::ostream& os, const sv sp = " ")
{
    os << '(';
    matlab::convert(a, os);
    os << sp << op << sp;
    matlab::convert(b, os);
    os << ')';
}
static void apply(const pugi::xml_node& node, std::ostream& os)
{
    const auto f = node.first_child();
    const auto fname = sv(f.name());
    if (fname == "ml:id")
    {
        matlab::convert(f, os);
        os << '(';
        auto next = f.next_sibling();
    	while (next)
        {
            matlab::convert(next, os);
            next = next.next_sibling();
            if (next)
                os << ", ";
        }
        os << ')';
        return;
    }

    const auto a = f.next_sibling();
    if (a.type() == pugi::xml_node_type::node_null)
    {
		os << "'apply' contains <" << fname << "> and no other tags\n";
        return;
    }
    const auto b = a.next_sibling();
    if (b.type() == pugi::xml_node_type::node_null)
    {
        if (fname == "ml:neg")
        {
            os << "(-";
            matlab::convert(a, os);
            os << ')';
            return;
        }
        if (fname == "ml:sqrt")
        {
            os << "sqrt(";
            matlab::convert(a, os);
            os << ')';
            return;
        }
        if (fname == "ml:Find")
        {
            os << "Find(";
            matlab::convert(a, os);
            os << ')';
            return;
        }
		os << "'apply' contains <" << fname << "> with one argument <" << a.name() << ">\n";
        return;
    }
    const auto c = b.next_sibling();
    if (c.type() == pugi::xml_node_type::node_null)
    {
        if (fname == "ml:plus")
            return apply_op(a, "+", b, os);
        if (fname == "ml:minus")
            return apply_op(a, "-", b, os);
        if (fname == "ml:mult")
            return apply_op(a, "*", b, os);
        if (fname == "ml:div")
            return apply_op(a, "/", b, os);
        if (fname == "ml:pow")
            return apply_op(a, "^", b, os, "");
        if (fname == "ml:equal")
            return apply_op(a, "==", b, os);
		os << "'apply' contains <" << fname << "> with two arguments <" << a.name() << ">, <" << b.name() << ">\n";
        return;
    }
	os << "'apply' contains <" << fname << "> with three (or more) arguments <" << a.name() << ">, <" << b.name() << ">, <" << c.name() << ">\n";
    return;
}
static void define(const pugi::xml_node& node, std::ostream& os)
{
	const auto lhs = node.first_child();
    const auto rhs = lhs.next_sibling();
    matlab::convert(lhs, os);
	os << " = ";
    matlab::convert(rhs, os);
}
static void math(const pugi::xml_node& node, std::ostream& os)
{
    matlab::convert(node.first_child(), os);
    os << ";\n";
}
static void text(const pugi::xml_node& node, std::ostream& os)
{
    matlab::convert(node.first_child(), os);
    os << "\n";
}
static void comment(const pugi::xml_node& node, std::ostream& os)
{
    os << "% " << node.text().get();
}
static void result(const pugi::xml_node& node, std::ostream& os)
{
    os << "; \% expected result: ";
    matlab::convert(node.first_child(), os);
}
static const std::unordered_map<std::string_view, converter_func> node_funcs = {
	{"document", traverse},
	{"worksheet", traverse},
	{"settings", traverse},
	{"regions", traverse},
	{"region", traverse},
	{"calculation", traverse},
	{"units", traverse},
	{"pointReleaseData", skip},
	{"metadata", skip},
	{"presentation", skip},
	{"calculationBehavior", skip},
	{"math", math},
	{"editor", skip},
	{"fileFormat", skip},
	{"miscellaneous", skip},
	{"textStyle", skip},
	{"rendering", skip},
	{"binaryContent", skip},
	{"ml:provenance", traverse},
	{"originRef", skip},
	{"parentRef", skip},
	{"comment", skip},
	{"originComment", skip},
	{"contentHash", skip},
	{"text", text},
	{"p", comment},
	{"ml:apply", apply},
	{"ml:parens", parens},
	{"ml:real", echo},
	{"ml:id", id},
	{"ml:define", define},
	{"ml:eval", traverse},
	{"result", result},
    {"unitReference", unitReference},
    {"unitMonomial", multimul},
    {"unitedValue", multimul},
    {"ml:sequence", sequence},
	//{"unitedValue", traverse},
	//{"unitMonomial", traverse},
	//{"unitReference", extract_unit}, // closure would help
};

void matlab::convert(const pugi::xml_node& node, std::ostream& os)
{
	auto t = node.type();
	if (t != pugi::xml_node_type::node_element && t != pugi::xml_node_type::node_document)
		return;
	const char* name = (t == pugi::xml_node_type::node_document) ? "document" : node.name();
	if (auto func = node_funcs.find(name); func != node_funcs.end() && func->second)
	{
		func->second(node, os);
	}
	else
	{
		os << "'" << name << "' function not found\n";
	}
}