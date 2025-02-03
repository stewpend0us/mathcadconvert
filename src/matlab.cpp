#include "matlab.hpp"
#include <unordered_map>
#include "converter_func.hpp"

static constexpr auto echo = [](const pugi::xml_node& node, std::ostream& os)
{
	os << node.text().get();
	//traverse_children(node);
};
static constexpr auto id = [](const pugi::xml_node& node, std::ostream& os)
{
	os << node.text().get();
	const auto subscript = node.attribute("subscript");
	if (subscript)
		os << '_' << subscript.value();
	//traverse_children(node);
};
static constexpr auto parens = [](const pugi::xml_node& node, std::ostream& os)
{
	os << '(';
    matlab::convert(node.first_child(), os);
	os << ')';
};
static constexpr auto operator_mult = [](const pugi::xml_node& node, std::ostream& os)
{
	os << " * ";
};
static constexpr auto apply = [](const pugi::xml_node& node, std::ostream& os)
{
    const auto f = node.first_child();
    const auto a = f.next_sibling();
    const auto b = a.next_sibling();
	os << '(';
    matlab::convert(a, os);
    matlab::convert(f, os);
    matlab::convert(b, os);
	os << ')';
};
static const std::unordered_map<std::string_view, converter_func> node_funcs = {
	//{"document", traverse},
	//{"worksheet", traverse},
	//{"settings", traverse},
	//{"calculation", traverse},
	//{"units", traverse},
	//{"pointReleaseData", skip},
	//{"metadata", skip},
	//{"presentation", skip},
	//{"calculationBehavior", skip},
	//{"regions", traverse},
	//{"region", traverse},
	//{"math", traverse},
	//{"editor", skip},
	//{"fileFormat", skip},
	//{"miscellaneous", skip},
	//{"textStyle", skip},
	//{"rendering", skip},
	//{"binaryContent", skip},
	//{"ml:provenance",traverse},
	//{"originRef", skip},
	//{"parentRef", skip},
	//{"comment", skip},
	//{"originComment", skip},
	//{"contentHash", skip},
	//{"text", traverse},
	//{"p", comment},
	{"ml:apply", apply},
	{"ml:parens", parens},
	{"ml:real", echo},
	{"ml:id", id},
	{"ml:mult", operator_mult}, // closure would help
	//{"ml:define", define},
	//{"result", result},
	//{"unitedValue", traverse},
	//{"unitMonomial", traverse},
	//{"unitReference", extract_unit}, // closure would help
};

void matlab::convert(const pugi::xml_node& node, std::ostream& os)
{
    if (!node)
        return;
	//auto t = node.type();
	//if (t != pugi::xml_node_type::node_element && t != pugi::xml_node_type::node_document)
	//	return;
	//const char* name = (t == pugi::xml_node_type::node_document) ? "document" : node.name();
    const char* name = node.name();
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