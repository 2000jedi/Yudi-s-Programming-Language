#pragma once

#include <vector>
#include <map>

#include "ast.hpp"

extern std::string unescape(std::string raw);

/*
    strings - set of all strings appear in context.
*/
extern std::vector<std::string> *strings;
