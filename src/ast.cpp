/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#include "ast.hpp"

#include <memory>

#include "ast_gen.hpp"
#include "util.hpp"

using namespace AST;

#define INTERPRET(x) ValueType *AST::x::interpret(SymTable *st)

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

void Option::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "Option(" << this->name << ')' << std::endl;

    for (int i = 0; i < indent + 1; ++i)
            std::cout << "  ";
        std::cout << "Params(" << std::endl;
    for (auto p : this->pars) {
        p->print(indent + 2);
    }
    for (int i = 0; i < indent + 1; ++i)
            std::cout << "  ";
    std::cout << ")" << std::endl;
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
        for (auto vt : d.back()) {
            delete vt.second;
        }
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
        throw std::runtime_error("variable " + name.str() + "is not declared");
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
            int *i = new int(std::stoi(e->constVal));
            return new ValueType(i, e->type, true);
        }
        case TypeDecl::t_char: {
            char *c = new char(std::stoi(e->constVal));
            return new ValueType(c, e->type, true);
        }
        case TypeDecl::t_fp32: {
            float *f = new float(std::stof(e->constVal));
            return new ValueType(f, e->type, true);
        }
        case TypeDecl::t_fp64: {
            double *f = new double(std::stod(e->constVal));
            return new ValueType(f, e->type, true);
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


int AST::interpret(Program prog) {
    SymTable *st = new SymTable();
    prog.interpret(st);
    delete st;

    return 0;
}

INTERPRET(Program) {
    st->addLayer();
    for (auto stmt : stmts) {
        stmt->declare(st);
    }
    auto fs = reinterpret_cast<FuncStore *>(
        st->lookup(Name("main"))->val);
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
            t = new ValueType(nullptr, this->type);
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
    ValueType *ret_val = nullptr;
    for (auto e : this->exprs) {
        ValueType *vt = e->interpret(st);
        if (vt != nullptr) {
            ret_val = vt;
        }
    }
    return ret_val;
}

INTERPRET(FuncCall) {
    st->addLayer();

    auto fn_ = st->lookup(this->function);
    auto fn = *(reinterpret_cast<FuncStore *>(fn_->val));

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
    // class declaration
    // runtime helper function to create initializer
}

void AST::UnionDecl::declare(SymTable *st) {
    // union declaration
}

bool vt_is_true(ValueType *vt) {
    if (vt == nullptr) {
        throw std::runtime_error("expression is nullptr");
    }
    if ((vt->type->baseType != TypeDecl::t_bool) && (vt->type->arrayT == 0)) {
        throw std::runtime_error("expression is not boolean");
    }
    return *reinterpret_cast<bool *>(vt->val);
}

INTERPRET(ForExpr) {
    this->init->interpret(st);
    auto cond_vt = this->cond->interpret(st);
    while (vt_is_true(cond_vt)) {
        for (auto expr : this->exprs) {
            expr->interpret(st);
        }
        this->step->interpret(st);
        cond_vt = this->cond->interpret(st);
    }
    return nullptr;
}

INTERPRET(WhileExpr) {
    auto cond_vt = this->cond->interpret(st);
    while (vt_is_true(cond_vt)) {
        for (auto expr : this->exprs) {
            expr->interpret(st);
        }
        cond_vt = this->cond->interpret(st);
    }
    return nullptr;
}

INTERPRET(ExprVal) {  // TODO: implementation
    return nullptr;
}

INTERPRET(RetExpr) {
    return this->stmt->interpret(st);
}

INTERPRET(MatchExpr) {  // TODO: implementation
    return nullptr;
}

INTERPRET(MatchLine) {  // TODO : implementation
    return nullptr;
}
