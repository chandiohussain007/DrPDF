#pragma once

#include "result.h"

#include <filesystem>
#include <string>
#include <utility>
#include <vector>


namespace drpdf::core {

enum class AnnotKind { Highlight, Comment, Ink, FreeText };

struct Annotation {
    int page = 0;
    int index = 0;
    AnnotKind kind = AnnotKind::Comment;
    std::string subtype;
    std::string contents;
    double llx = 0;
    double lly = 0;
    double urx = 0;
    double ury = 0;
};

struct HighlightSpec {
    int page = 0;
    double llx = 0;
    double lly = 0;
    double urx = 0;
    double ury = 0;
    double r = 1.0;
    double g = 0.92;
    double b = 0.2;
};

struct CommentSpec {
    int page = 0;
    double x = 72;
    double y = 720;
    std::string text;
};

struct InkSpec {
    int page = 0;
    std::vector<std::pair<double, double>> points; // PDF space
    double r = 0.07;
    double g = 0.45;
    double b = 0.85;
    double width = 1.8;
};

struct FreeTextSpec {
    int page = 0;
    double llx = 72;
    double lly = 640;
    double urx = 280;
    double ury = 700;
    std::string text;
    double fontSize = 12;
};

Result<std::vector<Annotation>> listAnnotations(const std::filesystem::path& file,
                                                const std::string& password = {});

Result<void> addHighlight(const std::filesystem::path& input, const std::filesystem::path& output,
                          const HighlightSpec& spec, const std::string& password = {});

Result<void> addComment(const std::filesystem::path& input, const std::filesystem::path& output,
                        const CommentSpec& spec, const std::string& password = {});

Result<void> addInk(const std::filesystem::path& input, const std::filesystem::path& output,
                    const InkSpec& spec, const std::string& password = {});

Result<void> addFreeText(const std::filesystem::path& input, const std::filesystem::path& output,
                         const FreeTextSpec& spec, const std::string& password = {});

Result<void> removeAnnotation(const std::filesystem::path& input,
                              const std::filesystem::path& output, int page, int index,
                              const std::string& password = {});

} // namespace drpdf::core
