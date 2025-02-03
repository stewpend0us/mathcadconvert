#include "matlab.hpp"

void matlab::convert(const pugi::xml_node& node, std::ostream& os)
{
	os << node.text().get();
    auto subscript = node.attribute("subscript");
    if (subscript)
        os << "_" << subscript.value();
}