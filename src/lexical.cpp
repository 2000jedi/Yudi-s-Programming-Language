/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#include <iostream>
#include <string>

#include "lexical.hpp"

void Lexical::print(void) {
  std::cout << this->name << '(' << this->data << ')' << std::endl;
}

Lexical::Lexical(std::string name, std::string data) {
  this->name = name;
  this->data = data;
  this->line = -1;
  this->position = -1;
}

std::ostream& operator<< (std::ostream &out, Lexical const& data) {
    out << '(' << data.name;
    if (data.data != "") {
      out << ':' << data.data;
    }
    if (data.line != -1) {
      out << ", line " << data.line << ':' << data.position;
    }
    out << ')';

    return out;
}
