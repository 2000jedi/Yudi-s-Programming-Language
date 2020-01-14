#include <string>
#include <iostream>
#include "tree.hpp"

template <class T> 
Node<T>::Node(T x) {
  /**
   * default constructor
   */
  this->t = x;
  child.clear();
}

template <class T> 
void tree_print_rec(Node<T> node, int indent) {
  /**
   * tree_print_rec - recursive helper function to print out the tree
   */
  int i;
  for (i = 0; i < indent; ++i) printf("  ");
  std::cout << node.t << std::endl;

  for (auto i = node.child.begin(); i != node.child.end(); ++i) {
    tree_print_rec(*i, indent + 1);
  }
}

template <class T> 
void Node<T>::print(void) {
  /**
   * print - print out the tree in indented form
   */
  tree_print_rec(*this, 0);
}

template <class T>
void Node<T>::reduce(std::string catagory) {
  /**
   * reduce - reduce left-factored catagories to a string.
   */
}

template class Node<std::string>;
