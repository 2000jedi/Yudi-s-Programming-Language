#include "lexical.hpp"
#include "tree.hpp"
#include "util.hpp"
#include "ast.hpp"

#include <vector>
using namespace AST;

extern std::vector<std::string> *strings;

BaseAST* build_ast(Node<Lexical> *curr, ClassDecl *ParentClass) {
    if (curr->t.name == "<EPS>") {
      return nullptr;
    }
    if (curr->t.name == "<STATEMENTS>") {
        Program* prog = new AST::Program();
        prog->insert(dynamic_cast<GlobalStatement *>(
            build_ast(&curr->child[0], nullptr)));
        while (curr->child.size() > 1) {
            curr = &curr->child[1];
            prog->insert(dynamic_cast<GlobalStatement *>(
                build_ast(&curr->child[0], nullptr)));
        }
        return prog;
    }

    if (curr->t.name == "<CLASS_DOMAIN>") {
        ASTs *parent = new ASTs();
        parent->insert(build_ast(&curr->child[0], ParentClass));
        while (curr->child.size() > 1) {
            curr = &curr->child[1];
            parent->insert(build_ast(&curr->child[0], ParentClass));
        }
        return parent;
    }

    if (curr->t.name == "<STATEMENT>") {
        return build_ast(&curr->child[0], ParentClass);
    }

    if (curr->t.name == "<CONSTDEF>") {
        EvalExpr *init = dynamic_cast<EvalExpr *>(
            build_ast(&curr->child[3], nullptr));
        VarDecl* parent = new VarDecl(
            curr->child[1].t.data, nullptr, init, ParentClass);
        parent->is_const = true;

        return parent;
    }

    if (curr->t.name == "<GLOBAL_VARDEF>") {
        auto res = dynamic_cast<VarDecl *>(
            build_ast(&curr->child[0], nullptr));
        if (! res) {
            return nullptr;
        } else {
            res->is_global = true;
            return res;
        }
    }

    if (curr->t.name == "<VARDEF>") {
        TypeDecl *type = dynamic_cast<TypeDecl *>(
            build_ast(&curr->child[3], nullptr));
        Expr *init = dynamic_cast<Expr *>(
            build_ast(&curr->child[4], nullptr));
        if (! init) {
            return new VarDecl(
                curr->child[1].t.data, type, nullptr, ParentClass);
        } else {
            return new VarDecl(
                curr->child[1].t.data, type, 
                dynamic_cast<EvalExpr *>(init), ParentClass);
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
            return build_ast(&curr->child[1], nullptr);
        }
    }

    if (curr->t.name == "<FUNCDEF>") {
        GenericDecl *gen = dynamic_cast<GenericDecl *>(
            build_ast(&curr->child[2], nullptr));
        TypeDecl *ret = dynamic_cast<TypeDecl *>(
            build_ast(&curr->child[6], nullptr));
        FuncDecl *parent = new FuncDecl(
            curr->child[1].t.data, gen, ret, ParentClass);

        ASTs *params = dynamic_cast<ASTs *>(
            build_ast(&curr->child[4], nullptr));
        ASTs *exprs  = dynamic_cast<ASTs *>(
            build_ast(&curr->child[8], nullptr));

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
            return build_ast(&curr->child[1], nullptr);
        }
    }

    if (curr->t.name == "<PARAMS_DEF>") {
        ASTs *params = new ASTs();
        if (curr->child[0].t.name == "<EPS>") {
          return params;
        }

        params->stmts.push_back(build_ast(&curr->child[0], nullptr));
        curr = &curr->child[1];
        while (curr->child[0].t.name != "<EPS>") {
            params->stmts.push_back(build_ast(&curr->child[1], nullptr));
            curr = &curr->child[2];
        }
        return params;
    }

    if (curr->t.name == "<PARAM>") {
        TypeDecl *type = dynamic_cast<TypeDecl *>(
            build_ast(&curr->child[2], nullptr));
        Param *param = new Param(curr->child[0].t.data, type);;
        return param;
    }

    if (curr->t.name == "<RETURN_DEF>") {
        if (curr->child[0].t.name == "<EPS>") {
            return new TypeDecl(TypeDecl::VOID, "0");
        } else {
            return build_ast(&curr->child[1], nullptr);
        }
    }

    if (curr->t.name == "<ENUMDEF>") {
        ASTs *options = dynamic_cast<ASTs *>(
            build_ast(&curr->child[3], nullptr));
        EnumDecl *parent = new EnumDecl(curr->child[1].t.data, ParentClass);
        
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
        ASTs *pars = dynamic_cast<ASTs *>(
            build_ast(&curr->child[1], nullptr));
        for (auto p : pars->stmts) {
            option->pars.push_back(dynamic_cast<Param *>(p));
        }
        options->stmts.push_back(option);

        curr = &curr->child[2];
        
        while (curr->child[0].t.name != "<EPS>") {
            Option *option = new Option(curr->child[1].t.data);
            ASTs *pars = dynamic_cast<ASTs *>(
                build_ast(&curr->child[2], nullptr));
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
            build_ast(&curr->child[2], nullptr));
        ClassDecl *parent = new ClassDecl(curr->child[1].t.data, g);
        ASTs *exprs = dynamic_cast<ASTs *>(
            build_ast(&curr->child[4], parent));
        
        for (auto p : exprs->stmts) {
            parent->stmts.push_back(dynamic_cast<GlobalStatement *>(p));
        }

        return parent;
    }

    if (curr->t.name == "<EXPRS>") {
        ASTs *exprs = new ASTs();
        BaseAST *expr = build_ast(&curr->child[0], nullptr);
        if (expr)
            exprs->stmts.push_back(expr);
        while (curr->child.size() > 1) {
            curr = &curr->child[1];
            expr = build_ast(&curr->child[0], nullptr);
            if (expr)
                exprs->stmts.push_back(expr);
        }
        return exprs;
    }

    if (curr->t.name == "<EXPR>")
        return build_ast(&curr->child[0], nullptr);

    if (curr->t.name == "<EMPTY_EXPR>")
        return new Expr();

    if (curr->t.name == "<RET_EXPR>") {
        return new RetExpr(dynamic_cast<EvalExpr *>(
            build_ast(&curr->child[1], nullptr)
        ));
    }

    if (curr->t.name == "<IF_EXPR>") {
        IfExpr *parent = new IfExpr(dynamic_cast<EvalExpr *>(
            build_ast(&curr->child[2], nullptr)));

        ASTs *iftrue = dynamic_cast<ASTs *>(
            build_ast(&curr->child[5], nullptr));
        ASTs *iffalse = dynamic_cast<ASTs *>(
            build_ast(&curr->child[7], nullptr));
        
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
            return build_ast(&curr->child[2], nullptr);
        }
    }

    if (curr->t.name == "<WHILE_EXPR>") {
        WhileExpr *parent = new WhileExpr(dynamic_cast<EvalExpr *>(
            build_ast(&curr->child[2], nullptr)));
        ASTs *exprs = dynamic_cast<ASTs *>(
            build_ast(&curr->child[5], nullptr));

        for (auto p : exprs->stmts) {
            parent->exprs.push_back(dynamic_cast<Expr *>(p));
        }
        return parent;
    }

    if (curr->t.name == "<FOR_EXPR>") {
        ForExpr *parent = new ForExpr(
            dynamic_cast<EvalExpr *>(build_ast(&curr->child[2], nullptr)),
            dynamic_cast<EvalExpr *>(build_ast(&curr->child[4], nullptr)),
            dynamic_cast<EvalExpr *>(build_ast(&curr->child[6], nullptr))
        );

        ASTs *exprs = dynamic_cast<ASTs *>(
            build_ast(&curr->child[9], nullptr));
        for (auto p : exprs->stmts) {
            parent->exprs.push_back(dynamic_cast<Expr *>(p));
        }

        return parent;
    }

    if (curr->t.name == "<MATCH_EXPR>") {
        MatchExpr *match = new MatchExpr(dynamic_cast<EvalExpr *>(
            build_ast(&curr->child[2], nullptr)));

        ASTs *exprs = dynamic_cast<ASTs *>(
            build_ast(&curr->child[5], nullptr));
        for (auto p : exprs->stmts) {
            match->lines.push_back(dynamic_cast<MatchLine *>(p));
        }

        return match;
    }

    if (curr->t.name == "<MATCH_LINES>") {
        ASTs *exprs = new ASTs();
        while (curr->child.size() > 1) {
            MatchLine *line = new MatchLine(curr->child[0].t.data);
            ASTs *pars = dynamic_cast<ASTs *>(
                build_ast(&curr->child[1], nullptr));
            ASTs *expr = dynamic_cast<ASTs *>(
                build_ast(&curr->child[4], nullptr));
            
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
        BaseAST *opr1 = build_ast(&curr->child[0], nullptr);
        if (!opr1) {
            std::cerr << "ast.cpp: missing left operation, terminating" << std::endl;
            exit(-1);
        }
        BaseAST *opr2 = build_ast(&curr->child[1], nullptr);
        if (!opr2) {
            return opr1;
        } else {
            return new EvalExpr(
                "=",
                dynamic_cast<EvalExpr *>(opr1),
                dynamic_cast<EvalExpr *>(opr2)
            );
        }
    }

    if (curr->t.name == "<ASSIGN>") {
        if (curr->child[0].t.name == "<EPS>")
            return nullptr;
        else {
            BaseAST *opr1 = build_ast(&curr->child[1], nullptr);
            BaseAST *opr2 = build_ast(&curr->child[2], nullptr);
            if (! opr2) {
                return opr1;
            } else {
                return new EvalExpr(
                    "=",
                    dynamic_cast<EvalExpr *>(opr1),
                    dynamic_cast<EvalExpr *>(opr2)
                );
            }
        }
    }

    if (curr->t.name == "<LOGICAL_OR>") {
        BaseAST *opr1 = build_ast(&curr->child[0], nullptr);
        if (!opr1) {
            std::cerr << "ast.cpp: missing left operation, terminating" << std::endl;
            exit(-1);
        }
        BaseAST *opr2 = build_ast(&curr->child[1], nullptr);
        if (!opr2) {
            return opr1;
        } else {
            return new EvalExpr(
                "||",
                dynamic_cast<EvalExpr *>(opr1),
                dynamic_cast<EvalExpr *>(opr2)
            );
        }
    }

    if (curr->t.name == "<LOR>") {
        if (curr->child[0].t.name == "<EPS>")
            return nullptr;
        else {
            BaseAST *opr1 = build_ast(&curr->child[1], nullptr);
            BaseAST *opr2 = build_ast(&curr->child[2], nullptr);
            if (! opr2) {
                return opr1;
            } else {
                return new EvalExpr(
                    "||",
                    dynamic_cast<EvalExpr *>(opr1),
                    dynamic_cast<EvalExpr *>(opr2)
                );
            }
        }
    }

    if (curr->t.name == "<LOGICAL_AND>") {
        BaseAST *opr1 = build_ast(&curr->child[0], nullptr);
        if (!opr1) {
            std::cerr << "ast.cpp: missing left operation, terminating" << std::endl;
            exit(-1);
        }
        BaseAST *opr2 = build_ast(&curr->child[1], nullptr);
        if (!opr2) {
            return opr1;
        } else {
            return new EvalExpr(
                "&&",
                dynamic_cast<EvalExpr *>(opr1),
                dynamic_cast<EvalExpr *>(opr2)
            );
        }
    }

    if (curr->t.name == "<LAND>") {
        if (curr->child[0].t.name == "<EPS>")
            return nullptr;
        else {
            BaseAST *opr1 = build_ast(&curr->child[1], nullptr);
            BaseAST *opr2 = build_ast(&curr->child[2], nullptr);
            if (! opr2) {
                return opr1;
            } else {
                return new EvalExpr(
                    "&&",
                    dynamic_cast<EvalExpr *>(opr1),
                    dynamic_cast<EvalExpr *>(opr2)
                );
            }
        }
    }

    if (curr->t.name == "<BITWISE_OR>") {
        BaseAST *opr1 = build_ast(&curr->child[0], nullptr);
        if (!opr1) {
            std::cerr << "ast.cpp: missing left operation, terminating" << std::endl;
            exit(-1);
        }
        BaseAST *opr2 = build_ast(&curr->child[1], nullptr);
        if (!opr2) {
            return opr1;
        } else {
            return new EvalExpr(
                "|",
                dynamic_cast<EvalExpr *>(opr1),
                dynamic_cast<EvalExpr *>(opr2)
            );
        }
    }

    if (curr->t.name == "<BOR>") {
        if (curr->child[0].t.name == "<EPS>")
            return nullptr;
        else {
            BaseAST *opr1 = build_ast(&curr->child[1], nullptr);
            BaseAST *opr2 = build_ast(&curr->child[2], nullptr);
            if (! opr2) {
                return opr1;
            } else {
                return new EvalExpr(
                    "|",
                    dynamic_cast<EvalExpr *>(opr1),
                    dynamic_cast<EvalExpr *>(opr2)
                );
            }
        }
    }

    if (curr->t.name == "<BITWISE_XOR>") {
        BaseAST *opr1 = build_ast(&curr->child[0], nullptr);
        if (!opr1) {
            std::cerr << "ast.cpp: missing left operation, terminating" << std::endl;
            exit(-1);
        }
        BaseAST *opr2 = build_ast(&curr->child[1], nullptr);
        if (!opr2) {
            return opr1;
        } else {
            return new EvalExpr(
                "^",
                dynamic_cast<EvalExpr *>(opr1),
                dynamic_cast<EvalExpr *>(opr2)
            );
        }
    }

    if (curr->t.name == "<BXOR>") {
        if (curr->child[0].t.name == "<EPS>")
            return nullptr;
        else {
            BaseAST *opr1 = build_ast(&curr->child[1], nullptr);
            BaseAST *opr2 = build_ast(&curr->child[2], nullptr);
            if (! opr2) {
                return opr1;
            } else {
                return new EvalExpr(
                    "^",
                    dynamic_cast<EvalExpr *>(opr1),
                    dynamic_cast<EvalExpr *>(opr2)
                );
            }
        }
    }

    if (curr->t.name == "<BITWISE_AND>") {
        BaseAST *opr1 = build_ast(&curr->child[0], nullptr);
        if (!opr1) {
            std::cerr << "ast.cpp: missing left operation, terminating" << std::endl;
            exit(-1);
        }
        BaseAST *opr2 = build_ast(&curr->child[1], nullptr);
        if (!opr2) {
            return opr1;
        } else {
            return new EvalExpr(
                "&",
                dynamic_cast<EvalExpr *>(opr1),
                dynamic_cast<EvalExpr *>(opr2)
            );
        }
    }

    if (curr->t.name == "<BAND>") {
        if (curr->child[0].t.name == "<EPS>")
            return nullptr;
        else {
            BaseAST *opr1 = build_ast(&curr->child[1], nullptr);
            BaseAST *opr2 = build_ast(&curr->child[2], nullptr);
            if (! opr2) {
                return opr1;
            } else {
                return new EvalExpr(
                    "&",
                    dynamic_cast<EvalExpr *>(opr1),
                    dynamic_cast<EvalExpr *>(opr2)
                );
            }
        }
    }

    if (curr->t.name == "<EQ_NEQ>" || curr->t.name == "<LGTE>" || curr->t.name == "<ADD_SUB>" || curr->t.name == "<MUL_DIV>" || curr->t.name == "<CDOT>") {
        BaseAST *opr1 = build_ast(&curr->child[0], nullptr);
        if (!opr1) {
            std::cerr << "ast.cpp: missing left operation, terminating" << std::endl;
            exit(-1);
        }
        BaseAST *opr2 = build_ast(&curr->child[1], nullptr);
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
                build_ast(&curr->child[1], nullptr));
            BaseAST *opr2 = build_ast(&curr->child[2], nullptr);
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
                build_ast(&curr->child[1], nullptr));
            EvalExpr *arr = dynamic_cast<EvalExpr *>(
                build_ast(&curr->child[2], nullptr));
            return new EvalExpr(new ExprVal(curr->child[0].t.data, fc, arr));
        }
        if (curr->child[0].t.name == "LPAR") {
            return build_ast(&curr->child[1], nullptr);
        }

        if (curr->child[0].t.name == "FLOAT")
            return new EvalExpr(new ExprVal(
                curr->child[0].t.data, new TypeDecl(TypeDecl::FP32, "0")));
        if (curr->child[0].t.name == "INT")
            return new EvalExpr(new ExprVal(
                curr->child[0].t.data, new TypeDecl(TypeDecl::INT32, "0")));
        if (curr->child[0].t.name == "CHAR")
            return new EvalExpr(new ExprVal(
                curr->child[0].t.data, new TypeDecl(TypeDecl::CHAR, "0")));
        if (curr->child[0].t.name == "STRING") {
            std::string str = unescape(curr->child[0].t.data);
            strings->push_back(str);
            return new EvalExpr(new ExprVal(str, 
                new TypeDecl(TypeDecl::STRING, "0")));
        }
    }

    if (curr->t.name == "<OPTIONAL_EVAL_EXPR>") {
        if (curr->child[0].t.name == "<EPS>") {
            return nullptr;
        } else {
            return build_ast(&curr->child[0], nullptr);
        }
    }

    if (curr->t.name == "<OPTIONAL_FUNCCALL>") {
        if (curr->child[0].t.name == "<EPS>") {
            return nullptr;
        } else {
            FuncCall *fc = new FuncCall();
            while (curr->child.size() > 1) {
                fc->pars.push_back(dynamic_cast<EvalExpr *>(
                    build_ast(&curr->child[1], nullptr)));
                curr = &curr->child[2];
            }
            return fc;
        }
    }

    if (curr->t.name == "<OPTIONAL_ARRAY>") {
        if (curr->child[0].t.name == "<EPS>") {
            return nullptr;
        } else {
            return build_ast(&curr->child[1], nullptr);
        }
    }

    std::cerr << curr->t.name << " is not defined" << std::endl;
    return nullptr;
}

Program AST::build_ast(Node<Lexical> *root) {
    return *dynamic_cast<Program *>(build_ast(root, nullptr));
}
