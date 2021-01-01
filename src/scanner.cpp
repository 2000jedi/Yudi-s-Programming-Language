/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#include "scanner.hpp"

#include <string>
#include <iostream>

#include "util.hpp"

void scanner::next(void) {
    if (this->input->eof()) {
        this->c = '\0';
        return;
    }
    (*this->input) >> this->c;
    if ((c == '\n') || (c == '\r')) {
        this->line = "";
        row++;
        col = 0;
    } else {
        this->line += c;
        col++;
    }
}

token scanner::scan(void) {
    this->data = "";
    while (isspace(c) || (c == '\n') || (c == '\r')) next();
    switch (c) {
        case '\0': {
            return t_eof;
        }
        case '"': {
            // string
            do {
                data += c;
                next();
                if (c == '\\') {
                    data += c;
                    next();
                    data += c;
                    next();
                }
            } while (c != '"');
            data += c;
            next();
            return t_str;
        }
        case '\'': {
            // char
            next();
            if (c == '\\') {
                next();
                if (c == 'n') c = '\n';
                if (c == 'r') c = '\r';
            }
            data += c;
            next();
            if (c != '\'') {
                throw std::runtime_error("scanner: unknown token " + c);
            }
            next();
            return t_char;
        }
        case '#': {
            // comment
            while (c != '\n') {
                next();
            }
            return scan();
        }
        case '(': {
            next();
            return lpar;
        }
        case ')': {
            next();
            return rpar;
        }
        case '{': {
            next();
            return lbra;
        }
        case '}': {
            next();
            return rbra;
        }
        case '[': {
            next();
            return larr;
        }
        case ']': {
            next();
            return rarr;
        }
        case '=': {
            next();
            if (c == '=') {
                next();
                return equ;
            } else {
                return assign;
            }
        }
        case '!': {
            next();
            if (c == '=') {
                next();
                return neq;
            } else {
                return neg;
            }
        }
        case '>': {
            next();
            if (c == '=') {
                next();
                return ge;
            } else {
                return gt;
            }
        }
        case '<': {
            next();
            if (c == '=') {
                next();
                return le;
            } else {
                return lt;
            }
        }
        case '|': {
            next();
            if (c == '|') {
                next();
                return lor;
            } else {
                return bor;
            }
        }
        case '&': {
            next();
            if (c == '&') {
                next();
                return land;
            } else {
                return band;
            }
        }
        case '^': {
            next();
            return bxor;
        }
        case '+': {
            next();
            return add;
        }
        case '-': {
            next();
            return sub;
        }
        case '*': {
            next();
            return mul;
        }
        case '/': {
            next();
            return div;
        }
        case '%': {
            next();
            return rem;
        }
        case ',': {
            next();
            return comma;
        }
        case ':': {
            next();
            return colon;
        }
        case ';': {
            next();
            return eol;
        }
        default: {
            if (isdigit(c)) {
                data += c;
                bool is_float = false;
                next();
                while (isdigit(c) || ((c == '.') && (!is_float))) {
                    data += c;
                    next();
                    if (c == '.') is_float = true;
                }
                if (is_float)
                    return t_float;
                else
                    return t_int;
            }
            if (isalpha(c)) {
                data += c;
                next();
                while (isalpha(c)) {
                    data += c;
                    next();
                }
                if (data == "var") return t_var;
                if (data == "const") return t_const;
                if (data == "function") return t_fn;
                if (data == "class") return t_class;
                if (data == "union") return t_union;
                if (data == "if") return t_if;
                if (data == "else") return t_else;
                if (data == "match") return t_match;
                if (data == "while") return t_while;
                if (data == "for") return t_for;
                if (data == "break") return t_break;
                if (data == "continue") return t_continue;
                if (data == "return") return t_return;
                if (data == "void") return type_void;
                if (data == "bool") return type_bool;
                if (data == "int32") return type_int32;
                if (data == "uint8") return type_uint8;
                if (data == "char") return type_char;
                if (data == "fp32") return type_fp32;
                if (data == "fp64") return type_fp64;
                if (data == "str") return type_str;
                throw std::runtime_error("unknown name: " + data);
            }
            throw std::runtime_error("unknown character: " + c);
        }
    }
}
