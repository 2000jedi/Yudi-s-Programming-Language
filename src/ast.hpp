#pragma once

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <memory>

#include "tree.hpp"
#include "lexical.hpp"

#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"

#include "err.hpp"

// Error Logging
#define LogError(e) std::cerr << "CodeGen Error: " << e << std::endl
#define DEBUG

static llvm::LLVMContext context;
static llvm::IRBuilder<> builder(context);
static std::unique_ptr<llvm::Module> module;

namespace AST {
class NameSpace {
 public:
    std::string ClassName;
    std::string BaseName;

    NameSpace() {
    }

    explicit NameSpace(std::string b) {
        this->ClassName = "";
        this->BaseName  = b;
    }

    NameSpace(std::string c, std::string b) {
        this->ClassName = c;
        this->BaseName  = b;
    }

    /**
     * Constructor given a parent namespace and current name.
     */
    NameSpace(NameSpace *parent, std::string b) {
        this->ClassName = parent->BaseName;
        this->BaseName  = b;
    }

    std::string str(void) {
        if (this->ClassName == "") {
            if (this->BaseName == "main" || this->BaseName == "printf")
                return this->BaseName;
        }
        std::stringstream ss;
        if (this->ClassName == "") {
            ss << "__" << this->BaseName;
        } else {
            if (this->BaseName == "") {
                ss << "_" << this->ClassName;
            } else {
                ss << "_" << this->ClassName << "__" << this->BaseName;
            }
        }
        return ss.str();
    }

    friend bool operator<(const NameSpace& l, const NameSpace& r){
        if (l.ClassName < r.ClassName)
            return true;
        if (l.ClassName == r.ClassName)
            if (l.BaseName < r.BaseName)
                return true;
        return false;
    }
};

class ValueType;

class ASTs;
class BaseAST;
class ClassDecl;
class EnumDecl;
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
class TypeDecl;
class ExprVal;
class WhileExpr;
class VarDecl;

class BaseAST {
 public:
    virtual ~BaseAST() {}
    virtual void print(int indent) {
        for (int i = 0; i < indent; ++i)
            std::cout << "  ";
        std::cout << "BaseAST()" << std::endl;
    }

    virtual ValueType *codegen() = 0;
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
    virtual ValueType *codegen();
};

class GlobalStatement : virtual public BaseAST  {
 public:
    static const int ERROR     = 0;
    static const int VARDECL   = 1;
    static const int CONSTDECL = 2;
    static const int FUNCDECL  = 3;
    static const int CLASSDECL = 4;
    static const int ENUMDECL  = 5;

    int stmtType = ERROR;

    virtual void print(int indent) {
        std::cout << "NOT COMPLETED" << std::endl;
    }
    virtual ValueType *codegen() {
        return nullptr;
    }
};

class Expr : virtual public BaseAST  {
 public:
    static const int EMPTY     = 0;
    static const int VARDECL   = 1;
    static const int CONSTDECL = 2;
    static const int IF        = 3;
    static const int WHILE     = 4;
    static const int FOR       = 5;
    static const int MATCH     = 6;
    static const int RET       = 7;
    static const int EVAL      = 8;

    int exprType = EMPTY;

    virtual void print(int indent) {
        std::cout << "NOT COMPLETED" << std::endl;
    }

    virtual ValueType *codegen() {
        return nullptr;
    }
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
    virtual ValueType *codegen();
};

class TypeDecl : public BaseAST  {
 public:
    static const int VOID   = 0;
    static const int INT32  = 1;
    static const int UINT8  = 2;
    static const int FP32   = 3;
    static const int FP64   = 4;
    static const int CHAR   = 5;
    static const int STRING = 6;
    static const int OTHER  = 7;

    int baseType;
    int arrayT;
    std::string other;

    inline bool eq(TypeDecl *other) {
        return (this->baseType == other->baseType) &&
                (this->arrayT == other->arrayT);
    }

    TypeDecl(int t, std::string i) {
        this->baseType = t;
        this->arrayT = std::stoi(i);
    }

    TypeDecl(std::string t, std::string i) {
        this->arrayT = std::stoi(i);

        if (t == "VOIDT") {
            this->baseType = VOID;
            return;
        }
        if (t == "INT32") {
            this->baseType = INT32;
            return;
        }
        if (t == "UINT8") {
            this->baseType = UINT8;
            return;
        }
        if (t == "FP32") {
            this->baseType = FP32;
            return;
        }
        if (t == "FP64") {
            this->baseType = FP64;
            return;
        }
        if (t == "CHART") {
            this->baseType = CHAR;
            return;
        }
        if (t == "STR") {
            this->baseType = STRING;
            return;
        }

        this->baseType = OTHER;
        this->other = t;
    }

    void print(int);
    virtual ValueType *codegen();
};

class EvalExpr : public Expr {
 public:
    bool isVal;
    ExprVal* val;
    std::string op;
    EvalExpr *l, *r;

    explicit EvalExpr(ExprVal *v) {
        this->exprType = EVAL;
        this->isVal = true;

        this->val = v;
    }

    EvalExpr(std::string o, EvalExpr *l, EvalExpr *r) {
        this->exprType = EVAL;
        this->isVal = false;

        this->op = o;
        this->l = l;
        this->r = r;
    }

    void print(int);
    virtual ValueType *codegen();
};

class FuncCall : public BaseAST {
 public:
    std::vector<EvalExpr*> pars;

    FuncCall() {
        pars.clear();
    }

    void print(int);
    virtual ValueType *codegen();
};

class ExprVal : public BaseAST {
 public:
    bool isConst;

    std::string constVal;
    TypeDecl *type;

    NameSpace refName;
    FuncCall *call;
    EvalExpr *array;

    ExprVal(std::string v, TypeDecl *t) {
        this->isConst = true;
        this->constVal = v;
        this->type = t;
    }

    ExprVal(NameSpace n, FuncCall *c, EvalExpr *a) {
        this->isConst = false;
        this->refName = n;
        this->call = c;
        this->array = a;
    }

    void print(int);
    virtual ValueType *codegen();
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
    virtual ValueType *codegen();
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
    virtual ValueType *codegen();
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
    virtual ValueType *codegen();
};

class RetExpr : public Expr {
 public:
    EvalExpr *stmt;

    explicit RetExpr(EvalExpr* s) {
        this->exprType = RET;

        this->stmt = s;
    }

    void print(int);
    virtual ValueType *codegen();
};

class ClassDecl : public GlobalStatement {
 public:
    NameSpace name;
    GenericDecl *genType;
    std::vector<GlobalStatement*> stmts;
    std::vector<VarDecl *> var_members;

    ClassDecl(std::string n, GenericDecl* g) {
        this->stmtType = CLASSDECL;

        this->name = NameSpace(n, "");
        this->genType = g;
        this->stmts.clear();
    }

    void print(int);
    virtual ValueType *codegen();
};

class VarDecl : public GlobalStatement, public Expr {
 public:
    NameSpace name;
    TypeDecl *type;
    EvalExpr *init;
    bool is_global = false;
    bool is_const = false;

    VarDecl(std::string n, TypeDecl* t, EvalExpr* i, ClassDecl *cl) {
        this->stmtType = GlobalStatement::VARDECL;
        this->exprType = Expr::VARDECL;

        if (cl)
            this->name = NameSpace(cl->name.ClassName, n);
        else
            this->name = NameSpace(n);
        this->type = t;
        this->init = i;
    }

    void print(int);
    virtual ValueType *codegen();
};

class FuncDecl : public GlobalStatement {
 public:
    NameSpace name;
    GenericDecl *genType;
    std::vector<Param*> pars;
    TypeDecl *ret;
    std::vector<Expr*> exprs;

    FuncDecl(std::string n, GenericDecl* g, TypeDecl* r, ClassDecl* cl) {
        std::stringstream ss;

        this->stmtType = FUNCDECL;

        if (cl)
            this->name = NameSpace(cl->name.ClassName, n);
        else
            this->name = NameSpace(n);

        this->genType = g;
        pars.clear();
        this->ret = r;
        exprs.clear();
    }

    void print(int);
    virtual ValueType *codegen();
};

class EnumDecl : public GlobalStatement {
 public:
    NameSpace name;
    std::vector<Option*> options;

    explicit EnumDecl(std::string n, ClassDecl *cl = nullptr) {
        this->stmtType = ENUMDECL;

        if (cl)
            this->name = NameSpace(cl->name.ClassName, n);
        else
            this->name = NameSpace(n);
        this->options.clear();
    }

    void print(int);
    virtual ValueType *codegen();
};

class IfExpr : public Expr {
 public:
    EvalExpr *cond;
    std::vector<Expr*> iftrue;
    std::vector<Expr*> iffalse;

    explicit IfExpr(EvalExpr *c) {
        this->exprType = IF;

        this->cond = c;
        this->iftrue.clear();
        this->iffalse.clear();
    }

    void print(int);
    virtual ValueType *codegen();
};

class WhileExpr : public Expr {
 public:
    EvalExpr *cond;
    std::vector<Expr*> exprs;

    explicit WhileExpr(EvalExpr* c) {
        this->exprType = WHILE;

        this->cond = c;
        this->exprs.clear();
    }

    void print(int);
    virtual ValueType *codegen();
};

class ForExpr : public Expr {
 public:
    EvalExpr *init;
    EvalExpr *cond;
    EvalExpr *step;
    std::vector<Expr*> exprs;

    ForExpr(EvalExpr* i, EvalExpr* c, EvalExpr* s) {
        this->exprType = FOR;

        this->init = i;
        this->cond = c;
        this->step = s;
        this->exprs.clear();
    }

    void print(int);
    virtual ValueType *codegen();
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
    virtual ValueType *codegen();
};

class MatchExpr : public Expr {
 public:
    EvalExpr *var;
    std::vector<MatchLine*> lines;

    explicit MatchExpr(EvalExpr* v) {
        this->exprType = MATCH;

        this->var = v;
        this->lines.clear();
    }

    void print(int);
    virtual ValueType *codegen();
};

class ValueType {
 public:
    llvm::Value *val;
    TypeDecl *type;
    bool isConst;

    ValueType(llvm::Value *v, TypeDecl *t) {
        this->isConst = false;
        this->val = v;
        this->type = t;
    }

    ValueType(llvm::Value *v, TypeDecl *t, bool c) {
        this->isConst = c;
        this->val = v;
        this->type = t;
    }
};

extern int codegen(Program prog, std::string outFile);
extern Program build_ast(Node<Lexical> *root);
}  // namespace AST

extern void ClearSymLayer(void);
extern void NewSymLayer(void);
extern void RemoveSymLayer(void);
extern void InsertVar(AST::NameSpace name, llvm::Value *v, AST::TypeDecl *t);
extern void InsertConst(AST::NameSpace name, llvm::Value *v, AST::TypeDecl *t);
extern AST::ValueType *FindVar(AST::NameSpace name);
extern AST::ValueType *FindTopVar(AST::NameSpace name);
