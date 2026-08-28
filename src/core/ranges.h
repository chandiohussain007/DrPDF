#pragma once

#include "result.h"

#include <cctype>
#include <string>
#include <vector>

namespace drpdf::core {

// Parse 1-based page specs like "1-3,5,8-10" into 0-based indices.
// "z" means last page (qpdf convention).
inline Result<std::vector<int>> parsePageRanges(const std::string& spec, int pageCount) {
    std::vector<int> out;
    if (pageCount <= 0) {
        return Error{"document has no pages"};
    }
    if (spec.empty()) {
        return Error{"empty page range"};
    }

    auto parseToken = [&](const std::string& token) -> Result<int> {
        std::string t = token;
        while (!t.empty() && std::isspace(static_cast<unsigned char>(t.front()))) {
            t.erase(t.begin());
        }
        while (!t.empty() && std::isspace(static_cast<unsigned char>(t.back()))) {
            t.pop_back();
        }
        if (t.empty()) {
            return Error{"invalid page range"};
        }
        if (t == "z" || t == "Z") {
            return pageCount;
        }
        try {
            int n = std::stoi(t);
            return n;
        } catch (...) {
            return Error{"invalid page number: " + token};
        }
    };

    std::string cur;
    std::vector<std::string> parts;
    for (char c : spec) {
        if (c == ',') {
            parts.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    parts.push_back(cur);

    for (const auto& part : parts) {
        auto dash = part.find('-');
        if (dash == std::string::npos) {
            auto n = parseToken(part);
            if (!n) {
                return Error{n.error()};
            }
            int oneBased = n.value();
            if (oneBased < 1 || oneBased > pageCount) {
                return Error{"page " + std::to_string(oneBased) + " out of range"};
            }
            out.push_back(oneBased - 1);
        } else {
            auto a = parseToken(part.substr(0, dash));
            auto b = parseToken(part.substr(dash + 1));
            if (!a) {
                return Error{a.error()};
            }
            if (!b) {
                return Error{b.error()};
            }
            int from = a.value();
            int to = b.value();
            if (from > to) {
                std::swap(from, to);
            }
            if (from < 1 || to > pageCount) {
                return Error{"range " + part + " out of range"};
            }
            for (int i = from; i <= to; ++i) {
                out.push_back(i - 1);
            }
        }
    }
    if (out.empty()) {
        return Error{"no pages selected"};
    }
    return out;
}

} // namespace drpdf::core
