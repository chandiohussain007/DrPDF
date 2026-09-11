#pragma once

#include "core/result.h"

#include <QString>

#include <filesystem>
#include <functional>

namespace drpdf {

class WorkingCopy {
public:
    bool load(const QString& path);
    bool isOpen() const { return !current_.isEmpty(); }
    QString currentPath() const { return current_; }
    QString originalPath() const { return original_; }

    core::Result<void> apply(const std::function<core::Result<void>(
                                 const std::filesystem::path&, const std::filesystem::path&)>& op);

    core::Result<void> saveAs(const QString& dest);

private:
    QString original_;
    QString current_;
};

} // namespace drpdf
