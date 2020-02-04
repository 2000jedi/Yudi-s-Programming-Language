#include "ast.hpp"
#include "lexical.hpp"
#include "tree.hpp"

#include <iostream>
#include <string>

std::ostream& operator<< (std::ostream &out, ASTtype const& data) {
    out << data.name;
    return out;
}

void print_rec(std::string indent, ASTtype curr) {
  std::cout << indent << curr << std::endl;
  for (auto i : curr.child) {
    print_rec(indent + "  ", i);
  }
}

void ASTtype::print(void) {
  print_rec("", *this);
}

ASTtype recursive_gen(Node<Lexical> curr) {
    if (curr.t.name == "<EPS>") {
      return ASTtype(":null:");
    }
    if (curr.t.name == "<STATEMENTS>") {
        ASTtype parent(":statements:");
        parent.child.push_back(recursive_gen(curr.child[0]));
        while (curr.child.size() > 1) {
            curr = curr.child[1];
            parent.child.push_back(recursive_gen(curr.child[0]));
        }
        return parent;
    }

    if (curr.t.name == "<CLASS_DOMAIN>") {
        ASTtype parent(":statements:");
        parent.child.push_back(recursive_gen(curr.child[0]));
        while (curr.child.size() > 1) {
            curr = curr.child[1];
            parent.child.push_back(recursive_gen(curr.child[0]));
        }
        return parent;
    }

    if (curr.t.name == "<STATEMENT>") {
        return recursive_gen(curr.child[0]);
    }

    if (curr.t.name == "<CONSTDEF>") {
        ASTtype parent(":const_def:");
        ASTtype const_name(":name:");
        const_name.child.push_back(curr.child[1].t.data);
        parent.child.push_back(const_name);
        parent.child.push_back(recursive_gen(curr.child[3])); // initializer
        return parent;
    }

    if (curr.t.name == "<VARDEF>") {
        ASTtype parent(":var_def:");
        ASTtype var_name(":name:");
        ASTtype var_name_inside(curr.child[1].t.data);
        var_name.child.push_back(var_name_inside);
        parent.child.push_back(var_name);
        parent.child.push_back(recursive_gen(curr.child[3])); // type of the var
        parent.child.push_back(recursive_gen(curr.child[4])); // initializer
        return parent;
    }

    if (curr.t.name == "<TYPENAME>") {
        ASTtype parent(":type:");
        if (curr.child[0].t.name == "NAME") {
            parent.child.push_back(ASTtype(":struct:"));
            parent.child[0].child.push_back(curr.child[0].t.data);
        } else {
            parent.child.push_back(curr.child[0].t.name);
        }

        if (curr.child[1].child[0].t.name == "<EPS>") {
            parent.child.push_back(ASTtype(":null:"));
        } else {
            parent.child.push_back(ASTtype(curr.child[1].child[1].t.data));
        }
        return parent;
    }

    if (curr.t.name == "<OPTIONAL_INIT>") {
        ASTtype parent(":init:");
        if (curr.child[0].t.name == "<EPS>")
            parent.child.push_back(ASTtype(":null:"));
        else {
            parent.child.push_back(recursive_gen(curr.child[1]));
        }
        return parent;
    }

    if (curr.t.name == "<FUNCDEF>") {
        ASTtype parent(":func_def:");
        ASTtype name(":name:");
        name.child.push_back(curr.child[1].t.data);
        parent.child.push_back(name);
        parent.child.push_back(recursive_gen(curr.child[2])); // generic
        parent.child.push_back(recursive_gen(curr.child[4])); // params
        parent.child.push_back(recursive_gen(curr.child[6])); // optional return
        parent.child.push_back(recursive_gen(curr.child[8])); // EXPRS
        return parent;
    }

    if (curr.t.name == "<OPTIONAL_GENERIC_DEF>") {
        ASTtype parent(":generic_defs:");
        if (curr.child[0].t.name == "<EPS>") {
            parent.child.push_back(ASTtype(":null:"));
        } else {
            parent.child.push_back(ASTtype(curr.child[1].t.data));
        }

        return parent;
    }

    if (curr.t.name == "<OPTIONAL_PARAMS_DEF>") {
        if (curr.child[0].t.name == "<EPS>") {
            return ASTtype(":null:");
        } else {
            return recursive_gen(curr.child[1]);
        }
    }

    if (curr.t.name == "<PARAMS_DEF>") {
        ASTtype parent(":params:");
        if (curr.child[0].t.name == "<EPS>") {
          parent.child.push_back(ASTtype(":null:"));
          return parent;
        }

        parent.child.push_back(recursive_gen(curr.child[0]));
        curr = curr.child[1];
        while (curr.child[0].t.name != "<EPS>") {
            parent.child.push_back(recursive_gen(curr.child[1]));
            curr = curr.child[2];
        }
        return parent;
    }

    if (curr.t.name == "<PARAM>") {
        ASTtype parent(":param:");
        ASTtype name(":name:");
        name.child.push_back(ASTtype(curr.child[0].t.data));
        parent.child.push_back(name);
        parent.child.push_back(recursive_gen(curr.child[2]));
        return parent;
    }

    if (curr.t.name == "<RETURN_DEF>") {
        ASTtype parent(":return_type:");
        if (curr.child[0].t.name == "<EPS>") {
            parent.child.push_back(ASTtype(":null:"));
            return parent;
        } else {
            parent.child.push_back(recursive_gen(curr.child[1]));
            return parent;
        }
    }

    if (curr.t.name == "<ENUMDEF>") {
        ASTtype parent(":enum_def:");
        ASTtype name(":name:");
        name.child.push_back(ASTtype(curr.child[1].t.data));
        parent.child.push_back(name);
        parent.child.push_back(recursive_gen(curr.child[2])); // generic
        parent.child.push_back(recursive_gen(curr.child[4])); // options
        return parent;
    }

    if (curr.t.name == "<OPTIONS>") {
        ASTtype parent(":options:");
        if (curr.child[0].t.name == "<EPS>") {
            parent.child.push_back(ASTtype(":null:"));
            return parent;
        }

        ASTtype option(":option:");
        option.child.push_back(ASTtype(curr.child[0].t.data));
        option.child.push_back(recursive_gen(curr.child[1]));
        parent.child.push_back(option);
        curr = curr.child[2];
        
        while (curr.child[0].t.name != "<EPS>") {
            ASTtype option(":option:");
            option.child.push_back(ASTtype(curr.child[1].t.data));
            option.child.push_back(recursive_gen(curr.child[2]));
            parent.child.push_back(option);
            curr = curr.child[3];
        }
        return parent;
    }

    if (curr.t.name == "<CLASSDEF>") {
        ASTtype parent(":class_def:");
        ASTtype name(":name:");
        name.child.push_back(ASTtype(curr.child[1].t.data));
        parent.child.push_back(name);
        parent.child.push_back(recursive_gen(curr.child[2])); // generic
        parent.child.push_back(recursive_gen(curr.child[4])); // domain
        return parent;
    }

    if (curr.t.name == "<EXPRS>") {
        ASTtype parent(":exprs:");
        parent.child.push_back(recursive_gen(curr.child[0]));
        while (curr.child.size() > 1) {
            curr = curr.child[1];
            parent.child.push_back(recursive_gen(curr.child[0]));
        }
        return parent;
    }

    if (curr.t.name == "<EXPR>")
        return recursive_gen(curr.child[0]);

    if (curr.t.name == "<EMPTY_EXPR>")
        return ASTtype(":null:");

    if (curr.t.name == "<RET_EXPR>") {
        ASTtype parent(":return:");
        parent.child.push_back(recursive_gen(curr.child[1]));
        return parent;
    }

    if (curr.t.name == "<IF_EXPR>") {
        ASTtype parent(":if:");
        ASTtype cond(":cond:");
        ASTtype iftrue(":iftrue:");
        ASTtype iffalse(":iffalse:");
        cond.child.push_back(recursive_gen(curr.child[2]));
        iftrue.child.push_back(recursive_gen(curr.child[5]));
        iffalse.child.push_back(recursive_gen(curr.child[7]));
        parent.child.push_back(cond);
        parent.child.push_back(iftrue);
        parent.child.push_back(iffalse);

        return parent;
    }

    if (curr.t.name == "<OPTIONAL_ELSE_EXPR>") {
        if (curr.child[0].t.name == "<EPS>")
            return ASTtype(":null:");
        else {
            return recursive_gen(curr.child[2]);
        }
    }

    if (curr.t.name == "<WHILE_EXPR>") {
        ASTtype parent(":while:");
        ASTtype cond(":cond:");
        cond.child.push_back(recursive_gen(curr.child[2]));
        parent.child.push_back(cond);
        return parent;
    }

    return ASTtype("in progress: " + curr.t.name);
}

ASTtype AST::generate(Node<Lexical> root) {
    return recursive_gen(root);
}
