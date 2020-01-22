#pragma once
#include <string>

class Lexical {
  public:
    std::string name;
    std::string data;

    void print(void);
    Lexical(std::string name, std::string data);
    Lexical() {this->name=""; this->data="";}
};

extern std::ostream& operator<< (std::ostream &out, Lexical const& data);
