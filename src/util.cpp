/**
 * Copyright (c) 2020 by Yudi Yang <2000jedi@gmail.com>.
 * All rights reserved.
 */

#include <sstream>
#include <iostream>
#include <vector>

#include "util.hpp"

std::string unescape(std::string raw) {
    raw = raw.substr(1, raw.size() - 2);

    std::ostringstream ss;

    unsigned int i = 0;
    while (i < raw.size()) {
        if (raw[i] != '\\') {
            ss << raw[i];
        } else {
            if ((i + 1) == raw.size()) {
                std::cerr << "Parser: invalid string literal" << std::endl;
                return raw;
            }

            switch (raw[i+1]) {
                case 'a': {
                    ss << '\a';
                    break;
                }
                case 'b': {
                    ss << '\b';
                    break;
                }
                case 't': {
                    ss << '\t';
                    break;
                }
                case 'n': {
                    ss << '\n';
                    break;
                }
                case 'v': {
                    ss << '\v';
                    break;
                }
                case 'f': {
                    ss << '\f';
                    break;
                }
                case 'r': {
                    ss << '\r';
                    break;
                }
                case '"': {
                    ss << '\"';
                    break;
                }
                case '\'': {
                    ss << '\'';
                    break;
                }
                case '\?': {
                    ss << '\?';
                    break;
                }
                case '\\': {
                    ss << '\\';
                    break;
                }
                default: {
                    std::cerr << "Parser: undefined escape string: \\"
                        << raw[i+1] << std::endl;
                }
            }
            i++;
        }
        i++;
    }

    return ss.str();
}
