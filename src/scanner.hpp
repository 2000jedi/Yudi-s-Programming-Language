#pragma once

#include <string>
#include <vector>
#include "lexical.hpp"

namespace scanner {
    extern void initialize(std::string file);
    extern std::vector<Lexical> scan(std::string orig);
}
