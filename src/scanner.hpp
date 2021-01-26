/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#pragma once

#include <iostream>
#include <fstream>
#include <string>

enum token {
    t_str, t_char, t_float, t_int,
    t_var, t_const, t_class, t_fn, t_union, t_enum,
    t_if, t_else, t_while, t_for, t_match,
    t_break, t_continue, t_return,
    type_void, type_bool, type_char, type_int32, type_uint8, type_fp32, type_fp64, type_str,
    lpar, rpar, lbra, rbra, larr, rarr, equ, neq, lor, land, move, copy, deepcopy,
    bor, band, bxor, gt, ge, lt, le, add, sub, mul, t_div, rem,
    colon, comma, eol,
    dot, t_name, gen,
    t_eof
};

const std::string terms[] = {
    "str", "char", "fp32", "int32",
    "var", "const", "class", "function", "union", "enum",
    "if", "else", "while", "for", "match",
    "break", "continue", "return",
    "type_void", "type_bool", "type_char", "type_int32", "type_uint8", "type_fp32", "type_fp64", "type_str",
    "(", ")", "{", "}", "[", "]", "==", "!=", "||", "&&", "=", ":=", "::="
    "|", "&", "^", ">", ">=", "<", "<=", "+", "-", "*", "/", "%",
    ":", ",", "EOL",
    ".", "name", "`",
    "EOF"
};

class scanner {
 private:
    std::istream *input;
    void next(void);
 public:
    std::string data;
    std::string line;
    int row, col;

    char c = ' ';  // current (look ahead) char

    explicit scanner(std::filebuf *in) {
        input = new std::istream(in);
        data = "";
        line = "";
        row = 0;
        col = 0;
    }
    token scan(void);

    void Free(void) {
        delete input;
    }
};
