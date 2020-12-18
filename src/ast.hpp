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
class ValueType;
class BaseAST;
// Runtime Information
class Name {
 public:
    std::vector<std::string> ClassName;
    std::string BaseName;

    Name() {
    }

    explicit Name(std::string b) : BaseName(b) {
        this->ClassName.clear();
    }
    Name(Name *p, std::string b) : BaseName(b) {
        for (auto n : p->ClassName)
            this->ClassName.push_back(n);
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

class TypeDecl : public BaseAST  {
 public:
    enum Types {
        t_void, t_int32, t_uint8, t_fp32, t_fp64, t_char, t_str, t_class, t_fn,
        t_other, t_bool,
        t_rtfn /* runtime function */
    } baseType;

    int arrayT;
    std::string other;

    inline bool eq(TypeDecl *other) {
        return (this->baseType == other->baseType) &&
                (this->arrayT == other->arrayT);
    }

    explicit TypeDecl(Types t, int i = 0) : baseType(t), arrayT(i) {}

    TypeDecl(Types t, std::string i) : baseType(t), arrayT(std::stoi(i)) {}

    TypeDecl(std::string t, int i) : arrayT(i) {
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

    TypeDecl(std::string t, std::string i) : TypeDecl(t, std::stoi(i)) {}

    void print(int);
    virtual ValueType *interpret(SymTable *st) {
        throw std::runtime_error("TypeDecl cannot be interpreted");
    }
};
static TypeDecl VoidType = TypeDecl(TypeDecl::t_void);
static TypeDecl BoolType = TypeDecl(TypeDecl::t_bool);
static TypeDecl IntType  = TypeDecl(TypeDecl::t_int32);
static TypeDecl RuntimeType = TypeDecl(TypeDecl::t_rtfn);

class ValueType {
 public:
    void *val;
    TypeDecl *type;
    bool isConst;
    bool one_bit = false;

    ValueType(void *v, TypeDecl *t, bool c = false) :
        val(v), type(t), isConst(c) {}
    explicit ValueType(bool b, bool c = true) : isConst(c), one_bit(b) {
        type = &BoolType;
    }
};

static ValueType None = ValueType(nullptr, &VoidType, true);

// Abstract Syntax Tree
class ASTs;
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
class Param;
class Program;
class RetExpr;
class ExprVal;
class WhileExpr;
class VarDecl;

typedef FuncDecl *FuncStore;
typedef UnionDecl *UnionStore;

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

class EvalExpr : public Expr {
 public:
    bool isVal;
    ExprVal* val;
    std::string op;
    EvalExpr *l, *r;

    explicit EvalExpr(ExprVal *v) : isVal(true), val(v) {
        this->exprType = e_eval;
    }

    EvalExpr(std::string o, EvalExpr *l, EvalExpr *r) :
    isVal(false), op(o), l(l), r(r) {
        this->exprType = e_eval;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);
};

class FuncCall : public BaseAST {
 public:
    std::vector<EvalExpr*> pars;
    Name function;

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

    ExprVal(std::string v, TypeDecl *t) : isConst(true), constVal(v), type(t) {}

    ExprVal(Name n, FuncCall *c, EvalExpr *a) :
        isConst(false), refName(n), call(c), array(a) {
        if (c != nullptr)
            c->function = n;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);
};

class GenericDecl : public BaseAST  {
 public:
    bool valid;
    std::string name;

    GenericDecl() : valid(false) {}

    explicit GenericDecl(std::string n) : valid(true), name(n) {}

    void print(int);
    virtual ValueType *interpret(SymTable *st) {
        throw std::runtime_error("GenericDecl cannot be interpreted");
    }
};

class Param : public BaseAST  {
 public:
    std::string name;
    TypeDecl *type;

    Param(std::string n, TypeDecl *t) : name(n), type(t) {}

    void print(int);
    virtual ValueType *interpret(SymTable *st) {
        throw std::runtime_error("Param cannot be interpreted");
    }
};

class RetExpr : public Expr {
 public:
    EvalExpr *stmt;

    explicit RetExpr(EvalExpr* s) : stmt(s) {
        this->exprType = e_ret;
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

    ClassDecl(std::string n, GenericDecl* g) : name(Name(n)), genType(g) {
        this->stmtType = gs_class;
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

    VarDecl(std::string n, TypeDecl* t, EvalExpr* i, ClassDecl *cl) :
        type(t), init(i) {
        this->stmtType = gs_var;
        this->exprType = e_var;

        if (cl)
            this->name = Name(& (cl->name), n);
        else
            this->name = Name(n);
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

    FuncDecl(std::string n, GenericDecl* g, TypeDecl* r, ClassDecl* cl) :
        genType(g), ret(r) {
        this->stmtType = gs_func;

        if (cl)
            this->name = Name(& (cl->name), n);
        else
            this->name = Name(n);

        pars.clear();
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

    explicit UnionDecl(std::string n, ASTs *cls = nullptr) : name(Name(n)) {
        this->stmtType = gs_union;

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

    explicit IfExpr(EvalExpr *c) : cond(c) {
        this->exprType = e_if;

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

    explicit WhileExpr(EvalExpr* c) : cond(c) {
        this->exprType = e_while;

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

    ForExpr(EvalExpr* i, EvalExpr* c, EvalExpr* s) : init(i), cond(c), step(s) {
        this->exprType = e_for;
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

    explicit MatchLine(std::string n) : name(n) {
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

    explicit MatchExpr(EvalExpr* v) : var(v) {
        this->exprType = e_match;
        this->lines.clear();
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);
};

extern int interpret(Program prog);
extern Program build_ast(Node<Lexical> *root);
}  // namespace AST
