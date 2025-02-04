#include <iostream>
#include <unordered_map>
#include <string_view>
#include "converter_func.hpp"
#include "matlab.hpp"

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
