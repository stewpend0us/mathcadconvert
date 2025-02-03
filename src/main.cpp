#include <iostream>
#include <unordered_map>
#include <string_view>
#include "converter_func.hpp"
#include "matlab.hpp"
/*

void apply_node_funcs(const pugi::xml_node& node);

static constexpr auto traverse_children = [](const pugi::xml_node& node)
{
	if (auto child = node.first_child())
		apply_node_funcs(child);
};
static constexpr auto traverse_next = [](const pugi::xml_node& node)
{
	if (auto next = node.next_sibling())
		apply_node_funcs(next);
};
static constexpr auto skip = traverse_next;
static constexpr auto traverse = [](const pugi::xml_node& node)
{
	traverse_children(node);
	traverse_next(node);
};


static constexpr auto comment = [](const pugi::xml_node& node)
{ 
	std::cout << "% " << node.text().get() << "\n";
	traverse_children(node);
};
static constexpr auto parens = [](const pugi::xml_node& node)
{
	std::cout << '(';
	traverse_children(node);
	std::cout << ')';
};
static constexpr auto echo = [](const pugi::xml_node& node)
{
	std::cout << node.text().get();
	traverse_children(node);
};
static constexpr auto id = [](const pugi::xml_node& node)
{
	std::cout << node.text().get();
	// TODO this is broken
	const auto subscript = node.attribute("subscript");
	//const auto subscript = node.attribute("xml:space");
	if (subscript)
		std::cout << '_' << subscript.value();
	traverse_children(node);
};
static constexpr auto operator_mult = [](const pugi::xml_node& node)
{
	const auto a = node.next_sibling();
	traverse_children(a);
	std::cout << " * ";
};
static constexpr auto define = [](const pugi::xml_node& node)
{
	const auto lhs = node.first_child();
	traverse_children(lhs);
	std::cout << " = ";
	traverse_children(lhs.next_sibling());
	std::cout << ";\n";
};
static constexpr auto result = [](const pugi::xml_node& node)
{
	std::cout << " ... expected value: "; // inline comment hack
	traverse_children(node);
	std::cout << '\n';
};
static constexpr auto extract_unit = [](const pugi::xml_node& node)
{
	std::cout << '*' << node.attribute("unit").value();
	const auto power = node.attribute("power-numerator");
	if (power)
		std::cout << '^' << power.value();
	traverse_children(node);
};

static const std::unordered_map<std::string_view, node_func> node_funcs = {
	{"document", traverse},
	{"worksheet", traverse},
	{"settings", traverse},
	{"calculation", traverse},
	{"units", traverse},
	{"pointReleaseData", skip},
	{"metadata", skip},
	{"presentation", skip},
	{"calculationBehavior", skip},
	{"regions", traverse},
	{"region", traverse},
	{"math", traverse},
	{"editor", skip},
	{"fileFormat", skip},
	{"miscellaneous", skip},
	{"textStyle", skip},
	{"rendering", skip},
	{"binaryContent", skip},
	{"ml:provenance",traverse},
	{"originRef", skip},
	{"parentRef", skip},
	{"comment", skip},
	{"originComment", skip},
	{"contentHash", skip},
	{"text", traverse},
	{"p", comment},
	{"ml:apply", parens},
	{"ml:parens", parens},
	{"ml:real", echo},
	{"ml:id", id},
	{"ml:mult", operator_mult}, // closure would help
	{"ml:define", define},
	{"result", result},
	{"unitedValue", traverse},
	{"unitMonomial", traverse},
	{"unitReference", extract_unit}, // closure would help
};

void apply_node_funcs(const pugi::xml_node& node)
{
	auto t = node.type();
	if (t != pugi::xml_node_type::node_element && t != pugi::xml_node_type::node_document)
		return;
	const char* name = (t == pugi::xml_node_type::node_document) ? "document" : node.name();
	if (auto func = node_funcs.find(name); func != node_funcs.end() && func->second)
	{
		func->second(node);
	}
	else
	{
		std::cerr << "'" << name << "' function not found (traversing)\n";
		traverse(node);
	}
}
*/
int main(int argc, char* argv[])
{
	if (argc <= 1)
	{
		std::cout << "usage: " << std::string_view(argv[0]) << " <file name>\n";
		return 1;
	}

    std::unordered_map<std::string_view, converter_func> converters;
	converters["matlab"] = matlab::convert;


	pugi::xml_document doc;
	auto result = doc.load_file(argv[1]);
	if (!result)
	{
		std::cout << "error: " << result.description() << '\n';
		return 2;
	}
	
	auto convert = converters.at("matlab");
	convert(doc, std::cout);
}
