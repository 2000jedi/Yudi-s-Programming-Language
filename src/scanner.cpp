#include <string>
#include <algorithm>
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
        } catch (const std::regex_error &) {
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

                current.line = std::count(orig.cbegin(), search[0].first, '\n') + 1;
                current.position = -1;
                auto p = search[0].first;
                while (*p != '\n') {
                    p--;
                    current.position++;
                }

                result.push_back(current);
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
