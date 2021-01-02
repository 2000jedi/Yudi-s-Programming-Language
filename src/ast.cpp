/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#include "ast.hpp"

#include <memory>
#include <utility>

#include "util.hpp"
#include "runtime.hpp"

using namespace AST;

#define INTERPRET(x) ValueType *x::interpret(SymTable *st)

#ifdef DEBUG

/**
 * Message Printing Interface.
 * Defines `print()` method for every AST class.
 */

void Program::print(void) {
    std::cout << "Program AST:" << std::endl;
    for (auto p = this->stmts.begin(); p != this->stmts.end(); p++) {
        (*p)->print(1);
    }
}

void ASTs::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "ASTs()" << std::endl;
    for (auto p : this->stmts) p->print(indent + 1);
}

void TypeDecl::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "TypeDecl(" << this->baseType << ','
        << this->arrayT << ')' << std::endl;
    this->gen.print(indent + 1);
}

void EvalExpr::print(int indent) {
    if (this->isVal) {
        this->val->print(indent);
    } else {
        for (int i = 0; i < indent; ++i)
            std::cout << "  ";
        std::cout << "BinaryOp(" << this->op << ')' << std::endl;
        this->l->print(indent + 1);
        this->r->print(indent + 1);
    }
}

void FuncCall::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "FuncCall()" << std::endl;
    for (auto&& p : this->pars) p->print(indent + 1);
}

void ExprVal::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    if (this->isConst) {
        std::cout << "ConstVal(" << this->constVal << ')' << std::endl;
    } else {
        std::cout << "ExprVal(" << this->refName.str() << ')' << std::endl;
        if (this->call)
            this->call->print(indent + 1);
        if (this->array)
            this->array->print(indent + 1);
    }
}

void GenericDecl::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    if (this->valid)
        std::cout << "GenericDecl(" << this->name << ')' << std::endl;
    else
        std::cout << "GenericDecl()" << std::endl;
}

void Param::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "Param(" << this->name << ')' << std::endl;

    this->type.print(indent + 1);
}

void RetExpr::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "RetExpr()" << std::endl;
    this->stmt->print(indent + 1);
}

void ContExpr::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "Continue()" << std::endl;
}

void BreakExpr::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "Break()" << std::endl;
}

void VarDecl::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "VarDecl(" << this->name.str() << ')' << std::endl;
    this->type.print(indent + 1);
    if (this->init)
        this->init->print(indent + 1);
}

void FuncDecl::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "FuncDecl(" << this->name.str() << ')' << std::endl;
    if (this->genType.valid)
        this->genType.print(indent + 1);

    for (int i = 0; i < indent + 1; ++i)
            std::cout << "  ";
        std::cout << "Params(" << std::endl;
    for (auto p : this->pars) {
        p.print(indent + 2);
    }
    for (int i = 0; i < indent + 1; ++i)
            std::cout << "  ";
    std::cout << ")" << std::endl;

    this->ret.print(indent + 1);

    for (int i = 0; i < indent + 1; ++i)
            std::cout << "  ";
        std::cout << "Expressions(" << std::endl;
    for (auto&& p : this->exprs) {
        p->print(indent + 2);
    }
    for (int i = 0; i < indent + 1; ++i)
            std::cout << "  ";
    std::cout << ")" << std::endl;
}

void ClassDecl::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "ClassDecl(" << this->name.str() << ')' << std::endl;
    if (this->genType.valid)
        this->genType.print(indent + 1);

    for (int i = 0; i < indent + 1; ++i)
            std::cout << "  ";
        std::cout << "Statements(" << std::endl;
    for (auto&& p : this->stmts) {
        p->print(indent + 2);
    }
    for (int i = 0; i < indent + 1; ++i)
            std::cout << "  ";
    std::cout << ")" << std::endl;
}

void UnionDecl::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "UnionDecl(" << this->name.str() << ')' << std::endl;
    for (int i = 0; i < indent + 1; ++i)
            std::cout << "  ";
        std::cout << "Options(" << std::endl;
    for (auto&& p : this->classes) {
        p->print(indent + 2);
    }
    for (int i = 0; i < indent + 1; ++i)
            std::cout << "  ";
    std::cout << ")" << std::endl;
}

void IfExpr::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "IfExpr()" << std::endl;

    this->cond->print(indent + 1);
    for (int i = 0; i < indent + 1; ++i)
        std::cout << "  ";
    std::cout << "True()" << std::endl;
    for (auto&& p : this->iftrue) {
        p->print(indent + 2);
    }
    for (int i = 0; i < indent + 1; ++i)
        std::cout << "  ";
    std::cout << "False()" << std::endl;
    for (auto&& p : this->iffalse) {
        p->print(indent + 2);
    }
}

void WhileExpr::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "WhileExpr()" << std::endl;

    this->cond->print(indent + 1);
    for (int i = 0; i < indent + 1; ++i)
        std::cout << "  ";
    std::cout << "Expressions(" << std::endl;
    for (auto&& p : this->exprs) {
        p->print(indent + 2);
    }
    for (int i = 0; i < indent + 1; ++i)
        std::cout << "  ";
    std::cout << ")" << std::endl;
}

void ForExpr::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "ForExpr()" << std::endl;

    this->init->print(indent + 1);
    this->cond->print(indent + 1);
    this->step->print(indent + 1);
    for (int i = 0; i < indent + 1; ++i)
        std::cout << "  ";
    std::cout << "Expressions(" << std::endl;
    for (auto&& p : this->exprs) {
        p->print(indent + 2);
    }
    for (int i = 0; i < indent + 1; ++i)
        std::cout << "  ";
    std::cout << ")" << std::endl;
}

void MatchLine::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "MatchExpr(" << this->name << ": " << this->cl_name << ")" << std::endl;

    for (int i = 0; i < indent + 1; ++i)
            std::cout << "  ";
        std::cout << "Expessions(" << std::endl;
    for (auto&& p : this->exprs) {
        p->print(indent + 2);
    }
    for (int i = 0; i < indent + 1; ++i)
            std::cout << "  ";
    std::cout << ")" << std::endl;
}

void MatchExpr::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "MatchExpr()" << std::endl;

    this->var->print(indent + 1);
    for (int i = 0; i < indent + 1; ++i)
            std::cout << "  ";
        std::cout << "MatchLines(" << std::endl;
    for (auto&& p : this->lines) {
        p.print(indent + 2);
    }
    for (int i = 0; i < indent + 1; ++i)
            std::cout << "  ";
    std::cout << ")" << std::endl;
}

#endif

// Symble Table - record Variable and Type Information

void SymTable::reset(void) {
    this->d.clear();
}

void SymTable::addLayer(void) {
    d.push_back(std::map<Name, ValueType *>());
}

void SymTable::removeLayer(void) {
    /*for (auto vt : d.back()) {
        vt.second->Free();
        delete vt.second;
    }*/
    // TODO: need to find a method to auto-deallocate
    d.pop_back();
}

void SymTable::insert(Name name, ValueType *vt) {
    d.back()[name] = vt;
}

ValueType* SymTable::lookup(Name name, BaseAST* ast) {
    if (name.ClassName.size() > 0) {
        auto owner = this->lookup(name.owner(), ast);
        if (!((owner->type.baseType) == t_rtfn && (name.BaseName == "new"))) {
            if (owner->type.baseType != t_class) {
                throw InterpreterException(name.owner().str() + " is not a compound type", *ast);
            }
            auto clst = owner->data.st;
            return clst->lookup(Name(name.BaseName), ast);
        }
    }

    for (int i = d.size() - 1; i >= 0; i--) {
        if (d[i].find(name) != d[i].end()) {
            return d[i][name];
        }
    }
    throw InterpreterException("variable " + name.str() + " is not declared", *ast);
}

ValueType* SymTable::lookup(ExprVal *name) {
    if (name->call != nullptr)
        throw InterpreterException("cannot lookup a function call", *name);

    if (name->array != nullptr) {
        auto arr = this->lookup(name->refName, name);
        auto arr_index_vt = name->array->interpret(this);
        if (!arr_index_vt->type.eq(& IntType)) {
            throw InterpreterException("array index must be an int", *name);
        }
        int  arr_index = arr_index_vt->data.ival;

        if (arr->type.arrayT <= arr_index) {
            throw InterpreterException("array index out of bound", *name);
        }

        auto vts = arr->data.vt;
        return &(vts[arr_index]);
    } else {
        return this->lookup(name->refName, name);
    }
}

void ValueType::Free(void) {
        return;  // TODO: determine free method for each type
        if (this->type.arrayT != 0) {
            // auto arr = (ValueType*) this->data.ptr;
            for (int i = 0; i < this->type.arrayT; ++i);
                // TODO: remove arr[i]
        }

        switch (this->type.baseType) {
            case t_str:
                delete data.str;
            case t_fn:
                delete data.fs;
            case t_class:
                delete data.st;
            default:
                return;
        }
    }

/**
 * Interpreter Interface - interpret() methods
 */
ValueType *ConstEval(ExprVal *e) {
    switch (e->type.baseType) {
        case t_void: {
            throw InterpreterException("no constant type \"void\"", *e);
        }
        case t_int32: {
            return new ValueType(std::stoi(e->constVal));
        }
        case AST::t_char: {
            return new ValueType(e->constVal[1]);
        }
        case t_fp32: {
            return new ValueType(std::stof(e->constVal));
        }
        case t_fp64: {
            return new ValueType(std::stod(e->constVal));
        }
        case AST::t_str: {
            std::string *s = new std::string(e->constVal);
            return new ValueType(s, true);
        }
        default: {
            throw InterpreterException("TypeDecl index not handled", *e);
        }
    }
}


ValueType *TypeDecl::newVal(void) {
    if (this->arrayT == 0) {
        switch (this->baseType) {
            case t_bool:
                return new ValueType(false, false);
            case t_uint8:
                return new ValueType((uint8_t)0, false);
            case t_char:
                return new ValueType((char)0, false);
            case t_int32:
                return new ValueType(0, false);
            case t_fp32:
                return new ValueType((float)0.0, false);
            case t_fp64:
                return new ValueType(0.0, false);
            default:
                return &None;
        }
    } else {
        ValueType* arr = new ValueType[this->arrayT];
        for (int i = 0; i < this->arrayT; ++i) {
            arr[i] = ValueType(& None, new TypeDecl(this->baseType), false);
        }
        return new ValueType(arr, this, false);
    }
}

static int return_flag = 0;

int AST::interpret(Program prog) {
    SymTable *st = new SymTable();
    st->addLayer();
    runtime_bind(st);
    prog.interpret(st);
    st->removeLayer();
    delete st;

    return 0;
}

INTERPRET(Program) {
    st->addLayer();
    for (auto stmt = stmts.begin(); stmt != stmts.end(); stmt++) {
        (*stmt)->declare(st, nullptr);
    }
    auto fs = st->lookup(Name("main"), this)->data.fs;
    st->addLayer();
    fs->fd->interpret(st);
    st->removeLayer();
    st->removeLayer();
    return & None;
}

INTERPRET(VarDecl) {
    ValueType *t;
    if (this->type.baseType == AST::t_void) {
        if (this->init == nullptr) {
            throw InterpreterException(
                "variable " + this->name.str() + " has unknown type", *this);
        } else {
            t = this->init->interpret(st);
            if (t == & None) {
                throw InterpreterException(
                    "variable " + this->name.str() + " has void type", *this);
            }
        }
    } else {
        if (this->init != nullptr) {
            t = this->init->interpret(st);
            if (t == nullptr) {
                throw InterpreterException(
                    "variable \"" + this->name.str() + "\" has void type", *this);
            }
            if (!t->type.eq(& this->type)) {
                throw InterpreterException(
                    "type mismatch for \"" + this->name.str() + '\"', *this);
            }
        } else {
            t = this->type.newVal();
        }
    }
    if (this->is_const) {
            t->isConst = true;
        }
    st->insert(this->name, t);
    return t;
}

void FuncDecl::declare(SymTable *st, ValueType *context) {
    FuncStore *cur = new FuncStore(this, context);
    st->insert(this->name, new ValueType(cur, true));
}

INTERPRET(FuncDecl) {
    // handle "this"

    for (auto&& e : this->exprs) {
        ValueType *vt = e->interpret(st);
        if (return_flag) {
            return_flag--;
            return vt;
        }
    }
    return & None;
}

INTERPRET(FuncCall) {
    st->addLayer();

    auto fn_ = st->lookup(this->function, this);
    if (fn_->type.baseType == t_rtfn) {
        return runtime_handler(this->function, this, st);
    }
    if (fn_->type.baseType != t_fn)
        throw InterpreterException("type cannot be called", *this);

    auto fn = fn_->data.fs;

    for (unsigned int i = 0; i < this->pars.size(); ++i) {
        auto vt = this->pars[i]->interpret(st);
        auto prm = fn->fd->pars[i];
        if (!vt->type.eq(& prm.type)) {
            throw InterpreterException("type mismatch for argument " + prm.name, *this);
        }
        st->insert(Name(prm.name), vt);
    }

    if (fn->context != nullptr)
        st->insert(Name("this"), fn->context);

    auto ret = fn->fd->interpret(st);
    st->removeLayer();

    return ret;
}

// runtime helper function to create initializer
void ClassDecl::declare(SymTable *st, ValueType *context) {
    st->insert(this->name, new ValueType(this, true));
    for (auto&& stmt : this->stmts) {
        switch (stmt->stmtType) {
            case gs_func: {
                auto fd = (FuncDecl *)stmt.get();
                if (fd->name.BaseName == "new") {
                    st->insert(Name(& this->name, "new"), new ValueType(new FuncStore(fd, nullptr), true));
                }
                break;
            }
            default:
                break;
        }
    }
}

void UnionDecl::declare(SymTable *st, ValueType *context) {
    // TODO: union declaration
}

bool vt_is_true(ValueType *vt, BaseAST *ast) {
    if (vt == nullptr) {
        throw InterpreterException("expression is nullptr", *ast);
    }
    if ((vt->type.baseType != t_bool) && (vt->type.arrayT == 0)) {
        throw InterpreterException("expression is not boolean", *ast);
    }
    return vt->data.one_bit;
}

INTERPRET(IfExpr) {
    st->addLayer();
    auto cond_vt = this->cond->interpret(st);
    if (vt_is_true(cond_vt, this)) {
        for (auto&& expr : this->iftrue) {
            auto ret = expr->interpret(st);
            if (expr->exprType == e_ret) {
                return ret;
            }
        }
    } else {
        for (auto&& expr : this->iffalse) {
            auto ret = expr->interpret(st);
            if (expr->exprType == e_ret) {
                return ret;
            }
        }
    }
    st->removeLayer();
    return & None;
}

INTERPRET(ForExpr) {
    st->addLayer();
    this->init->interpret(st);
    auto cond_vt = this->cond->interpret(st);
    while (vt_is_true(cond_vt, this)) {
        for (auto&& expr : this->exprs) {
            auto ret = expr->interpret(st);
            if (expr->exprType == e_ret) {
                return ret;
            }
        }
        this->step->interpret(st);
        cond_vt = this->cond->interpret(st);
    }
    st->removeLayer();
    return & None;
}

INTERPRET(WhileExpr) {
    st->addLayer();
    auto cond_vt = this->cond->interpret(st);
    while (vt_is_true(cond_vt, this)) {
        for (auto&& expr : this->exprs) {
            auto ret = expr->interpret(st);
            if (expr->exprType == e_ret) {
                return ret;
            }
        }
        cond_vt = this->cond->interpret(st);
    }
    st->removeLayer();
    return & None;
}

INTERPRET(ExprVal) {
    if (this->isConst) {
        return ConstEval(this);
    }
    ValueType *vt;
    if (this->call != nullptr) {
        vt = this->call->interpret(st);
    } else {
        vt = st->lookup(this);
    }
    return vt;
}

INTERPRET(RetExpr) {
    return_flag++;
    if (this->stmt == nullptr)
        return & None;
    return this->stmt->interpret(st);
}

INTERPRET(ContExpr) {
    // todo
    return_flag++;
    return nullptr;
}

INTERPRET(BreakExpr) {
    // todo
    return_flag++;
    return nullptr;
}

INTERPRET(MatchExpr) {  // TODO: implementation
    return & None;
}

INTERPRET(MatchLine) {  // TODO : implementation
    return & None;
}

INTERPRET(EvalExpr) {
    if (this->isVal) {
        return this->val->interpret(st);
    }
    // std::cout << this->op << std::endl;  // TODO: print remove
    if (this->op == equ) {
        if (!this->l->isVal)
            throw InterpreterException("lvalue is unassignable", *this);
        auto lvt = st->lookup(this->l->val.get());
        if (lvt->isConst)
            throw InterpreterException("constant cannot be assigned", *this);
        auto rvt = this->r->interpret(st);

        if (!lvt->type.eq(& rvt->type))
            throw InterpreterException("type mismatch", *this);
        lvt->data = rvt->data;
        return & None;
    }

    auto lvt = this->l->interpret(st);
        auto rvt = this->r->interpret(st);
        if ((lvt->type.arrayT != 0) || (rvt->type.arrayT != 0))
            throw InterpreterException(terms[this->op] + " cannot operate on array", *this);
        if (!lvt->type.eq(& rvt->type))
            throw InterpreterException("type mismatch", *this);

    if (this->op == add) {
        switch (lvt->type.baseType) {
            case t_uint8:
                return new ValueType(lvt->data.bval + rvt->data.bval);
            case t_int32:
                return new ValueType(lvt->data.ival + rvt->data.ival);
            case t_fp32:
                return new ValueType(lvt->data.fval + rvt->data.fval);
            case t_fp64:
                return new ValueType(lvt->data.dval + rvt->data.dval);
            default:
                throw InterpreterException("+ cannot operate on this type", *this);
        }
    }
    if (this->op == sub) {
        switch (lvt->type.baseType) {
            case t_uint8:
                return new ValueType(lvt->data.bval - rvt->data.bval);
            case t_int32:
                return new ValueType(lvt->data.ival - rvt->data.ival);
            case t_fp32:
                return new ValueType(lvt->data.fval - rvt->data.fval);
            case t_fp64:
                return new ValueType(lvt->data.dval - rvt->data.dval);
            default:
                throw InterpreterException("- cannot operate on this type", *this);
        }
    }
    if (this->op == mul) {
        switch (lvt->type.baseType) {
            case t_uint8:
                return new ValueType(lvt->data.bval * rvt->data.bval);
            case t_int32:
                return new ValueType(lvt->data.ival * rvt->data.ival);
            case t_fp32:
                return new ValueType(lvt->data.fval * rvt->data.fval);
            case t_fp64:
                return new ValueType(lvt->data.dval * rvt->data.dval);
            default:
                throw InterpreterException("* cannot operate on this type", *this);
        }
    }
    if (this->op == lt) {
        switch (lvt->type.baseType) {
            case t_uint8:
                return new ValueType((bool)(lvt->data.bval < rvt->data.bval));
            case t_int32:
                return new ValueType((bool)(lvt->data.ival < rvt->data.ival));
            case t_fp32:
                return new ValueType((bool)(lvt->data.fval < rvt->data.fval));
            case t_fp64:
                return new ValueType((bool)(lvt->data.dval < rvt->data.dval));
            default:
                throw InterpreterException("< cannot operate on this type", *this);
        }
    }
    if (this->op == gt) {
        switch (lvt->type.baseType) {
            case t_uint8:
                return new ValueType((bool)(lvt->data.bval > rvt->data.bval));
            case t_int32:
                return new ValueType((bool)(lvt->data.ival > rvt->data.ival));
            case t_fp32:
                return new ValueType((bool)(lvt->data.fval > rvt->data.fval));
            case t_fp64:
                return new ValueType((bool)(lvt->data.dval > rvt->data.dval));
            default:
                throw InterpreterException("> cannot operate on this type", *this);
        }
    }
    throw InterpreterException("unhandled operator " + terms[this->op], *this);
}
