#pragma once
#include <string>

class Lexical {
  public:
    std::string name;
    std::string data;

    int line;
    int position;

    void print(void);
    Lexical(std::string name, std::string data);
    Lexical() {this->name=""; this->data=""; this->line=-1; this->position=-1;}
};

extern std::ostream& operator<< (std::ostream &out, Lexical const& data);
