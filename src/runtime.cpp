/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#include "runtime.hpp"

#include <string>
#include "ast.hpp"

void runtime_print(AST::FuncCall *call, AST::SymTable *st) {
    for (auto par : call->pars) {
        auto pst = par->interpret(st);
        if (pst != nullptr) {
            if (pst->type.arrayT != 0)
                throw std::runtime_error("cannot print an array");
            switch (pst->type.baseType) {
            case AST::t_int32:
                std::cout << pst->data.ival << " ";
                break;
            case AST::t_fp32:
                std::cout << pst->data.fval << " ";
                break;
            case AST::t_fp64:
                std::cout << pst->data.dval << " ";
                break;
            case AST::t_char:
                std::cout << pst->data.cval << " ";
                break;
            case AST::t_str:
                std::cout << *pst->data.str << " ";
                break;
            default:
                std::cout << "(Unsupported Type: " << pst->type.baseType << ") ";
                break;
            }
        }
    }
    std::cout << std::endl;
}

AST::ValueType *runtime_typeconv(AST::Types t, AST::FuncCall *call, AST::SymTable *st) {
    if (call->pars.size() != 1) {
        throw std::runtime_error("type cast: wrong number of parameters");
    }
    AST::ValueType *v = call->pars[0]->interpret(st);
    if (v->type.arrayT != 0) {
        throw std::runtime_error("type cast: parameter is an array");
    }

    switch (t) {
        case AST::t_char: {
            switch (v->type.baseType) {
                case AST::t_char:
                    return v;
                case AST::t_int32:
                    v->type.baseType = AST::t_char;
                    v->data.cval = (char) v->data.ival;
                    return v;
                case AST::t_fp32:
                    v->type.baseType = AST::t_char;
                    v->data.cval = (char) v->data.fval;
                    return v;
                case AST::t_fp64:
                    v->type.baseType = AST::t_char;
                    v->data.cval = (char) v->data.dval;
                    return v;
                case AST::t_uint8:
                    v->type.baseType = AST::t_char;
                    v->data.cval = (char) v->data.bval;
                    return v;
                default:
                    throw std::runtime_error("type cast: unsupported type");
            }
        }
        case AST::t_uint8: {
            switch (v->type.baseType) {
                case AST::t_char:
                    v->type.baseType = AST::t_uint8;
                    v->data.bval = (uint8_t) v->data.cval;
                    return v;
                case AST::t_int32:
                    v->type.baseType = AST::t_uint8;
                    v->data.bval = (uint8_t) v->data.ival;
                    return v;
                case AST::t_fp32:
                    v->type.baseType = AST::t_uint8;
                    v->data.bval = (uint8_t) v->data.fval;
                    return v;
                case AST::t_fp64:
                    v->type.baseType = AST::t_uint8;
                    v->data.bval = (uint8_t) v->data.dval;
                    return v;
                case AST::t_uint8:
                    return v;
                default:
                    throw std::runtime_error("type cast: unsupported type");
            }
        }
        case AST::t_int32: {
            switch (v->type.baseType) {
                case AST::t_char:
                    v->type.baseType = AST::t_int32;
                    v->data.ival = (int) v->data.cval;
                    return v;
                case AST::t_int32:
                    return v;
                case AST::t_fp32:
                    v->type.baseType = AST::t_int32;
                    v->data.ival = (int) v->data.fval;
                    return v;
                case AST::t_fp64:
                    v->type.baseType = AST::t_int32;
                    v->data.ival = (int) v->data.dval;
                    return v;
                case AST::t_uint8:
                    v->type.baseType = AST::t_int32;
                    v->data.ival = (int) v->data.bval;
                    return v;
                default:
                    throw std::runtime_error("type cast: unsupported type");
            }
        }
        case AST::t_fp32: {
            switch (v->type.baseType) {
                case AST::t_char:
                    v->type.baseType = AST::t_fp32;
                    v->data.fval = (float) v->data.cval;
                    return v;
                case AST::t_int32:
                    v->type.baseType = AST::t_fp32;
                    v->data.fval = (float) v->data.ival;
                    return v;
                case AST::t_fp32:
                    return v;
                case AST::t_fp64:
                    v->type.baseType = AST::t_fp32;
                    v->data.fval = (float) v->data.dval;
                    return v;
                case AST::t_uint8:
                    v->type.baseType = AST::t_fp32;
                    v->data.fval = (float) v->data.bval;
                    return v;
                default:
                    throw std::runtime_error("type cast: unsupported type");
            }
        }
        case AST::t_fp64: {
            switch (v->type.baseType) {
                case AST::t_char:
                    v->type.baseType = AST::t_fp64;
                    v->data.dval = (double) v->data.cval;
                    return v;
                case AST::t_int32:
                    v->type.baseType = AST::t_fp64;
                    v->data.dval = (double) v->data.ival;
                    return v;
                case AST::t_fp32:
                    v->type.baseType = AST::t_fp64;
                    v->data.dval = (double) v->data.fval;
                    return v;
                case AST::t_fp64:
                    return v;
                case AST::t_uint8:
                    v->type.baseType = AST::t_fp64;
                    v->data.dval = (double) v->data.bval;
                    return v;
                default:
                    throw std::runtime_error("type cast: unsupported type");
            }
        }
        default: {
            return & AST::None;
        }
    }
}

AST::ValueType *runtime_handler(AST::Name fn, AST::FuncCall *call, AST::SymTable *st) {
    if (fn.str() == "print") {
        runtime_print(call, st);
        return & AST::None;
    }
    if (fn.str() == "char") {
        return runtime_typeconv(AST::t_char, call, st);
    }

    // class constructors
    st->addLayer();
    AST::Name constructor_name = AST::Name(new AST::Name(fn), "new");
    auto constructor = st->lookup(constructor_name)->data.fs;

    auto clty = new AST::TypeDecl(AST::t_class);
    clty->other = fn.str();
    auto fnst = new AST::SymTable();
    fnst->addLayer();

    AST::ValueType *context = new AST::ValueType(fnst, clty);

    st->insert(AST::Name("this"), context);

    auto cl = st->lookup(fn)->data.cd;
    for (auto stmt : cl->stmts) {
        switch (stmt->stmtType) {
            case AST::gs_var:
            case AST::gs_func: {
                stmt->declare(fnst, context);
                break;
            }
            default:
                break;
        }
    }

    for (unsigned int i = 0; i < call->pars.size(); ++i) {
        auto vt = call->pars[i]->interpret(st);
        auto prm = constructor->fd->pars[i];
        if (!vt->type.eq(prm->type)) {
            throw std::runtime_error("type mismatch for argument " + prm->name);
        }
        st->insert(AST::Name(prm->name), vt);
    }

    constructor->fd->interpret(st);
    auto ret = st->lookup(AST::Name("this"));
    st->removeLayer();

    return ret;
}

void runtime_bind(AST::SymTable *st) {
    st->insert(AST::Name("print"), new AST::ValueType(& AST::None, &AST::RuntimeType, true));
    st->insert(AST::Name("char"), new AST::ValueType(& AST::None, &AST::RuntimeType, true));
    st->insert(AST::Name("int32"), new AST::ValueType(& AST::None, &AST::RuntimeType, true));
    st->insert(AST::Name("fp32"), new AST::ValueType(& AST::None, &AST::RuntimeType, true));
    st->insert(AST::Name("fp64"), new AST::ValueType(& AST::None, &AST::RuntimeType, true));
}
