#include <string>
#include <regex>
#include <vector>
#include <numeric>
#include <fstream>
#include <iostream>
#include "scanner.hpp"

std::vector<std::pair<std::string, std::regex>> ScanTable;

void scanner::initialize(std::string file) {
    // initialize
    std::ifstream fp(file);
    ScanTable.clear();
    std::string buf;

    while (std::getline(fp, buf)) {
        ScanTable.push_back(std::pair<std::string, std::regex>(buf.substr(0, buf.find("=")), std::regex("^" + buf.substr(buf.find("=") + 1))));
    }
}

std::string scanner::scan(std::string orig) {
    std::vector<std::string> StringBuilder;
    std::smatch search;
    bool match;

    std::string::const_iterator iter = orig.begin();
    std::string::const_iterator iter_end = orig.end();

    while (iter != orig.end()) {
        match = false;
        for (std::pair<std::string, std::regex> kv : ScanTable) {
            if (std::regex_search(iter, iter_end, search, kv.second)) {
                match = true;

                if (kv.first == "EPS") {
                    iter = search[0].second;
                    break;
                }
                if (kv.first[0] == '_') {
                    StringBuilder.push_back(kv.first.substr(1));
                } else {
                    StringBuilder.push_back(kv.first);
                }

                StringBuilder.push_back("(");
                if (kv.first[0] != '_') {
                    StringBuilder.push_back(search[0]);
                }
                StringBuilder.push_back(")");
                iter = search[0].second;
                break;
            }
        }
        if (!match) {
            std::cerr << "Match failed at " << std::string(iter, std::find(iter, iter_end, '\n')) << std::endl;
            exit(-1);
        }
    }

    std::string result = "";
    result = std::accumulate(StringBuilder.begin(), StringBuilder.end(), result);
    return result;
}
