#pragma once
#include <ostream>
#include <functional>
#include "pugixml.hpp"

//using converter_func = void(*)(const pugi::xml_node&, std::ostream&);
using converter_func = std::function<void(const pugi::xml_node&, std::ostream&)>;
