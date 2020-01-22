#include "lexical.hpp"
#include "scanner.hpp"
#include "parser.hpp"

int main(int argc, char** argv) {
  scanner::initialize("scan_table");
  parser::initialize("parse_table");
  
  auto result = scanner::scan("a=for();");
  for (auto i : result) {
    i.print();
  }
  
  auto result_2 = parser::parse(result);
  result_2.print();

  return 0;
}
