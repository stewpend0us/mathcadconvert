#pragma once
#include <ostream>
#include "pugixml.hpp"

namespace matlab
{
    void convert(const pugi::xml_node&, std::ostream&);
}