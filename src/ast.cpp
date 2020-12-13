/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#include "ast.hpp"

#include <memory>

#include "ast_gen.hpp"
#include "util.hpp"

using namespace AST;

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

// Global Variables

// Symble Table - record Variable and Type Information
// static std::vector<
//     std::unique_ptr<std::map<AST::NameSpace, AST::ValueType*>>> SymTable;

/**
 * Interpreter Interface - interpret() methods
 */

void ClearSymLayer(void) {
    SymTable.clear();
}

void NewSymLayer(void) {
    SymTable.push_back(
        std::make_unique<std::map<NameSpace, ValueType *>>());
}

void RemoveSymLayer(void) {
    SymTable.pop_back();
}

void InsertVar(NameSpace name, void *v, TypeDecl *t) {
    (*SymTable.back())[name] = new ValueType(v, t);
}

void InsertConst(NameSpace name, void *v, TypeDecl *t) {
    (*SymTable.back())[name] = new ValueType(v, t, true);
}

ValueType *FindTopVar(NameSpace name) {
    if (SymTable.back()->find(name) != SymTable.back()->end()) {
        return (*SymTable.back())[name];
    } else {
        return nullptr;
    }
}

ValueType *FindVarRaw(NameSpace name) {
    for (int i = SymTable.size() - 1; i >= 0; i--) {
        if (SymTable[i]->find(name) != SymTable[i]->end()) {
            return (*SymTable[i])[name];
        }
    }

    return nullptr;
}

ValueType *FindVar(NameSpace name) {
    ValueType *vt;
    if (name.ClassName == "") {
        vt = FindVarRaw(name);
        if (!vt) {
            LogError("variable " << name.str()
                << " not found");
            return nullptr;
        }
    } else {
        return nullptr;
    }

    return vt;
}

ValueType *ConstToValue(ExprVal *e) {
    switch (e->type->baseType) {
        case TypeDecl::VOID:
            LogError("no constant type \"void\"");
            return nullptr;
        case TypeDecl::INT32:
            int *i = new int(std::stoi(e->constVal));
            return new ValueType(i, e->type, true);
        case TypeDecl::CHAR:
            char *c = new char(std::stoi(e->constVal));
            return new ValueType(c, e->type, true);
        case TypeDecl::FP32:
        case TypeDecl::FP64:
            double *f = new double(std::stod(e->constVal));
            return new ValueType(f, e->type, true);
        case TypeDecl::STRING: {
            std::string *s = new std::string(e->constVal);
            return new ValueType(s, e->type, true);
        }
        default:
            LogError("TypeDecl index " << e->type->baseType << " not found");
            return nullptr;
    }
}


int AST::interpret(Program prog, std::string outputFileName) {
}
