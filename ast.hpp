#pragma once

#include <vector>
#include <string>
#include <iostream>

#include "tree.hpp"
#include "lexical.hpp"

class ASTtype {
public:
    std::string name;
    std::vector<ASTtype> child;
    
    ASTtype(void) {
        this->name = "";
        this->child.clear();
    }
   
    ASTtype(std::string name) {
        this->name = name;
    }

    void print(void);
};

extern std::ostream& operator<< (std::ostream &out, ASTtype const& data);

namespace AST {
    extern ASTtype generate(Node<Lexical> *root);
}
