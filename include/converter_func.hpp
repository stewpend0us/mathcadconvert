#pragma once
#include <ostream>
#include "pugixml.hpp"

using converter_func = void(*)(const pugi::xml_node&, std::ostream&);
