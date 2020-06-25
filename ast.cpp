#include "ast.hpp"
#include "lexical.hpp"
#include "tree.hpp"

#include <iostream>
#include <sstream>
#include <string>

using namespace AST;

static std::vector<std::string> strings;

std::string unescape(std::string raw) {
    raw = raw.substr(1, raw.size() - 2);

    std::ostringstream ss;

    unsigned int i = 0;
    while (i < raw.size()) {
        if (raw[i] != '\\') {
            ss << raw[i];
        } else {
            if ((i + 1) == raw.size()) {
                std::cerr << "Parser: invalid string literal" << std::endl;
                return raw;
            }

            switch (raw[i+1]) {
                case 'a': {
                    ss << '\a';
                    break;
                }
                case 'b': {
                    ss << '\b';
                    break;
                }
                case 't': {
                    ss << '\t';
                    break;
                }
                case 'n': {
                    ss << '\n';
                    break;
                }
                case 'v': {
                    ss << '\v';
                    break;
                }
                case 'f': {
                    ss << '\f';
                    break;
                }
                case 'r': {
                    ss << '\r';
                    break;
                }
                case '"': {
                    ss << '\"';
                    break;
                }
                case '\'': {
                    ss << '\'';
                    break;
                }
                case '\?': {
                    ss << '\?';
                    break;
                }
                case '\\': {
                    ss << '\\';
                    break;
                }
                default: {
                    std::cerr << "Parser: undefined escape string: \\" 
                        << raw[i+1] << std::endl;
                }
            }
            i++;
        }
        i++;
    }

    return ss.str();
}

BaseAST* recursive_gen(Node<Lexical> *curr) {
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
            return new TypeDecl(TypeDecl::VOID, "0");
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
            BaseAST *opr1 = recursive_gen(&curr->child[1]);
            BaseAST *opr2 = recursive_gen(&curr->child[2]);
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
            BaseAST *opr1 = recursive_gen(&curr->child[1]);
            BaseAST *opr2 = recursive_gen(&curr->child[2]);
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
            BaseAST *opr1 = recursive_gen(&curr->child[1]);
            BaseAST *opr2 = recursive_gen(&curr->child[2]);
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
            BaseAST *opr1 = recursive_gen(&curr->child[1]);
            BaseAST *opr2 = recursive_gen(&curr->child[2]);
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
            BaseAST *opr1 = recursive_gen(&curr->child[1]);
            BaseAST *opr2 = recursive_gen(&curr->child[2]);
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
            BaseAST *opr1 = recursive_gen(&curr->child[1]);
            BaseAST *opr2 = recursive_gen(&curr->child[2]);
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
                curr->child[0].t.data, new TypeDecl(TypeDecl::FP32, "0")));
        if (curr->child[0].t.name == "INT")
            return new EvalExpr(new ExprVal(
                curr->child[0].t.data, new TypeDecl(TypeDecl::INT32, "0")));
        if (curr->child[0].t.name == "CHAR")
            return new EvalExpr(new ExprVal(
                curr->child[0].t.data, new TypeDecl(TypeDecl::CHAR, "0")));
        if (curr->child[0].t.name == "STRING") {
            std::string str = unescape(curr->child[0].t.data);
            strings.push_back(str);
            return new EvalExpr(new ExprVal(str, 
                new TypeDecl(TypeDecl::STRING, "0")));
        }
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

static llvm::LLVMContext context;
static llvm::IRBuilder<> builder(context);
static std::unique_ptr<llvm::Module> module;
// static std::map<std::string, llvm::Value *> NamedValues;
static std::map<std::string, llvm::Value *> strLits;

/*
    Symble Table - record Variable and Type Information
    TODO: type inference
*/
class ValueType {
    public:
    llvm::Value *val;
    TypeDecl *type;

    ValueType(llvm::Value *v, TypeDecl *t) {
        this->val = v;
        this->type = t;
    }
};

static std::vector<std::unique_ptr<std::map<std::string, ValueType*>>> SymTable;

static void ClearSymLayer(void) {
    SymTable.clear();
}

static void NewSymLayer(void) {
    SymTable.push_back(std::make_unique<std::map<std::string, ValueType *>>());
}

static void RemoveSymLayer(void) {
    SymTable.pop_back();
}

static void InsertVar(std::string name, llvm::Value *v, TypeDecl *t) {
    (*SymTable.back())[name] = new ValueType(v, t);
}

static ValueType *FindVar(std::string name) {
    for (int i = SymTable.size() - 1; i >= 0; i--) {
        if (SymTable[i]->find(name) != SymTable[i]->end()) {
            return (*SymTable[i])[name];
        }
    }

    return nullptr;
}

static ValueType *FindTopVar(std::string name) {
    if (SymTable.back()->find(name) != SymTable.back()->end()) {
        return (*SymTable.back())[name];
    } else {
        return nullptr;
    }
}


/* Variable Allocate Helper Function */
static llvm::AllocaInst *CreateEntryBlockAlloca(
    llvm::Function *F, const std::string &name, llvm::Type *type, int size) {
    llvm::IRBuilder<> tmpBuilder(
        &F->getEntryBlock(), F->getEntryBlock().begin());

    llvm::Value *arr = 0;
    if (size)
        arr = llvm::ConstantInt::get(context, llvm::APInt(32, size, true));

    return tmpBuilder.CreateAlloca(type, arr, name.c_str());
}

/* Type Translate */
llvm::Type *type_trans(TypeDecl *td) {
    switch (td->baseType) {
        case TypeDecl::VOID:
            return llvm::Type::getVoidTy(context);
        case TypeDecl::INT32:
            return llvm::Type::getInt32Ty(context);
        case TypeDecl::CHAR:
            return llvm::Type::getInt8Ty(context);
        case TypeDecl::FP32:
            return llvm::Type::getFloatTy(context);
        case TypeDecl::FP64:
            return llvm::Type::getDoubleTy(context);
        case TypeDecl::STRING:
            return llvm::Type::getInt8PtrTy(context);
        default:
            LogError("TypeDecl index " << td->baseType << " not found");
            return nullptr;
    }
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

llvm::Value *ConstToValue(ExprVal *e) {
    switch (e->type->baseType) {
        case TypeDecl::VOID:
            LogError("no constant type \"void\"");
            return nullptr;
        case TypeDecl::INT32:
            return llvm::ConstantInt::get(
                context, llvm::APInt(32, std::stoi(e->constVal), true));
        case TypeDecl::CHAR:
            return llvm::ConstantInt::get(
                context, llvm::APInt(8, std::stoi(e->constVal), false));
        case TypeDecl::FP32:
        case TypeDecl::FP64:
            return llvm::ConstantFP::get(
                context, llvm::APFloat(std::stod(e->constVal)));
        case TypeDecl::STRING: {
            return strLits[e->constVal];
        }
        default:
            LogError("TypeDecl index " << e->type->baseType << " not found");
            return nullptr;
    }
}

llvm::Value *ExprVal::codegen() {
    if (this->isConst) {
        return ConstToValue(this);
    }

    if (this->call) {
        llvm::Function *F = module->getFunction(this->refName);
        if (!F) {
            LogError("function " << this->refName <<" is not defined");
            return nullptr;
        }

        // TODO: check argument types
        if ((!F->isVarArg()) && (F->arg_size() != this->call->pars.size())) {
            LogError(
                "function " << this->refName <<" requires " << F->arg_size() 
                << " argument(s), " << this->call->pars.size() 
                << " argument(s) found");
            return nullptr;
        }

        std::vector<llvm::Value *> args;
        for (auto p : this->call->pars) {
            args.push_back(p->codegen());
            if (!args.back()) {
                // Argument has no return value
                return nullptr;
            }
        }

        return builder.CreateCall(F, args, "_");
    }

    ValueType* vt = FindVar(this->refName);
    if (!vt) {
        LogError("variable " << this->refName << " used before defined");
        return nullptr;
    }
    llvm::Value *v = vt->val;

    if (this->array) {
        llvm::Value *index = this->array->codegen();
        if (! index) {
            LogError("invalid array index");
            return nullptr;
        }
        return new llvm::LoadInst(
            builder.CreateGEP(v, index, "_"), 
            nullptr, builder.GetInsertBlock()
        );
    } else {
        return new llvm::LoadInst(v, nullptr, builder.GetInsertBlock());
    }
}

llvm::Value *EvalExpr::codegen() {
    if (this->isVal)
        return this->val->codegen();

    if (this->op == "=") {
        if (! this->l->isVal) {
            LogError("LHS of assignment must be a variable");
            return nullptr;
        }
        if (this->l->val->isConst)  {
            LogError("constant cannot be assigned");
            return nullptr;
        }
        if (this->l->val->call) {
            LogError("function return value cannot be used in LHS of assignment");
            return nullptr;
        }

        ValueType *vt = FindVar(this->l->val->refName);
        if (!vt) {
            LogError("variable " << this->l->val->refName << "not found");
            return nullptr;
        }
        llvm::Value *v = vt->val;

        if (this->l->val->array) {
            llvm::Value *index = this->l->val->array->codegen();
            if (! index) {
                LogError("invalid array index");
                return nullptr;
            }
            v = builder.CreateGEP(v, index, "_");
        }
        builder.CreateStore(this->r->codegen(), v, false);
        return new llvm::LoadInst(v, nullptr, builder.GetInsertBlock());
    }

    llvm::Value *lv = this->l->codegen();
    llvm::Value *rv = this->r->codegen();

    if (!lv || !rv) {
        LogError("operator " << this->op << " must accept binary inputs");
        return nullptr;
    }

    /*
    TODO: type inference
    */
    if (this->op == "ADD")
        return builder.CreateAdd(lv, rv, "_");
    if (this->op == "SUB")
        return builder.CreateSub(lv, rv, "_");
    if (this->op == "MUL")
        return builder.CreateMul(lv, rv, "_");
    if (this->op == "DIV")
        return builder.CreateSDiv(lv, rv, "_");
    if (this->op == "GT")
        return builder.CreateICmpSGT(lv, rv, "_");
    if (this->op == "LT")
        return builder.CreateICmpSLT(lv, rv, "_");
    if (this->op == "GE")
        return builder.CreateICmpSGE(lv, rv, "_");
    if (this->op == "LE")
        return builder.CreateICmpSLE(lv, rv, "_");
    if (this->op == "EQ")
        return builder.CreateICmpEQ(lv, rv, "_");
    if (this->op == "NEQ")
        return builder.CreateICmpNE(lv, rv, "_");

    LogError("operator " << this->op << " under construction");
    return nullptr;
}

llvm::Value *FuncDecl::codegen() {
    llvm::Function *prev = module->getFunction(this->name);
    if (prev) {
        LogError("function " << this->name << " already declared");
        return nullptr;
    }

    std::vector<llvm::Type *> ArgTypes;
    for (auto p : this->pars) {
        ArgTypes.push_back(type_trans(p->type));
    }
    llvm::FunctionType *Ft = llvm::FunctionType::get(
        type_trans(this->ret), ArgTypes, false);
    llvm::Function *F = llvm::Function::Create(
        Ft, llvm::Function::ExternalLinkage, this->name, module.get());
    
    unsigned Idx = 0;
    for (auto &Arg : F->args())
        Arg.setName(this->pars[Idx++]->name);

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(
        context, this->name + "_entry", F);
    builder.SetInsertPoint(BB);

    ClearSymLayer();
    NewSymLayer();
    
    Idx = 0;
    for (auto &Arg : F->args()) {
        llvm::AllocaInst *alloca = CreateEntryBlockAlloca(
            F, Arg.getName(), type_trans(this->pars[Idx]->type), 
            this->pars[Idx]->type->arrayT);
        builder.CreateStore(&Arg, alloca);
        InsertVar(Arg.getName(), alloca, nullptr);
        Idx++;
    }

    /*
        Insert Global String Definitions
    */

    for (auto p : strings) {
        if (strLits.find(p) == strLits.end()) {
            llvm::Value *v = builder.CreateGlobalStringPtr(llvm::StringRef(p));
            strLits[p] = v;
        }
    }
    strings.clear();

    for (auto p : this->exprs) {
        p->codegen();
    }

    if (F->getReturnType()->isVoidTy()) {
        builder.CreateRetVoid();
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
    this->init->codegen();

    llvm::Function *parent = builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *condBB = llvm::BasicBlock::Create(
        context, "lc", parent);
    llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(
        context, "l", parent);
    llvm::BasicBlock *finalBB = llvm::BasicBlock::Create(
        context, "le", parent);

    builder.CreateBr(condBB);
    builder.SetInsertPoint(condBB);

    llvm::Value *condV = this->cond->codegen();
    if (condV) {
        llvm::Value *condB = builder.CreateICmpNE(
            condV, 
            llvm::ConstantInt::get(context, llvm::APInt(1, 0, true)), 
            "_");
        builder.CreateCondBr(condB, loopBB, finalBB);
    }

    builder.SetInsertPoint(loopBB);
    for (auto p : this->exprs) {
        p->codegen();
    }
    this->step->codegen();
    builder.CreateBr(condBB);

    builder.SetInsertPoint(finalBB);
    return nullptr;
}

llvm::Value *FuncCall::codegen() {
    return nullptr;
}

llvm::Value *GenericDecl::codegen() {
    return nullptr;
}

llvm::Value *IfExpr::codegen() {
    llvm::Value *condV = this->cond->codegen();
    if (!condV) {
        return nullptr;
    }
    condV = builder.CreateICmpNE(
        condV, 
        llvm::ConstantInt::get(context, llvm::APInt(1, 0, true)), 
        "_");

    llvm::Function *parent = builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *trueBB = llvm::BasicBlock::Create(
        context, "it", parent);
    llvm::BasicBlock *falseBB;
    llvm::BasicBlock *finalBB = llvm::BasicBlock::Create(
        context, "ic");
    if (this->iffalse.size() == 0) {
        falseBB = finalBB;
    } else {
        falseBB = llvm::BasicBlock::Create(context, "if");
    }
    builder.CreateCondBr(condV, trueBB, falseBB);

    builder.SetInsertPoint(trueBB);
    for (auto p : this->iftrue) {
        p->codegen();
    }
    builder.CreateBr(finalBB);

    builder.SetInsertPoint(falseBB);
    for (auto p : this->iffalse) {
        p->codegen();
    }
    if (this->iffalse.size() != 0)
        builder.CreateBr(finalBB);

    parent->getBasicBlockList().push_back(finalBB);
    builder.SetInsertPoint(finalBB);

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
    if (! this->stmt) {
        builder.CreateRetVoid();
        return nullptr;
    }

    llvm::Value *r = this->stmt->codegen();

    if (r) {
        builder.CreateRet(r);
    }

    return nullptr;
}

llvm::Value *TypeDecl::codegen() {
    LogError("TypeDecl() cannot generate code");
    return nullptr;
}

llvm::Value *VarDecl::codegen() {
    if (FindTopVar(this->name)) {
        LogError("variable " << this->name << " already declared");
        return nullptr;
    }

    llvm::Function *curF = builder.GetInsertBlock()->getParent();
    llvm::AllocaInst *alloca = CreateEntryBlockAlloca(
        curF, this->name, type_trans(this->type), this->type->arrayT);
    llvm::Value *v = nullptr;

    if (this->init) {
        v = this->init->codegen();
        builder.CreateStore(v, alloca);
    }
    InsertVar(this->name, alloca, nullptr);

    return nullptr;
}

llvm::Value *WhileExpr::codegen() {
    llvm::Function *parent = builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *condBB = llvm::BasicBlock::Create(
        context, "lc", parent);
    llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(
        context, "l", parent);
    llvm::BasicBlock *finalBB = llvm::BasicBlock::Create(
        context, "lf", parent);

    builder.CreateBr(condBB);
    builder.SetInsertPoint(condBB);

    llvm::Value *condV = this->cond->codegen();
    if (condV) {
        condV = builder.CreateICmpNE(
            condV, 
            llvm::ConstantInt::get(context, llvm::APInt(1, 0, true)), 
            "_");
    }
    builder.CreateCondBr(condV, loopBB, finalBB);
    builder.SetInsertPoint(loopBB);

    for (auto p : this->exprs) {
        p->codegen();
    }
    builder.CreateBr(condBB);
    builder.SetInsertPoint(finalBB);
    
    return nullptr;
}

llvm::Value *Program::codegen() {
    for (auto p : this->stmts) {
        p->codegen();
    }
    return nullptr;
}

int AST::codegen(Program prog, std::string outputFileName) {
    module = llvm::make_unique<llvm::Module>(outputFileName, context);

    /* insert standard C library functions */
    module->getOrInsertFunction(
        "printf",
        llvm::FunctionType::get(
            llvm::IntegerType::getInt32Ty(context),
            llvm::PointerType::get(llvm::Type::getInt8Ty(context), 0), 
            true /* this is var arg func type*/
        )
    );

    /* Generate IR code */
    prog.codegen();
    module->print(llvm::errs(), nullptr);

    /*
        Initialize Targets
    */

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(TargetTriple);
    std::string Err;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Err);
    if (!Target) {
        LogError(Err);
        return ERR_OBJCODE;
    }
    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto TargetMachine = Target->createTargetMachine(
        TargetTriple, llvm::sys::getHostCPUName(), "", opt, RM);
    module->setDataLayout(TargetMachine->createDataLayout());
    module->setTargetTriple(TargetTriple);

    std::error_code ec;
    llvm::raw_fd_ostream dest(outputFileName + ".o", ec, llvm::sys::fs::F_None);
    if (ec) {
        LogError("could not open file: " << ec.message());
        return ERR_OBJCODE;
    }
    
    /* Pass Manager and File Output */
    llvm::legacy::PassManager pass;
    auto FileType = llvm::TargetMachine::CGFT_ObjectFile;

    if (TargetMachine->addPassesToEmitFile(pass, dest, FileType)) {
        LogError("TargetMachine can't emit a file of this type");
        return ERR_OBJCODE;
    }

    pass.run(*module);
    dest.flush();
    return 0;
}
