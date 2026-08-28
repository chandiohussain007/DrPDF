#pragma once

#include "result.h"

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace drpdf::core {

struct PageSpec {
    std::filesystem::path file;
    int index = 0;     // 0-based in the source file
    int rotation = 0;  // additional rotation: 0, 90, 180, 270
    bool included = true;
};

struct PdfInfo {
    std::filesystem::path path;
    int pageCount = 0;
    bool encrypted = false;
    std::string title;
    std::uintmax_t bytes = 0;
};

struct WriteOptions {
    std::string userPassword;
    std::string ownerPassword;
    bool optimizeStreams = true;
    bool objectStreams = true;
};

class Assembly {
public:
    Result<PdfInfo> addDocument(const std::filesystem::path& file,
                                const std::string& password = {});

    const std::vector<PageSpec>& pages() const { return pages_; }
    std::vector<PageSpec>& pages() { return pages_; }

    void move(int from, int to);
    void remove(int index);
    void rotate(int index, int deltaDegrees);
    void setIncluded(int index, bool included);
    void clear();

    int count() const;
    int includedCount() const;

    Result<void> write(const std::filesystem::path& output,
                       const WriteOptions& options = {}) const;

private:
    std::vector<PageSpec> pages_;
    std::map<std::filesystem::path, std::string> passwords_;
};

Result<PdfInfo> inspectPdf(const std::filesystem::path& file, const std::string& password = {});

Result<void> optimizePdf(const std::filesystem::path& input, const std::filesystem::path& output,
                         const std::string& password = {});

Result<void> decryptPdf(const std::filesystem::path& input, const std::filesystem::path& output,
                        const std::string& password);

Result<void> encryptPdf(const std::filesystem::path& input, const std::filesystem::path& output,
                        const std::string& userPassword, const std::string& ownerPassword,
                        const std::string& currentPassword = {});

} // namespace drpdf::core
