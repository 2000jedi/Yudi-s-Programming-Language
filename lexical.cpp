#include <iostream>
#include "lexical.hpp"

void Lexical::print(void) {
  std::cout << this->name << '(' << this->data << ')' << std::endl;
}
