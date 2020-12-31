/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#pragma once

#include <sstream>
#include <string>
#include <exception>

#include "ast.hpp"

extern std::string unescape(std::string raw);

class InterpreterException : public std::exception {
 protected:
    std::string message;
    AST::BaseAST ast;
 public:
    InterpreterException(std::string msg, AST::BaseAST info) :
        message(msg), ast(info) {}
    virtual ~InterpreterException() throw() {}

    virtual const char* what() const throw() {
        std::stringstream ss;
        ss << "At line (" << ast.row << ',' << ast.col << "): " << ast.line << std::endl;
        ss << '\t' << message << std::endl;
        std::cout << ss.str();
        return "";
    }

    /** Returns error number.
     *  @return #error_number
     */
    virtual AST::BaseAST getAST() const throw() {
        return ast;
    }
};
