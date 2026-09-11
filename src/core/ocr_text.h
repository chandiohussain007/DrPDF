#pragma once

#include "result.h"

#include <filesystem>
#include <string>
#include <vector>

namespace drpdf::core {

struct TsvWord {
    int page = 0;
    int left = 0;
    int top = 0;
    int width = 0;
    int height = 0;
    int conf = 0;
    std::string text;
};

struct SearchableWord {
    int page = 0;
    double x = 0;
    double y = 0;
    double width = 0;
    double height = 0;
    std::string text;
};

std::vector<TsvWord> parseTesseractTsv(const std::string& tsv, int page, int minConf = 35);

Result<void> overlaySearchableText(const std::filesystem::path& input,
                                   const std::filesystem::path& output,
                                   const std::vector<SearchableWord>& words,
                                   const std::string& password = {});

} // namespace drpdf::core
