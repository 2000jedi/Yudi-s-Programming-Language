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
    x& operator=(x&&) = default;

namespace AST {
// Abstract Syntax Tree
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
class EnumDecl;
class UnionDecl;
class ExprVal;

class MemStore {
 private:
    ValueType *v;
 public:
    bool placehold;

    MemStore() : v(nullptr), placehold(false) {}
    explicit MemStore(ValueType *vt) : v(vt), placehold(false) {}

    void Free(void);
    void set(ValueType *v);
    ValueType *get(void);
};

class SymTable {
 private:
    std::vector<std::map<Name, MemStore>> d;

 public:
    ~SymTable();
    void addLayer(void);
    void removeLayer(void);
    MemStore insert(Name name, ValueType *vt);
    MemStore update(ExprVal *name, ValueType *vt);
    MemStore *lookup(Name name, ErrInfo *ast);
    MemStore *lookup(ExprVal *name);
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

    friend bool operator<(const Name& l, const Name& r) {
        if (l.BaseName == r.BaseName) {
            if (l.ClassName.size() == r.ClassName.size()) {
                for (int i = 0; i < (int)l.ClassName.size(); ++i) {
                    if (l.ClassName[i] != r.ClassName[i]) {
                        return l.ClassName[i] < r.ClassName[i];
                    }
                }
            }
            return l.ClassName.size() < r.ClassName.size();
        }
        return l.BaseName < r.BaseName;
    }

    friend bool operator==(const Name& l, const Name& r) {
        if (l.BaseName != r.BaseName)
            return false;
        if (l.ClassName.size() != r.ClassName.size())
            return false;
        for (unsigned int i = 0; i < l.ClassName.size(); ++i) {
            if (l.ClassName[i] != r.ClassName[i])
                return false;
        }
        return true;
    }
};

class GenericDecl : public ErrInfo {
 public:
    bool valid;
    std::string name;

    GenericDecl() : valid(false) {}
    GenericDecl(scanner *Scanner, std::string n) : ErrInfo(Scanner), valid(true), name(n) {}
};

enum Types {
    t_void, t_int32, t_uint8, t_fp32, t_fp64, t_char, t_str, t_class, t_fn,
    t_bool, t_rtfn, t_enum /* runtime function */
};

class TypeDecl : public ErrInfo {
 public:
    Types baseType;

    int arrayT;
    Name other;
    GenericDecl gen;

    friend bool operator==(const TypeDecl& lhs, const TypeDecl& rhs) {
        if (lhs.baseType == t_class) {
            return (lhs.baseType == rhs.baseType) && (lhs.other == rhs.other);
        }
        return (lhs.baseType == rhs.baseType) &&
                (lhs.arrayT == rhs.arrayT);
    }

    friend bool operator!=(const TypeDecl& lhs, const TypeDecl& rhs) {
        return !(lhs == rhs);
    }

    explicit TypeDecl(Types t, int i = 0) :
        baseType(t), arrayT(i), gen(GenericDecl()) {}

    TypeDecl(Types t, std::string i) : baseType(t), arrayT(std::stoi(i)), gen(GenericDecl()) {}

    TypeDecl(Name t, int i) : arrayT(i), other(t), gen(GenericDecl()) {
        this->baseType = t_class;
    }

    TypeDecl(scanner *Scanner, Types t, std::string o, GenericDecl g, int i) :
        ErrInfo(Scanner), baseType(t), arrayT(i), other(Name(o)), gen(g) {
        if ((t != t_class) && (g.valid))
            throw std::runtime_error("no generic is possible");
    }

    ValueType *newVal(void);
    std::string str(void);
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
static TypeDecl EnumType = TypeDecl(t_enum);

class FuncStore {
 public:
    FuncDecl *fd;
    MemStore context;

    FuncStore(FuncDecl *a, SymTable *b, TypeDecl t);
    ~FuncStore();
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
        MemStore *vt;
        SymTable *st;
        FuncStore* fs;
        UnionDecl* ud;
        ClassDecl* cd;
        EnumDecl* ed;
        std::string* str;
    } data;

    TypeDecl type;
    std::vector<MemStore*> ms;  // records
    bool isConst;

    ValueType() : type(VoidType) {
        data.ival = 0;
    }

    ~ValueType() {
        if (this->ms.size() == 0) {
            if (this->type.arrayT != 0) {
                for (int i = 0; i < this->type.arrayT; ++i) {
                    this->data.vt[i].Free();
                }
                delete[] this->data.vt;
                return;
            }

            switch (this->type.baseType) {
                case t_str:
                    delete data.str;
                    return;
                case t_fn:
                    delete data.fs;
                    return;
                case t_class:
                    delete data.st;
                    return;
                default:
                    return;
            }
        }
    }

    ValueType(SymTable *v, TypeDecl *t, bool c = false) : type(*t), isConst(c) {
        data.st = v;
    }

    explicit ValueType(TypeDecl *t, bool c = false) : type(*t), isConst(c) {
        if (t->baseType == t_rtfn) {
            data.ival = 0;
            return;
        }
        if (t->arrayT != 0) {
            data.vt = new MemStore[t->arrayT];
            return;
        }
    }

    explicit ValueType(FuncStore *v, bool c = false) : type(FuncType), isConst(c) {
        data.fs = v;
    }

    explicit ValueType(ClassDecl *v, bool c = false) : type(RuntimeType), isConst(c) {
        data.cd = v;
    }

    explicit ValueType(EnumDecl *v, bool c = false) : type(EnumType), isConst(c) {
        data.ed = v;
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
};

static ValueType None = ValueType();

enum globalStmtTypes {
    gs_error, gs_var, gs_func, gs_class, gs_union
};
class GlobalStatement {
 public:
    globalStmtTypes stmtType = gs_error;
    virtual ValueType *interpret(SymTable *st) = 0;
    virtual void declare(SymTable *st, SymTable *context) = 0;
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
    virtual ValueType *interpret(SymTable *st) {return nullptr;}
    Expr() {}
    virtual ~Expr() {}
};

class Program : public ErrInfo {
 public:
    std::vector<std::unique_ptr<GlobalStatement>> stmts;
    explicit Program(scanner *Scanner) : ErrInfo(Scanner) {}

    void insert(std::unique_ptr<GlobalStatement> s) {
        if (s)
            this->stmts.push_back(std::move(s));
    }

    ValueType *interpret(SymTable *st);

    D_MOVE_COPY(Program)
};

class EvalExpr : public ErrInfo, public Expr {
 public:
    bool isVal;
    std::unique_ptr<ExprVal> val;
    token op;
    std::unique_ptr<EvalExpr> l, r;

    EvalExpr(scanner *Scanner, std::unique_ptr<ExprVal> v) :
        ErrInfo(Scanner), isVal(true), val(std::move(v)) {
        this->exprType = e_eval;
    }

    EvalExpr(
        scanner *Scanner,
        token o,
        std::unique_ptr<EvalExpr> l,
        std::unique_ptr<EvalExpr> r) :
        ErrInfo(Scanner), isVal(false), op(o), l(std::move(l)), r(std::move(r)) {
        this->exprType = e_eval;
    }
    virtual ValueType *interpret(SymTable *st);
};

class FuncCall : public ErrInfo {
 public:
    std::vector<std::unique_ptr<EvalExpr>> pars;
    Name function;

    explicit FuncCall(scanner *Scanner) : ErrInfo(Scanner) {}
    ValueType *interpret(SymTable *st);

    D_MOVE_COPY(FuncCall)
};

class ExprVal : public ErrInfo {
 public:
    bool isConst;

    std::string constVal;
    TypeDecl type;

    Name refName;
    std::unique_ptr<FuncCall> call;
    std::unique_ptr<EvalExpr> array;

    ExprVal(scanner *Scanner, std::string v, TypeDecl t) :
        ErrInfo(Scanner), isConst(true), constVal(v), type(t) {}

    ExprVal(scanner *Scanner, Name n, std::unique_ptr<FuncCall> c, std::unique_ptr<EvalExpr> a) :
        ErrInfo(Scanner), isConst(false), type(TypeDecl(t_void)), refName(n), call(std::move(c)),
        array(std::move(a)) {
        if (c != nullptr)
            c->function = n;
    }
    ValueType *interpret(SymTable *st);

    D_MOVE_COPY(ExprVal)
};

class Param : public ErrInfo  {
 public:
    std::string name;
    TypeDecl type;

    Param(scanner *Scanner, std::string n, TypeDecl t) : ErrInfo(Scanner), name(n), type(t) {}
};

class RetExpr : public ErrInfo, public Expr {
 public:
    std::unique_ptr<EvalExpr> stmt;

    RetExpr(scanner *Scanner, std::unique_ptr<EvalExpr> s) : ErrInfo(Scanner), stmt(std::move(s)) {
        this->exprType = e_ret;
    }
    virtual ValueType *interpret(SymTable *st);

    D_MOVE_COPY(RetExpr)
};

class ContExpr : public ErrInfo, public Expr {
 public:
    explicit ContExpr(scanner *Scanner) : ErrInfo(Scanner) {
        this->exprType = e_cont;
    }
    virtual ValueType *interpret(SymTable *st);
};

class BreakExpr : public ErrInfo, public Expr {
 public:
    explicit BreakExpr(scanner *Scanner) : ErrInfo(Scanner) {
        this->exprType = e_break;
    }
    virtual ValueType *interpret(SymTable *st);
};

class ClassDecl : public ErrInfo, public GlobalStatement {
 public:
    Name name;
    GenericDecl genType;
    std::vector<std::unique_ptr<GlobalStatement>> stmts;

    ClassDecl(scanner *Scanner, std::string n, GenericDecl g) :
        ErrInfo(Scanner), name(Name(n)), genType(g) {
        this->stmtType = gs_class;
    }
    virtual ValueType *interpret(SymTable *st) {
        throw std::runtime_error("ClassDecl cannot be evaluated");
    }

    virtual void declare(SymTable *st, SymTable *context);

    D_MOVE_COPY(ClassDecl)
};

class EnumDecl : public ErrInfo {
 public:
    Name name;
    std::vector<std::unique_ptr<VarDecl>> vars;

    EnumDecl(scanner *Scanner, std::string n) :
        ErrInfo(Scanner), name(Name(n)) {}

    D_MOVE_COPY(EnumDecl)
};

class VarDecl : public ErrInfo, public GlobalStatement, public Expr {
 public:
    Name name;
    TypeDecl type;
    std::unique_ptr<EvalExpr> init;
    bool is_global = false;
    bool is_const = false;

    VarDecl(
        scanner *Scanner,
        std::string n,
        TypeDecl t,
        std::unique_ptr<EvalExpr> i) :
        ErrInfo(Scanner), name(Name(n)), type(t), init(std::move(i)) {
        this->stmtType = gs_var;
        this->exprType = e_var;
    }
    virtual ValueType *interpret(SymTable *st);
    virtual void declare(SymTable *st, SymTable *context) {
        this->interpret(st);
    }

    D_MOVE_COPY(VarDecl)
};

class FuncDecl : public ErrInfo, public GlobalStatement {
 public:
    Name name;
    GenericDecl genType;
    std::vector<Param> pars;
    TypeDecl ret;
    std::vector<std::unique_ptr<Expr>> exprs;

    FuncDecl(scanner *Scanner, Name n, GenericDecl g, std::vector<Param> prms, TypeDecl r) :
        ErrInfo(Scanner), name(n), genType(g), pars(prms), ret(r) {
        this->stmtType = gs_func;
    }
    virtual ValueType *interpret(SymTable *st);
    virtual void declare(SymTable *st, SymTable *context);

    D_MOVE_COPY(FuncDecl)
};

class UnionDecl : public ErrInfo, public GlobalStatement {
 public:
    Name name;
    std::vector<std::unique_ptr<EnumDecl>> classes;
    GenericDecl gen;

    UnionDecl(scanner *Scanner, Name n, GenericDecl gen) :
        ErrInfo(Scanner), name(n), gen(gen) {
        this->stmtType = gs_union;
    }
    virtual ValueType *interpret(SymTable *st) {
        throw std::runtime_error("UnionDecl cannot be interpreted");
    }
    virtual void declare(SymTable *st, SymTable *context);

    D_MOVE_COPY(UnionDecl)
};

class IfExpr : public ErrInfo, public Expr {
 public:
    std::unique_ptr<EvalExpr> cond;
    std::vector<std::unique_ptr<Expr>> iftrue;
    std::vector<std::unique_ptr<Expr>> iffalse;

    IfExpr(scanner *Scanner, std::unique_ptr<EvalExpr> c) :
        ErrInfo(Scanner), cond(std::move(c)) {
        this->exprType = e_if;
    }
    virtual ValueType *interpret(SymTable *st);

    D_MOVE_COPY(IfExpr)
};

class WhileExpr : public ErrInfo, public Expr {
 public:
    std::unique_ptr<EvalExpr> cond;
    std::vector<std::unique_ptr<Expr>> exprs;

    WhileExpr(scanner *Scanner, std::unique_ptr<EvalExpr> c) :
        ErrInfo(Scanner), cond(std::move(c)) {
        this->exprType = e_while;
    }
    virtual ValueType *interpret(SymTable *st);

    D_MOVE_COPY(WhileExpr)
};

class ForExpr : public ErrInfo, public Expr {
 public:
    std::unique_ptr<EvalExpr> init, cond, step;
    std::vector<std::unique_ptr<Expr>> exprs;

    ForExpr(
        scanner *Scanner,
        std::unique_ptr<EvalExpr> i,
        std::unique_ptr<EvalExpr> c,
        std::unique_ptr<EvalExpr> s) :
        ErrInfo(Scanner), init(std::move(i)), cond(std::move(c)), step(std::move(s)) {
        this->exprType = e_for;
    }
    virtual ValueType *interpret(SymTable *st);

    D_MOVE_COPY(ForExpr)
};

class MatchLine : public ErrInfo  {
 public:
    std::string name;
    std::string cl_name;
    std::vector<std::unique_ptr<Expr>> exprs;

    MatchLine(scanner *Scanner, std::string n, std::string cl) :
        ErrInfo(Scanner), name(n), cl_name(cl) {}
    ValueType *interpret(SymTable *st);

    D_MOVE_COPY(MatchLine)
};

class MatchExpr : public ErrInfo, public Expr {
 public:
    std::unique_ptr<EvalExpr> var;
    std::vector<MatchLine> lines;

    MatchExpr(scanner *Scanner, std::unique_ptr<EvalExpr> v) :
        ErrInfo(Scanner), var(std::move(v)) {
        this->exprType = e_match;
    }
    virtual ValueType *interpret(SymTable *st);

    D_MOVE_COPY(MatchExpr)
};

extern int interpret(Program prog);
}  // namespace AST
