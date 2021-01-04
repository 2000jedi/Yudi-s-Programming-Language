/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#include <iostream>
#include <fstream>

#include "scanner.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "err.hpp"

int main(int argc, char** argv) {
    std::filebuf file;
    if (argc > 1) {
        if (!file.open(argv[1], std::ios::in))
            std::runtime_error("open() error: " + std::string(argv[1]));
    } else {
        if (!file.open("input.yc", std::ios::in))
            std::runtime_error("open() error: input.yc");
    }
    auto result_scanner = scanner(&file);
    auto result_ast = parse(result_scanner);
    result_scanner.Free();
    // result_ast->print();

    AST::interpret(std::move(*result_ast));

    return 0;
}
