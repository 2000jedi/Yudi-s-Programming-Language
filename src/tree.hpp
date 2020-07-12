#pragma once

#include <string>
#include <vector>

template <class T> 
class Node {
public:
  T t;
  std::vector<Node<T>> child;

  Node(T x);
  void print(void);
  void reduce(std::string catagory);
};
