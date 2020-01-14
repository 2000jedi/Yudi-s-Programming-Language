#include "lexical.hpp"
#include "scanner.hpp"

int main(int argc, char** argv) {
  scanner::initialize("scan_table");
  
  auto result = scanner::scan("SELECT * FROM table;");
  for (auto i : result) {
    i.print();
  }
  return 0;
}
