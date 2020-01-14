#pragma once

#include <string>
#include <vector>
#include "tree.hpp"
#include "lexical.hpp"

class catagory {
public:
  std::string name;
  std::vector<std::string> reps;

  catagory(std::string name){
    this->name = name;
    reps.clear();
  }
};

extern void initialize_table(std::string file);
extern Node<Lexical> parser(std::vector<Lexical> input);
