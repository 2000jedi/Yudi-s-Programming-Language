#include "tree.hpp"

#include <string>
#include <iostream>

#include "lexical.hpp"
#include "ast.hpp"

// default constructor
template <class T>
Node<T>::Node(T x) {
  this->t = x;
  child.clear();
}

// tree_print_rec - recursive helper function to print out the tree
template <class T>
void tree_print_rec(Node<T> node, int indent) {
  int i;
  for (i = 0; i < indent; ++i) printf("  ");
  std::cout << node.t << std::endl;

  for (auto i = node.child.begin(); i != node.child.end(); ++i) {
    tree_print_rec(*i, indent + 1);
  }
}

// print - print out the tree in indented form
template <class T>
void Node<T>::print(void) {
  tree_print_rec(*this, 0);
}

template class Node<Lexical>;
