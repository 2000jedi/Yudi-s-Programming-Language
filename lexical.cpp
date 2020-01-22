#include <iostream>
#include <string>

#include "lexical.hpp"

void Lexical::print(void) {
  std::cout << this->name << '(' << this->data << ')' << std::endl;
}

Lexical::Lexical(std::string name, std::string data) {
  this->name = name;
  this->data = data;
}

std::ostream& operator<< (std::ostream &out, Lexical const& data) {
    out << '(' << data.name << ':';
    out << data.data << ')';
    return out;
}
