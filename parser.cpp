/*
 * Table Driver Parser
 */

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <stack>
#include <algorithm>
#include <cstring>
#include "tree.hpp"
#include "parser.hpp"

#define BUFFER 255

class catagory {
public:
  std::string name;
  std::vector<std::string> reps;

  catagory(std::string name){
    this->name = name;
    reps.clear();
  }
};

int start_rule;

std::vector<std::string> lexicals;
std::vector<std::string> rule_table;
std::vector<catagory> rules;
int selection[100][255];

void print_rules() {
  /** debug use only
   * print_rules - print all catagory rules out
   */
  #ifdef DEBUG_RULES
  std::cout << "RULES:" << std::endl;
  for (unsigned int i = 0; i < rules.size(); ++i) {
    std::cout << rules[i].name << " ";
    for (unsigned int j = 0; j < rules[i].reps.size() ; ++j)
      std::cout << rules[i].reps[j] << ' ';
    std::cout << std::endl;
  }
  #endif
}

void push_lexical(std::string lex) {
  for (auto i : lexicals) {
    if (i == lex) return;
  }
  lexicals.push_back(lex);
}

int get_lexical(std::string lex) {
  for (int i = 0; i < (int) lexicals.size(); ++i)
    if (lexicals[i] == lex) return i;
  return -1;
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
Node<Lexical> FilterEmpty(Node<Lexical> root) {
  if (root.t.name[0] == '<' && root.child.size() == 0)
    return Node<Lexical>(Lexical());

  auto tree = Node<Lexical>(root.t);

  for (unsigned int i = 0; i < root.child.size(); ++i) {
    auto fe = FilterEmpty(root.child[i]);
    if (fe.t.name != "")
      tree.child.push_back(fe);
  }

  return tree;
}

// construct parse table from file
void TableConstructor(std::ifstream *fd) {
  // initialize rule tables
  rules.clear();
  rule_table.clear();
  for (int i = 0; i < 100; ++i) {
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
    std::string s = buf.substr(0, buf.find(" "));
    rules.push_back(catagory(s));
    int j = buf.find("=") + 2;

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
        j++;
      } else {
        s = "";
        while (j < (int) buf.length() && buf[j] != ' ') {
          s += buf[j];
          j++;
        }
        rules.back().reps.push_back(s);
        push_lexical(s);
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
    } else { // non-epsilon
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
        int c = get_lexical(rules.back().reps[0]);
        selection[cur_rule][c] = rules.size() - 1;
      }
    }
  }

  start_rule = find_rule(start_symbol);
  if (start_rule == -1) {
    std::cerr << "Cannot find suitable start symbol: " << start_symbol << std::endl;
    exit(-1);
  }
}

Node<Lexical> PushdownAutomata(std::vector<Lexical> str) {
  auto tree = Node<Lexical>(Lexical(rule_table[start_rule], ""));
  std::stack<Node<Lexical>*> stk;
  stk.push(&tree);
  auto it = str.begin();
  int p;
  while (stk.size() > 0) {
    if (it == str.end()) {
      p = -1;
    } else {
      p = get_lexical(it->name);
    }

    auto cur_node = stk.top();
    stk.pop();
    if (cur_node->t.name == "<EPS>") { // epsilon, skip
      continue;
    }
    if (cur_node->t.name[0] == '<') { // non-terminal
      int cur_sym_int = find_rule(cur_node->t.name);
      if (selection[cur_sym_int][p] != -1) { // the rule is found
        int cur_rul = selection[cur_sym_int][p];
        for (auto i = rules[cur_rul].reps.begin(); i != rules[cur_rul].reps.end(); ++i) {
          cur_node->child.push_back(Node<Lexical>(Lexical(*i, "")));
        }
        for (int i = cur_node->child.size() - 1; i >= 0; --i) {
          stk.push(&(cur_node->child[i]));
        }
      } else { // does not match
        std::cerr << "Rejected at " << cur_node->t << ": " << *it << std::endl;
        std::cerr << "Current string location: " << it->data;
        return Node<Lexical>(Lexical());
      }
    } else { // terminal
      if (cur_node->t.name == it->name) {
        cur_node->t = *it;
      } else {
        std::cerr << "Required literal '" << cur_node->t << "', found " << *it << std::endl;
        return Node<Lexical>(Lexical());
      }
      it++;
    }
  }
  if (stk.size() > 0) {
    std::cerr << "Required " << stk.top()->t << ", found EOF" << std::endl;
    return Node<Lexical>(Lexical());
  }
  return tree;
}

void parser::initialize(std::string parse_table) {
  std::ifstream fp(parse_table);
  TableConstructor(&fp);
  print_rules();
}

Node<Lexical> parser::parse(std::vector<Lexical> input) {
  Node<Lexical> PA = PushdownAutomata(input);
  #ifdef DEBUG_PA
  std::cout << "Printing Parse Tree: " << std::endl;
  PA.print();
  #endif
  if (PA.t.name == "") {
    return PA; // ERROR
  }

  Node<Lexical> FE = FilterEmpty(PA);
  #ifdef DEBUG_FE
  std::cout << "Printing Parse Tree: " << std::endl;
  FE.print();
  #endif
  return FE;
}
