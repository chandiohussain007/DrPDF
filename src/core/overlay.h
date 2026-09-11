#pragma once

#include "result.h"

#include <filesystem>
#include <string>
#include <vector>

namespace drpdf::core {

struct WatermarkOptions {
    std::string text = "CONFIDENTIAL";
    double fontSize = 48;
    double rotationDeg = 45;
    double opacity = 0.16;
    double r = 0.45;
    double g = 0.45;
    double b = 0.45;
    bool tiled = false;
    int pageFrom = 0;
    int pageTo = -1; // inclusive, -1 = last
};

enum class PageMarkPos {
    TopLeft,
    TopCenter,
    TopRight,
    BottomLeft,
    BottomCenter,
    BottomRight,
};

struct HeaderFooterOptions {
    std::string header;
    std::string footer;
    std::string pageNumberFormat; // empty = off; supports {n} and {total}
    PageMarkPos pageNumberPos = PageMarkPos::BottomCenter;
    double fontSize = 10;
    double margin = 36;
    double r = 0.15;
    double g = 0.15;
    double b = 0.18;
    int pageFrom = 0;
    int pageTo = -1;
};

struct RasterImage {
    int width = 0;
    int height = 0;
    int components = 3; // 1 = gray, 3 = rgb
    std::vector<unsigned char> samples;
    std::vector<unsigned char> alpha; // optional, width*height
};


struct ImageStamp {
    int page = 0;
    double x = 72;
    double y = 72;
    double width = 200;
    double height = 200;
    RasterImage image;
};

Result<void> applyWatermark(const std::filesystem::path& input, const std::filesystem::path& output,
                            const WatermarkOptions& options, const std::string& password = {});

Result<void> applyHeaderFooter(const std::filesystem::path& input,
                               const std::filesystem::path& output,
                               const HeaderFooterOptions& options,
                               const std::string& password = {});

Result<void> stampImage(const std::filesystem::path& input, const std::filesystem::path& output,
                        const ImageStamp& stamp, const std::string& password = {});

} // namespace drpdf::core
