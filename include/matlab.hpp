#pragma once
#include <ostream>
#include <set>
#include "pugixml.hpp"

namespace matlab
{
    void convert(const pugi::xml_node&, std::ostream&);
    std::set<std::string> get_undefined_ids();
}