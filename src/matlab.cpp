#include "matlab.hpp"
#include <unordered_map>
#include <string_view>
#include "converter_func.hpp"
#include <stdlib.h>

using sv = std::string_view;
std::set<std::string> defined_id;
std::set<std::string> undefined_id;

static void skip(const pugi::xml_node &node, std::ostream &os)
{
}
static void traverse(const pugi::xml_node &node, std::ostream &os)
{
	auto child = node.first_child();
	while (child)
	{
		matlab::convert(child, os);
		child = child.next_sibling();
	}
}
static void multi(const pugi::xml_node &node, std::ostream &os, sv between)
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
static void function_args(pugi::xml_node args, std::ostream &os)
{
	os << '(';
	while (args)
	{
		matlab::convert(args, os);
		args = args.next_sibling();
		if (args)
			os << ", ";
	}
	os << ')';
}
static void multimul(const pugi::xml_node &node, std::ostream &os)
{
	multi(node, os, " * ");
}
static void sequence(const pugi::xml_node &node, std::ostream &os)
{
	multi(node, os, ", ");
}
static void unitOverride(const pugi::xml_node &node, std::ostream &os)
{
    os << "; % ";
    traverse(node,os);
}
static void echo(const pugi::xml_node &node, std::ostream &os)
{
	os << node.text().get();
}
static std::string id_string(const pugi::xml_node &node)
{
	std::string id(node.text().get());
	const auto subscript = node.attribute("subscript");
	if (subscript)
	{
		id += "_";
		id += subscript.value();
	}
	return id;
}
static void id(const pugi::xml_node &node, std::ostream &os)
{
	std::string name = id_string(node);

	auto it = std::find(defined_id.begin(), defined_id.end(), name);
    if (it == defined_id.end())
		undefined_id.insert(name);

	os << name;
}
static void unitReference(const pugi::xml_node &node, std::ostream &os)
{
	const auto unit = node.attribute("unit");
	if (unit)
		os << unit.value();
	const auto pow_num = node.attribute("power-numerator");
	if (pow_num)
		os << "^" << pow_num.value();
}
static void parens(const pugi::xml_node &node, std::ostream &os)
{
	os << '(';
	matlab::convert(node.first_child(), os);
	os << ')';
}
static void apply_op(const pugi::xml_node &a, sv op, const pugi::xml_node &b, std::ostream &os, const sv sp = " ")
{
	os << '(';
	matlab::convert(a, os);
	os << sp << op << sp;
	matlab::convert(b, os);
	os << ')';
}
static void apply_function(const sv name, const pugi::xml_node& args, std::ostream &os)
{
	os << name;
  function_args(args, os);
}
static void apply_function(const pugi::xml_node fun, std::ostream &os)
{
	matlab::convert(fun, os);
  function_args(fun.next_sibling(), os);
}
static void apply(const pugi::xml_node &node, std::ostream &os)
{
	const auto f = node.first_child();
	const auto fname = sv(f.name());
	if (fname == "ml:id")
    {
        if (sv(f.text().get()) == "if")
            return apply_function("if_", f.next_sibling(), os);
        return apply_function(f,os);
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
            return apply_function("sqrt",a,os);
		if (fname == "ml:Find")
            return apply_function("Find",a,os);
        if (fname == "ml:absval")
            return apply_function("abs",a,os);

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
		if (fname == "ml:greaterThan")
			return apply_op(a, ">", b, os);
		if (fname == "ml:lessThan")
			return apply_op(a, "<", b, os);
        if (fname == "ml:indexer")
            return apply_function(a, os);

		os << "'apply' contains <" << fname << "> with two arguments <" << a.name() << ">, <" << b.name() << ">\n";
		return;
	}
	os << "'apply' contains <" << fname << "> with three (or more) arguments <" << a.name() << ">, <" << b.name() << ">, <" << c.name() << ">\n";
	return;
}
static void define(const pugi::xml_node &node, std::ostream &os)
{
	const auto lhs = node.first_child();
	const auto fname = sv(lhs.name());
	const auto rhs = lhs.next_sibling();
	if (fname == "ml:id")
	{
		defined_id.insert(id_string(lhs));
	}
	matlab::convert(lhs, os);
	if (fname != "ml:function")
	{
		os << " = ";
	}
	matlab::convert(rhs, os);
}
static void boundVars(const pugi::xml_node &node, std::ostream &os)
{
	os << " = @(";
	multi(node,os,", ");
	os << ") ";
}
static void math(const pugi::xml_node &node, std::ostream &os)
{
	matlab::convert(node.first_child(), os);
	os << ";\n";
}
static void range(const pugi::xml_node &node, std::ostream &os)
{
	const auto a = node.first_child();
    const auto b = a.next_sibling();
    os << "((";
    matlab::convert(a,os);
    os << ':';
    matlab::convert(b,os);
    os << ") + ARRAY_OFFSET)";
}
static void text(const pugi::xml_node &node, std::ostream &os)
{
	matlab::convert(node.first_child(), os);
	os << "\n";
}
static void comment(const pugi::xml_node &node, std::ostream &os)
{
	os << "% " << node.text().get();
}
static void result(const pugi::xml_node &node, std::ostream &os)
{
	os << "; \% expected result: ";
	matlab::convert(node.first_child(), os);
}
static void imag(const pugi::xml_node &node, std::ostream &os)
{
	const auto symbol = node.attribute("symbol");
	os << node.text().get() << symbol.value();
}
static void plot(const pugi::xml_node &node, std::ostream &os)
{
    os << "\% a mathcad plot was here but there is no good way to know what was in it\n";
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
		{"ml:imag", imag},
        {"plot", plot},
        {"ml:range", range},
		//{"unitedValue", traverse},
		//{"unitMonomial", traverse},
		//{"unitReference", extract_unit}, // closure would help
		{"ml:unitOverride", unitOverride},
		{"ml:function", traverse},
		{"ml:boundVars", boundVars},
};

void matlab::convert(const pugi::xml_node &node, std::ostream &os)
{
	auto t = node.type();
	if (t != pugi::xml_node_type::node_element && t != pugi::xml_node_type::node_document)
		return;
	const char *name = (t == pugi::xml_node_type::node_document) ? "document" : node.name();
	if (auto func = node_funcs.find(name); func != node_funcs.end() && func->second)
		func->second(node, os);
	else
		os << "'" << name << "' function not found\n";
}

std::set<std::string> matlab::get_undefined_ids()
{
	return undefined_id;
}