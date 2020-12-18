/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#pragma once

#include <iostream>

#include "ast.hpp"

extern AST::ValueType *runtime_handler(std::string fn, AST::FuncCall *call, AST::SymTable *st);
