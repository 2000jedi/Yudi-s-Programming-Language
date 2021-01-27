/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#include "runtime.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <stack>

#include "ast.hpp"
#include "err.hpp"
#include "scanner.hpp"
#include "parser.hpp"

static std::stack<std::unique_ptr<AST::Program>> imports;

void runtime_print(AST::FuncCall *call, AST::SymTable *st) {
    for (auto&& par : call->pars) {
        auto pst = par->interpret(st);
        if (pst != nullptr) {
            if (pst->type.arrayT != 0)
                throw InterpreterException("cannot print an array", call);
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
                throw InterpreterException("Unsupported Type: " + pst->type.str(), call);
                break;
            }
            if (pst->ms.size() == 0) {
                delete pst;
            }
        }
    }
    std::cout << std::endl;
}

void runtime_debug(AST::FuncCall *call, AST::SymTable *st) {
    for (auto&& par : call->pars) {
        auto pst = par->interpret(st);
        if (pst == nullptr) {
            std::cout << "debug(): value vanished" << std::endl;
            return;
        }
        std::cout << "Debug info for: ";
        if (par->isVal) {
            std::cout << call->line << std::endl;
        }
        std::cout << "\tConst Flag: " << pst->isConst << std::endl;
        std::cout << "\tReference Counter: " << pst->ms.size() << std::endl;
        std::cout << "\tType: " << pst->type.str() << std::endl;
        std::cout << "\tValue: ";
        if (pst != nullptr) {
            if (pst->type.arrayT != 0)
                return;
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
                std::cout << "Unsupported Type: " << pst->type.str();
                break;
            }
        }
        std::cout << std::endl;
    }
}

AST::ValueType *runtime_typeconv(AST::Types t, AST::FuncCall *call, AST::SymTable *st) {
    if (call->pars.size() != 1) {
        throw InterpreterException("type cast: wrong number of parameters", call);
    }
    AST::ValueType *v = call->pars[0]->interpret(st);
    if (v->type.arrayT != 0) {
        throw InterpreterException("type cast: parameter is an array", call);
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
                    throw InterpreterException("type cast: unsupported type" + v->type.str(), call);
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
                    throw InterpreterException("type cast: unsupported type" + v->type.str(), call);
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
                    throw InterpreterException("type cast: unsupported type" + v->type.str(), call);
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
                    throw InterpreterException("type cast: unsupported type" + v->type.str(), call);
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
                    throw InterpreterException("type cast: unsupported type" + v->type.str(), call);
            }
        }
        default: {
            return & AST::None;
        }
    }
}

AST::ValueType *runtime_enum_handler(
    AST::ValueType *vt, AST::FuncCall *call, AST::SymTable *st) {
    auto vars = & vt->data.ed->vars;
    if (vars->size() != call->pars.size()) {
        throw InterpreterException("enum initializer parameters do not match", call);
    }
    AST::SymTable *enst = new AST::SymTable();
    enst->addLayer();
    if (call->gen_val.str() != "") {
        // associate generics
        enst->insert(AST::Name(vt->data.ed->gen.name),
            new AST::ValueType(call->gen_val));
    }
    for (unsigned int i = 0; i < vars->size(); ++i) {
        auto init = call->pars[i]->interpret(st);
        auto ty = (*vars)[i]->type;
        if (ty.baseType == AST::t_class) {
            if (ty.other.str() == vt->data.ed->gen.name.str()) {
                ty.other = call->gen_val;
            }
        }
        if (ty != init->type) {
            throw InterpreterException(err_type_mismatch(
                (*vars)[i]->name.str(), (*vars)[i]->type.str(), init->type.str()
            ), call);
        }
        (*vars)[i]->interpret(enst);
        auto ms = enst->lookup((*vars)[i]->name, call);
        ms->set(init);
        init->ms.push_back(ms);
    }
    auto clty = AST::TypeDecl(AST::t_class);
    clty.other = vt->data.ed->name.owner();
    clty.enum_base = vt->data.ed->name.BaseName;
    if (call->gen_val.str() != "") {
        clty.gen.valid = true;
        clty.gen.name = call->gen_val;
    }
    return new AST::ValueType(enst, &clty);
}

AST::ValueType *runtime_string_size(AST::FuncCall *call, AST::SymTable *st) {
    if (call->pars.size() != 1) {
        throw InterpreterException(err_par_size_mismatch("size()", 1, call->pars.size()), call);
    }
    auto vt = call->pars[0]->interpret(st);
    if (vt->type != AST::StrType) {
        throw InterpreterException(err_type_mismatch(
            "size()", AST::StrType.str(), vt->type.str()), call);
    }
    int s = vt->data.str->size();
    return new AST::ValueType(s, true);
}

AST::ValueType *runtime_handler(
    AST::Name fn, AST::FuncCall *call, AST::SymTable *st) {
    if (fn.str() == "print") {
        runtime_print(call, st);
        return & AST::None;
    }
    if (fn.str() == "debug") {
        runtime_debug(call, st);
        return & AST::None;
    }
    if (fn.str() == "import") {
        if (call->pars.size() != 1) {
            throw InterpreterException("import(): wrong number of params",call);
        }
        for (auto&& par : call->pars) {
            auto vt = par->interpret(st);
            if (vt->type != AST::StrType) {
                throw InterpreterException(err_type_mismatch(
                    "", AST::StrType.str(), vt->type.str()
                ), call);
            }
            auto file_name = *vt->data.str;
            std::filebuf file;
            if (!file.open(file_name, std::ios::in))
                std::runtime_error("import(): " + file_name + " not found");

            auto fnst = new AST::SymTable();
            fnst->addLayer();
            auto sc = scanner(&file);
            auto ast = parse(sc);
            sc.Free();
            ast->declare(fnst);
            imports.push(std::move(ast));
            AST::TypeDecl clty = AST::TypeDecl(AST::Name("import"), 0);
            return new AST::ValueType(fnst, &clty);
        }
        return & AST::None;
    }
    if (fn.str() == "open") {
        // TODO: open()
    }
    if (fn.str() == "to_char")
        return runtime_typeconv(AST::t_char, call, st);
    if (fn.str() == "to_uint8")
        return runtime_typeconv(AST::t_uint8, call, st);
    if (fn.str() == "to_int32")
        return runtime_typeconv(AST::t_int32, call, st);
    if (fn.str() == "to_fp32")
        return runtime_typeconv(AST::t_fp32, call, st);
    if (fn.str() == "to_fp64")
        return runtime_typeconv(AST::t_fp64, call, st);
    if (fn.str() == "__string_size")
        return runtime_string_size(call, st);

    // class constructors
    st->addLayer();
    AST::Name constructor_name = AST::Name(&fn, "new");
    auto constructor = st->lookup(constructor_name, call)->get()->data.fs;

    auto clty = AST::TypeDecl(AST::t_class);
    clty.other = fn;
    auto fnst = new AST::SymTable();
    fnst->addLayer();
    if (call->gen_val.str() != "") {
        // associate generics
        auto cl = st->lookup(fn, call)->get()->data.cd;
        fnst->insert(AST::Name(cl->gen.name),
            new AST::ValueType(call->gen_val));
    }

    AST::ValueType *context = new AST::ValueType(fnst, &clty);
    st->insert(AST::Name("this"), context);

    auto cl = st->lookup(fn, call)->get()->data.cd;
    for (auto&& stmt : cl->stmts) {
        switch (stmt->stmtType) {
            case AST::gs_var:
            case AST::gs_func: {
                stmt->declare(fnst, fnst);
                break;
            }
            default:
                throw InterpreterException("unsupported behavior", nullptr);
        }
    }

    if (call->pars.size() != constructor->fd->pars.size()) {
        throw InterpreterException("new(): param number mismatch", nullptr);
    }
    for (unsigned int i = 0; i < call->pars.size(); ++i) {
        auto vt = call->pars[i]->interpret(st);
        auto prm = constructor->fd->pars[i];
        if (vt->type != prm.type) {
            throw InterpreterException(err_type_mismatch(
                prm.name, prm.type.str(), vt->type.str()
            ), call);
        }
        st->insert(AST::Name(prm.name), vt);
    }

    constructor->fd->interpret(st);

    for (auto&& msi : context->ms) {
        msi->set(nullptr);
    }
    context->ms.clear();
    st->removeLayer();
    return context;
}

void runtime_bind(AST::SymTable *st) {
    st->insert(AST::Name("print"), new AST::ValueType(&AST::RuntimeType, true));
    st->insert(AST::Name("debug"), new AST::ValueType(&AST::RuntimeType, true));
    st->insert(AST::Name("to_char"), new AST::ValueType(&AST::RuntimeType, true));
    st->insert(AST::Name("to_uint8"), new AST::ValueType(&AST::RuntimeType, true));
    st->insert(AST::Name("to_int32"), new AST::ValueType(&AST::RuntimeType, true));
    st->insert(AST::Name("to_fp32"), new AST::ValueType(&AST::RuntimeType, true));
    st->insert(AST::Name("to_fp64"), new AST::ValueType(&AST::RuntimeType, true));
    st->insert(AST::Name("import"),new AST::ValueType(&AST::RuntimeType, true));
    st->insert(AST::Name("open"), new AST::ValueType(&AST::RuntimeType, true));
    st->insert(AST::Name("__string_size"), new AST::ValueType(&AST::RuntimeType, true));
}
