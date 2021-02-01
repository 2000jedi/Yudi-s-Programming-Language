/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 * -------------------
 * ll(1) parser
 */

#include <string>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "parser.hpp"
#include "util.hpp"
#include "err.hpp"

static token input_token;

inline bool error(scanner *Scanner, std::string prompt) {
    std::cerr << "Error  (Parser): at line " << Scanner->row << ":" << Scanner->col << std::endl
            << Scanner->line << std::endl
            << "\t" << prompt << " cannot accept "
            << terms[input_token] << "(" << Scanner->data << ")" << std::endl;
    throw std::runtime_error("Parser Error");
}

std::string match(scanner *Scanner, token expected) {
    if (input_token == expected) {
        std::string result = Scanner->data;

        if (expected != t_eof)
            input_token = Scanner->scan();

        return result;
    } else {
        error(Scanner, "Terminal \"" + terms[expected] + '"');
        return "E";
    }
}

std::unique_ptr<AST::Program> statements(scanner *Scanner);
std::unique_ptr<AST::GlobalStatement> statement(scanner *Scanner);
std::unique_ptr<AST::FuncDecl> func_decl(scanner *Scanner);
AST::GenericDecl generic(scanner *Scanner);
int array(scanner *Scanner);
std::vector<AST::Param> params_decl(scanner *Scanner);
AST::TypeDecl ret_decl(scanner *Scanner);
AST::TypeDecl type_name(scanner *Scanner);
std::vector<std::unique_ptr<AST::Expr>> expr_list(scanner *Scanner);
std::unique_ptr<AST::EvalExpr> eval_expr(scanner *Scanner);
std::unique_ptr<AST::VarDecl> var_def(scanner *Scanner);
std::unique_ptr<AST::EnumDecl> enum_def(scanner *Scanner, AST::Name *parent);
std::unique_ptr<AST::VarDecl> const_def(scanner *Scanner);
AST::Name name_space(scanner *Scanner);

std::unique_ptr<AST::Program> statements(scanner *Scanner) {
    std::unique_ptr<AST::Program> program = std::make_unique<AST::Program>(Scanner);
    while (input_token == t_import) {
        match(Scanner, t_import);
        match(Scanner, lpar);
        auto path = match(Scanner, t_str);
        match(Scanner, rpar);
        match(Scanner, eol);
        program->imports.push_back(path);
    }
    while (input_token != t_eof) {
        auto gs = statement(Scanner);
        if (gs != nullptr)
            program->insert(std::move(gs));
    }
    return program;
}

std::unique_ptr<AST::GlobalStatement> statement(scanner *Scanner) {
    switch (input_token) {
        case t_fn: {
            return func_decl(Scanner);
        }
        case t_var: {
            auto vd = var_def(Scanner);
            vd->is_global = true;
            return vd;
        }
        case t_const: {
            auto cd = const_def(Scanner);
            cd->is_global = true;
            return cd;
        }
        case t_class: {
            match(Scanner, t_class);
            auto cn = match(Scanner, t_name);
            auto gen = generic(Scanner);
            match(Scanner, lbra);
            auto cl = std::make_unique<AST::ClassDecl>(Scanner, cn, gen);
            while ((input_token == t_const) ||
                   (input_token == t_var) ||
                   (input_token == t_fn)) {
                // class domain
                auto clstmt = statement(Scanner);
                if (clstmt != nullptr) {
                    cl->stmts.push_back(std::move(clstmt));
                }
            }
            match(Scanner, rbra);
            return cl;
        }
        case t_union: {
            match(Scanner, t_union);
            auto un = AST::Name(match(Scanner, t_name));
            auto gen = generic(Scanner);
            match(Scanner, lbra);
            auto u = std::make_unique<AST::UnionDecl>(Scanner, un, gen);
            while (input_token == t_enum) {
                auto cl = enum_def(Scanner, &un);
                u->classes.push_back(std::move(cl));
            }
            match(Scanner, rbra);
            return u;
        }
        default: {
            return nullptr;
        }
    }
}

std::unique_ptr<AST::EnumDecl> enum_def(scanner *Scanner, AST::Name *parent) {
    match(Scanner, t_enum);
    auto en = match(Scanner, t_name);
    auto e = std::make_unique<AST::EnumDecl>(Scanner, AST::Name(parent, en));
    match(Scanner, lbra);
    while (input_token == t_var) {
        auto vd = var_def(Scanner);
        e->vars.push_back(std::move(vd));
    }
    match(Scanner, rbra);
    return e;
}

std::unique_ptr<AST::FuncDecl> func_decl(scanner *Scanner) {
    match(Scanner, t_fn);
    auto n = match(Scanner, t_name);
    auto gen = generic(Scanner);
    match(Scanner, lpar);
    auto prms = params_decl(Scanner);
    match(Scanner, rpar);
    auto ret_type = ret_decl(Scanner);
    match(Scanner, lbra);
    auto exprs = expr_list(Scanner);
    match(Scanner, rbra);
    auto fn = std::make_unique<AST::FuncDecl>(
        Scanner, AST::Name(n), gen, prms, ret_type);
    for (auto&& e : exprs)
        fn->exprs.push_back(std::move(e));
    return fn;
}

AST::GenericDecl generic(scanner *Scanner) {
    if (input_token == lt) {
        match(Scanner, lt);
        auto gen_name = name_space(Scanner);
        match(Scanner, gt);
        return AST::GenericDecl(Scanner, gen_name);
    } else {
        // epsilon
        return AST::GenericDecl();
    }
}

int array(scanner *Scanner) {
    int v = 0;
    if (input_token == larr) {
        match(Scanner, larr);
        v = std::stoi(match(Scanner, t_int));
        match(Scanner, rarr);
    }  // else: epsilon
    return v;
}

std::vector<AST::Param> params_decl(scanner *Scanner) {
    std::vector<AST::Param> prms;
    if (input_token == t_name) {
        auto par_name = match(Scanner, t_name);
        match(Scanner, colon);
        auto tn = type_name(Scanner);
        prms.push_back(AST::Param(Scanner, par_name, tn));
        while (input_token == comma) {
            match(Scanner, comma);
            auto par_name = match(Scanner, t_name);
            match(Scanner, colon);
            auto tn = type_name(Scanner);
            prms.push_back(AST::Param(Scanner, par_name, tn));
        }
    }
    return prms;
}

AST::TypeDecl ret_decl(scanner *Scanner) {
    if (input_token == colon) {
        match(Scanner, colon);
        return type_name(Scanner);
    } else {
        // epsilon
        return AST::VoidType;
    }
}

AST::TypeDecl type_name(scanner *Scanner) {
    AST::Types base = AST::t_void;
    AST::Name other;
    switch (input_token) {
        case type_void:
            base = AST::t_void;
            match(Scanner, input_token);
            break;
        case type_bool:
            base = AST::t_bool;
            match(Scanner, input_token);
            break;
        case type_char:
            base = AST::t_char;
            match(Scanner, input_token);
            break;
        case type_fp32:
            base = AST::t_fp32;
            match(Scanner, input_token);
            break;
        case type_fp64:
            base = AST::t_fp64;
            match(Scanner, input_token);
            break;
        case type_int32:
            base = AST::t_int32;
            match(Scanner, input_token);
            break;
        case type_uint8:
            base = AST::t_uint8;
            match(Scanner, input_token);
            break;
        case type_str:
            base = AST::t_str;
            match(Scanner, input_token);
            break;
        case t_name: {
            base = AST::t_class;
            other = name_space(Scanner);
            break;
        }
        default:
            error(Scanner, "type() rejects");
    }
    auto gen = generic(Scanner);
    auto arr = array(Scanner);
    return AST::TypeDecl(Scanner, base, other, gen, arr);
}

std::unique_ptr<AST::EvalExpr> init_def(scanner *Scanner) {
    if (input_token == move) {
        match(Scanner, move);
        return eval_expr(Scanner);
    } else {
        // epsilon
        return nullptr;
    }
}

std::unique_ptr<AST::VarDecl> var_def(scanner *Scanner) {
    match(Scanner, t_var);
    auto vn = match(Scanner, t_name);
    match(Scanner, colon);
    auto tn = type_name(Scanner);
    auto init = init_def(Scanner);
    match(Scanner, eol);
    return std::make_unique<AST::VarDecl>(
        Scanner, vn, tn, std::move(init));
}

std::unique_ptr<AST::VarDecl> const_def(scanner *Scanner) {
    match(Scanner, t_const);
    auto cn = match(Scanner, t_name);
    match(Scanner, move);
    auto init = eval_expr(Scanner);
    match(Scanner, eol);
    auto cd = std::make_unique<AST::VarDecl>(
        Scanner, cn, AST::VoidType, std::move(init));
    cd->is_const = true;
    return cd;
}

std::unique_ptr<AST::IfExpr> if_expr(scanner *Scanner) {
    match(Scanner, t_if);
    match(Scanner, lpar);
    auto cond = eval_expr(Scanner);
    match(Scanner, rpar);
    match(Scanner, lbra);
    auto iftrue = expr_list(Scanner);
    match(Scanner, rbra);
    if (input_token == t_else) {
        // if-then-else
        match(Scanner, t_else);
        match(Scanner, lbra);
        auto iffalse = expr_list(Scanner);
        match(Scanner, rbra);
        auto if_expr = std::make_unique<AST::IfExpr>(
            Scanner, std::move(cond));
        for (auto&& e : iftrue) {
            if_expr->iftrue.push_back(std::move(e));
        }
        for (auto&& e : iffalse) {
            if_expr->iffalse.push_back(std::move(e));
        }
        return if_expr;
    } else {
        // if-then
        auto if_expr = std::make_unique<AST::IfExpr>(Scanner, std::move(cond));
        for (auto&& e : iftrue) {
            if_expr->iftrue.push_back(std::move(e));
        }
        return if_expr;
    }
}

std::unique_ptr<AST::ForExpr> for_expr(scanner *Scanner) {
    match(Scanner, t_for);
    match(Scanner, lpar);
    auto init = eval_expr(Scanner);
    match(Scanner, eol);
    auto cond = eval_expr(Scanner);
    match(Scanner, eol);
    auto step = eval_expr(Scanner);
    match(Scanner, rpar);
    match(Scanner, lbra);
    auto es = expr_list(Scanner);
    match(Scanner, rbra);
    auto fe = std::make_unique<AST::ForExpr>(
        Scanner, std::move(init), std::move(cond), std::move(step));
    for (auto&& e : es) {
        fe->exprs.push_back(std::move(e));
    }
    return fe;
}

std::unique_ptr<AST::WhileExpr> while_expr(scanner *Scanner) {
    match(Scanner, t_while);
    match(Scanner, lpar);
    auto cond = eval_expr(Scanner);
    match(Scanner, rpar);
    match(Scanner, lbra);
    auto exprs = expr_list(Scanner);
    match(Scanner, rbra);
    auto we = std::make_unique<AST::WhileExpr>(
        Scanner, std::move(cond));
    for (auto&& e : exprs) {
        we->exprs.push_back(std::move(e));
    }
    return we;
}

std::vector<AST::MatchLine> match_line(scanner *Scanner) {
    std::vector<AST::MatchLine> lines;
    while (input_token == t_name) {
        std::string enum_name = match(Scanner, t_name);
        std::string opt_name = "";
        if (input_token == lpar) {
            match(Scanner, lpar);
            opt_name = match(Scanner, t_name);
            match(Scanner, rpar);
        }
        auto ml = AST::MatchLine(Scanner, enum_name, opt_name);
        match(Scanner, lbra);
        ml.exprs = expr_list(Scanner);
        match(Scanner, rbra);
        lines.push_back(std::move(ml));
    }
    return lines;
}

std::unique_ptr<AST::MatchExpr> match_expr(scanner *Scanner) {
    match(Scanner, t_match);
    match(Scanner, lpar);
    auto expr = eval_expr(Scanner);
    match(Scanner, rpar);
    match(Scanner, lbra);
    auto lines = match_line(Scanner);
    match(Scanner, rbra);
    auto me = std::make_unique<AST::MatchExpr>(
        Scanner, std::move(expr));
    for (auto&& line : lines) {
        me->lines.push_back(std::move(line));
    }
    return me;
}

std::unique_ptr<AST::RetExpr> ret_expr(scanner *Scanner) {
    match(Scanner, t_return);
    auto expr = eval_expr(Scanner);
    match(Scanner, eol);
    return std::make_unique<AST::RetExpr>(Scanner, std::move(expr));
}

std::unique_ptr<AST::Expr> expr(scanner *Scanner) {
    switch (input_token) {
            case t_var:
                return var_def(Scanner);
            case t_const:
                return const_def(Scanner);
            case t_if:
                return if_expr(Scanner);
            case t_for:
                return for_expr(Scanner);
            case t_while:
                return while_expr(Scanner);
            case t_match:
                return match_expr(Scanner);
            case t_return:
                return ret_expr(Scanner);
            case t_continue:
                match(Scanner, t_continue);
                match(Scanner, eol);
                return std::make_unique<AST::ContExpr>(Scanner);
            case t_break:
                match(Scanner, t_break);
                match(Scanner, eol);
                return std::make_unique<AST::BreakExpr>(Scanner);
            case eol:
                // empty expression
                match(Scanner, eol);
                return std::make_unique<AST::Expr>();
            case t_int:
            case t_float:
            case t_str:
            case t_char:
            case t_name:
            case lpar: {
                auto ee = eval_expr(Scanner);
                match(Scanner, eol);
                return ee;
            }
            default:
                LogError("Impossible to reach");
                return nullptr;
        }
}

std::vector<std::unique_ptr<AST::Expr>> expr_list(scanner *Scanner) {
    std::vector<std::unique_ptr<AST::Expr>> result;
    while (1) {
        switch (input_token) {
            case t_var:
            case t_const:
            case t_if:
            case t_for:
            case t_while:
            case t_match:
            case t_return:
            case t_continue:
            case t_break:
            case eol:
            case t_int:
            case t_float:
            case t_str:
            case t_char:
            case t_name:
            case lpar:
                result.push_back(expr(Scanner));
                break;
            default:
                // epsilon
                return result;
        }
    }
}

AST::Name name_space(scanner *Scanner) {
    std::vector<std::string> names;
    names.push_back(match(Scanner, t_name));
    while (input_token == dot) {
        match(Scanner, dot);
        names.push_back(match(Scanner, t_name));
    }
    AST::Name n = AST::Name(names.back());
    names.pop_back();
    for (auto&& i : names)
        n.ClassName.push_back(i);
    return n;
}

/*
 * Integrated Expression Parser
 * parses binary operations
 **/

#define DEF_EL(x) std::unique_ptr<AST::EvalExpr> x(scanner *Scanner)
#define DEF_ER(x) std::unique_ptr<AST::EvalExpr> x(\
    scanner *Scanner, std::unique_ptr<AST::EvalExpr> l)

DEF_EL(e_pars) {
    switch (input_token) {
        case t_int: {
            auto val_str = match(Scanner, t_int);
            auto val = std::make_unique<AST::ExprVal>(
                Scanner, val_str, AST::TypeDecl(AST::t_int32));
            return std::make_unique<AST::EvalExpr>(Scanner, std::move(val));
        }
        case t_float: {
            auto val_str = match(Scanner, t_float);
            auto val = std::make_unique<AST::ExprVal>(
                Scanner, val_str, AST::TypeDecl(AST::t_fp32));
            return std::make_unique<AST::EvalExpr>(Scanner, std::move(val));
        }
        case t_char: {
            auto val_str = match(Scanner, t_char);
            auto val = std::make_unique<AST::ExprVal>(
                Scanner, val_str, AST::TypeDecl(AST::t_char));
            return std::make_unique<AST::EvalExpr>(Scanner, std::move(val));
        }
        case t_str: {
            auto raw = match(Scanner, t_str);
            auto val = std::make_unique<AST::ExprVal>(
                Scanner, raw, AST::TypeDecl(AST::t_str));
            return std::make_unique<AST::EvalExpr>(Scanner, std::move(val));
        }
        case t_name: {
            auto n = name_space(Scanner);
            std::unique_ptr<AST::FuncCall> fc = nullptr;
            AST::Name gen_n;
            if (input_token == gen) {
                match(Scanner, gen);
                gen_n = name_space(Scanner);
                match(Scanner, gen); 
            }
            if (input_token == lpar) {
                // optional function call
                fc = std::make_unique<AST::FuncCall>(Scanner);
                fc->function = n;
                fc->gen_val = gen_n;
                match(Scanner, lpar);
                if (input_token == rpar) {
                    match(Scanner, rpar);
                } else {
                    auto e = eval_expr(Scanner);
                    fc->pars.push_back(std::move(e));
                    while (input_token == comma) {
                        match(Scanner, comma);
                        e = eval_expr(Scanner);
                        fc->pars.push_back(std::move(e));
                    }
                    match(Scanner, rpar);
                }
            }
            std::unique_ptr<AST::EvalExpr> arr = nullptr;
            if (input_token == larr) {
                match(Scanner, larr);
                arr = eval_expr(Scanner);
                match(Scanner, rarr);
            }
            auto ee = std::make_unique<AST::ExprVal>(
                Scanner, n, std::move(fc), std::move(arr));
            return std::make_unique<AST::EvalExpr>(Scanner, std::move(ee));
        }
        case lpar: {
            match(Scanner, lpar);
            auto ex = eval_expr(Scanner);
            match(Scanner, rpar);
            return ex;
        }
        default: {
            error(Scanner, "non-terminal <pars> rejects");
        }
    }
    throw;
}

DEF_ER(e_mul_div_) {
    if ((input_token == mul) ||
        (input_token == t_div) || (input_token == rem)) {
        auto op = input_token;
        match(Scanner, input_token);
        auto r = e_pars(Scanner);
        auto ex = std::make_unique<AST::EvalExpr>(
            Scanner, op, std::move(l), std::move(r));
        return e_mul_div_(Scanner, std::move(ex));
    } else {
        // epsilon
        return l;
    }
}

DEF_EL(e_mul_div) {
    auto l = e_pars(Scanner);
    return e_mul_div_(Scanner, std::move(l));
}

DEF_ER(e_add_sub_) {
    if ((input_token == add) || (input_token == sub)) {
        auto op = input_token;
        match(Scanner, input_token);
        auto r = e_mul_div(Scanner);
        auto ex = std::make_unique<AST::EvalExpr>(
            Scanner, op, std::move(l), std::move(r));
        return e_add_sub_(Scanner, std::move(ex));
    } else {
        // epsilon
        return l;
    }
}

DEF_EL(e_add_sub) {
    auto l = e_mul_div(Scanner);
    return e_add_sub_(Scanner, std::move(l));
}

DEF_ER(e_lgte_) {
    if ((input_token == le) || (input_token == lt) ||
        (input_token == ge) || (input_token == gt) ) {
        auto op = input_token;
        match(Scanner, input_token);
        auto r = e_add_sub(Scanner);
        auto ex = std::make_unique<AST::EvalExpr>(
            Scanner, op, std::move(l), std::move(r));
        return e_lgte_(Scanner, std::move(ex));
    } else {
        // epsilon
        return l;
    }
}

DEF_EL(e_lgte) {
    auto l = e_add_sub(Scanner);
    return e_lgte_(Scanner, std::move(l));
}

DEF_ER(e_eq_neq_) {
    if ((input_token == equ) || (input_token == neq)) {
        auto op = input_token;
        match(Scanner, input_token);
        auto r = e_lgte(Scanner);
        auto ex = std::make_unique<AST::EvalExpr>(
            Scanner, op, std::move(l), std::move(r));
        return e_eq_neq_(Scanner, std::move(ex));
    } else {
        // epsilon
        return l;
    }
}

DEF_EL(e_eq_neq) {
    auto l = e_lgte(Scanner);
    return e_eq_neq_(Scanner, std::move(l));
}

DEF_ER(e_band) {
    if (input_token == band) {
        match(Scanner, band);
        auto r = e_eq_neq(Scanner);
        auto ex = std::make_unique<AST::EvalExpr>(
            Scanner, band, std::move(l), std::move(r));
        return e_band(Scanner, std::move(ex));
    } else {
        // epsilon
        return l;
    }
}

DEF_EL(e_bitwise_and) {
    auto l = e_eq_neq(Scanner);
    return e_band(Scanner, std::move(l));
}

DEF_ER(e_bxor) {
    if (input_token == bxor) {
        match(Scanner, bxor);
        auto r = e_bitwise_and(Scanner);
        auto ex = std::make_unique<AST::EvalExpr>(
            Scanner, bxor, std::move(l), std::move(r));
        return e_bxor(Scanner, std::move(ex));
    } else {
        // epsilon
        return l;
    }
}

DEF_EL(e_bitwise_xor) {
    auto l = e_bitwise_and(Scanner);
    return e_bxor(Scanner, std::move(l));
}

DEF_ER(e_bor) {
    if (input_token == bor) {
        match(Scanner, bor);
        auto r = e_bitwise_xor(Scanner);
        auto ex = std::make_unique<AST::EvalExpr>(
            Scanner, bor, std::move(l), std::move(r));
        return e_bor(Scanner, std::move(ex));
    } else {
        // epsilon
        return l;
    }
}

DEF_EL(e_bitwise_or) {
    auto l = e_bitwise_xor(Scanner);
    return e_bor(Scanner, std::move(l));
}

DEF_ER(e_land) {
    if (input_token == land) {
        match(Scanner, land);
        auto r = e_bitwise_or(Scanner);
        auto ex = std::make_unique<AST::EvalExpr>(
            Scanner, land, std::move(l), std::move(r));
        return e_land(Scanner, std::move(ex));
    } else {
        // epsilon
        return l;
    }
}

DEF_EL(e_logical_and) {
    auto l = e_bitwise_or(Scanner);
    return e_land(Scanner, std::move(l));
}

DEF_ER(e_lor) {
    if (input_token == lor) {
        match(Scanner, lor);
        auto r = e_logical_and(Scanner);
        auto ex = std::make_unique<AST::EvalExpr>(
            Scanner, lor, std::move(l), std::move(r));
        return e_lor(Scanner, std::move(ex));
    } else {
        // epsilon
        return l;
    }
}

DEF_EL(e_logical_or) {
    auto l = e_logical_and(Scanner);
    return e_lor(Scanner, std::move(l));
}

DEF_ER(e_assign) {
    switch (input_token) {
        case move: {
            match(Scanner, move);
            auto r = e_logical_or(Scanner);
            auto ex = std::make_unique<AST::EvalExpr>(
                Scanner, move, std::move(l), std::move(r));
            return e_assign(Scanner, std::move(ex));
        }
        case copy: {
            match(Scanner, copy);
            auto r = e_logical_or(Scanner);
            auto ex = std::make_unique<AST::EvalExpr>(
                Scanner, copy, std::move(l), std::move(r));
            return e_assign(Scanner, std::move(ex));
        }
        default:
            // epsilon
            return l;
    }
}

DEF_EL(eval_expr) {
    auto l = e_logical_or(Scanner);
    return e_assign(Scanner, std::move(l));
}

std::unique_ptr<AST::Program> parse(scanner Scanner) {
    input_token = Scanner.scan();
    return statements(&Scanner);
}
