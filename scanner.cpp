#include <string>
#include <regex>
#include <vector>
#include <numeric>
#include <fstream>
#include <iostream>
#include "scanner.hpp"
#include "lexical.hpp"

std::vector<std::pair<std::string, std::regex>> ScanTable;

void scanner::initialize(std::string file) {
    // initialize
    std::ifstream fp(file);
    ScanTable.clear();
    std::string buf;

    while (std::getline(fp, buf)) {
        try{
            ScanTable.push_back(std::pair<std::string, std::regex>(buf.substr(0, buf.find("=")), std::regex(buf.substr(buf.find("=") + 1))));
        } catch (std::regex_error e) {
            std::cerr << "scanner::initialize: regex error at line '" << buf << "'" << std::endl;
            exit(-1);
        }
    }
}

std::vector<Lexical> scanner::scan(std::string orig) {
    std::vector<Lexical> result;
    std::smatch search;
    bool match;

    std::string::const_iterator iter = orig.begin();
    std::string::const_iterator iter_end = orig.end();

    while (iter != orig.end()) {
        match = false;
        for (std::pair<std::string, std::regex> kv : ScanTable) {
            if (std::regex_search(iter, iter_end, search, kv.second, std::regex_constants::match_continuous)) {
                Lexical current;

                match = true;

                if (kv.first == "EPS") {
                    iter = search[0].second;
                    break;
                }
                if (kv.first[0] == '_') {
                    current.name = kv.first.substr(1);
                } else {
                    current.name = kv.first;
                }

                if (kv.first[0] != '_') {
                    current.data = search[0];
                }
                iter = search[0].second;

                result.push_back(current);
                std::cout << current << std::endl;
                break;
            }
        }
        if (!match) {
            std::cerr << "scanner::scan: match failed at: " << std::string(iter, std::find(iter, iter_end, '\n')) << std::endl;
            exit(-1);
        }
    }

    return result;
}
