#include "ast.hpp"

#include <memory>

#include "ast_gen.hpp"
#include "util.hpp"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

using namespace AST;

#ifdef DEBUG

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
    for (auto p : this->stmts) p->print(indent + 1);
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
    for (auto p : this->pars) p->print(indent + 1);
}

void ExprVal::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    if (this->isConst) {
        std::cout << "ConstVal(" << this->constVal << ')' << std::endl;
    } else {
        std::cout << "ExprVal(" << this->refName.str() << ')' << std::endl;
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
    std::cout << "VarDecl(" << this->name.str() << ')' << std::endl;
    if (this->type)
        this->type->print(indent + 1);
    if (this->init)
        this->init->print(indent + 1);
}

void FuncDecl::print(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
    std::cout << "FuncDecl(" << this->name.str() << ')' << std::endl;
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
    std::cout << "ClassDecl(" << this->name.str() << ')' << std::endl;
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
    std::cout << "EnumDecl(" << this->name.str() << ')' << std::endl;
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

#endif

// Code Generation Global Variables
static std::map<std::string, AST::ValueType *> g_str_lits;
static std::map<AST::NameSpace, AST::FuncDecl *> g_func_decls;
static std::map<AST::NameSpace, AST::ClassDecl *> g_class_decls;
static std::map<AST::NameSpace, AST::EnumDecl *> g_enum_decls;
static std::map<std::string, llvm::StructType *> g_struct_decls;
extern std::vector<std::string> *g_strings;

// Symble Table - record Variable and Type Information
// TODO: type inference. Arrays.
static std::vector<
    std::unique_ptr<std::map<AST::NameSpace, AST::ValueType*>>> SymTable;

/**
 * Code Generation Interface.
 * codeGen() methods to convert to llvm IR form.
 */

void ClearSymLayer(void) {
    SymTable.clear();
}

void NewSymLayer(void) {
    SymTable.push_back(
        std::make_unique<std::map<NameSpace, ValueType *>>());
}

void RemoveSymLayer(void) {
    SymTable.pop_back();
}

void InsertVar(NameSpace name, llvm::Value *v, TypeDecl *t) {
    (*SymTable.back())[name] = new ValueType(v, t);
}

void InsertConst(NameSpace name, llvm::Value *v, TypeDecl *t) {
    (*SymTable.back())[name] = new ValueType(v, t, true);
}

ValueType *FindTopVar(NameSpace name) {
    if (SymTable.back()->find(name) != SymTable.back()->end()) {
        return (*SymTable.back())[name];
    } else {
        return nullptr;
    }
}

ValueType *FindVarRaw(NameSpace name) {
    for (int i = SymTable.size() - 1; i >= 0; i--) {
        if (SymTable[i]->find(name) != SymTable[i]->end()) {
            return (*SymTable[i])[name];
        }
    }

    return nullptr;
}

llvm::Value *InsertClassElemRef(ClassDecl *cl, NameSpace var) {
    int i = 0;
    for (auto v : cl->var_members) {
        if (v->name.BaseName == var.BaseName)
            break;
        i++;
    }

    llvm::Value *val = builder.CreateInBoundsGEP(
        FindVarRaw(NameSpace("", var.ClassName))->val,
        builder.getInt32(i),
        "");
    InsertVar(
        var,
        val,
        cl->var_members[i]->type);
    return val;
}

ValueType *FindVar(NameSpace name) {
    ValueType *vt;
    if (name.ClassName == "") {
        vt = FindVarRaw(name);
        if (!vt) {
            LogError("variable " << name.str()
                << " not found");
            return nullptr;
        }
    } else {
        vt = FindVarRaw(name);
        if (!vt) {
            ValueType *cl = FindVarRaw(NameSpace("", name.ClassName));
            InsertClassElemRef(
                g_class_decls[NameSpace(cl->type->other, "")],
                name);
            vt = FindVarRaw(name);
            if (!vt) {
                LogError("variable " << name.str()
                    << " not found");
                return nullptr;
            }
        }
    }

    return vt;
}

llvm::Type *type_trans(TypeDecl *td, bool pointer = true) {
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
        case TypeDecl::OTHER: {
            auto t = g_struct_decls[td->other];
            if (t) {
                if (pointer)
                    return llvm::PointerType::get(t, 0);
                else
                    return t;
            } else {
                LogError("Type " << td->other << " not found");
                std::abort();
                return nullptr;
            }
        }
        default:
            LogError("TypeDecl index " << td->baseType << " not found");
            return nullptr;
    }
    // TODO: array typing
}

llvm::Function *FuncASTDef(AST::FuncDecl *f) {
    g_func_decls[f->name] = f;

    llvm::Function *prev = module->getFunction(f->name.str());
    if (prev) {
        LogError("function " << f->name.ClassName << "."
            << f->name.BaseName << " already declared");
        return nullptr;
    }
    if (f->name.ClassName != "") {
        f->pars.insert(f->pars.begin(),
            new Param("this", new TypeDecl(f->name.ClassName, "0")));
    }

    std::vector<llvm::Type *> ArgTypes;
    for (auto arg : f->pars) {
        ArgTypes.push_back(type_trans(arg->type));
    }
    llvm::FunctionType *Ft = llvm::FunctionType::get(
        type_trans(f->ret), ArgTypes, false);
    llvm::Function *F = llvm::Function::Create(
        Ft, llvm::Function::ExternalLinkage, f->name.str(), module.get());

    unsigned Idx = 0;

    for (auto &Arg : F->args()) {
        Arg.setName(f->pars[Idx++]->name);
    }

    return F;
}

// Variable Allocation Helper Function
static llvm::AllocaInst *CreateEntryBlockAlloca(
    llvm::Function *F, const std::string &name, llvm::Type *type, int size) {
    llvm::IRBuilder<> tmpBuilder(
        &F->getEntryBlock(), F->getEntryBlock().begin());

    llvm::Value *arr = 0;
    if (size)
        arr = llvm::ConstantInt::get(context, llvm::APInt(32, size, true));

    return tmpBuilder.CreateAlloca(type, arr, name.c_str());
}

ValueType *BaseAST::codegen() {
    LogError("BaseAST() cannot generate IR code.");
    return nullptr;
}

ValueType *ASTs::codegen() {
    LogError("ASTs() cannot generate IR code.");
    return nullptr;
}

ValueType *ConstToValue(ExprVal *e) {
    switch (e->type->baseType) {
        case TypeDecl::VOID:
            LogError("no constant type \"void\"");
            return nullptr;
        case TypeDecl::INT32:
            return new ValueType(
                llvm::ConstantInt::get(context,
                llvm::APInt(32, std::stoi(e->constVal), true)),
                e->type, true);
        case TypeDecl::CHAR:
            return new ValueType(
                llvm::ConstantInt::get(context,
                llvm::APInt(8, std::stoi(e->constVal), false)),
                e->type, true);
        case TypeDecl::FP32:
        case TypeDecl::FP64:
            return new ValueType(
                llvm::ConstantFP::get(context,
                llvm::APFloat(std::stod(e->constVal))),
                e->type, true);
        case TypeDecl::STRING: {
            return g_str_lits[e->constVal];
        }
        default:
            LogError("TypeDecl index " << e->type->baseType << " not found");
            return nullptr;
    }
}

ValueType *ExprVal::codegen() {
    if (this->isConst) {
        return ConstToValue(this);
    }

    if (this->call) {
        NameSpace called_function = this->refName;
        bool is_class = false;
        unsigned num_args = this->call->pars.size();
        if (called_function.ClassName != "") {
            called_function.ClassName =
                FindVar(NameSpace(called_function.ClassName))->type->other;
            num_args++;
            is_class = true;
        }

        llvm::Function *F = module->getFunction(called_function.str());
        if (!F) {
            LogError("function " << called_function.str() <<" is not defined");
            return nullptr;
        }

        if ((!F->isVarArg()) && (F->arg_size() != num_args)) {
            LogError(
                "function " << called_function.str() <<" requires "
                << F->arg_size() << " argument(s), " << this->call->pars.size()
                << " argument(s) found");
            return nullptr;
        }

        std::vector<llvm::Value *> args;

        if (is_class) {
            args.push_back(FindVar(NameSpace(this->refName.ClassName))->val);
        }

        for (auto p : this->call->pars) {
            ValueType *arg = p->codegen();
            // TODO: check argument type match
            args.push_back(arg->val);
            if (!args.back()) {
                LogError("no return received from argument");
                return nullptr;
            }
        }
        // TODO: receive function return type
        if (F->getName() == "printf") {
            return new ValueType(builder.CreateCall(F, args, ""),
                new TypeDecl(TypeDecl::VOID, "0"));
        }
        return new ValueType(builder.CreateCall(F, args, ""),
            g_func_decls[called_function]->ret);
    }

    ValueType* vt = FindVar(this->refName);
    if (!vt) {
        return nullptr;
    }
    llvm::Value *v = vt->val;

    if (this->array) {
        ValueType *index = this->array->codegen();
        if (!index) {
            LogError("invalid array index");
            return nullptr;
        }
        if (index->type->baseType != TypeDecl::INT32) {
            LogError("invalid array index type");
            return nullptr;
        }
        TypeDecl *singletonType = new TypeDecl(vt->type->baseType, "0");
        return new ValueType(
            new llvm::LoadInst(
                builder.CreateGEP(v, index->val, ""),
                "", builder.GetInsertBlock()),
            singletonType);
    } else {
        return new ValueType(
            new llvm::LoadInst(v, "", builder.GetInsertBlock()),
            vt->type);
    }
}

ValueType *EvalExpr::codegen() {
    if (this->isVal)
        return this->val->codegen();

    if (this->op == "=") {
        if (!this->l->isVal) {
            LogError("LHS of assignment must be a variable");
            return nullptr;
        }
        if (this->l->val->isConst)  {
            LogError("constant cannot be assigned");
            return nullptr;
        }
        if (this->l->val->call) {
            LogError(
                "function return value cannot be used in LHS of assignment");
            return nullptr;
        }

        ValueType *vt = FindVar(this->l->val->refName);
        if (!vt) {
            return nullptr;
        }

        if (this->l->val->array) {
            ValueType *index = this->l->val->array->codegen();
            if (!index) {
                LogError("invalid array index");
                return nullptr;
            }
            if (index->type->baseType != TypeDecl::INT32) {
                LogError("invalid array index type");
                return nullptr;
            }
            vt = new ValueType(
                builder.CreateGEP(vt->val, index->val, ""),
                new TypeDecl(vt->type->baseType, "0"));
        }

        ValueType *rVT = this->r->codegen();
        if (!vt->type->eq(rVT->type)) {
            LogError("assignment has different types" << std::endl << "  L: " <<
                vt->type->baseType << ", R: " << rVT->type->baseType);
            return nullptr;
        }
        builder.CreateStore(rVT->val, vt->val, false);
        return new ValueType(
            new llvm::LoadInst(vt->val, "", builder.GetInsertBlock()),
            vt->type);
    }

    ValueType *lv = this->l->codegen();
    ValueType *rv = this->r->codegen();

    if (!lv || !rv) {
        LogError("operator " << this->op << " must accept binary inputs");
        return nullptr;
    }

    if (lv->type->arrayT == 0) {  // not an array type
        if (!lv->type->eq(rv->type)) {
            LogError("assignment has different types" << std::endl << "  L: " <<
                lv->type->baseType << ", R: " << rv->type->baseType);
            return nullptr;
        }
        switch (lv->type->baseType) {
            case TypeDecl::INT32: {
                if (this->op == "ADD") {
                    return new ValueType(
                        builder.CreateAdd(lv->val, rv->val, ""),
                        lv->type);
                }
                if (this->op == "SUB") {
                    return new ValueType(
                        builder.CreateSub(lv->val, rv->val, ""),
                        lv->type);
                }
                if (this->op == "MUL") {
                    return new ValueType(
                        builder.CreateMul(lv->val, rv->val, ""),
                        lv->type);
                }
                if (this->op == "DIV") {
                    return new ValueType(
                        builder.CreateSDiv(lv->val, rv->val, ""),
                        lv->type);
                }
                if (this->op == "GT") {
                    return new ValueType(
                        builder.CreateICmpSGT(lv->val, rv->val, ""),
                        lv->type);
                }
                if (this->op == "LT") {
                    return new ValueType(
                        builder.CreateICmpSLT(lv->val, rv->val, ""),
                        lv->type);
                }
                if (this->op == "GE") {
                    return new ValueType(
                        builder.CreateICmpSGE(lv->val, rv->val, ""),
                        lv->type);
                }
                if (this->op == "LE") {
                    return new ValueType(
                        builder.CreateICmpSLE(lv->val, rv->val, ""),
                        lv->type);
                }
                if (this->op == "EQ") {
                    return new ValueType(
                        builder.CreateICmpEQ(lv->val, rv->val, ""),
                        lv->type);
                }
                if (this->op == "NEQ") {
                    return new ValueType(
                        builder.CreateICmpNE(lv->val, rv->val, ""),
                        lv->type);
                }
                LogError("operator " << this->op << " not supported");
                return nullptr;
            }
            default: {
                LogError("type " << lv->type << " not supported");
                return nullptr;
            }
        }
    } else {
        LogError("array arithmetics still not supported");
        return nullptr;
    }

    LogError("operator " << this->op << " under construction");
    return nullptr;
}

ValueType *FuncDecl::codegen() {
    llvm::Function *F = module->getFunction(this->name.str());
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(
        context, this->name.str() + "_entry", F);
    builder.SetInsertPoint(BB);

    NewSymLayer();

    unsigned int Idx = 0;
    for (auto &Arg : F->args()) {
        llvm::AllocaInst *alloca = CreateEntryBlockAlloca(
            F, "", type_trans(this->pars[Idx]->type),
            this->pars[Idx]->type->arrayT);
        builder.CreateStore(&Arg, alloca);
        InsertVar(NameSpace(Arg.getName()), alloca, this->pars[Idx]->type);
        Idx++;
    }

    if (this->name.ClassName != "") {
        auto cl = g_class_decls[NameSpace(this->name.ClassName, "")];
        for (unsigned i = 0; i < cl->var_members.size(); ++i) {
            llvm::Value *ld = builder.CreateLoad(
                FindVar(NameSpace("", "this"))->val,
                "");
            llvm::Value *val = builder.CreateInBoundsGEP(ld,
                {builder.getInt32(i), builder.getInt32(0)},
                "");
            InsertVar(
                NameSpace("", cl->var_members[i]->name.BaseName),
                val, cl->var_members[i]->type);
        }
    }

    for (auto p : this->exprs) {
        p->codegen();
    }

    if (F->getReturnType()->isVoidTy()) {
        builder.CreateRetVoid();
    }

    RemoveSymLayer();

    llvm::verifyFunction(*F);
    return nullptr;
}

ValueType *ClassDecl::codegen() {
    std::vector<llvm::Type *> members;  // struct members
    std::vector<VarDecl *> member_raw;

    for (auto statement : this->stmts) {
        if (statement->stmtType == GlobalStatement::VARDECL) {
            // declare variable in the struct
            auto pp = dynamic_cast<VarDecl *>(statement);
            members.push_back(type_trans(pp->type));
            member_raw.push_back(pp);
        }
    }
    g_struct_decls[this->name.ClassName]->setBody(members);
    g_class_decls[this->name]->var_members = member_raw;

    for (auto statement : this->stmts) {
        if (statement->stmtType != GlobalStatement::VARDECL) {
            statement->codegen();
        }
    }

    return nullptr;
}

ValueType *EnumDecl::codegen() {
    g_enum_decls[this->name] = this;

    // Create struct type for the enum
    llvm::StructType *cur_enum = llvm::StructType::create(
        context, this->name.str() + "_enum");

    cur_enum->setBody({
        llvm::Type::getInt8Ty(context),    // index of enum choice
        llvm::Type::getInt8PtrTy(context)  // void pointer to data field
    });
    g_struct_decls[this->name.str()] = cur_enum;

    // Create struct types for data field
    for (auto option : this->options) {
        if (option->pars.size() == 0) {
            continue;
        }

        NameSpace curr_ns = NameSpace(&this->name, option->name);
        llvm::StructType *cur_option = llvm::StructType::create(
            context, curr_ns.str() + "_option");

        std::vector<llvm::Type *> cur_option_fields;
        for (auto field : option->pars) {
            cur_option_fields.push_back(type_trans(field->type));
        }
        cur_option->setBody(cur_option_fields);

        g_struct_decls[curr_ns.str() + "_option"] = cur_option;

        // create constructors for each enum
        FuncDecl *fd = new FuncDecl(
            option->name,
            nullptr,
            new TypeDecl(this->name.str(), "0"),
            nullptr);
        fd->name.BaseName = this->name.BaseName;

        for (auto field : option->pars) {
            fd->pars.push_back(field);
        }

        llvm::Function *f = FuncASTDef(fd);
        llvm::BasicBlock *BB = llvm::BasicBlock::Create(
            context, this->name.str() + "_entry", f);
        builder.SetInsertPoint(BB);

        auto alloca = CreateEntryBlockAlloca(f, "this", cur_option, 0);

        int Idx = 0;
        for (auto &Arg : f->args()) {
            llvm::Value *param = builder.CreateInBoundsGEP(alloca,
                {builder.getInt32(0), builder.getInt32(Idx)},
                "");
            builder.CreateStore(&Arg, param);
            Idx++;
        }
        builder.CreateRet(alloca);
    }

    return nullptr;
}

ValueType *ForExpr::codegen() {
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

    ValueType *condV = this->cond->codegen();
    if (!condV->type->eq(new TypeDecl(TypeDecl::INT32, "0"))) {
        LogError("type " << condV->type->baseType
            << " cannot be used in condition");
        return nullptr;
    }
    if (condV) {
        llvm::Value *condB = builder.CreateICmpNE(
            condV->val,
            llvm::ConstantInt::get(context, llvm::APInt(1, 0, true)),
            "");
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

ValueType *FuncCall::codegen() {
    return nullptr;
}

ValueType *GenericDecl::codegen() {
    return nullptr;
}

ValueType *IfExpr::codegen() {
    ValueType *condV = this->cond->codegen();
    if (!condV) {
        return nullptr;
    }
    if (!condV->type->eq(new TypeDecl(TypeDecl::INT32, "0"))) {
        LogError("type " << condV->type->baseType
            << " cannot be used in condition");
        return nullptr;
    }
    llvm::Value *condE = builder.CreateICmpNE(
        condV->val,
        llvm::ConstantInt::get(context, llvm::APInt(1, 0, true)),
        "");

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
    builder.CreateCondBr(condE, trueBB, falseBB);

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

ValueType *MatchExpr::codegen() {
    return nullptr;
}

ValueType *MatchLine::codegen() {
    return nullptr;
}

ValueType *Option::codegen() {
    return nullptr;
}

ValueType *Param::codegen() {
    return nullptr;
}

ValueType *RetExpr::codegen() {
    if (!this->stmt) {
        builder.CreateRetVoid();
        return nullptr;
    }

    ValueType *r = this->stmt->codegen();

    if (r) {
        builder.CreateRet(r->val);
    }

    return nullptr;
}

ValueType *TypeDecl::codegen() {
    LogError("TypeDecl() cannot generate code");
    return nullptr;
}

ValueType *VarDecl::codegen() {
    if (this->is_global) {
        module->getOrInsertGlobal(
            this->name.str(),
            type_trans(this->type));
        llvm::GlobalVariable *v = module->getNamedGlobal(this->name.str());
        v->setLinkage(llvm::GlobalValue::CommonLinkage);

        if (this->init) {
            LogError("Global Initializer is not supported");
        }
        if (this->is_const) {
            return nullptr;
        } else {
            InsertVar(this->name, v, this->type);
        }
        return nullptr;
    }

    if (this->is_const && (!this->init)) {
        LogError("constant must have initializers.");
        return nullptr;
    }

    if (FindTopVar(this->name)) {
        LogError("variable " << this->name.str() << " already declared");
        return nullptr;
    }

    llvm::Function *curF = builder.GetInsertBlock()->getParent();

    ValueType *v = nullptr;
    if (this->init) {
        v = this->init->codegen();
        if (!v) {
            LogError("initialization failure of variable " << this->name.str());
            return nullptr;
        }
        if (!this->type) {
            this->type = v->type;
        }
        if (!v->type->eq(this->type)) {
            LogError("initialization type mismatch");
            return nullptr;
        }
    }

    llvm::AllocaInst *alloca = CreateEntryBlockAlloca(
        curF, this->name.str(),
        type_trans(this->type, false), this->type->arrayT);
    if (this->init) {
        builder.CreateStore(v->val, alloca);
    }

    if (this->is_const) {
        InsertConst(this->name, alloca, v->type);
    } else {
        InsertVar(this->name, alloca, this->type);
    }

    return nullptr;
}

ValueType *WhileExpr::codegen() {
    llvm::Function *parent = builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *condBB = llvm::BasicBlock::Create(
        context, "lc", parent);
    llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(
        context, "l", parent);
    llvm::BasicBlock *finalBB = llvm::BasicBlock::Create(
        context, "lf", parent);

    builder.CreateBr(condBB);
    builder.SetInsertPoint(condBB);

    ValueType *condV = this->cond->codegen();
    llvm::Value *condE;
    if (condV) {
        condE = builder.CreateICmpNE(
            condV->val,
            llvm::ConstantInt::get(context, llvm::APInt(1, 0, true)),
            "");
    }
    builder.CreateCondBr(condE, loopBB, finalBB);
    builder.SetInsertPoint(loopBB);

    for (auto p : this->exprs) {
        p->codegen();
    }
    builder.CreateBr(condBB);
    builder.SetInsertPoint(finalBB);

    return nullptr;
}

ValueType *Program::codegen() {
    // Record functions and classes prior to code generation
    for (auto p : this->stmts) {
        if (p->stmtType == GlobalStatement::FUNCDECL) {
            auto pp = dynamic_cast<FuncDecl *>(p);
            FuncASTDef(pp);
        }
        if (p->stmtType == GlobalStatement::CLASSDECL) {
            auto cl = dynamic_cast<ClassDecl *>(p);
            g_class_decls[cl->name] = cl;

            // Create struct type for the class
            llvm::StructType *cur_class = llvm::StructType::create(
                context, cl->name.ClassName + "_struct");
            g_struct_decls[cl->name.ClassName] = cur_class;

            // Declare member functions
            for (auto statement : cl->stmts) {
                if (statement->stmtType == GlobalStatement::FUNCDECL) {
                    auto pp = dynamic_cast<FuncDecl *>(statement);
                    FuncASTDef(pp);
                }
            }
        }
    }

    ClearSymLayer();
    NewSymLayer();  // Global Variable Layer

    for (auto p : this->stmts) {
        if (p->stmtType == GlobalStatement::VARDECL)
            p->codegen();
    }

    // Insert Global String Definitions.
    llvm::FunctionType *Ft = llvm::FunctionType::get(
        builder.getVoidTy(), {}, false);
    llvm::Function *F = llvm::Function::Create(
        Ft, llvm::Function::ExternalLinkage, "globals", module.get());
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(
        context, "_globals", F);
    builder.SetInsertPoint(BB);
    for (auto p : *g_strings) {
        if (g_str_lits.find(p) == g_str_lits.end()) {
            llvm::Value *v = builder.CreateGlobalStringPtr(
                llvm::StringRef(p));
            g_str_lits[p] = new ValueType(
                v, new TypeDecl(TypeDecl::STRING, "0"));
        }
    }
    g_strings->clear();

    builder.CreateRetVoid();

    // Code Generation Entrance
    for (auto p : this->stmts) {
        if (p->stmtType != GlobalStatement::VARDECL)
            p->codegen();
    }
    return nullptr;
}

int AST::codegen(Program prog, std::string outputFileName) {
    module = llvm::make_unique<llvm::Module>(outputFileName, context);

    // insert standard C library functions
    module->getOrInsertFunction(
        "printf",
        llvm::FunctionType::get(
            llvm::IntegerType::getInt32Ty(context),
            llvm::PointerType::get(llvm::Type::getInt8Ty(context), 0),
            true));  // printf is var arg func type

    // Generate IR code
    prog.codegen();
    module->print(llvm::errs(), nullptr);

    // Initialize Targets
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
    auto RM = llvm::Optional<llvm::Reloc::Model>(llvm::Reloc::Model::PIC_);
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

    // Pass Manager and Object File Output
    llvm::legacy::PassManager pass;
    auto FileType = llvm::TargetMachine::CGFT_ObjectFile;

    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        LogError("TargetMachine can't emit a file of this type");
        return ERR_OBJCODE;
    }

    pass.run(*module);
    dest.flush();
    return 0;
}
