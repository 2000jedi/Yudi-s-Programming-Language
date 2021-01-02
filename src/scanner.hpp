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
    t_var, t_const, t_class, t_fn, t_union,
    t_if, t_else, t_while, t_for, t_match,
    t_break, t_continue, t_return,
    type_void, type_bool, type_char, type_int32, type_uint8, type_fp32, type_fp64, type_str,
    lpar, rpar, lbra, rbra, larr, rarr, equ, neq, lor, land, assign,
    neg, bor, band, bxor, gt, ge, lt, le, add, sub, mul, div, rem,
    colon, comma, eol,
    dot, name,
    t_eof
};

const std::string terms[] = {
    "t_str", "t_char", "t_float", "t_int",
    "t_var", "t_const", "t_class", "t_fn", "t_union",
    "t_if", "t_else", "t_while", "t_for", "t_match",
    "t_break", "t_continue", "t_return",
    "type_void", "type_bool", "type_char", "type_int32", "type_uint8", "type_fp32", "type_fp64", "type_str",
    "lpar", "rpar", "lbra", "rbra", "larr", "rarr", "equ", "neq", "lor", "land", "le", "ge", "assign",
    "neg", "bor", "band", "bxor", "gt", "ge", "lt", "le", "add", "sub", "mul", "div", "rem",
    "colon", "comma", "eol",
    "dot", "name",
    "t_eof"
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
};
