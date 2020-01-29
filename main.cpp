#include "lexical.hpp"
#include "scanner.hpp"
#include "parser.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>

int main(int argc, char** argv) {
  std::ifstream file("input.yc");
  std::stringstream ss;
  std::string buf;

  while (std::getline(file, buf)) {
    ss << buf << '\n';
  }
  
  std::string input = ss.str();

  scanner::initialize("scan_table");
  parser::initialize("parse_table");
  
  auto result = scanner::scan(input);
  for (auto i : result) {
    i.print();
  }
  
  auto result_2 = parser::parse(result);
  result_2.print();

  return 0;
}
