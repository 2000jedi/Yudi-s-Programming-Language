/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>

#include "lexical.hpp"
#include "scanner.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "err.hpp"

int main(int argc, char** argv) {
    std::ifstream file;
    if (argc > 1) {
        file = std::ifstream(argv[1]);
    } else {
        file = std::ifstream("input.yc");
    }
    std::stringstream ss;
    std::string buf;

    while (std::getline(file, buf)) {
        ss << buf << '\n';
    }

    std::string input = ss.str();

    scanner::initialize("scan_table");
    parser::initialize("parse_table");

    auto result_scanner = scanner::scan(input);

    if (result_scanner.size() == 0)
        exit(ERR_SCANNER);

    auto result_parser  = parser::parse(result_scanner);
    // result_parser.print();

    auto result_ast     = AST::build(&result_parser);
    result_ast.print();

    AST::interpret(std::move(result_ast));

    return 0;
}
