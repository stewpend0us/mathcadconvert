#include "matlab.hpp"
#include <unordered_map>
#include "converter_func.hpp"

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
static void parens(const pugi::xml_node& node, std::ostream& os)
{
	os << '(';
    matlab::convert(node.first_child(), os);
	os << ')';
}
static converter_func operator_(char op)
{
    return [op](const pugi::xml_node& node, std::ostream& os)
    {
        os << ' ' << op << ' ';
    };
}
static converter_func operator_(std::string_view op)
{
    return [op](const pugi::xml_node& node, std::ostream& os)
    {
        os << ' ' << op << ' ';
    };
}
static void apply(const pugi::xml_node& node, std::ostream& os)
{
    const auto f = node.first_child();
    const auto a = f.next_sibling();
    const auto b = a.next_sibling();
	os << '(';
    matlab::convert(a, os);
    matlab::convert(f, os);
    matlab::convert(b, os);
	os << ')';
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
	{"ml:mult", operator_('*')}, // closure would help
	{"ml:div", operator_('/')}, // closure would help
	{"ml:pow", operator_('^')}, // closure would help
	{"ml:plus", operator_('+')}, // closure would help
	{"ml:minus", operator_('+')}, // closure would help
	{"ml:equal", operator_("==")}, // closure would help
	{"ml:define", define},
	{"ml:eval", traverse},
	{"result", result},
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
		os << "'" << name << "' function not found (traversing)\n";
		//traverse(node);
	}
}