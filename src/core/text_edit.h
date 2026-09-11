#pragma once

#include "result.h"

#include <filesystem>
#include <string>
#include <vector>

namespace drpdf::core {

struct TextRun {
    int page = 0;     // 0-based
    int index = 0;    // nth show-string on that page
    std::string text;
    std::string font;
    double size = 12;
    double x = 0;
    double y = 0;
    bool simpleEncoding = true;
};

struct NewTextBox {
    int page = 0;
    std::string text;
    double x = 72;
    double y = 720;
    double fontSize = 12;
    double r = 0;
    double g = 0;
    double b = 0;
    std::string baseFont = "Helvetica";
};

struct RedactBox {
    int page = 0;
    double llx = 0;
    double lly = 0;
    double urx = 0;
    double ury = 0;
};

Result<std::vector<TextRun>> extractTextRuns(const std::filesystem::path& file,
                                             const std::string& password = {});

Result<void> replaceTextRun(const std::filesystem::path& input, const std::filesystem::path& output,
                            int page, int runIndex, const std::string& newText,
                            const std::string& password = {});

Result<void> addTextBox(const std::filesystem::path& input, const std::filesystem::path& output,
                        const NewTextBox& box, const std::string& password = {});

Result<void> redactBoxes(const std::filesystem::path& input, const std::filesystem::path& output,
                         const std::vector<RedactBox>& boxes, const std::string& password = {});

} // namespace drpdf::core
