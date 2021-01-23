/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#include "ast.hpp"

#include <memory>
#include <utility>

#include "util.hpp"
#include "runtime.hpp"
#include "err.hpp"

using namespace AST;

#define INTERPRET(x) ValueType *x::interpret(SymTable *st)

void MemStore::Free(void) {
    if (v == nullptr) return;
    for (auto msi = v->ms.begin(); msi != v->ms.end(); ++msi)
        if (*msi == this) {
            v->ms.erase(msi);
            if (v->ms.size() != 0) {
                v = nullptr;
            } else {
                if (!placehold) {
                    delete v;
                }
            }
            return;
        }
    if ((v->ms.size() == 0) && (!placehold)) {
        delete v;
    }
}

void MemStore::set(ValueType *vt) {
    Free();
    v = vt;
}

ValueType *MemStore::get(void) {
    return v;
}

FuncStore::FuncStore(FuncDecl *a, SymTable *b, TypeDecl t) : fd(a) {
    if (b != nullptr) {
        auto vt = new ValueType(b, &t);
        vt->ms.push_back(& context);
        context.set(vt);
    }
}

FuncStore::~FuncStore() {
    if (context.get()) {
        context.get()->data.st = nullptr;
    }
    context.set(nullptr);
}

// Symble Table - record Variable and Type Information
void SymTable::addLayer(void) {
    d.push_back(std::map<Name, MemStore>());
}

void SymTable::removeLayer(void) {
    for (auto&& ms : d.back()) {
        ms.second.Free();
    }
    d.pop_back();
}

SymTable::~SymTable() {
    while (!this->d.empty())
        this->removeLayer();
}

MemStore SymTable::insert(Name name, ValueType *vt) {
    if (name.str() == "this") {
        d.back()[name].placehold = true;
    }
    vt->ms.push_back(& d.back()[name]);
    d.back()[name].set(vt);
    return d.back()[name];
}

MemStore SymTable::update(ExprVal *name, ValueType *vt) {
    MemStore *ms = this->lookup(name);
    vt->ms.push_back(ms);
    ms->set(vt);
    return *ms;
}

MemStore *SymTable::lookup(Name name, ErrInfo* ast) {
    if (name.ClassName.size() > 0) {
        auto owner = this->lookup(name.owner(), ast)->get();
        if (!((owner->type.baseType) == t_rtfn && (name.BaseName == "new"))) {
            if (owner->type.baseType != t_class) {
                throw InterpreterException(name.owner().str() + " is not a compound type", ast);
            }
            auto clst = owner->data.st;
            return clst->lookup(Name(name.BaseName), ast);
        }
    }

    for (int i = d.size() - 1; i >= 0; i--) {
        if (d[i].find(name) != d[i].end()) {
            return & d[i][name];
        }
    }
    throw InterpreterException("variable " + name.str() + " is not declared", ast);
}

MemStore *SymTable::lookup(ExprVal *name) {
    if (name->call != nullptr)
        throw InterpreterException("cannot lookup a function call", name);

    if (name->array != nullptr) {
        auto arr = this->lookup(name->refName, name)->get();
        auto arr_index_vt = name->array->interpret(this);
        if (arr_index_vt->type != IntType) {
            throw InterpreterException("array index must be an int", name);
        }
        int arr_index = arr_index_vt->data.ival;

        if (arr->type.arrayT <= arr_index) {
            throw InterpreterException("array index out of bound", name);
        }

        auto vts = arr->data.vt;
        return &vts[arr_index];
    } else {
        return this->lookup(name->refName, name);
    }
}

/**
 * Interpreter Interface - interpret() methods
 */
ValueType *ConstEval(ExprVal *e) {
    switch (e->type.baseType) {
        case t_int32: {
            return new ValueType(std::stoi(e->constVal));
        }
        case AST::t_char: {
            return new ValueType(e->constVal[0]);
        }
        case t_fp32: {
            return new ValueType(std::stof(e->constVal));
        }
        case t_fp64: {
            return new ValueType(std::stod(e->constVal));
        }
        case AST::t_str: {
            std::string *s = new std::string(e->constVal);
            return new ValueType(s, true);
        }
        default: {
            throw InterpreterException("Type `" + e->type.str() + "` is invalid", e);
        }
    }
}

std::string TypeDecl::str(void) {
    std::stringstream ss;
    switch (this->baseType) {
        case t_bool:
            ss << "bool";
            break;
        case t_uint8:
            ss << "uint8";
            break;
        case t_char:
            ss << "char";
            break;
        case t_int32:
            ss << "int32";
            break;
        case t_fp32:
            ss << "fp32";
            break;
        case t_fp64:
            ss << "fp64";
            break;
        case t_str:
            ss << "str";
            break;
        case t_void:
            ss << "void";
            break;
        case t_class:
            ss << other.str();
            break;
        default:
            ss << "internal";
    }
    if (this->arrayT != 0) {
        ss << '[' << this->arrayT << ']';
    }
    return ss.str();
}

ValueType *TypeDecl::newVal(void) {
    if (this->arrayT == 0) {
        switch (this->baseType) {
            case t_bool:
                return new ValueType(false, false);
            case t_uint8:
                return new ValueType((uint8_t)0, false);
            case t_char:
                return new ValueType((char)0, false);
            case t_int32:
                return new ValueType(0, false);
            case t_fp32:
                return new ValueType((float)0.0, false);
            case t_fp64:
                return new ValueType(0.0, false);
            default:
                return new ValueType((SymTable*)nullptr, this);
        }
    } else {
        ValueType* arr = new ValueType(this, false);
        auto td = new TypeDecl(this->baseType);
        for (int i = 0; i < this->arrayT; ++i) {
            arr->data.vt[i].set(td->newVal());
        }
        delete td;
        return arr;
    }
}

static int return_flag = 0;
static int continue_flag = 0;
static int break_flag = 0;

int AST::interpret(Program prog) {
    SymTable *st = new SymTable();
    st->addLayer();
    runtime_bind(st);
    prog.interpret(st);
    st->removeLayer();
    delete st;

    return 0;
}

INTERPRET(Program) {
    st->addLayer();
    for (auto stmt = stmts.begin(); stmt != stmts.end(); stmt++) {
        (*stmt)->declare(st, nullptr);
    }
    auto fs = st->lookup(Name("main"), this)->get()->data.fs;
    st->addLayer();
    fs->fd->interpret(st);
    st->removeLayer();
    st->removeLayer();
    return & None;
}

INTERPRET(VarDecl) {
    ValueType *t;
    if (this->type.baseType == AST::t_void) {
        if (this->init == nullptr) {
            throw InterpreterException(
                "variable " + this->name.str() + " has unknown type", this);
        } else {
            t = this->init->interpret(st);
            if (t == & None) {
                throw InterpreterException(
                    "variable " + this->name.str() + " has void type", this);
            }
        }
    } else {
        if (this->init != nullptr) {
            t = this->init->interpret(st);
            if (t == nullptr) {
                throw InterpreterException(
                    "variable \"" + this->name.str() + "\" has void type", this);
            }
            if (t->type != this->type) {
                throw InterpreterException(
                    "variable \"" + this->name.str() + "\": type " + this->type.str() + " cannot be assigned to " + t->type.str(), this);
            }
        } else {
            t = this->type.newVal();
        }
    }
    if (this->is_const) {
        t->isConst = true;
    }
    t = st->insert(this->name, t).get();
    return t;
}

void FuncDecl::declare(SymTable *st, SymTable *context) {
    auto clty = AST::TypeDecl(AST::t_class);
    clty.other = this->name;
    FuncStore *cur = new FuncStore(this, context, clty);
    st->insert(this->name, new ValueType(cur, true));
}

INTERPRET(FuncDecl) {
    for (auto&& e : this->exprs) {
        ValueType *vt = e->interpret(st);
        if (return_flag) {
            return_flag--;
            return vt;
        }
    }
    return & None;
}

INTERPRET(FuncCall) {
    st->addLayer();

    auto fn_ = st->lookup(this->function, this)->get();
    if (fn_->type.baseType == t_rtfn) {
        st->removeLayer();
        return runtime_handler(this->function, this, st);
    }
    if (fn_->type.baseType != t_fn)
        throw InterpreterException("type cannot be called", this);

    auto fn = fn_->data.fs;

    for (unsigned int i = 0; i < this->pars.size(); ++i) {
        auto vt = this->pars[i]->interpret(st);
        auto prm = fn->fd->pars[i];
        if (vt->type != prm.type) {
            throw InterpreterException("type mismatch for argument " + prm.name, this);
        }
        st->insert(Name(prm.name), vt);
    }

    if (fn->context.get() != nullptr) {
        st->insert(Name("this"), fn->context.get());
    }

    auto ret = fn->fd->interpret(st);
    st->removeLayer();

    return ret;
}

// runtime helper function to create initializer
void ClassDecl::declare(SymTable *st, SymTable *context) {
    st->insert(this->name, new ValueType(this, true));
    for (auto&& stmt : this->stmts) {
        switch (stmt->stmtType) {
            case gs_func: {
                auto fd = (FuncDecl *)stmt.get();
                if (fd->name.BaseName == "new") {
                    st->insert(Name(& this->name, "new"), new ValueType(new FuncStore(fd, nullptr, VoidType), true));
                }
                break;
            }
            default:
                break;
        }
    }
}

void UnionDecl::declare(SymTable *st, SymTable *context) {
    // TODO: union declaration
}

bool vt_is_true(ValueType *vt, ErrInfo *ast) {
    if (vt == nullptr) {
        throw InterpreterException("expression is nullptr", ast);
    }
    if ((vt->type.baseType != t_bool) && (vt->type.arrayT == 0)) {
        throw InterpreterException("expression is not boolean", ast);
    }
    bool result = vt->data.one_bit;
    delete vt;
    return result;
}

INTERPRET(IfExpr) {
    st->addLayer();
    auto cond_vt = this->cond->interpret(st);
    if (vt_is_true(cond_vt, this)) {
        for (auto&& expr : this->iftrue) {
            auto ret = expr->interpret(st);
            if (expr->exprType == e_ret) {
                st->removeLayer();
                return ret;
            }
        }
    } else {
        for (auto&& expr : this->iffalse) {
            auto ret = expr->interpret(st);
            if (expr->exprType == e_ret) {
                st->removeLayer();
                return ret;
            }
        }
    }
    st->removeLayer();
    return & None;
}

INTERPRET(ForExpr) {
    st->addLayer();
    this->init->interpret(st);
    auto cond_vt = this->cond->interpret(st);
    while (vt_is_true(cond_vt, this)) {
        for (auto&& expr : this->exprs) {
            auto ret = expr->interpret(st);
            if (return_flag) {
                st->removeLayer();
                return ret;
            }
            if (continue_flag || break_flag) {
                break;
            }
        }
        if (break_flag) {
            break;
        }
        this->step->interpret(st);
        cond_vt = this->cond->interpret(st);
    }
    st->removeLayer();
    return & None;
}

INTERPRET(WhileExpr) {
    st->addLayer();
    auto cond_vt = this->cond->interpret(st);
    while (vt_is_true(cond_vt, this)) {
        for (auto&& expr : this->exprs) {
            auto ret = expr->interpret(st);
            if (return_flag) {
                st->removeLayer();
                return ret;
            }
            if (continue_flag || break_flag) {
                break;
            }
        }
        if (break_flag) {
            break;
        }
        cond_vt = this->cond->interpret(st);
    }
    st->removeLayer();
    return & None;
}

INTERPRET(ExprVal) {
    if (this->isConst) {
        return ConstEval(this);
    }
    ValueType *vt;
    if (this->call != nullptr) {
        vt = this->call->interpret(st);
    } else {
        vt = st->lookup(this)->get();
    }
    return vt;
}

INTERPRET(RetExpr) {
    return_flag++;
    if (this->stmt == nullptr)
        return & None;
    return this->stmt->interpret(st);
}

INTERPRET(ContExpr) {
    continue_flag++;
    return nullptr;
}

INTERPRET(BreakExpr) {
    break_flag++;
    return nullptr;
}

INTERPRET(MatchExpr) {  // TODO: implementation
    return & None;
}

INTERPRET(MatchLine) {  // TODO : implementation
    return & None;
}

INTERPRET(EvalExpr) {
    if (this->isVal) {
        return this->val->interpret(st);
    }

    if ((this->op == move) || (this->op == copy) || (this->op == deepcopy)) {
        if (!this->l->isVal)
            throw InterpreterException("lvalue is not a variable", this);
        auto lvt = st->lookup(this->l->val.get())->get();
        if (lvt->isConst)
            throw InterpreterException("constant cannot be assigned", this);
        auto rvt = this->r->interpret(st);
        if (lvt->type != rvt->type)
            throw InterpreterException("type mismatch", this);

        if (this->op == move) {
            for (auto&& msi : rvt->ms) {
                msi->placehold = true;
                msi->set(nullptr);
                msi->placehold = false;
            }
            rvt->ms.clear();
            rvt->isConst = false;
            st->update(this->l->val.get(), rvt);
            return & None;
        }
        if (this->op == copy) {
            st->update(this->l->val.get(), rvt);
            return & None;
        }
    }

    auto lvt = this->l->interpret(st);
    auto rvt = this->r->interpret(st);
    if ((lvt->type.arrayT != 0) || (rvt->type.arrayT != 0))
        throw InterpreterException(terms[this->op] + " cannot operate on " + lvt->type.str(), this);
    if (lvt->type != rvt->type)
        throw InterpreterException("type mismatch", this);

    auto lv = *lvt;
    if (lvt->ms.size() == 0) {
        delete lvt;
        lvt = & lv;
    }
    auto rv = *rvt;
    if (rvt->ms.size() == 0) {
        delete rvt;
        rvt = & rv;
    }

    switch (this->op) {
    case add: {
        switch (lvt->type.baseType) {
            case t_uint8:
                return new ValueType(lvt->data.bval + rvt->data.bval);
            case t_int32:
                return new ValueType(lvt->data.ival + rvt->data.ival);
            case t_fp32:
                return new ValueType(lvt->data.fval + rvt->data.fval);
            case t_fp64:
                return new ValueType(lvt->data.dval + rvt->data.dval);
            default:
                throw InterpreterException(terms[this->op] + " cannot operate on " + lvt->type.str(), this);
        }
    }
    case sub: {
        switch (lvt->type.baseType) {
            case t_uint8:
                return new ValueType(lvt->data.bval - rvt->data.bval);
            case t_int32:
                return new ValueType(lvt->data.ival - rvt->data.ival);
            case t_fp32:
                return new ValueType(lvt->data.fval - rvt->data.fval);
            case t_fp64:
                return new ValueType(lvt->data.dval - rvt->data.dval);
            default:
                throw InterpreterException(terms[this->op] + " cannot operate on " + lvt->type.str(), this);
        }
    }
    case mul: {
        switch (lvt->type.baseType) {
            case t_uint8:
                return new ValueType(lvt->data.bval * rvt->data.bval);
            case t_int32:
                return new ValueType(lvt->data.ival * rvt->data.ival);
            case t_fp32:
                return new ValueType(lvt->data.fval * rvt->data.fval);
            case t_fp64:
                return new ValueType(lvt->data.dval * rvt->data.dval);
            default:
                throw InterpreterException(terms[this->op] + " cannot operate on " + lvt->type.str(), this);
        }
    }
    case t_div: {
        switch (lvt->type.baseType) {
            case t_uint8:
                return new ValueType(lvt->data.bval / rvt->data.bval);
            case t_int32:
                return new ValueType(lvt->data.ival / rvt->data.ival);
            case t_fp32:
                return new ValueType(lvt->data.fval / rvt->data.fval);
            case t_fp64:
                return new ValueType(lvt->data.dval / rvt->data.dval);
            default:
                throw InterpreterException(terms[this->op] + " cannot operate on " + lvt->type.str(), this);
        }
    }
    case rem: {
        switch (lvt->type.baseType) {
            case t_uint8:
                return new ValueType(lvt->data.bval % rvt->data.bval);
            case t_int32:
                return new ValueType(lvt->data.ival % rvt->data.ival);
            default:
                throw InterpreterException(terms[this->op] + " cannot operate on " + lvt->type.str(), this);
        }
    }
    case band: {
        switch (lvt->type.baseType) {
            case t_uint8:
                return new ValueType(lvt->data.bval & rvt->data.bval);
            case t_int32:
                return new ValueType(lvt->data.ival & rvt->data.ival);
            default:
                throw InterpreterException(terms[this->op] + " cannot operate on " + lvt->type.str(), this);
        }
    }
    case bor: {
        switch (lvt->type.baseType) {
            case t_uint8:
                return new ValueType(lvt->data.bval | rvt->data.bval);
            case t_int32:
                return new ValueType(lvt->data.ival | rvt->data.ival);
            default:
                throw InterpreterException(terms[this->op] + " cannot operate on " + lvt->type.str(), this);
        }
    }
    case bxor: {
        switch (lvt->type.baseType) {
            case t_uint8:
                return new ValueType(lvt->data.bval ^ rvt->data.bval);
            case t_int32:
                return new ValueType(lvt->data.ival ^ rvt->data.ival);
            default:
                throw InterpreterException(terms[this->op] + " cannot operate on " + lvt->type.str(), this);
        }
    }
    case equ: {
        switch (lvt->type.baseType) {
            case t_uint8:
                return new ValueType((bool)(lvt->data.bval == rvt->data.bval));
            case t_int32:
                return new ValueType((bool)(lvt->data.ival == rvt->data.ival));
            case t_fp32:
                return new ValueType((bool)(lvt->data.fval == rvt->data.fval));
            case t_fp64:
                return new ValueType((bool)(lvt->data.dval == rvt->data.dval));
            case t_char:
                return new ValueType((bool)(lvt->data.cval == rvt->data.cval));
            case t_str:
                return new ValueType((bool)((*lvt->data.str) == (*rvt->data.str)));
            default:
                throw InterpreterException(terms[this->op] + " cannot operate on " + lvt->type.str(), this);
        }
    }
    case neq: {
        switch (lvt->type.baseType) {
            case t_uint8:
                return new ValueType((bool)(lvt->data.bval != rvt->data.bval));
            case t_int32:
                return new ValueType((bool)(lvt->data.ival != rvt->data.ival));
            case t_fp32:
                return new ValueType((bool)(lvt->data.fval != rvt->data.fval));
            case t_fp64:
                return new ValueType((bool)(lvt->data.dval != rvt->data.dval));
            case t_char:
                return new ValueType((bool)(lvt->data.cval != rvt->data.cval));
            case t_str:
                return new ValueType((bool)((*lvt->data.str) != (*rvt->data.str)));
            default:
                throw InterpreterException(terms[this->op] + " cannot operate on " + lvt->type.str(), this);
        }
    }
    case lt: {
        switch (lvt->type.baseType) {
            case t_uint8:
                return new ValueType((bool)(lvt->data.bval < rvt->data.bval));
            case t_int32:
                return new ValueType((bool)(lvt->data.ival < rvt->data.ival));
            case t_fp32:
                return new ValueType((bool)(lvt->data.fval < rvt->data.fval));
            case t_fp64:
                return new ValueType((bool)(lvt->data.dval < rvt->data.dval));
            default:
                throw InterpreterException(terms[this->op] + " cannot operate on " + lvt->type.str(), this);
        }
    }
    case le: {
        switch (lvt->type.baseType) {
            case t_uint8:
                return new ValueType((bool)(lvt->data.bval <= rvt->data.bval));
            case t_int32:
                return new ValueType((bool)(lvt->data.ival <= rvt->data.ival));
            case t_fp32:
                return new ValueType((bool)(lvt->data.fval <= rvt->data.fval));
            case t_fp64:
                return new ValueType((bool)(lvt->data.dval <= rvt->data.dval));
            default:
                throw InterpreterException(terms[this->op] + " cannot operate on " + lvt->type.str(), this);
        }
    }
    case gt: {
        switch (lvt->type.baseType) {
            case t_uint8:
                return new ValueType((bool)(lvt->data.bval > rvt->data.bval));
            case t_int32:
                return new ValueType((bool)(lvt->data.ival > rvt->data.ival));
            case t_fp32:
                return new ValueType((bool)(lvt->data.fval > rvt->data.fval));
            case t_fp64:
                return new ValueType((bool)(lvt->data.dval > rvt->data.dval));
            default:
                throw InterpreterException(terms[this->op] + " cannot operate on " + lvt->type.str(), this);
        }
    }
    case ge: {
        switch (lvt->type.baseType) {
            case t_uint8:
                return new ValueType((bool)(lvt->data.bval >= rvt->data.bval));
            case t_int32:
                return new ValueType((bool)(lvt->data.ival >= rvt->data.ival));
            case t_fp32:
                return new ValueType((bool)(lvt->data.fval >= rvt->data.fval));
            case t_fp64:
                return new ValueType((bool)(lvt->data.dval >= rvt->data.dval));
            default:
                throw InterpreterException(terms[this->op] + " cannot operate on " + lvt->type.str(), this);
        }
    }
    case land: {
        switch (lvt->type.baseType) {
            case t_uint8:
                return new ValueType((bool)(lvt->data.bval && rvt->data.bval));
            case t_int32:
                return new ValueType((bool)(lvt->data.ival && rvt->data.ival));
            case t_fp32:
                return new ValueType((bool)(lvt->data.fval && rvt->data.fval));
            case t_fp64:
                return new ValueType((bool)(lvt->data.dval && rvt->data.dval));
            default:
                throw InterpreterException(terms[this->op] + " cannot operate on " + lvt->type.str(), this);
        }
    }
    case lor: {
        switch (lvt->type.baseType) {
            case t_uint8:
                return new ValueType((bool)(lvt->data.bval || rvt->data.bval));
            case t_int32:
                return new ValueType((bool)(lvt->data.ival || rvt->data.ival));
            case t_fp32:
                return new ValueType((bool)(lvt->data.fval || rvt->data.fval));
            case t_fp64:
                return new ValueType((bool)(lvt->data.dval || rvt->data.dval));
            default:
                throw InterpreterException(terms[this->op] + " cannot operate on " + lvt->type.str(), this);
        }
    }
    default:
        throw InterpreterException("unhandled operator " + terms[this->op], this);
    }
}
