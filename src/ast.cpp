/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#include "ast.hpp"

#include <memory>

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
    for (auto p : this->stmts) {
        p->print(1);
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
class AST::SymTable {
 private:
    std::vector<std::map<AST::Name, AST::ValueType*>> d;

 public:
    void reset(void) {
        this->d.clear();
    }

    void addLayer(void) {
        d.push_back(std::map<Name, ValueType *>());
    }

    void removeLayer(void) {
        // for (auto vt : d.back()) {
        //     delete vt.second;
        // }
        d.pop_back();
    }

    void insert(Name name, ValueType *vt) {
        d.back()[name] = vt;
    }

    void insert(Name name, void *v, TypeDecl *t, bool is_const = false) {
        if (is_const) {
            d.back()[name] = new ValueType(v, t, true);
        } else {
            d.back()[name] = new ValueType(v, t);
        }
    }

    ValueType* lookup(Name name, bool is_top = false) {
        if (is_top) {
            if (d.back().find(name) != d.back().end()) {
                return d.back()[name];
            }
        } else {
            for (int i = d.size() - 1; i >= 0; i--) {
                if (d[i].find(name) != d[i].end()) {
                    return d[i][name];
                }
            }
        }
        throw std::runtime_error("variable " + name.str() + " is not declared");
    }

    ValueType* lookup(ExprVal *name) {
        if (name->call != nullptr)
            throw std::runtime_error("cannot lookup a function call");

        if (name->array != nullptr) {
            auto arr = this->lookup(name->refName);
            auto arr_index_vt = name->array->interpret(this);
            if (!arr_index_vt->type->eq(& IntType)) {
                throw std::runtime_error("array index must be an int");
            }
            int  arr_index = arr_index_vt->data.ival;

            if (arr->type->arrayT <= arr_index) {
                throw std::runtime_error("array index out of bound");
            }

            auto vts = (ValueType *)arr->data.ptr;
            return &(vts[arr_index]);
        } else {
            return this->lookup(name->refName);
        }
    }
};

/**
 * Interpreter Interface - interpret() methods
 */
ValueType *ConstEval(ExprVal *e) {
    switch (e->type->baseType) {
        case TypeDecl::t_void: {
            LogError("no constant type \"void\"");
            return nullptr;
        }
        case TypeDecl::t_int32: {
            return new ValueType(std::stoi(e->constVal));
        }
        case TypeDecl::t_char: {
            return new ValueType((char)std::stoi(e->constVal));
        }
        case TypeDecl::t_fp32: {
            return new ValueType(std::stof(e->constVal));
        }
        case TypeDecl::t_fp64: {
            return new ValueType(std::stod(e->constVal));
        }
        case TypeDecl::t_str: {
            std::string *s = new std::string(e->constVal);
            return new ValueType(s, e->type, true);
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
            case TypeDecl::t_bool:
                return new ValueType(false, false);
            case TypeDecl::t_uint8:
                return new ValueType((uint8_t)0, false);
            case TypeDecl::t_char:
                return new ValueType((char)0, false);
            case TypeDecl::t_int32:
                return new ValueType(0, false);
            case TypeDecl::t_fp32:
                return new ValueType((float)0.0, false);
            case TypeDecl::t_fp64:
                return new ValueType(0.0, false);
            default:
                throw std::runtime_error("cannot initialize this type");
        }
    } else {
        ValueType* arr = new ValueType[this->arrayT];
        for (int i = 0; i < this->arrayT; ++i) {
            arr[i] = ValueType(nullptr, new TypeDecl(this->baseType), false);
        }
        return new ValueType((void*)arr, this, false);
    }
}

int AST::interpret(Program prog) {
    SymTable *st = new SymTable();
    st->addLayer();
    st->insert(Name("print"), new ValueType(nullptr, &RuntimeType, true));
    prog.interpret(st);
    st->removeLayer();
    delete st;

    return 0;
}

INTERPRET(Program) {
    st->addLayer();
    for (auto stmt : stmts) {
        stmt->declare(st);
    }
    auto fs = reinterpret_cast<FuncStore *>(
        st->lookup(Name("main"))->data.ptr);
    st->addLayer();
    (*fs)->interpret(st);
    st->removeLayer();
    st->removeLayer();
    return nullptr;
}

INTERPRET(VarDecl) {
    ValueType *t;
    if (this->type == nullptr) {
        if (this->init == nullptr) {
            throw std::runtime_error(
                "variable " + this->name.str() + " has unknown type");
        } else {
            t = this->init->interpret(st);
            if (t == nullptr) {
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
            if (!t->type->eq(this->type)) {
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

void AST::FuncDecl::declare(SymTable *st) {
    FuncStore *fs = new FuncStore(this);
    st->insert(this->name, fs, new TypeDecl(TypeDecl::t_fn, 0), true);
}

INTERPRET(FuncDecl) {
    // parameters already passed in st
    // st layer handled in FuncCall
    for (auto e : this->exprs) {
        ValueType *vt = e->interpret(st);
        if (e->exprType == e->e_ret) {
            return vt;
        }
    }
    return & AST::None;
}

INTERPRET(FuncCall) {
    st->addLayer();

    // std::cout << "this->function: " << this->function.str() << std::endl;  // TODO: remove
    auto fn_ = st->lookup(this->function);
    if (fn_->type->baseType == TypeDecl::t_rtfn) {
        return runtime_handler(this->function.str(), this, st);
    }
    if (fn_->type->baseType != TypeDecl::t_fn)
        throw std::runtime_error("type cannot be called");

    auto fn = *((FuncStore *)(fn_->data.ptr));

    for (unsigned int i = 0; i < this->pars.size(); ++i) {
        auto vt = this->pars[i]->interpret(st);
        auto prm = fn->pars[i];
        if (!vt->type->eq(prm->type)) {
            throw std::runtime_error("type mismatch for argument " + prm->name);
        }
        st->insert(Name(prm->name), vt);
    }

    auto ret = fn->interpret(st);
    st->removeLayer();

    return ret;
}

void AST::ClassDecl::declare(SymTable *st) {
    // TODO: class declaration
    // runtime helper function to create initializer
}

void AST::UnionDecl::declare(SymTable *st) {
    // TODO: union declaration
}

bool vt_is_true(ValueType *vt) {
    if (vt == nullptr) {
        throw std::runtime_error("expression is nullptr");
    }
    if ((vt->type->baseType != TypeDecl::t_bool) && (vt->type->arrayT == 0)) {
        throw std::runtime_error("expression is not boolean");
    }
    return vt->data.one_bit;
}

INTERPRET(IfExpr) {
    st->addLayer();
    auto cond_vt = this->cond->interpret(st);
    if (vt_is_true(cond_vt)) {
        for (auto expr : this->iftrue)
            expr->interpret(st);
    } else {
        for (auto expr : this->iffalse)
            expr->interpret(st);
    }
    st->removeLayer();
    return nullptr;
}

INTERPRET(ForExpr) {
    st->addLayer();
    this->init->interpret(st);
    auto cond_vt = this->cond->interpret(st);
    while (vt_is_true(cond_vt)) {
        for (auto expr : this->exprs) {
            expr->interpret(st);
        }
        this->step->interpret(st);
        cond_vt = this->cond->interpret(st);
    }
    st->removeLayer();
    return nullptr;
}

INTERPRET(WhileExpr) {
    st->addLayer();
    auto cond_vt = this->cond->interpret(st);
    while (vt_is_true(cond_vt)) {
        for (auto expr : this->exprs) {
            expr->interpret(st);
        }
        cond_vt = this->cond->interpret(st);
    }
    st->removeLayer();
    return nullptr;
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
    if (this->stmt == nullptr)
        return & AST::None;
    return this->stmt->interpret(st);
}

INTERPRET(MatchExpr) {  // TODO: implementation
    return nullptr;
}

INTERPRET(MatchLine) {  // TODO : implementation
    return nullptr;
}

void ValueType::assign(ValueType *that) {
    if (!this->type->eq(that->type))
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
        return nullptr;
    }
    if (this->op == "ADD") {
        auto lvt = this->l->interpret(st);
        auto rvt = this->r->interpret(st);
        if ((lvt->type->arrayT != 0) || (rvt->type->arrayT != 0))
            throw std::runtime_error("+ cannot operate on array");
        if (!lvt->type->eq(rvt->type))
            throw std::runtime_error("type mismatch");
        switch (lvt->type->baseType) {
            case TypeDecl::t_uint8:
                return new ValueType(lvt->data.bval + rvt->data.bval);
            case TypeDecl::t_int32:
                return new ValueType(lvt->data.ival + rvt->data.ival);
            case TypeDecl::t_fp32:
                return new ValueType(lvt->data.fval + rvt->data.fval);
            case TypeDecl::t_fp64:
                return new ValueType(lvt->data.dval + rvt->data.dval);
            default:
                throw std::runtime_error("+ cannot operate on this type");
        }
    }
    if (this->op == "SUB") {
        auto lvt = this->l->interpret(st);
        auto rvt = this->r->interpret(st);
        if ((lvt->type->arrayT != 0) || (rvt->type->arrayT != 0))
            throw std::runtime_error("- cannot operate on array");
        if (!lvt->type->eq(rvt->type))
            throw std::runtime_error("type mismatch");
        switch (lvt->type->baseType) {
            case TypeDecl::t_uint8:
                return new ValueType(lvt->data.bval - rvt->data.bval);
            case TypeDecl::t_int32:
                return new ValueType(lvt->data.ival - rvt->data.ival);
            case TypeDecl::t_fp32:
                return new ValueType(lvt->data.fval - rvt->data.fval);
            case TypeDecl::t_fp64:
                return new ValueType(lvt->data.dval - rvt->data.dval);
            default:
                throw std::runtime_error("- cannot operate on this type");
        }
    }
    if (this->op == "MUL") {
        auto lvt = this->l->interpret(st);
        auto rvt = this->r->interpret(st);
        if ((lvt->type->arrayT != 0) || (rvt->type->arrayT != 0))
            throw std::runtime_error("* cannot operate on array");
        if (!lvt->type->eq(rvt->type))
            throw std::runtime_error("type mismatch: ");
        switch (lvt->type->baseType) {
            case TypeDecl::t_uint8:
                return new ValueType(lvt->data.bval * rvt->data.bval);
            case TypeDecl::t_int32:
                return new ValueType(lvt->data.ival * rvt->data.ival);
            case TypeDecl::t_fp32:
                return new ValueType(lvt->data.fval * rvt->data.fval);
            case TypeDecl::t_fp64:
                return new ValueType(lvt->data.dval * rvt->data.dval);
            default:
                throw std::runtime_error("* cannot operate on this type");
        }
    }
    if (this->op == "LT") {
        auto lvt = this->l->interpret(st);
        auto rvt = this->r->interpret(st);
        if ((lvt->type->arrayT != 0) || (rvt->type->arrayT != 0))
            throw std::runtime_error("< cannot operate on array");
        if (!lvt->type->eq(rvt->type))
            throw std::runtime_error("type mismatch");
        switch (lvt->type->baseType) {
            case TypeDecl::t_uint8:
                return new ValueType((bool)(lvt->data.bval < rvt->data.bval));
            case TypeDecl::t_int32:
                return new ValueType((bool)(lvt->data.ival < rvt->data.ival));
            case TypeDecl::t_fp32:
                return new ValueType((bool)(lvt->data.fval < rvt->data.fval));
            case TypeDecl::t_fp64:
                return new ValueType((bool)(lvt->data.dval < rvt->data.dval));
            default:
                throw std::runtime_error("< cannot operate on this type");
        }
    }
    if (this->op == "GT") {
        auto lvt = this->l->interpret(st);
        auto rvt = this->r->interpret(st);
        if ((lvt->type->arrayT != 0) || (rvt->type->arrayT != 0))
            throw std::runtime_error("> cannot operate on array");
        if (!lvt->type->eq(rvt->type))
            throw std::runtime_error("type mismatch");
        switch (lvt->type->baseType) {
            case TypeDecl::t_uint8:
                return new ValueType((bool)(lvt->data.bval > rvt->data.bval));
            case TypeDecl::t_int32:
                return new ValueType((bool)(lvt->data.ival > rvt->data.ival));
            case TypeDecl::t_fp32:
                return new ValueType((bool)(lvt->data.fval > rvt->data.fval));
            case TypeDecl::t_fp64:
                return new ValueType((bool)(lvt->data.dval > rvt->data.dval));
            default:
                throw std::runtime_error("> cannot operate on this type");
        }
    }
    return nullptr;
}
