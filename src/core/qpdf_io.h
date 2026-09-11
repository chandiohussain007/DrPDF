#pragma once

#include <qpdf/QPDF.hh>

#include <filesystem>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace drpdf::core {

inline const char* qpdfPassword(const std::string& password) {
    return password.empty() ? nullptr : password.c_str();
}

inline std::string pathToUtf8(const std::filesystem::path& path) {
#ifdef _WIN32
    const std::wstring wstr = path.wstring();
    if (wstr.empty())
        return {};
    const int size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string result(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), &result[0], size, nullptr, nullptr);
    return result;
#else
    return path.string();
#endif
}

inline void qpdfProcessFile(QPDF& pdf, const std::filesystem::path& path,
                            const std::string& password = {}) {
    pdf.processFile(pathToUtf8(path).c_str(), qpdfPassword(password));
}

} // namespace drpdf::core
