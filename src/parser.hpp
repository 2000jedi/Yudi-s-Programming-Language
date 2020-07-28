/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#pragma once

#include <string>
#include <vector>

#include "tree.hpp"
#include "lexical.hpp"

namespace parser {
  extern void initialize(std::string file);
  extern Node<Lexical> parse(std::vector<Lexical> input);
}
