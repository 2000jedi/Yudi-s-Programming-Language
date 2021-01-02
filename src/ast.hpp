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
#include <utility>
#include <algorithm>

#include "err.hpp"
#include "scanner.hpp"

// Error Logging
#define LogError(e) std::cerr << "AST Error: " << e << std::endl
#define DEBUG
#define D_MOVE_COPY(x)\
    x(const x & other) = delete;\
    x& operator= (const x & other) = delete;\
    x(x&&) = default;\
    x& operator=(x&&) = default;\

namespace AST {
// Abstract Syntax Tree
class BaseAST;
class ASTs;
class UnionDecl;
class EvalExpr;
class Expr;
class ForExpr;
class FuncCall;
class GenericDecl;
class GlobalStatement;
class IfExpr;
class MatchExpr;
class MatchLine;
class Param;
class Program;
class RetExpr;
class WhileExpr;
class VarDecl;
class ValueType;
class Name;
class TypeDecl;
class FuncDecl;
class ClassDecl;
class UnionDecl;
class ExprVal;
class SymTable {
 private:
    std::vector<std::map<Name, ValueType*>> d;

 public:
    void reset(void);
    void addLayer(void);
    void removeLayer(void);
    void insert(Name name, ValueType *vt);
    void insert(Name name, void *v, TypeDecl t, bool is_const);
    ValueType* lookup(Name name, BaseAST *ast);
    ValueType* lookup(ExprVal *name);
};

// Runtime Information
class Name {
 public:
    std::vector<std::string> ClassName;
    std::string BaseName;

    Name() {
    }

    explicit Name(std::string b) : BaseName(b) {}
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

    Name owner(void) {
        Name parent(this->ClassName.back());
        for (int i = 0; i < (int)this->ClassName.size() - 1; ++i) {
            parent.ClassName.push_back(this->ClassName[i]);
        }

        return parent;
    }

    friend bool operator<(const Name& l, const Name& r){
        return l.BaseName < r.BaseName;
    }
};

class BaseAST {
 public:
    std::string line;
    int row;
    int col;
    virtual ~BaseAST() {}
    virtual void print(int indent) {
        for (int i = 0; i < indent; ++i)
            std::cout << "  ";
        std::cout << "BaseAST()" << std::endl;
    }

    virtual ValueType *interpret(SymTable *st) {
        LogError("BaseAST cannot be interpreted");
        return nullptr;
    }
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

enum Types {
    t_void, t_int32, t_uint8, t_fp32, t_fp64, t_char, t_str, t_class, t_fn,
    t_bool, t_rtfn /* runtime function */
};

class TypeDecl : public BaseAST {
 public:
    Types baseType;

    int arrayT;
    Name other;
    GenericDecl gen;

    inline bool eq(TypeDecl *other) {
        if (this->baseType == t_class) {
            return (this->baseType == other->baseType) && (this->other.str() == other->other.str());
        }
        return (this->baseType == other->baseType) &&
                (this->arrayT == other->arrayT);
    }

    explicit TypeDecl(Types t, int i = 0) : baseType(t), arrayT(i), gen(GenericDecl()) {}

    TypeDecl(Types t, std::string i) : baseType(t), arrayT(std::stoi(i)), gen(GenericDecl()) {}

    TypeDecl(Name t, int i) : arrayT(i), other(t), gen(GenericDecl()) {
        this->baseType = t_class;
    }

    TypeDecl(Types t, std::string o, GenericDecl g, int i) : baseType(t), arrayT(i), other(Name(o)), gen(g) {
        if ((t != t_class) && (g.valid))
            throw std::runtime_error("no generic is possible");
    }

    ValueType *newVal(void);

    void print(int);
    virtual ValueType *interpret(SymTable *st) {
        throw std::runtime_error("TypeDecl cannot be interpreted");
    }
};
static TypeDecl VoidType = TypeDecl(t_void);
static TypeDecl BoolType = TypeDecl(t_bool);
static TypeDecl CharType = TypeDecl(t_char);
static TypeDecl IntType  = TypeDecl(t_int32);
static TypeDecl FloatType = TypeDecl(t_fp32);
static TypeDecl DoubleType = TypeDecl(t_fp64);
static TypeDecl RuntimeType = TypeDecl(t_rtfn);
static TypeDecl FuncType = TypeDecl(t_fn);
static TypeDecl ClassType = TypeDecl(t_class);
static TypeDecl StrType = TypeDecl(t_str);

class FuncStore {
 public:
    FuncDecl *fd;
    ValueType *context;

    FuncStore(FuncDecl *a, ValueType *b) : fd(a), context(b) {}
};

class ValueType {
 public:
    union {
        int ival;
        float fval;
        double dval;
        char cval;
        uint8_t bval;
        bool one_bit;
        ValueType *vt;
        SymTable *st;
        FuncStore* fs;
        UnionDecl* ud;
        ClassDecl* cd;
        std::string* str;
    } data;

    TypeDecl type;
    bool isConst;

    ValueType() : type(VoidType) {
        data.ival = 0;
    }

    ValueType(SymTable *v, TypeDecl *t, bool c = false) : type(*t), isConst(c) {
        data.st = v;
    }

    ValueType(ValueType *v, TypeDecl *t, bool c = false) : type(*t), isConst(c) {
        data.vt = v;
    }

    explicit ValueType(FuncStore *v, bool c = false) : type(FuncType), isConst(c) {
        data.fs = v;
    }

    explicit ValueType(ClassDecl *v, bool c = false) : type(RuntimeType), isConst(c) {
        data.cd = v;
    }

    explicit ValueType(std::string *v, bool c = false) : type(StrType), isConst(c) {
        data.str = v;
    }

    explicit ValueType(bool b, bool c = true) : type(BoolType), isConst(c) {
        data.one_bit = b;
    }

    explicit ValueType(char b, bool c = true) : type(CharType), isConst(c) {
        data.cval = b;
    }

    explicit ValueType(int b, bool c = true) : type(IntType), isConst(c) {
        data.ival = b;
    }

    explicit ValueType(float b, bool c = true) : type(FloatType), isConst(c) {
        data.fval = b;
    }

    explicit ValueType(double b, bool c = true) : type(DoubleType), isConst(c) {
        data.dval = b;
    }

    void Free(void);
};

static ValueType None = ValueType();

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

enum globalStmtTypes {
    gs_error, gs_var, gs_func, gs_class, gs_union
};
class GlobalStatement {
 public:
    globalStmtTypes stmtType = gs_error;

    virtual void print(int indent) {
        std::cout << "NOT COMPLETED" << std::endl;
    }
    virtual ValueType *interpret(SymTable *st) = 0;
    virtual void declare(SymTable *st, ValueType *context) = 0;
    GlobalStatement() {}
    virtual ~GlobalStatement() {}
};

enum exprTypes {
    e_empty, e_var, e_if, e_while, e_for, e_match, e_ret, e_eval,
    e_cont, e_break
};
class Expr {
 public:
    exprTypes exprType = e_empty;

    virtual void print(int indent) {
        std::cout << "NOT COMPLETED" << std::endl;
    }

    virtual ValueType *interpret(SymTable *st) {return nullptr;}
    Expr() {}
    virtual ~Expr() {}
};

class Program : public BaseAST {
 public:
    std::vector<std::unique_ptr<GlobalStatement>> stmts;
    Program() {}

    void insert(std::unique_ptr<GlobalStatement> s) {
        if (s)
            this->stmts.push_back(std::move(s));
    }

    void print(void);
    virtual ValueType *interpret(SymTable *st);

    D_MOVE_COPY(Program)
};

class EvalExpr : public BaseAST, public Expr {
 public:
    bool isVal;
    std::unique_ptr<ExprVal> val;
    token op;
    std::unique_ptr<EvalExpr> l, r;

    explicit EvalExpr(std::unique_ptr<ExprVal> v) :
        isVal(true), val(std::move(v)) {
        this->exprType = e_eval;
    }

    EvalExpr(
        token o,
        std::unique_ptr<EvalExpr> l,
        std::unique_ptr<EvalExpr> r) :
        isVal(false), op(o), l(std::move(l)), r(std::move(r)) {
        this->exprType = e_eval;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);
};

class FuncCall : public BaseAST {
 public:
    std::vector<std::unique_ptr<EvalExpr>> pars;
    Name function;

    FuncCall() {}

    void print(int);
    virtual ValueType *interpret(SymTable *st);

    D_MOVE_COPY(FuncCall)
};

class ExprVal : public BaseAST {
 public:
    bool isConst;

    std::string constVal;
    TypeDecl type;

    Name refName;
    std::unique_ptr<FuncCall> call;
    std::unique_ptr<EvalExpr> array;

    ExprVal(std::string v, TypeDecl t) : isConst(true), constVal(v), type(t) {}

    ExprVal(Name n, std::unique_ptr<FuncCall> c, std::unique_ptr<EvalExpr> a) :
        isConst(false), type(TypeDecl(t_void)), refName(n), call(std::move(c)),
        array(std::move(a)) {
        if (c != nullptr)
            c->function = n;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);

    D_MOVE_COPY(ExprVal)
};

class Param : public BaseAST  {
 public:
    std::string name;
    TypeDecl type;

    Param(std::string n, TypeDecl t) : name(n), type(t) {}

    void print(int);
    virtual ValueType *interpret(SymTable *st) {
        throw std::runtime_error("Param cannot be interpreted");
    }
};

class RetExpr : public BaseAST, public Expr {
 public:
    std::unique_ptr<EvalExpr> stmt;

    explicit RetExpr(std::unique_ptr<EvalExpr> s) : stmt(std::move(s)) {
        this->exprType = e_ret;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);

    D_MOVE_COPY(RetExpr)
};

class ContExpr : public BaseAST, public Expr {
 public:
    ContExpr(void) {
        this->exprType = e_cont;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);
};

class BreakExpr : public BaseAST, public Expr {
 public:
    BreakExpr(void) {
        this->exprType = e_break;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);
};

class ClassDecl : public BaseAST, public GlobalStatement {
 public:
    Name name;
    GenericDecl genType;
    std::vector<std::unique_ptr<GlobalStatement>> stmts;
    // std::vector<std::VarDecl *> var_members;

    ClassDecl(std::string n, GenericDecl g) : name(Name(n)), genType(g) {
        this->stmtType = gs_class;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st) {
        throw std::runtime_error("ClassDecl cannot be evaluated");
    }

    virtual void declare(SymTable *st, ValueType *context);

    D_MOVE_COPY(ClassDecl)
};

class VarDecl : public BaseAST, public GlobalStatement, public Expr {
 public:
    Name name;
    TypeDecl type;
    std::unique_ptr<EvalExpr> init;
    bool is_global = false;
    bool is_const = false;

    VarDecl(
        std::string n,
        TypeDecl t,
        std::unique_ptr<EvalExpr> i) :
        name(Name(n)), type(t), init(std::move(i)) {
        this->stmtType = gs_var;
        this->exprType = e_var;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);
    virtual void declare(SymTable *st, ValueType *context) {
        this->interpret(st);
    }

    D_MOVE_COPY(VarDecl)
};

class FuncDecl : public BaseAST, public GlobalStatement {
 public:
    Name name;
    GenericDecl genType;
    std::vector<Param> pars;
    TypeDecl ret;
    std::vector<std::unique_ptr<Expr>> exprs;

    FuncDecl(Name n, GenericDecl g, std::vector<Param> prms, TypeDecl r) :
        name(n), genType(g), pars(prms), ret(r) {
        this->stmtType = gs_func;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);
    virtual void declare(SymTable *st, ValueType *context);

    D_MOVE_COPY(FuncDecl)
};

class UnionDecl : public BaseAST, public GlobalStatement {
 public:
    Name name;
    std::vector<std::unique_ptr<ClassDecl>> classes;
    GenericDecl gen;

    explicit UnionDecl(Name n, GenericDecl gen) :
        name(n), gen(gen) {
        this->stmtType = gs_union;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st) {
        throw std::runtime_error("UnionDecl cannot be interpreted");
    }
    virtual void declare(SymTable *st, ValueType *context);

    D_MOVE_COPY(UnionDecl)
};

class IfExpr : public BaseAST, public Expr {
 public:
    std::unique_ptr<EvalExpr> cond;
    std::vector<std::unique_ptr<Expr>> iftrue;
    std::vector<std::unique_ptr<Expr>> iffalse;

    explicit IfExpr(std::unique_ptr<EvalExpr> c) : cond(std::move(c)) {
        this->exprType = e_if;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);

    D_MOVE_COPY(IfExpr)
};

class WhileExpr : public BaseAST, public Expr {
 public:
    std::unique_ptr<EvalExpr> cond;
    std::vector<std::unique_ptr<Expr>> exprs;

    explicit WhileExpr(std::unique_ptr<EvalExpr> c) : cond(std::move(c)) {
        this->exprType = e_while;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);

    D_MOVE_COPY(WhileExpr)
};

class ForExpr : public BaseAST, public Expr {
 public:
    std::unique_ptr<EvalExpr> init, cond, step;
    std::vector<std::unique_ptr<Expr>> exprs;

    ForExpr(
        std::unique_ptr<EvalExpr> i,
        std::unique_ptr<EvalExpr> c,
        std::unique_ptr<EvalExpr> s) :
        init(std::move(i)), cond(std::move(c)), step(std::move(s)) {
        this->exprType = e_for;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);

    D_MOVE_COPY(ForExpr)
};

class MatchLine : public BaseAST  {
 public:
    std::string name;
    std::string cl_name;
    std::vector<std::unique_ptr<Expr>> exprs;

    explicit MatchLine(std::string n, std::string cl) : name(n), cl_name(cl) {}

    void print(int);
    virtual ValueType *interpret(SymTable *st);

    D_MOVE_COPY(MatchLine)
};

class MatchExpr : public BaseAST, public Expr {
 public:
    std::unique_ptr<EvalExpr> var;
    std::vector<MatchLine> lines;

    explicit MatchExpr(std::unique_ptr<EvalExpr> v) : var(std::move(v)) {
        this->exprType = e_match;
    }

    void print(int);
    virtual ValueType *interpret(SymTable *st);

    D_MOVE_COPY(MatchExpr)
};

extern int interpret(Program prog);
}  // namespace AST
