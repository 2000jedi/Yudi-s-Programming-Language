/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 * -------------------
 * ll(1) parser
 */

#include <string>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "parser.hpp"
#include "util.hpp"
#include "err.hpp"

static token input_token;

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

inline bool error(scanner *Scanner, std::string prompt) {
    std::cerr << "Error  (Parser): at line " << Scanner->row << ":" << Scanner->col << std::endl
            << Scanner->line << std::endl
            << "\t" << prompt << " cannot accept "
            << terms[input_token] << "(" << Scanner->data << ")" << std::endl;
    throw std::runtime_error("Parser Error");
}

std::string match(scanner *Scanner, token expected) {
    if (input_token == expected) {
        std::string result = Scanner->data;

        if (expected != t_eof)
            input_token = Scanner->scan();

        return result;
    } else {
        error(Scanner, "Terminal \"" + terms[expected] + '"');
        return "E";
    }
}

std::unique_ptr<AST::Program> statements(scanner *Scanner);
std::unique_ptr<AST::GlobalStatement> statement(scanner *Scanner);
std::unique_ptr<AST::FuncDecl> func_decl(scanner *Scanner);
AST::GenericDecl generic(scanner *Scanner);
int array(scanner *Scanner);
std::vector<AST::Param> params_decl(scanner *Scanner);
AST::TypeDecl ret_decl(scanner *Scanner);
AST::TypeDecl type_name(scanner *Scanner);
std::vector<std::unique_ptr<AST::Expr>> expr_list(scanner *Scanner);

std::unique_ptr<AST::Program> statements(scanner *Scanner) {
    std::unique_ptr<AST::Program> program = std::make_unique<AST::Program>();
    while (input_token != t_eof) {
        auto gs = statement(Scanner);
        program->insert(std::move(gs));
    }
    return program;
}

std::unique_ptr<AST::GlobalStatement> statement(scanner *Scanner) {
    switch (input_token) {
        case t_fn: {
            return funcdecl(Scanner);
        }
    }
}

std::unique_ptr<AST::FuncDecl> func_decl(scanner *Scanner) {
    match(Scanner, t_fn);
    match(Scanner, name);
    auto fn_name = Scanner->data;
    auto gen = generic(Scanner);
    match(Scanner, lpar);
    auto prms = params_decl(Scanner);
    match(Scanner, rpar);
    auto ret_type = ret_decl(Scanner);
    match(Scanner, lbra);
    auto exprs = expr_list(Scanner, nullptr);
    match(Scanner, rbra);
}

AST::GenericDecl generic(scanner *Scanner) {
    if (input_token == lt) {
        match(Scanner, lt);
        match(Scanner, name);
        auto gen_name = Scanner->data;
        match(Scanner, gt);
        return AST::GenericDecl(gen_name);
    } else {
        // epsilon
        return AST::GenericDecl();
    }
}

int array(scanner *Scanner) {
    int v = 0;
    if (input_token == larr) {
        match(Scanner, larr);
        match(Scanner, t_int);
        v = std::stoi(Scanner->data);
        match(Scanner, rarr);
    }  // else: epsilon
    return v;
}

std::vector<AST::Param> params_decl(scanner *Scanner) {
    std::vector<AST::Param> prms;
    if (input_token == name) {
        match(Scanner, name);
        auto par_name = Scanner->data;
        match(Scanner, colon);
        auto tn = type_name(Scanner);
        prms.push_back(AST::Param(par_name, tn));
        while (input_token == comma) {
            match(Scanner, comma);
            match(Scanner, name);
            auto par_name = Scanner->data;
            match(Scanner, colon);
            auto tn = type_name(Scanner);
            prms.push_back(AST::Param(par_name, tn));
        }
    }
    return prms;
}

AST::TypeDecl ret_decl(scanner *Scanner) {
    if (input_token == colon) {
        match(Scanner, colon);
        match(Scanner, name);
        return AST::TypeDecl(Scanner->data, AST::GenericDecl(), 0);
    } else {
        return AST::VoidType;
    }
}

AST::TypeDecl type_name(scanner *Scanner) {
    AST::Types base;
    std::string other;
    switch (input_token) {
        case type_void:
            base = AST::t_void;
            match(Scanner, input_token);
            break;
        case type_bool:
            base = AST::t_bool;
            match(Scanner, input_token);
            break;
        case type_char:
            base = AST::t_char;
            match(Scanner, input_token);
            break;
        case type_fp32:
            base = AST::t_fp32;
            match(Scanner, input_token);
            break;
        case type_fp64:
            base = AST::t_fp64;
            match(Scanner, input_token);
            break;
        case type_int32:
            base = AST::t_int32;
            match(Scanner, input_token);
            break;
        case type_uint8:
            base = AST::t_uint8;
            match(Scanner, input_token);
            break;
        case type_str:
            base = AST::t_str;
            match(Scanner, input_token);
            break;
        case name:
            base = AST::t_class;
            match(Scanner, input_token);
            other = Scanner->data;
            break;
        default:
            error(Scanner, "type() rejects");
    }
    auto gen = generic(Scanner);
    auto arr = array(Scanner);
    return AST::TypeDecl(base, other, gen, arr);
}

std::unique_ptr<AST::Expr> expr(scanner *Scanner) {
    switch (input_token) {
            case t_var: {
            }
            case t_const:
            case t_if:
            case t_while:
            case t_match:
            case t_return:
            case t_continue:
            case t_break:
            case eol:
            case t_int:
            case t_float:
            case t_str:
            case t_char:
            case name:
            case lpar:
                result.push_back(std::move(expr(Scanner)));
            default:
                LogError("Impossible to reach");
                return nullptr;
        }
}

std::vector<std::unique_ptr<AST::Expr>> expr_list(scanner *Scanner) {
    std::vector<std::unique_ptr<AST::Expr>> result;
    while (1) {
        switch (input_token) {
            case t_var:
            case t_const:
            case t_if:
            case t_while:
            case t_match:
            case t_return:
            case t_continue:
            case t_break:
            case eol:
            case t_int:
            case t_float:
            case t_str:
            case t_char:
            case name:
            case lpar:
                result.push_back(std::move(expr(Scanner)));
            default:
                // epsilon
                return result;
        }
    }
}

std::unique_ptr<AST::Program> parse(scanner Scanner) {
    input_token = Scanner.scan();
    return statements(&Scanner);
}
