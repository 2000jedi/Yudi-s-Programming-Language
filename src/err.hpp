/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#pragma once

#include <string>
#include <exception>

#include "scanner.hpp"

class ErrInfo {
 public:
    int row;
    int col;
    std::string line;
    ErrInfo() {}
    explicit ErrInfo(scanner *Scanner) : row(Scanner->row), col(Scanner->col), line(Scanner->line) {}
};

class InterpreterException : public std::exception {
 public:
    std::string message;
    ErrInfo *ast;

    InterpreterException(std::string msg, ErrInfo *info) :
        message(msg), ast(info) {}
    virtual ~InterpreterException() throw() {}

    virtual const char* what() const throw();
};

extern std::string err_type_mismatch(
    std::string var, std::string ltype, std::string rtype);