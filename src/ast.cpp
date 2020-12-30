/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#include "ast.hpp"

#include <memory>
#include <utility>

#include "ast_gen.hpp"
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
    for (auto p : this->pars) p->print(indent + 1);
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

    if (this->type)
        this->type->print(indent + 1);
}

void RetExpr::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "RetExpr()" << std::endl;
    this->stmt->print(indent + 1);
}

void VarDecl::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "VarDecl(" << this->name.str() << ')' << std::endl;
    if (this->type)
        this->type->print(indent + 1);
    if (this->init)
        this->init->print(indent + 1);
}

void FuncDecl::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "FuncDecl(" << this->name.str() << ')' << std::endl;
    if (this->genType)
        this->genType->print(indent + 1);

    for (int i = 0; i < indent + 1; ++i)
            std::cout << "  ";
        std::cout << "Params(" << std::endl;
    for (auto p : this->pars) {
        p->print(indent + 2);
    }
    for (int i = 0; i < indent + 1; ++i)
            std::cout << "  ";
    std::cout << ")" << std::endl;

    if (this->ret)
        this->ret->print(indent + 1);

    for (int i = 0; i < indent + 1; ++i)
            std::cout << "  ";
        std::cout << "Expressions(" << std::endl;
    for (auto p : this->exprs) {
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
    if (this->genType)
        this->genType->print(indent + 1);

    for (int i = 0; i < indent + 1; ++i)
            std::cout << "  ";
        std::cout << "Statements(" << std::endl;
    for (auto p : this->stmts) {
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
    for (auto p : this->classes) {
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
    for (auto p : this->iftrue) {
        p->print(indent + 2);
    }
    for (int i = 0; i < indent + 1; ++i)
        std::cout << "  ";
    std::cout << "False()" << std::endl;
    for (auto p : this->iffalse) {
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
    for (auto p : this->exprs) {
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
    for (auto p : this->exprs) {
        p->print(indent + 2);
    }
    for (int i = 0; i < indent + 1; ++i)
        std::cout << "  ";
    std::cout << ")" << std::endl;
}

void MatchLine::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "MatchExpr(" << this->name << ")" << std::endl;

    for (int i = 0; i < indent + 1; ++i)
            std::cout << "  ";
        std::cout << "Params(" << std::endl;
    for (auto p : this->pars) {
        p->print(indent + 2);
    }
    for (int i = 0; i < indent + 1; ++i)
            std::cout << "  ";
    std::cout << ")" << std::endl;

    for (int i = 0; i < indent + 1; ++i)
            std::cout << "  ";
        std::cout << "Expessions(" << std::endl;
    for (auto p : this->exprs) {
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
    for (auto p : this->lines) {
        p->print(indent + 2);
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

ValueType* SymTable::lookup(Name name) {
    if (name.ClassName.size() > 0) {
        auto owner = this->lookup(name.owner());
        if (!((owner->type.baseType) == t_rtfn && (name.BaseName == "new"))) {
            if (owner->type.baseType != t_class) {
                throw std::runtime_error(name.owner().str() + " is not a compound type");
            }
            auto clst = owner->data.st;
            return clst->lookup(Name(name.BaseName));
        }
    }

    for (int i = d.size() - 1; i >= 0; i--) {
        if (d[i].find(name) != d[i].end()) {
            return d[i][name];
        }
    }
    throw std::runtime_error("variable " + name.str() + " is not declared");
}

ValueType* SymTable::lookup(ExprVal *name) {
    if (name->call != nullptr)
        throw std::runtime_error("cannot lookup a function call");

    if (name->array != nullptr) {
        auto arr = this->lookup(name->refName);
        auto arr_index_vt = name->array->interpret(this);
        if (!arr_index_vt->type.eq(& IntType)) {
            throw std::runtime_error("array index must be an int");
        }
        int  arr_index = arr_index_vt->data.ival;

        if (arr->type.arrayT <= arr_index) {
            throw std::runtime_error("array index out of bound");
        }

        auto vts = arr->data.vt;
        return &(vts[arr_index]);
    } else {
        return this->lookup(name->refName);
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
    switch (e->type->baseType) {
        case t_void: {
            throw std::runtime_error("no constant type \"void\"");
        }
        case t_int32: {
            return new ValueType(std::stoi(e->constVal));
        }
        case t_char: {
            return new ValueType(e->constVal[1]);
        }
        case t_fp32: {
            return new ValueType(std::stof(e->constVal));
        }
        case t_fp64: {
            return new ValueType(std::stod(e->constVal));
        }
        case t_str: {
            std::string *s = new std::string(e->constVal);
            return new ValueType(s, true);
        }
        default: {
            LogError("TypeDecl index " << e->type->baseType << " not handled");
            return nullptr;
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
                throw std::runtime_error("cannot initialize this type");
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
    auto fs = st->lookup(Name("main"))->data.fs;
    st->addLayer();
    fs->fd->interpret(st);
    st->removeLayer();
    st->removeLayer();
    return & None;
}

INTERPRET(VarDecl) {
    ValueType *t;
    if (this->type == nullptr) {
        if (this->init == nullptr) {
            throw std::runtime_error(
                "variable " + this->name.str() + " has unknown type");
        } else {
            t = this->init->interpret(st);
            if (t == & None) {
                throw std::runtime_error(
                    "variable " + this->name.str() + " has void type");
            }
        }
    } else {
        if (this->init != nullptr) {
            t = this->init->interpret(st);
            if (t == nullptr) {
                throw std::runtime_error(
                    "variable \"" + this->name.str() + "\" has void type");
            }
            if (!t->type.eq(this->type)) {
                throw std::runtime_error(
                    "type mismatch for \"" + this->name.str() + '\"');
            }
        } else {
            t = this->type->newVal();  // TODO: construct arrays & new ValueTypes
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

    for (auto e : this->exprs) {
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

    auto fn_ = st->lookup(this->function);
    if (fn_->type.baseType == t_rtfn) {
        return runtime_handler(this->function, this, st);
    }
    if (fn_->type.baseType != t_fn)
        throw std::runtime_error("type cannot be called");

    auto fn = fn_->data.fs;

    for (unsigned int i = 0; i < this->pars.size(); ++i) {
        auto vt = this->pars[i]->interpret(st);
        auto prm = fn->fd->pars[i];
        if (!vt->type.eq(prm->type)) {
            throw std::runtime_error("type mismatch for argument " + prm->name);
        }
        st->insert(Name(prm->name), vt);
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
    for (auto stmt : this->stmts) {
        switch (stmt->stmtType) {
            case gs_func: {
                FuncDecl *fd = dynamic_cast<FuncDecl*>(stmt);
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

bool vt_is_true(ValueType *vt) {
    if (vt == nullptr) {
        throw std::runtime_error("expression is nullptr");
    }
    if ((vt->type.baseType != t_bool) && (vt->type.arrayT == 0)) {
        throw std::runtime_error("expression is not boolean");
    }
    return vt->data.one_bit;
}

INTERPRET(IfExpr) {
    st->addLayer();
    auto cond_vt = this->cond->interpret(st);
    if (vt_is_true(cond_vt)) {
        for (auto expr : this->iftrue) {
            auto ret = expr->interpret(st);
            if (expr->exprType == e_ret) {
                return ret;
            }
        }
    } else {
        for (auto expr : this->iffalse) {
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
    while (vt_is_true(cond_vt)) {
        for (auto expr : this->exprs) {
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
    while (vt_is_true(cond_vt)) {
        for (auto expr : this->exprs) {
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

INTERPRET(MatchExpr) {  // TODO: implementation
    return & None;
}

INTERPRET(MatchLine) {  // TODO : implementation
    return & None;
}

void ValueType::assign(ValueType *that) {
    if (!this->type.eq(& that->type))
        throw std::runtime_error("type mismatch");
    this->data = that->data;
}

INTERPRET(EvalExpr) {
    if (this->isVal) {
        return this->val->interpret(st);
    }
    // std::cout << this->op << std::endl;  // TODO: print remove
    if (this->op == "=") {
        if (!this->l->isVal)
            throw std::runtime_error("lvalue is unassignable");
        auto lvt = st->lookup(this->l->val);
        if (lvt->isConst)
            throw std::runtime_error("constant cannot be assigned");
        auto rvt = this->r->interpret(st);
        lvt->assign(rvt);
        return & None;
    }

    auto lvt = this->l->interpret(st);
        auto rvt = this->r->interpret(st);
        if ((lvt->type.arrayT != 0) || (rvt->type.arrayT != 0))
            throw std::runtime_error(this->op + " cannot operate on array");
        if (!lvt->type.eq(& rvt->type))
            throw std::runtime_error("type mismatch");

    if (this->op == "ADD") {
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
                throw std::runtime_error("+ cannot operate on this type");
        }
    }
    if (this->op == "SUB") {
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
                throw std::runtime_error("- cannot operate on this type");
        }
    }
    if (this->op == "MUL") {
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
                throw std::runtime_error("* cannot operate on this type");
        }
    }
    if (this->op == "LT") {
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
                throw std::runtime_error("< cannot operate on this type");
        }
    }
    if (this->op == "GT") {
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
                throw std::runtime_error("> cannot operate on this type");
        }
    }
    return & None;
}
