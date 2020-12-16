/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#pragma once

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <memory>

#include "tree.hpp"
#include "lexical.hpp"

#include "err.hpp"

// Error Logging
#define LogError(e) std::cerr << "AST Error: " << e << std::endl
#define DEBUG

namespace AST {
class SymTable;
class TypeDecl;
// Runtime Information
class Name {
 public:
    std::vector<std::string> ClassName;
    std::string BaseName;

    Name() {
    }

    explicit Name(std::string b) : BaseName(b) {}
    Name(Name *p, std::string b) : BaseName(b) {
        this->ClassName = p->ClassName;
        this->ClassName.push_back(p->BaseName);
    }

    std::string str(void) {
        std::stringstream ss;
        for (auto n : this->ClassName)
            ss << n << ".";
        ss << this->BaseName;
        return ss.str();
    }

    friend bool operator<(const Name& l, const Name& r){
        return l.BaseName < r.BaseName;
    }
};

class ValueType {
 public:
    void *val;
    TypeDecl *type;
    bool isConst;

    ValueType(void *v, TypeDecl *t) {
        this->isConst = false;
        this->val = v;
        this->type = t;
    }

    ValueType(void *v, TypeDecl *t, bool c) {
        this->isConst = c;
        this->val = v;
        this->type = t;
    }
};

// Abstract Syntax Tree
class ASTs;
class BaseAST;
class ClassDecl;
class UnionDecl;
class EvalExpr;
class Expr;
class ForExpr;
class FuncDecl;
class FuncCall;
class GenericDecl;
class GlobalStatement;
class IfExpr;
class MatchExpr;
class MatchLine;
class Option;
class Param;
class Program;
class RetExpr;
class ExprVal;
class WhileExpr;
class VarDecl;

typedef FuncDecl *FuncStore;
typedef UnionDecl *UnionStore;

class BaseAST {
 public:
    virtual ~BaseAST() {}
    virtual void print(int indent) {
        for (int i = 0; i < indent; ++i)
            std::cout << "  ";
        std::cout << "BaseAST()" << std::endl;
    }

    virtual ValueType *interpret(SymTable *st) = 0;
};
class ASTs : public BaseAST  {
 public:
    std::vector<BaseAST*> stmts;
    ASTs() {}

    inline void insert(BaseAST* p) {
        if (p)
            this->stmts.push_back(p);
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st) {
        throw std::runtime_error("ASTs cannot be interpreted");
    }
};

class GlobalStatement : virtual public BaseAST  {
 public:
    enum globalStmtTypes {
        gs_error, gs_var, gs_func, gs_class, gs_union
    };

    globalStmtTypes stmtType = gs_error;

    virtual void print(int indent) {
        std::cout << "NOT COMPLETED" << std::endl;
    }
    virtual ValueType *interpret(SymTable *st) = 0;
    virtual void declare(SymTable *st) = 0;
};

class Expr : virtual public BaseAST  {
 public:
    enum exprTypes {
        e_empty, e_var, e_if, e_while, e_for, e_match, e_ret, e_eval
    };

    exprTypes exprType = e_empty;

    virtual void print(int indent) {
        std::cout << "NOT COMPLETED" << std::endl;
    }

    virtual ValueType *interpret(SymTable *st) {return nullptr;}
};

class Program : public BaseAST {
 public:
    std::vector<GlobalStatement*> stmts;
    Program() {stmts.clear();}

    void insert(GlobalStatement *s) {
        if (s)
            this->stmts.push_back(s);
    }

    void print(void);
    virtual ValueType *interpret(SymTable *st);
};

class TypeDecl : public BaseAST  {
 public:
    enum Types {
        t_void, t_int32, t_uint8, t_fp32, t_fp64, t_char, t_str, t_class, t_fn,
        t_other
    } baseType;

    int arrayT;
    std::string other;

    inline bool eq(TypeDecl *other) {
        return (this->baseType == other->baseType) &&
                (this->arrayT == other->arrayT);
    }

    TypeDecl(Types t, std::string i) {
        this->baseType = t;
        this->arrayT = std::stoi(i);
    }

    TypeDecl(std::string t, std::string i) {
        this->arrayT = std::stoi(i);

        if (t == "VOIDT") {
            this->baseType = t_void;
            return;
        }
        if (t == "INT32") {
            this->baseType = t_int32;
            return;
        }
        if (t == "UINT8") {
            this->baseType = t_uint8;
            return;
        }
        if (t == "FP32") {
            this->baseType = t_fp32;
            return;
        }
        if (t == "FP64") {
            this->baseType = t_fp64;
            return;
        }
        if (t == "CHART") {
            this->baseType = t_char;
            return;
        }
        if (t == "STR") {
            this->baseType = t_str;
            return;
        }

        this->baseType = t_other;
        this->other = t;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st) {
        throw std::runtime_error("TypeDecl cannot be interpreted");
    }
};

class EvalExpr : public Expr {
 public:
    bool isVal;
    ExprVal* val;
    std::string op;
    EvalExpr *l, *r;

    explicit EvalExpr(ExprVal *v) {
        this->exprType = e_eval;
        this->isVal = true;

        this->val = v;
    }

    EvalExpr(std::string o, EvalExpr *l, EvalExpr *r) {
        this->exprType = e_eval;
        this->isVal = false;

        this->op = o;
        this->l = l;
        this->r = r;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);
};

class FuncCall : public BaseAST {
 public:
    std::vector<EvalExpr*> pars;

    FuncCall() {
        pars.clear();
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);
};

class ExprVal : public BaseAST {
 public:
    bool isConst;

    std::string constVal;
    TypeDecl *type;

    Name refName;
    FuncCall *call;
    EvalExpr *array;

    ExprVal(std::string v, TypeDecl *t) {
        this->isConst = true;
        this->constVal = v;
        this->type = t;
    }

    ExprVal(Name n, FuncCall *c, EvalExpr *a) {
        this->isConst = false;
        this->refName = n;
        this->call = c;
        this->array = a;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);
};

class GenericDecl : public BaseAST  {
 public:
    bool valid;
    std::string name;

    GenericDecl() {
        this->valid = false;
    }

    explicit GenericDecl(std::string n) {
        this->valid = true;
        this->name = n;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st) {
        throw std::runtime_error("GenericDecl cannot be interpreted");
    }
};

class Param : public BaseAST  {
 public:
    std::string name;
    TypeDecl *type;

    Param(std::string n, TypeDecl *t) {
        this->name = n;
        this->type = t;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st) {
        throw std::runtime_error("Param cannot be interpreted");
    }
};

class Option : public BaseAST  {
 public:
    std::string name;
    std::vector<Param*> pars;

    explicit Option(std::string n) {
        this->name = n;
        this->pars.clear();
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);
};

class RetExpr : public Expr {
 public:
    EvalExpr *stmt;

    explicit RetExpr(EvalExpr* s) {
        this->exprType = e_ret;

        this->stmt = s;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);
};

class ClassDecl : public GlobalStatement {
 public:
    Name name;
    GenericDecl *genType;
    std::vector<GlobalStatement*> stmts;
    std::vector<VarDecl *> var_members;

    ClassDecl(std::string n, GenericDecl* g) {
        this->stmtType = gs_class;

        this->name = Name(n);
        this->genType = g;
        this->stmts.clear();
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st) {
        throw std::runtime_error("ClassDecl cannot be evaluated");
    }

    virtual void declare(SymTable *st);
};

class VarDecl : public GlobalStatement, public Expr {
 public:
    Name name;
    TypeDecl *type;
    EvalExpr *init;
    bool is_global = false;
    bool is_const = false;

    VarDecl(std::string n, TypeDecl* t, EvalExpr* i, ClassDecl *cl) {
        this->stmtType = gs_var;
        this->exprType = e_var;

        if (cl)
            this->name = Name(& (cl->name), n);
        else
            this->name = Name(n);
        this->type = t;
        this->init = i;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);
    virtual void declare(SymTable *st) {
        this->interpret(st);
    }
};

class FuncDecl : public GlobalStatement {
 public:
    Name name;
    GenericDecl *genType;
    std::vector<Param*> pars;
    TypeDecl *ret;
    std::vector<Expr*> exprs;

    FuncDecl(std::string n, GenericDecl* g, TypeDecl* r, ClassDecl* cl) {
        this->stmtType = gs_func;

        if (cl)
            this->name = Name(& (cl->name), n);
        else
            this->name = Name(n);

        this->genType = g;
        pars.clear();
        this->ret = r;
        exprs.clear();
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);
    virtual void declare(SymTable *st);
};

class UnionDecl : public GlobalStatement {
 public:
    Name name;
    std::vector<ClassDecl *> classes;

    explicit UnionDecl(std::string n, ASTs *cls = nullptr) {
        this->stmtType = gs_union;
        this->name = Name(n);

        for (auto p : cls->stmts) {
            classes.push_back(dynamic_cast<ClassDecl *>(p));
        }
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st) {
        throw std::runtime_error("UnionDecl cannot be interpreted");
    }
    virtual void declare(SymTable *st);
};

class IfExpr : public Expr {
 public:
    EvalExpr *cond;
    std::vector<Expr*> iftrue;
    std::vector<Expr*> iffalse;

    explicit IfExpr(EvalExpr *c) {
        this->exprType = e_if;

        this->cond = c;
        this->iftrue.clear();
        this->iffalse.clear();
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);
};

class WhileExpr : public Expr {
 public:
    EvalExpr *cond;
    std::vector<Expr*> exprs;

    explicit WhileExpr(EvalExpr* c) {
        this->exprType = e_while;

        this->cond = c;
        this->exprs.clear();
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);
};

class ForExpr : public Expr {
 public:
    EvalExpr *init;
    EvalExpr *cond;
    EvalExpr *step;
    std::vector<Expr*> exprs;

    ForExpr(EvalExpr* i, EvalExpr* c, EvalExpr* s) {
        this->exprType = e_for;

        this->init = i;
        this->cond = c;
        this->step = s;
        this->exprs.clear();
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);
};

class MatchLine : public BaseAST  {
 public:
    std::string name;
    std::vector<Param*> pars;
    std::vector<Expr*> exprs;

    explicit MatchLine(std::string n) {
        this->name = n;
        this->pars.clear();
        this->exprs.clear();
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);
};

class MatchExpr : public Expr {
 public:
    EvalExpr *var;
    std::vector<MatchLine*> lines;

    explicit MatchExpr(EvalExpr* v) {
        this->exprType = e_match;

        this->var = v;
        this->lines.clear();
    }

    void print(int);
    virtual ValueType *interpret();
};

extern int interpret(Program prog);
extern Program build_ast(Node<Lexical> *root);
}  // namespace AST
