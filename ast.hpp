#pragma once

#include <vector>
#include <string>
#include <iostream>

#include "tree.hpp"
#include "lexical.hpp"

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
    class Value;
    class WhileExpr;

    class BaseAST {
        public:
        virtual ~BaseAST() {}
        virtual void print(int indent) {
            for (int i = 0; i < indent; ++i)
                std::cout << "  ";
            std::cout << "BaseAST()" << std::endl;
        }
    };
    class ASTs : public BaseAST  {
        public:
        std::vector<BaseAST*> stmts;
        ASTs(){}

        inline void insert(BaseAST* p) {
            if (p)
                this->stmts.push_back(p);
        }

        void print(int indent) {
            for (int i = 0; i < indent; ++i)
                std::cout << "  ";
            std::cout << "ASTs()" << std::endl;
            for (auto p : this->stmts) 
                p->print(indent + 1);
        }
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
    };

    class Program : public BaseAST {
        public:
        std::vector<GlobalStatement*> stmts;
        Program() {stmts.clear();}

        void insert(GlobalStatement *s) {
            if (s)
                this->stmts.push_back(s);
        }

        void print(void) {
            std::cout << "Program AST:" << std::endl;
            for (auto p : this->stmts) {
                p->print(1);
            }
        }
    };

    class TypeDecl : public BaseAST  {
        public:
        std::string baseType;
        std::string arrayT;

        TypeDecl(std::string t, std::string i) {
            this->baseType = t;
            this->arrayT = i;
        }

        void print(int indent) {
            for (int i = 0; i < indent; ++i)
                std::cout << "  ";
            std::cout << "TypeDecl(" << this->baseType << ',' 
                << this->arrayT << ')' << std::endl;
        }
    };

    class EvalExpr : public Expr {
        public:
        bool isVal;
        Value* val;
        std::string op;
        EvalExpr *l, *r;

        EvalExpr(Value *v) {
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
    };

    class FuncCall : public BaseAST {
        public:
        std::vector<EvalExpr*> pars;

        FuncCall() {
            pars.clear();
        }

        void print(int indent) {
            for (int i = 0; i < indent; ++i)
                std::cout << "  ";
            std::cout << "FuncCall()" << std::endl;
            for (auto p : this->pars) 
                p->print(indent + 1);
        }
    };

    class Value : public BaseAST {
        public:
        bool isConst;

        std::string constVal;
        
        std::string refName;
        FuncCall *call;
        EvalExpr *array;

        Value(std::string v) {
            this->isConst = true;
            this->constVal = v;
        }

        Value(std::string n, FuncCall *c, EvalExpr *a) {
            this->isConst = false;
            this->refName = n;
            this->call = c;
            this->array = a;
        }

        void print(int indent) {
            for (int i = 0; i < indent; ++i)
                std::cout << "  ";
            if (this->isConst)
                std::cout << "ConstVal(" << this->constVal << ')' << std::endl;
            else {
                std::cout << "Value(" << this->refName << ')' << std::endl;
                if (this->call)
                    this->call->print(indent + 1);
                if (this->array)
                    this->array->print(indent + 1);
            }
        }
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

        void print(int indent) {
            for (int i = 0; i < indent; ++i)
                std::cout << "  ";
            if (this->valid)
                std::cout << "GenericDecl(" << this->name << ')' << std::endl;
            else
                std::cout << "GenericDecl()" << std::endl;
            
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

        void print(int indent) {
            for (int i = 0; i < indent; ++i)
                std::cout << "  ";
            std::cout << "Param(" << this->name << ')' << std::endl;

            if (this->type)
                this->type->print(indent + 1);
        }
    };

    class Option : public BaseAST  {
        public:
        std::string name;
        std::vector<Param*> pars;

        Option(std::string n) {
            this->name = n;
            this->pars.clear();
        }

        void print(int indent) {
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
    };

    class RetExpr : public Expr {
        public:
        EvalExpr *stmt;

        RetExpr(EvalExpr* s) {
            this->exprType = RET;

            this->stmt = s;
        }

        void print(int indent) {
            for (int i = 0; i < indent; ++i)
                std::cout << "  ";
            std::cout << "RetExpr()" << std::endl;
            this->stmt->print(indent + 1);
        }
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

        void print(int indent) {
            for (int i = 0; i < indent; ++i)
                std::cout << "  ";
            std::cout << "VarDecl(" << this->name << ')' << std::endl;
            if (this->type)
                this->type->print(indent + 1);
            if (this->init)
                this->init->print(indent + 1);
        }
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

        void print(int indent) {
            for (int i = 0; i < indent; ++i)
                std::cout << "  ";
            std::cout << "ConstDecl(" << this->name << ')' << std::endl;
            if (this->type)
                this->type->print(indent + 1);
            if (this->init)
                this->init->print(indent + 1);
        }
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

        void print(int indent) {
            for (int i = 0; i < indent; ++i)
                std::cout << "  ";
            std::cout << "FuncDecl(" << this->name << ')' << std::endl;
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

        void print(int indent) {
            for (int i = 0; i < indent; ++i)
                std::cout << "  ";
            std::cout << "ClassDecl(" << this->name << ')' << std::endl;
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

        void print(int indent) {
            for (int i = 0; i < indent; ++i)
                std::cout << "  ";
            std::cout << "EnumDecl(" << this->name << ')' << std::endl;
            for (int i = 0; i < indent + 1; ++i)
                    std::cout << "  ";
                std::cout << "Options(" << std::endl;
            for (auto p : this->options) {
                p->print(indent + 2);
            }
            for (int i = 0; i < indent + 1; ++i)
                    std::cout << "  ";
            std::cout << ")" << std::endl;
        }
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

        void print(int indent) {
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

        void print(int indent) {
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

        void print(int indent) {
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

        void print(int indent) {
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

        void print(int indent) {
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
    };

    extern Program generate(Node<Lexical> *root);
}
