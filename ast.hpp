#pragma once

#include <vector>
#include <string>
#include <iostream>

#include "tree.hpp"
#include "lexical.hpp"

#include "err.hpp"

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

/* Error Logging */
#define LogError(e) std::cerr << "CodeGen Error: " << e << std::endl

namespace AST {
    class ASTs;
    class BaseAST;
    class ClassDecl;
    class ConstDecl;
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

    class BaseAST {
        public:
        virtual ~BaseAST() {}
        virtual void print(int indent) {
            for (int i = 0; i < indent; ++i)
                std::cout << "  ";
            std::cout << "BaseAST()" << std::endl;
        }

        virtual llvm::Value *codegen() = 0;
    };
    class ASTs : public BaseAST  {
        public:
        std::vector<BaseAST*> stmts;
        ASTs(){}

        inline void insert(BaseAST* p) {
            if (p)
                this->stmts.push_back(p);
        }

        void print(int);
        virtual llvm::Value *codegen();
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

        virtual void print(int indent){
            std::cout << "NOT COMPLETED" << std::endl;
        }
        virtual llvm::Value *codegen() {
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

        virtual void print(int indent){
            std::cout << "NOT COMPLETED" << std::endl;
        }

        virtual llvm::Value *codegen() {
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
        virtual llvm::Value *codegen();
    };

    class TypeDecl : public BaseAST  {
        public:
        static const int VOID  = 0;
        static const int INT32 = 1;
        static const int UINT8 = 2;
        static const int FP32  = 3;
        static const int FP64  = 4;
        static const int CHAR  = 5;
        static const int STRING= 6;

        int baseType;
        int arrayT;

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
            
            LogError("internal: type " << t << " undefined");
            exit(ERR_PARSER);
        }

        void print(int);
        virtual llvm::Value *codegen();
    };

    class EvalExpr : public Expr {
        public:
        bool isVal;
        ExprVal* val;
        std::string op;
        EvalExpr *l, *r;

        EvalExpr(ExprVal *v) {
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
        virtual llvm::Value *codegen();
    };

    class FuncCall : public BaseAST {
        public:
        std::vector<EvalExpr*> pars;

        FuncCall() {
            pars.clear();
        }

        void print(int);
        virtual llvm::Value *codegen();
    };

    class ExprVal : public BaseAST {
        public:
        bool isConst;

        std::string constVal;
        TypeDecl *type;
        
        std::string refName;
        FuncCall *call;
        EvalExpr *array;

        ExprVal(std::string v, TypeDecl *t) {
            this->isConst = true;
            this->constVal = v;
            this->type = t;
        }

        ExprVal(std::string n, FuncCall *c, EvalExpr *a) {
            this->isConst = false;
            this->refName = n;
            this->call = c;
            this->array = a;
        }

        void print(int);
        virtual llvm::Value *codegen();
    };

    class GenericDecl : public BaseAST  {
        public:
        bool valid;
        std::string name;

        GenericDecl() {
            this->valid = false;
        }

        GenericDecl(std::string n) {
            this->valid = true;
            this->name = n;
        }

        void print(int);
        virtual llvm::Value *codegen();
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
        virtual llvm::Value *codegen();
    };

    class Option : public BaseAST  {
        public:
        std::string name;
        std::vector<Param*> pars;

        Option(std::string n) {
            this->name = n;
            this->pars.clear();
        }

        void print(int);
        virtual llvm::Value *codegen();
    };

    class RetExpr : public Expr {
        public:
        EvalExpr *stmt;

        RetExpr(EvalExpr* s) {
            this->exprType = RET;

            this->stmt = s;
        }

        void print(int);
        virtual llvm::Value *codegen();
    };

    class VarDecl : public GlobalStatement, public Expr {
        public:
        std::string name;
        TypeDecl *type;
        EvalExpr *init;

        VarDecl(std::string n, TypeDecl* t, EvalExpr* i) {
            this->stmtType = GlobalStatement::VARDECL;
            this->exprType = Expr::VARDECL;

            this->name = n;
            this->type = t;
            this->init = i;
        }

        void print(int);
        virtual llvm::Value *codegen();
    };

    class ConstDecl : public GlobalStatement, public Expr {
        public:
        std::string name;
        TypeDecl *type;
        EvalExpr *init;

        ConstDecl(std::string n, EvalExpr* i) {
            this->stmtType = GlobalStatement::CONSTDECL;
            this->exprType = EvalExpr::CONSTDECL;

            this->name = n;
            this->type = NULL;
            this->init = i;
        }

        void print(int);
        virtual llvm::Value *codegen();
    };

    class FuncDecl : public GlobalStatement {
        public:
        std::string name;
        GenericDecl *genType;
        std::vector<Param*> pars;
        TypeDecl *ret;
        std::vector<Expr*> exprs;

        FuncDecl(std::string n, GenericDecl* g, TypeDecl* r) {
            this->stmtType = FUNCDECL;

            this->name = n;
            this->genType = g;
            pars.clear();
            this->ret = r;
            exprs.clear();
        }

        void print(int);
        virtual llvm::Value *codegen();
    };

    class ClassDecl : public GlobalStatement {
        public:
        std::string name;
        GenericDecl *genType;
        std::vector<GlobalStatement*> stmts;

        ClassDecl(std::string n, GenericDecl* g){
            this->stmtType = CLASSDECL;

            this->name = n;
            this->genType = g;
            this->stmts.clear();
        }

        void print(int);
        virtual llvm::Value *codegen();
    };

    class EnumDecl : public GlobalStatement {
        public:
        std::string name;
        std::vector<Option*> options;

        EnumDecl(std::string n) {
            this->stmtType = ENUMDECL;

            this->name = n;
            this->options.clear();
        }

        void print(int);
        virtual llvm::Value *codegen();
    };

    class IfExpr : public Expr {
        public:
        EvalExpr *cond;
        std::vector<Expr*> iftrue;
        std::vector<Expr*> iffalse;

        IfExpr(EvalExpr *c) {
            this->exprType = IF;

            this->cond = c;
            this->iftrue.clear();
            this->iffalse.clear();
        }

        void print(int);
        virtual llvm::Value *codegen();
    };

    class WhileExpr : public Expr {
        public:
        EvalExpr *cond;
        std::vector<Expr*> exprs;

        WhileExpr(EvalExpr* c) {
            this->exprType = WHILE;

            this->cond = c;
            this->exprs.clear();
        }

        void print(int);
        virtual llvm::Value *codegen();
    };

    class ForExpr : public Expr {
        public:
        EvalExpr *init;
        EvalExpr *cond;
        EvalExpr *step;
        std::vector<Expr*> exprs;

        ForExpr(EvalExpr* i, EvalExpr* c, EvalExpr* s){
            this->exprType = FOR;

            this->init = i;
            this->cond = c;
            this->step = s;
            this->exprs.clear();
        }

        void print(int);
        virtual llvm::Value *codegen();
    };

    class MatchLine : public BaseAST  {
        public:
        std::string name;
        std::vector<Param*> pars;
        std::vector<Expr*> exprs;

        MatchLine(std::string n){
            this->name = n;
            this->pars.clear();
            this->exprs.clear();
        }

        void print(int);
        virtual llvm::Value *codegen();
    };

    class MatchExpr : public Expr {
        public:
        EvalExpr *var;
        std::vector<MatchLine*> lines;

        MatchExpr(EvalExpr* v) {
            this->exprType = MATCH;

            this->var = v;
            this->lines.clear();
        }

        void print(int);
        virtual llvm::Value *codegen();
    };

    extern Program generate(Node<Lexical> *root);
    extern int codegen(Program prog, std::string outFile);
}
