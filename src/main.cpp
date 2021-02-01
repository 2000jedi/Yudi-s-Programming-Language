/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#include <iostream>
#include <fstream>
#include <filesystem>

#include "scanner.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "err.hpp"

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    std::filebuf file;
    fs::path path;
    if (argc > 1) {
        path = fs::path(argv[1]);
        if (!file.open(argv[1], std::ios::in))
            std::runtime_error("open() error: " + std::string(argv[1]));
    } else {
        path = fs::path("./input.yc");
        if (!file.open("input.yc", std::ios::in))
            std::runtime_error("open() error: input.yc");
    }
    auto result_scanner = scanner(&file, path.filename().string());
    auto result_ast = parse(result_scanner);
    result_scanner.Free();
    // result_ast->print();

    AST::interpret(std::move(*result_ast));

    return 0;
}
