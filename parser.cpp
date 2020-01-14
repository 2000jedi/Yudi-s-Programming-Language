/*
 * Table Driver Parser
 */

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <algorithm>
#include <cstring>
#include "tree.hpp"
#include "parser.hpp"

#define MAX_RULE 500
#define BUFFER 255

int start_rule;
std::vector<std::string> rule_table;
std::vector<catagory> rules;
int selection[MAX_RULE][255];

void print_rules() {
  /** debug use only
   * print_rules - print all catagory rules out
   */
  #ifdef DEBUG_RULES
  for (unsigned int i = 0; i < rules.size(); ++i) {
    std::cout << rules[i].name << " ";
    for (unsigned int j = 0; j < rules[i].reps.size() ; ++j)
      std::cout << rules[i].reps[j] << ' ';
    std::cout << std::endl;
  }
  #endif
}

int find_rule(std::string rule) {
  /**
   * find_rule - find the integer representation of non-terminal
   * 
   * @inputs
   * rule - the destiny rule to find
   */
  int i;
  for (i = 0; rule_table[i][0] != 0; ++i) {
    if (rule_table[i] == rule) {
      return i;
    }
  }
  return -1;
}

// filter empty non-terminals (epsilons)
Node<std::string> FilterEmpty(Node<std::string> root) {
  if (root.t.substr(0, 9) == "<LITERAL_") {
    // a literal style catagory, unnecessary to put its children in
    return Node<std::string>(root.t);
  }

  if (root.t[0] == '<' && root.child.size() == 0)
    return Node<std::string>("");

  auto tree = Node<std::string>(root.t);

  for (unsigned int i = 0; i < root.child.size(); ++i) {
    auto fe = FilterEmpty(root.child[i]);
    if (fe.t != "")
      tree.child.push_back(fe);
  }

  return tree;
}

// construct parse table from file
void TableConstructor(std::ifstream *fd) {
  rules.clear();
  rule_table.clear();
  for (int i = 0; i < MAX_RULE; ++i) {
    for (int j = 0; j < 255; ++j)
      selection[i][j] = -1;
  }

  std::string start_symbol;
  std::string buf;
  // get start symbol
  if (! std::getline(*fd, start_symbol)) {
    std::cerr << "Failed to open parse table" << std::endl;
    exit(-1);
  }

  // get transition tables
  while (std::getline(*fd, buf)) {
    // find rule for non-terminal

    // get substring for catagory name
    std::string s = buf.substr(0, buf.find("="));
    rules.push_back(catagory(s));
    int j = buf.find("=") + 1;

    // get each identifier
    while (j < (int) buf.length()) {
      if (buf[j] == '<') {
        s = "";
        while (buf[j] != '>') {
          s += buf[j];
          j++;
        }
        s += '>';
        rules.back().reps.push_back(s);
      } else {
        s = "";
        while ((j < (int) buf.size() - 2) && (buf[j+1] == '|')) { // concats a|b
          s += buf[j];
          j += 2;
        }
        s += buf[j];
        rules.back().reps.push_back(s);
      }
      j++;
    }

    // put the non-terminal into rule table
    int cur_rule = -1;
    std::vector<std::string>::iterator it = std::find(rule_table.begin(), rule_table.end(), rules.back().name);
    if (it == rule_table.end()) {
      // rule not found
      rule_table.push_back(rules.back().name);
      cur_rule = rule_table.size() - 1;
    } else {
      cur_rule = std::distance(rule_table.begin(), it);
    }

    // set the selection table
    if (rules.back().reps[0] == "<EPS>") { // epsilon transition
      if (rules.back().reps.size() != 1) {
        std::cerr << "Concating Epsilon transition" << std::endl;
        exit(-1);
      }
      for (int q = 0; q < 255; ++q) {
        if (selection[cur_rule][q] == -1)
          selection[cur_rule][q] = rules.size() - 1;
      }
    } else {
      if (rules.back().reps[0][0] == '<') {
        for (int k = 0; rule_table[k][0] != 0; ++k) {
          if (rule_table[k] == rules.back().reps[0]) {
            for (int q = 0; q < 255; ++q) {
              if (selection[k][q] != -1) {
                selection[cur_rule][q] = rules.size() - 1;
              }
            }
            break;
          }
        }
      } else {
        for (auto c : rules.back().reps[0]) { // terminals
          selection[cur_rule][(int) c] = rules.size() - 1;
        }
      }
    }
  }

  start_rule = find_rule(start_symbol);
  if (start_rule == -1) {
    std::cerr << "Cannot find suitable start symbol: " << start_symbol << std::endl;
    exit(-1);
  }
}

Node<std::string> PushdownAutomata(std::string str) {
  auto tree = Node<std::string>(rule_table[start_rule]);
  std::stack<Node<std::string>*> stk;
  stk.push(&tree);
  auto p = str.begin();
  while (*p == ' ') p++;
  while (stk.size() > 0) {
    auto cur_node = stk.top();
    stk.pop();
    if (cur_node->t == "<EPS>") { // epsilon, skip
      continue;
    }
    if (cur_node->t[0] == '<') { // non-terminal
      int cur_sym_int = find_rule(cur_node->t);
      if (selection[cur_sym_int][(int) *p] != -1) { // the rule is found
        int cur_rul = selection[cur_sym_int][(int) *p];
        for (auto i = rules[cur_rul].reps.begin(); i != rules[cur_rul].reps.end(); ++i) {
          cur_node->child.push_back(Node<std::string>(*i));
        }
        for (int i = cur_node->child.size() - 1; i >= 0; --i) {
          stk.push(&(cur_node->child[i]));
        }
      } else { // does not match
        std::cerr << "Rejected at " << cur_node->t << ": " << *p << std::endl;
        std::cerr << "Current string location: " << std::string(p, str.end());
        return Node<std::string>("");
      }
    } else { // terminal
      auto it = std::find(cur_node->t.begin(), cur_node->t.end(), *p);
      if (it == cur_node->t.end()) {
        std::cerr << "Required literal in '" << cur_node->t << "', found " << *p << std::endl;
        return Node<std::string>("");
      } else {
        cur_node->t = *p;
      }
      ++p;
    }
  }
  if (stk.size() > 0) {
    std::cerr << "Required " << stk.top()->t << ", found EOF" << std::endl;
    return Node<std::string>("");
  }
  return tree;
}

void initialize_table(std::string parse_table) {
  std::ifstream fp(parse_table);
  TableConstructor(&fp);
  print_rules();
}

Node<std::string> parser(std::string input) {
  Node<std::string> PA = PushdownAutomata(input);
  #ifdef DEBUG_PA
  std::cout << "Printing Parse Tree: " << std::endl;
  PA.print();
  #endif
  if (PA.t == "") {
    return PA; // ERROR
  }

  Node<std::string> FE = FilterEmpty(PA);
  #ifdef DEBUG_FE
  std::cout << "Printing Parse Tree: " << std::endl;
  FE.print();
  #endif
  return FE;
}
