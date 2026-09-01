#pragma once

#include <qpdf/QPDF.hh>

#include <filesystem>
#include <string>

namespace drpdf::core {

inline const char* qpdfPassword(const std::string& password) {
    return password.empty() ? nullptr : password.c_str();
}

inline void qpdfProcessFile(QPDF& pdf, const std::filesystem::path& path,
                            const std::string& password = {}) {
pdf.processFile(path.string().c_str(), qpdfPassword(password));
}
} // namespace drpdf::core
