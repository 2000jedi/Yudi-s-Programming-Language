#include "ast.hpp"
#include "lexical.hpp"
#include "tree.hpp"

#include <iostream>

std::ostream& operator<< (std::ostream &out, ASTtype const& data) {
    out << data.name;
    return out;
}

void ASTtype::print(void) {
    return;
}

ASTtype recursive_gen(Node<Lexical> curr) {
    if (curr.t.name == "<STATEMENTS>") {
        ASTtype parent(":statements:");
        parent.child.push_back(recursive_gen(curr.child[0]));
        while (curr.child.size() > 1) {
            curr = curr.child[1];
            parent.child.push_back(recursive_gen(curr.child[0]));
        }
        return parent;
    }
    return ASTtype("");
}

ASTtype AST::generate(Node<Lexical> root) {
    return recursive_gen(root);
}
