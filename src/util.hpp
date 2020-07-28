/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#pragma once

#include <vector>
#include <map>
#include <string>

#include "ast.hpp"

extern std::string unescape(std::string raw);

// g_strings - set of all strings appear in context.
extern std::vector<std::string> *g_strings;
