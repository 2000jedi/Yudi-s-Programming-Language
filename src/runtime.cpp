/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#include "runtime.hpp"

#include <string>

AST::ValueType *runtime_handler(std::string fn, AST::FuncCall *call, AST::SymTable *st) {
    if (fn == "print") {
        for (auto par : call->pars) {
            auto pst = par->interpret(st);
            if (pst != nullptr) {
                if (pst->type->arrayT != 0) {
                    return & AST::None;
                }
                switch (pst->type->baseType) {
                case AST::TypeDecl::t_int32:
                    std::cout << pst->data.ival << " ";
                    break;
                case AST::TypeDecl::t_fp32:
                    std::cout << pst->data.fval << " ";
                    break;
                case AST::TypeDecl::t_fp64:
                    std::cout << pst->data.dval << " ";
                    break;
                case AST::TypeDecl::t_char:
                    std::cout << pst->data.cval << " ";
                    break;
                case AST::TypeDecl::t_str:
                    std::cout << *(std::string *)(pst->data.ptr) << " ";
                    break;
                default:
                    std::cout << "Unsupported Type: " << pst->type->baseType << " ";
                    break;
                }
            }
        }
        std::cout << std::endl;
    }
    return & AST::None;
}