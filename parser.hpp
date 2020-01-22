#pragma once

#include <string>
#include <vector>

#include "tree.hpp"
#include "lexical.hpp"

namespace parser {
  extern void initialize(std::string file);
  extern Node<Lexical> parse(std::vector<Lexical> input);
}
