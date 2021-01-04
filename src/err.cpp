/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#include "err.hpp"

#include <sstream>
#include <iostream>

const char *InterpreterException::what() const throw() {
    std::stringstream ss;
    if (ast)
        ss << "line " << ast->row << ':' << ast->col << ": " << ast->line << std::endl;
    ss << "AST Error: " << message << std::endl;
    std::cout << ss.str();
    return "";
}
