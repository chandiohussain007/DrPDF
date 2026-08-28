#pragma once

#include <QString>

namespace drpdf {

enum class Tool {
    Home,
    Create,
    Images,
    Merge,
    Split,
    Organize,
    Compress,
    Protect,
    Viewer,
    Sign,
    Ocr,
    Edit,
    Watermark,
    Annotate,
};

inline QString toolTitle(Tool t) {
    switch (t) {
    case Tool::Home:
        return QStringLiteral("Home");
    case Tool::Create:
        return QStringLiteral("Create PDF");
    case Tool::Images:
        return QStringLiteral("Images to PDF");
    case Tool::Merge:
        return QStringLiteral("Merge");
    case Tool::Split:
        return QStringLiteral("Split / Extract");
    case Tool::Organize:
        return QStringLiteral("Organize pages");
    case Tool::Compress:
        return QStringLiteral("Compress");
    case Tool::Protect:
        return QStringLiteral("Protect");
    case Tool::Viewer:
        return QStringLiteral("Viewer");
    case Tool::Sign:
        return QStringLiteral("Sign");
    case Tool::Ocr:
        return QStringLiteral("OCR");
    case Tool::Edit:
        return QStringLiteral("Edit text");
    case Tool::Watermark:
        return QStringLiteral("Watermark");
    case Tool::Annotate:
        return QStringLiteral("Annotate");
    }
    return {};
}

} // namespace drpdf
