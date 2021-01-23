/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#pragma once

#include <iostream>
#include <string>

#include "ast.hpp"

extern AST::ValueType *runtime_handler(AST::Name fn, AST::FuncCall *call, AST::SymTable *st);
extern AST::ValueType *runtime_enum_handler(AST::ValueType *vt, AST::FuncCall *call, AST::SymTable *st);
extern void runtime_bind(AST::SymTable *st);
