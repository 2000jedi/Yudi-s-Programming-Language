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
            return new EvalExpr(new ExprVal(curr->child[0].t.data, fc, arr));
        }
        if (curr->child[0].t.name == "LPAR") {
            return recursive_gen(&curr->child[1]);
        }

        if (curr->child[0].t.name == "FLOAT")
            return new EvalExpr(new ExprVal(
                curr->child[0].t.data, new TypeDecl("FLOAT", "0")));
        if (curr->child[0].t.name == "INT")
            return new EvalExpr(new ExprVal(
                curr->child[0].t.data, new TypeDecl("INT32", "0")));
        if (curr->child[0].t.name == "CHAR")
            return new EvalExpr(new ExprVal(
                curr->child[0].t.data, new TypeDecl("CHART", "0")));
        if (curr->child[0].t.name == "STRING")
            return new EvalExpr(new ExprVal(
                curr->child[0].t.data, new TypeDecl("STR", "0")));
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
    for (auto p : this->stmts) 
        p->print(indent + 1);
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
    for (auto p : this->pars) 
        p->print(indent + 1);
}

void ExprVal::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    if (this->isConst)
        std::cout << "ConstVal(" << this->constVal << ')' << std::endl;
    else {
        std::cout << "ExprVal(" << this->refName << ')' << std::endl;
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
    std::cout << "VarDecl(" << this->name << ')' << std::endl;
    if (this->type)
        this->type->print(indent + 1);
    if (this->init)
        this->init->print(indent + 1);
}

void ConstDecl::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "ConstDecl(" << this->name << ')' << std::endl;
    if (this->type)
        this->type->print(indent + 1);
    if (this->init)
        this->init->print(indent + 1);
}

void FuncDecl::print(int indent) {
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

void ClassDecl::print(int indent) {
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

void EnumDecl::print(int indent) {
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

/**
 * Code Generation Interface.
 * codeGen() methods to convert to llvm IR form.
 */

static llvm::LLVMContext TheContext;
static llvm::IRBuilder<> Builder(TheContext);
static std::unique_ptr<llvm::Module> TheModule;
static std::map<std::string, llvm::Value *> NamedValues;

/* Error Logging */
BaseAST *LogError(const char *Str) {
    std::cerr << "Error: " << Str << std::endl;
    return nullptr;
}

llvm::Value *LogErrorV(const char *Str) {
  LogError(Str);
  return nullptr;
}

/* Variable Allocate Helper Function */
static llvm::AllocaInst *CreateEntryBlockAlloca(
    llvm::Function *F, const std::string &name, llvm::Type *type) {
  llvm::IRBuilder<> tmpBuilder(
      &F->getEntryBlock(), F->getEntryBlock().begin());
  return tmpBuilder.CreateAlloca(type, 0, name.c_str());
}

/* Type Translate */
llvm::Type *type_trans(TypeDecl *td) {
    if (td->baseType == "FLOAT") {
        return llvm::Type::getDoubleTy(TheContext);
    }
    if (td->baseType == "INT32") {
        return llvm::Type::getInt32Ty(TheContext);
    }
    if (td->baseType == "CHART") {
        return llvm::Type::getInt8Ty(TheContext);
    }
    if (td->baseType == "STR") {
        return llvm::Type::getInt8PtrTy(TheContext);
    }
    if (td->baseType == "void") {
        return llvm::Type::getVoidTy(TheContext);
    }
    return nullptr;
    /* todo: array typing */
}

llvm::Value *BaseAST::codegen() {
    LogError("BaseAST() cannot generate IR code.");
    return nullptr;
}

llvm::Value *ASTs::codegen() {
    LogError("ASTs() cannot generate IR code.");
    return nullptr;
}

llvm::Value *ExprVal::codegen() {
    if (this->isConst) {
        if (this->type->baseType == "FLOAT") {
            return llvm::ConstantFP::get(
                TheContext, llvm::APFloat(std::stod(this->constVal)));
        }
        if (this->type->baseType == "INT32") {
            return llvm::ConstantInt::get(
                TheContext, llvm::APInt(32, std::stoi(this->constVal), true));
        }
        if (this->type->baseType == "CHART") {
            return llvm::ConstantInt::get(
                TheContext, llvm::APInt(8, std::stoi(this->constVal), false));
        }
        if (this->type->baseType == "STR") {
            return llvm::ConstantDataArray::getString(
                TheContext, this->constVal, true);
        }

        return LogErrorV("Constant met strange types");
    } else {
        llvm::Value* v = NamedValues[this->refName];
        if (!v)
            return LogErrorV(this->refName.c_str());
        /*
        TODO: function call and array reference
        */
       return NamedValues[this->refName];
    }
}

llvm::Value *EvalExpr::codegen() {
    if (this->isVal)
        return this->val->codegen();
    llvm::Value *lv = this->l->codegen();
    llvm::Value *rv = this->r->codegen();

    if (!lv || !rv)
        return LogErrorV(this->op.c_str());

    /* TODO: Type Inference */
    if (this->op == "ADD")
        return Builder.CreateAdd(lv, rv, "addtmp");
    if (this->op == "SUB")
        return Builder.CreateSub(lv, rv, "subtmp");
    if (this->op == "MUL")
        return Builder.CreateMul(lv, rv, "multmp");
    if (this->op == "DIV")
        return Builder.CreateSDiv(lv, rv, "divtmp");
    if (this->op == "ASSIGN") {
        Builder.CreateStore(rv, lv, false);
        return lv;
    }
    if (this->op == "GT")
        return Builder.CreateICmpSGT(lv, rv, "gttmp");
    if (this->op == "LT")
        return Builder.CreateICmpSLT(lv, rv, "lttmp");
    if (this->op == "GE")
        return Builder.CreateICmpSGE(lv, rv, "getmp");
    if (this->op == "LE")
        return Builder.CreateICmpSLE(lv, rv, "letmp");
    if (this->op == "EQ")
        return Builder.CreateICmpEQ(lv, rv, "eqtmp");
    if (this->op == "NEQ")
        return Builder.CreateICmpNE(lv, rv, "netmp");

    std::cerr << "Current Op: " << this->op << std::endl;
    return LogErrorV("Operator in Construction");
}

llvm::Value *FuncDecl::codegen() {
    llvm::Function *prev = TheModule->getFunction(this->name);
    if (prev) {
        return LogErrorV("Function already declared");
    }

    std::vector<llvm::Type *> ArgTypes;
    for (auto p : this->pars) {
        ArgTypes.push_back(type_trans(p->type));
    }
    llvm::FunctionType *Ft = llvm::FunctionType::get(
        type_trans(this->ret), ArgTypes, false);
    llvm::Function *F = llvm::Function::Create(
        Ft, llvm::Function::ExternalLinkage, this->name, TheModule.get());
    
    unsigned Idx = 0;
    for (auto &Arg : F->args())
        Arg.setName(this->pars[Idx++]->name);

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(
        TheContext, this->name + "_entry", F);
    Builder.SetInsertPoint(BB);

    NamedValues.clear();
    
    Idx = 0;
    for (auto &Arg : F->args()) {
        llvm::AllocaInst *alloca = CreateEntryBlockAlloca(
            F, Arg.getName(), type_trans(this->pars[Idx++]->type));
        Builder.CreateStore(&Arg, alloca);
        NamedValues[Arg.getName()] = alloca;
    }

    for (auto p : this->exprs) {
        p->codegen();
    }

    llvm::verifyFunction(*F);
    return F;
}

llvm::Value *ClassDecl::codegen() {
    return nullptr;
}

llvm::Value *ConstDecl::codegen() {
    return nullptr;
}

llvm::Value *EnumDecl::codegen() {
    return nullptr;
}

llvm::Value *ForExpr::codegen() {
    return nullptr;
}

llvm::Value *FuncCall::codegen() {
    return nullptr;
}

llvm::Value *GenericDecl::codegen() {
    return nullptr;
}

llvm::Value *IfExpr::codegen() {
    return nullptr;
}

llvm::Value *MatchExpr::codegen() {
    return nullptr;
}

llvm::Value *MatchLine::codegen() {
    return nullptr;
}

llvm::Value *Option::codegen() {
    return nullptr;
}

llvm::Value *Param::codegen() {
    return nullptr;
}

llvm::Value *RetExpr::codegen() {
    llvm::Value *r = this->stmt->codegen();

    if (r) {
        Builder.CreateRet(r);
    }

    return nullptr;
}

llvm::Value *TypeDecl::codegen() {
    return nullptr;
}

llvm::Value *VarDecl::codegen() {
    if (NamedValues.find(this->name) != NamedValues.end())
        return LogErrorV(this->name.c_str());

    llvm::Function *curF = Builder.GetInsertBlock()->getParent();
    llvm::AllocaInst *alloca = CreateEntryBlockAlloca(
        curF, this->name, type_trans(this->type));
    llvm::Value *v = nullptr;

    if (this->init) {
        v = this->init->codegen();
        Builder.CreateStore(v, alloca);
    }
    NamedValues[this->name] = alloca;

    return nullptr;
}

llvm::Value *WhileExpr::codegen() {
    return nullptr;
}

llvm::Value *Program::codegen() {
    for (auto p : this->stmts) {
        p->codegen();
    }
    return nullptr;
}

int AST::codegen(Program prog, std::string outFile) {
    TheModule = llvm::make_unique<llvm::Module>(outFile, TheContext);
    prog.codegen();
    TheModule->print(llvm::errs(), nullptr);
    return 0;
}