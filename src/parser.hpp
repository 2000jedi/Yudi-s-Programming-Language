/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#pragma once

#include <string>
#include <memory>

#include "scanner.hpp"
#include "ast.hpp"

extern std::unique_ptr<AST::Program> parse(scanner);
