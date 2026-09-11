#include "working_copy.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUuid>

namespace drpdf {
namespace {

QString tempPdfPath() {
    return QDir::temp().filePath(QStringLiteral("drpdf-") +
                                 QUuid::createUuid().toString(QUuid::WithoutBraces) +
                                 QStringLiteral(".pdf"));
}

bool copyFile(const QString& from, const QString& to) {
    if (QFile::exists(to)) {
        QFile::remove(to);
    }
    return QFile::copy(from, to);
}

} // namespace

bool WorkingCopy::load(const QString& path) {
    if (!QFileInfo::exists(path)) {
        return false;
    }
    const QString dst = tempPdfPath();
    if (!copyFile(path, dst)) {
        return false;
    }
    if (!current_.isEmpty()) {
        QFile::remove(current_);
    }
    original_ = path;
    current_ = dst;
    return true;
}

core::Result<void> WorkingCopy::apply(
    const std::function<core::Result<void>(const std::filesystem::path&,
                                           const std::filesystem::path&)>& op) {
    if (current_.isEmpty()) {
        return core::Error{"no document open"};
    }
    const QString next = tempPdfPath();
    auto r = op(std::filesystem::path(current_.toStdString()),
                std::filesystem::path(next.toStdString()));
    if (!r) {
        QFile::remove(next);
        return r;
    }
    QFile::remove(current_);
    current_ = next;
    return {};
}

core::Result<void> WorkingCopy::saveAs(const QString& dest) {
    if (current_.isEmpty()) {
        return core::Error{"no document open"};
    }
    if (!copyFile(current_, dest)) {
        return core::Error{"could not write " + dest.toStdString()};
    }
    return {};
}

} // namespace drpdf
