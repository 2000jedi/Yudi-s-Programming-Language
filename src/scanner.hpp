/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#pragma once

#include <string>
#include <vector>
#include "lexical.hpp"

namespace scanner {
    extern void initialize(std::string file);
    extern std::vector<Lexical> scan(std::string orig);
}
