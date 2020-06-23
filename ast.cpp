#include "ast.hpp"
#include "lexical.hpp"
#include "tree.hpp"

#include <iostream>
#include <string>

using namespace AST;

BaseAST* recursive_gen(Node<Lexical> *curr) {
    /*
        std::cout << (unsigned long) curr << curr->t.name << std::endl;
    */
    if (curr->t.name == "<EPS>") {
      return nullptr;
    }
    if (curr->t.name == "<STATEMENTS>") {
        Program* prog = new AST::Program();
        prog->insert(dynamic_cast<GlobalStatement *>(
            recursive_gen(&curr->child[0])));
        while (curr->child.size() > 1) {
            curr = &curr->child[1];
            prog->insert(dynamic_cast<GlobalStatement *>(
                recursive_gen(&curr->child[0])));
        }
        return prog;
    }

    if (curr->t.name == "<CLASS_DOMAIN>") {
        ASTs *parent = new ASTs();
        parent->insert(recursive_gen(&curr->child[0]));
        while (curr->child.size() > 1) {
            curr = &curr->child[1];
            parent->insert(recursive_gen(&curr->child[0]));
        }
        return parent;
    }

    if (curr->t.name == "<STATEMENT>") {
        return recursive_gen(&curr->child[0]);
    }

    if (curr->t.name == "<CONSTDEF>") {
        EvalExpr *init = dynamic_cast<EvalExpr *>(
            recursive_gen(&curr->child[3]));
        ConstDecl* parent = new ConstDecl(curr->child[1].t.data, init);

        return parent;
    }

    if (curr->t.name == "<VARDEF>") {
        TypeDecl *type = dynamic_cast<TypeDecl *>(
            recursive_gen(&curr->child[3]));
        Expr *init = dynamic_cast<Expr *>(
            recursive_gen(&curr->child[4]));
        if (! init) {
            return new VarDecl(curr->child[1].t.data, type, nullptr);
        } else {
            return new VarDecl(
                curr->child[1].t.data, type, dynamic_cast<EvalExpr *>(init));
        }
    }

    if (curr->t.name == "<TYPENAME>") {
        std::string name;
        if (curr->child[0].t.name == "NAME") {
            name = curr->child[0].t.data;
        } else {
            name = curr->child[0].t.name;
        }

        if (curr->child[1].child[0].t.name == "<EPS>") {
            return new TypeDecl(name, "0");
        } else {
            return new TypeDecl(name, curr->child[1].child[1].t.data);
        }
    }

    if (curr->t.name == "<OPTIONAL_INIT>") {
        if (curr->child[0].t.name == "<EPS>")
            return new Expr(); // empty statement
        else {
            return recursive_gen(&curr->child[1]);
        }
    }

    if (curr->t.name == "<FUNCDEF>") {
        GenericDecl *gen = dynamic_cast<GenericDecl *>(
            recursive_gen(&curr->child[2]));
        TypeDecl *ret = dynamic_cast<TypeDecl *>(
            recursive_gen(&curr->child[6]));
        FuncDecl *parent = new FuncDecl(curr->child[1].t.data, gen, ret);

        ASTs *params = dynamic_cast<ASTs *>(
            recursive_gen(&curr->child[4]));
        ASTs *exprs  = dynamic_cast<ASTs *>(
            recursive_gen(&curr->child[8]));

        for (auto p : params->stmts) {
            parent->pars.push_back(dynamic_cast<Param *>(p));
        }
        
        for (auto p : exprs->stmts) {
            parent->exprs.push_back(dynamic_cast<Expr *>(p));
        }

        return parent;
    }

    if (curr->t.name == "<OPTIONAL_GENERIC_DEF>") {
        if (curr->child[0].t.name == "<EPS>") {
            return new GenericDecl();
        } else {
            return new GenericDecl(curr->child[1].t.data);
        }
    }

    if (curr->t.name == "<OPTIONAL_PARAMS_DEF>") {
        if (curr->child[0].t.name == "<EPS>") {
            return new ASTs();
        } else {
            return recursive_gen(&curr->child[1]);
        }
    }

    if (curr->t.name == "<PARAMS_DEF>") {
        ASTs *params = new ASTs();
        if (curr->child[0].t.name == "<EPS>") {
          return params;
        }

        params->stmts.push_back(recursive_gen(&curr->child[0]));
        curr = &curr->child[1];
        while (curr->child[0].t.name != "<EPS>") {
            params->stmts.push_back(recursive_gen(&curr->child[1]));
            curr = &curr->child[2];
        }
        return params;
    }

    if (curr->t.name == "<PARAM>") {
        TypeDecl *type = dynamic_cast<TypeDecl *>(
            recursive_gen(&curr->child[2]));
        Param *param = new Param(curr->child[0].t.data, type);;
        return param;
    }

    if (curr->t.name == "<RETURN_DEF>") {
        if (curr->child[0].t.name == "<EPS>") {
            return new TypeDecl("void", "0");
        } else {
            return recursive_gen(&curr->child[1]);
        }
    }

    if (curr->t.name == "<ENUMDEF>") {
        ASTs *options = dynamic_cast<ASTs *>(recursive_gen(&curr->child[3]));
        EnumDecl *parent = new EnumDecl(curr->child[1].t.data);
        
        for (auto p : options->stmts) {
            parent->options.push_back(dynamic_cast<Option *>(p));
        }

        return parent;
    }

    if (curr->t.name == "<OPTIONS>") {
        ASTs *options = new ASTs();
        if (curr->child[0].t.name == "<EPS>") {
            return options;
        }
        Option *option = new Option(curr->child[0].t.data);
        ASTs *pars = dynamic_cast<ASTs *>(recursive_gen(&curr->child[1]));
        for (auto p : pars->stmts) {
            option->pars.push_back(dynamic_cast<Param *>(p));
        }
        options->stmts.push_back(option);

        curr = &curr->child[2];
        
        while (curr->child[0].t.name != "<EPS>") {
            Option *option = new Option(curr->child[1].t.data);
            ASTs *pars = dynamic_cast<ASTs *>(recursive_gen(&curr->child[2]));
            for (auto p : pars->stmts) {
                option->pars.push_back(dynamic_cast<Param *>(p));
            }
            options->stmts.push_back(option);
            curr = &curr->child[3];
        }
        return options;
    }

    if (curr->t.name == "<MATCH_OPTION>") {
        ASTs *options = new ASTs();
        if (curr->child[0].t.name == "<EPS>") {
            return options;
        }
        curr = &curr->child[1];
        options->insert(new Param(curr->child[0].t.data, nullptr));

        curr = &curr->child[1];
        while (curr->child[0].t.name != "<EPS>") {
            options->insert(new Param(curr->child[1].t.data, nullptr));
            curr = &curr->child[2];
        }

        return options;
    }

    if (curr->t.name == "<CLASSDEF>") {
        GenericDecl *g = dynamic_cast<GenericDecl *>(
            recursive_gen(&curr->child[2]));
        ASTs *exprs = dynamic_cast<ASTs *>(recursive_gen(&curr->child[4]));
        ClassDecl *parent = new ClassDecl(curr->child[1].t.data, g);
        
        for (auto p : exprs->stmts) {
            parent->stmts.push_back(dynamic_cast<GlobalStatement *>(p));
        }
        return parent;
    }

    if (curr->t.name == "<EXPRS>") {
        ASTs *exprs = new ASTs();
        BaseAST *expr = recursive_gen(&curr->child[0]);
        if (expr)
            exprs->stmts.push_back(expr);
        while (curr->child.size() > 1) {
            curr = &curr->child[1];
            expr = recursive_gen(&curr->child[0]);
            if (expr)
                exprs->stmts.push_back(expr);
        }
        return exprs;
    }

    if (curr->t.name == "<EXPR>")
        return recursive_gen(&curr->child[0]);

    if (curr->t.name == "<EMPTY_EXPR>")
        return new Expr();

    if (curr->t.name == "<RET_EXPR>") {
        return new RetExpr(dynamic_cast<EvalExpr *>(
            recursive_gen(&curr->child[1])
        ));
    }

    if (curr->t.name == "<IF_EXPR>") {
        IfExpr *parent = new IfExpr(dynamic_cast<EvalExpr *>(
            recursive_gen(&curr->child[2])));

        ASTs *iftrue = dynamic_cast<ASTs *>(recursive_gen(&curr->child[5]));
        ASTs *iffalse = dynamic_cast<ASTs *>(recursive_gen(&curr->child[7]));
        
        for (auto p : iftrue->stmts) {
            parent->iftrue.push_back(dynamic_cast<Expr *>(p));
        }

        for (auto p : iffalse->stmts) {
            parent->iffalse.push_back(dynamic_cast<Expr *>(p));
        }

        return parent;
    }

    if (curr->t.name == "<OPTIONAL_ELSE_EXPR>") {
        if (curr->child[0].t.name == "<EPS>")
            return new ASTs();
        else {
            return recursive_gen(&curr->child[2]);
        }
    }

    if (curr->t.name == "<WHILE_EXPR>") {
        WhileExpr *parent = new WhileExpr(dynamic_cast<EvalExpr *>(
            recursive_gen(&curr->child[2])));
        ASTs *exprs = dynamic_cast<ASTs *>(recursive_gen(&curr->child[5]));

        for (auto p : exprs->stmts) {
            parent->exprs.push_back(dynamic_cast<Expr *>(p));
        }
        return parent;
    }

    if (curr->t.name == "<FOR_EXPR>") {
        ForExpr *parent = new ForExpr(
            dynamic_cast<EvalExpr *>(recursive_gen(&curr->child[2])),
            dynamic_cast<EvalExpr *>(recursive_gen(&curr->child[4])),
            dynamic_cast<EvalExpr *>(recursive_gen(&curr->child[6]))
        );

        ASTs *exprs = dynamic_cast<ASTs *>(recursive_gen(&curr->child[9]));
        for (auto p : exprs->stmts) {
            parent->exprs.push_back(dynamic_cast<Expr *>(p));
        }

        return parent;
    }

    if (curr->t.name == "<MATCH_EXPR>") {
        MatchExpr *match = new MatchExpr(dynamic_cast<EvalExpr *>(
            recursive_gen(&curr->child[2])));

        ASTs *exprs = dynamic_cast<ASTs *>(recursive_gen(&curr->child[5]));
        for (auto p : exprs->stmts) {
            match->lines.push_back(dynamic_cast<MatchLine *>(p));
        }

        return match;
    }

    if (curr->t.name == "<MATCH_LINES>") {
        ASTs *exprs = new ASTs();
        while (curr->child.size() > 1) {
            MatchLine *line = new MatchLine(curr->child[0].t.data);
            ASTs *pars = dynamic_cast<ASTs *>(recursive_gen(&curr->child[1]));
            ASTs *expr = dynamic_cast<ASTs *>(recursive_gen(&curr->child[4]));
            
            for (auto p : pars->stmts) {
                line->pars.push_back(dynamic_cast<Param *>(p));
            }
            for (auto p : expr->stmts) {
                line->exprs.push_back(dynamic_cast<Expr *>(p));
            }

            curr = &curr->child[6];
            exprs->insert(line);
        }
        return exprs;
    }

    if (curr->t.name == "<EVAL_EXPR>") {
        BaseAST *opr1 = recursive_gen(&curr->child[0]);
        if (!opr1) {
            std::cerr << "ast.cpp: missing left operation, terminating" << std::endl;
            exit(-1);
        }
        BaseAST *opr2 = recursive_gen(&curr->child[1]);
        if (!opr2) {
            return opr1;
        } else {
            return new EvalExpr(
                "assign",
                dynamic_cast<EvalExpr *>(opr1),
                dynamic_cast<EvalExpr *>(opr2)
            );
        }
    }

    if (curr->t.name == "<ASSIGN>") {
        if (curr->child[0].t.name == "<EPS>")
            return nullptr;
        else {
            BaseAST *opr1 = recursive_gen(&curr->child[1]);
            BaseAST *opr2 = recursive_gen(&curr->child[2]);
            if (! opr2) {
                return opr1;
            } else {
                return new EvalExpr(
                    "assign",
                    dynamic_cast<EvalExpr *>(opr1),
                    dynamic_cast<EvalExpr *>(opr2)
                );
            }
        }
    }

    if (curr->t.name == "<LOGICAL_OR>") {
        BaseAST *opr1 = recursive_gen(&curr->child[0]);
        if (!opr1) {
            std::cerr << "ast.cpp: missing left operation, terminating" << std::endl;
            exit(-1);
        }
        BaseAST *opr2 = recursive_gen(&curr->child[1]);
        if (!opr2) {
            return opr1;
        } else {
            return new EvalExpr(
                "LOR",
                dynamic_cast<EvalExpr *>(opr1),
                dynamic_cast<EvalExpr *>(opr2)
            );
        }
    }

    if (curr->t.name == "<LOR>") {
        if (curr->child[0].t.name == "<EPS>")
            return nullptr;
        else {
            BaseAST *opr1 = recursive_gen(&curr->child[1]);
            BaseAST *opr2 = recursive_gen(&curr->child[2]);
            if (! opr2) {
                return opr1;
            } else {
                return new EvalExpr(
                    "LOR",
                    dynamic_cast<EvalExpr *>(opr1),
                    dynamic_cast<EvalExpr *>(opr2)
                );
            }
        }
    }

    if (curr->t.name == "<LOGICAL_AND>") {
        BaseAST *opr1 = recursive_gen(&curr->child[0]);
        if (!opr1) {
            std::cerr << "ast.cpp: missing left operation, terminating" << std::endl;
            exit(-1);
        }
        BaseAST *opr2 = recursive_gen(&curr->child[1]);
        if (!opr2) {
            return opr1;
        } else {
            return new EvalExpr(
                "LAND",
                dynamic_cast<EvalExpr *>(opr1),
                dynamic_cast<EvalExpr *>(opr2)
            );
        }
    }

    if (curr->t.name == "<LAND>") {
        if (curr->child[0].t.name == "<EPS>")
            return nullptr;
        else {
            BaseAST *opr1 = recursive_gen(&curr->child[1]);
            BaseAST *opr2 = recursive_gen(&curr->child[2]);
            if (! opr2) {
                return opr1;
            } else {
                return new EvalExpr(
                    "LAND",
                    dynamic_cast<EvalExpr *>(opr1),
                    dynamic_cast<EvalExpr *>(opr2)
                );
            }
        }
    }

    if (curr->t.name == "<BITWISE_OR>") {
        BaseAST *opr1 = recursive_gen(&curr->child[0]);
        if (!opr1) {
            std::cerr << "ast.cpp: missing left operation, terminating" << std::endl;
            exit(-1);
        }
        BaseAST *opr2 = recursive_gen(&curr->child[1]);
        if (!opr2) {
            return opr1;
        } else {
            return new EvalExpr(
                "BOR",
                dynamic_cast<EvalExpr *>(opr1),
                dynamic_cast<EvalExpr *>(opr2)
            );
        }
    }

    if (curr->t.name == "<BOR>") {
        if (curr->child[0].t.name == "<EPS>")
            return nullptr;
        else {
            BaseAST *opr1 = recursive_gen(&curr->child[1]);
            BaseAST *opr2 = recursive_gen(&curr->child[2]);
            if (! opr2) {
                return opr1;
            } else {
                return new EvalExpr(
                    "BOR",
                    dynamic_cast<EvalExpr *>(opr1),
                    dynamic_cast<EvalExpr *>(opr2)
                );
            }
        }
    }

    if (curr->t.name == "<BITWISE_XOR>") {
        BaseAST *opr1 = recursive_gen(&curr->child[0]);
        if (!opr1) {
            std::cerr << "ast.cpp: missing left operation, terminating" << std::endl;
            exit(-1);
        }
        BaseAST *opr2 = recursive_gen(&curr->child[1]);
        if (!opr2) {
            return opr1;
        } else {
            return new EvalExpr(
                "BXOR",
                dynamic_cast<EvalExpr *>(opr1),
                dynamic_cast<EvalExpr *>(opr2)
            );
        }
    }

    if (curr->t.name == "<BXOR>") {
        if (curr->child[0].t.name == "<EPS>")
            return nullptr;
        else {
            BaseAST *opr1 = recursive_gen(&curr->child[1]);
            BaseAST *opr2 = recursive_gen(&curr->child[2]);
            if (! opr2) {
                return opr1;
            } else {
                return new EvalExpr(
                    "BXOR",
                    dynamic_cast<EvalExpr *>(opr1),
                    dynamic_cast<EvalExpr *>(opr2)
                );
            }
        }
    }

    if (curr->t.name == "<BITWISE_AND>") {
        BaseAST *opr1 = recursive_gen(&curr->child[0]);
        if (!opr1) {
            std::cerr << "ast.cpp: missing left operation, terminating" << std::endl;
            exit(-1);
        }
        BaseAST *opr2 = recursive_gen(&curr->child[1]);
        if (!opr2) {
            return opr1;
        } else {
            return new EvalExpr(
                "BAND",
                dynamic_cast<EvalExpr *>(opr1),
                dynamic_cast<EvalExpr *>(opr2)
            );
        }
    }

    if (curr->t.name == "<BAND>") {
        if (curr->child[0].t.name == "<EPS>")
            return nullptr;
        else {
            BaseAST *opr1 = recursive_gen(&curr->child[1]);
            BaseAST *opr2 = recursive_gen(&curr->child[2]);
            if (! opr2) {
                return opr1;
            } else {
                return new EvalExpr(
                    "BAND",
                    dynamic_cast<EvalExpr *>(opr1),
                    dynamic_cast<EvalExpr *>(opr2)
                );
            }
        }
    }

    if (curr->t.name == "<EQ_NEQ>" || curr->t.name == "<LGTE>" || curr->t.name == "<ADD_SUB>" || curr->t.name == "<MUL_DIV>" || curr->t.name == "<CDOT>") {
        BaseAST *opr1 = recursive_gen(&curr->child[0]);
        if (!opr1) {
            std::cerr << "ast.cpp: missing left operation, terminating" << std::endl;
            exit(-1);
        }
        BaseAST *opr2 = recursive_gen(&curr->child[1]);
        if (!opr2) {
            return opr1;
        } else {
            EvalExpr *opr2_ = dynamic_cast<EvalExpr *>(opr2);
            return new EvalExpr(
                opr2_->op,
                dynamic_cast<EvalExpr *>(opr1),
                opr2_->l
            );
        }
    }

    if (curr->t.name == "<EQ_NEQ_>" || curr->t.name == "<LGTE_>" || curr->t.name == "<ADD_SUB_>" || curr->t.name == "<MUL_DIV_>" || curr->t.name == "<CDOT_>") {
        if (curr->child[0].t.name == "<EPS>")
            return nullptr;
        else {
            EvalExpr *opr1 = dynamic_cast<EvalExpr *>(
                recursive_gen(&curr->child[1]));
            BaseAST *opr2 = recursive_gen(&curr->child[2]);
            if (!opr2)
                return new EvalExpr(
                    curr->child[0].t.name,
                    opr1,
                    nullptr
                );
            else {
                EvalExpr *opr2_ = dynamic_cast<EvalExpr *>(
                    opr2);
                EvalExpr *rec = new EvalExpr(
                    opr2_->op,
                    opr1,
                    opr2_->l
                );
                return new EvalExpr(
                    "recursive",
                    rec,
                    nullptr
                );
            }
        }
    }

    if (curr->t.name == "<PARS>") {
        if (curr->child[0].t.name == "NAME") {
            FuncCall *fc = dynamic_cast<FuncCall *>(
                recursive_gen(&curr->child[1]));
            EvalExpr *arr = dynamic_cast<EvalExpr *>(
                recursive_gen(&curr->child[2]));
            return new EvalExpr(new Value(curr->child[0].t.data, fc, arr));
        }
        if (curr->child[0].t.name == "LPAR") {
            return recursive_gen(&curr->child[1]);
        }

        return new EvalExpr(new Value(curr->child[0].t.data));
    }

    if (curr->t.name == "<OPTIONAL_FUNCCALL>") {
        if (curr->child[0].t.name == "<EPS>") {
            return nullptr;
        } else {
            FuncCall *fc = new FuncCall();
            while (curr->child.size() > 1) {
                fc->pars.push_back(dynamic_cast<EvalExpr *>(
                    recursive_gen(&curr->child[1])));
                curr = &curr->child[2];
            }
            return fc;
        }
    }

    if (curr->t.name == "<OPTIONAL_ARRAY>") {
        if (curr->child[0].t.name == "<EPS>") {
            return nullptr;
        } else {
            return recursive_gen(&curr->child[1]);
        }
    }

    std::cerr << curr->t.name << " is not defined" << std::endl;
    return nullptr;
}

Program AST::generate(Node<Lexical> *root) {
    return *dynamic_cast<Program *>(recursive_gen(root));
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
