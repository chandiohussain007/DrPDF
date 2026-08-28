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
#ifdef _WIN32
    pdf.processFile(path.wstring().c_str(), qpdfPassword(password));
#else
    pdf.processFile(path.string().c_str(), qpdfPassword(password));
#endif
}

} // namespace drpdf::core
