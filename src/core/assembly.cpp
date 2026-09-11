#include "assembly.h"

#include "qpdf_compat.h"
#include "qpdf_io.h"


#include <qpdf/Constants.h>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFPageObjectHelper.hh>
#include <qpdf/QPDFWriter.hh>

#include <algorithm>
#include <exception>
#include <map>
#include <memory>
#include <stdexcept>
#include <system_error>

namespace drpdf::core {
namespace {

int normalizeRotation(int degrees) {
    int d = degrees % 360;
    if (d < 0) {
        d += 360;
    }
    d = ((d + 45) / 90) * 90;
    if (d == 360) {
        d = 0;
    }
    return d;
}

void applyWriterFlags(QPDFWriter& writer, const WriteOptions& options) {
    writer.setCompressStreams(options.optimizeStreams);
    writer.setRecompressFlate(options.optimizeStreams);
    writer.setObjectStreamMode(options.objectStreams ? qpdf_o_generate : qpdf_o_disable);
    writer.setNewlineBeforeEndstream(true);
}

void applyEncryption(QPDFWriter& writer, const WriteOptions& options) {
    if (options.userPassword.empty() && options.ownerPassword.empty()) {
        return;
    }
    const std::string user =
        options.userPassword.empty() ? options.ownerPassword : options.userPassword;
    const std::string owner =
        options.ownerPassword.empty() ? options.userPassword : options.ownerPassword;
    writer.setR6EncryptionParameters(user.c_str(), owner.c_str(),
                                     true,  // allow accessibility
                                     true,  // extract
                                     true,  // assemble
                                     true,  // annotate + form
                                     true,  // form filling
                                     true,  // modify other
                                     DRPDF_R3_PRINT_FULL, true);

}

PdfInfo makeInfo(const std::filesystem::path& file, QPDF& pdf) {
    PdfInfo info;
    info.path = file;
    info.encrypted = pdf.isEncrypted();
    info.pageCount = static_cast<int>(QPDFPageDocumentHelper(pdf).getAllPages().size());
    std::error_code ec;
    info.bytes = std::filesystem::file_size(file, ec);
    try {
        auto trailer = pdf.getTrailer();
        if (trailer.hasKey("/Info")) {
            auto dict = trailer.getKey("/Info");
            if (dict.isDictionary() && dict.hasKey("/Title") && dict.getKey("/Title").isString()) {
                info.title = dict.getKey("/Title").getUTF8Value();
            }
        }
    } catch (...) {
        // title is optional
    }
    return info;
}

} // namespace

Result<PdfInfo> inspectPdf(const std::filesystem::path& file, const std::string& password) {
    try {
        if (!std::filesystem::exists(file)) {
            return Error{"file not found: " + file.string()};
        }
        QPDF pdf;
        pdf.setSuppressWarnings(true);
        qpdfProcessFile(pdf, file, password);
        return makeInfo(file, pdf);
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

Result<PdfInfo> Assembly::addDocument(const std::filesystem::path& file,
                                      const std::string& password) {
    auto info = inspectPdf(file, password);
    if (!info) {
        return Error{info.error()};
    }
    passwords_[file] = password;
    for (int i = 0; i < info.value().pageCount; ++i) {
        pages_.push_back(PageSpec{file, i, 0, true});
    }
    return info.value();
}

void Assembly::move(int from, int to) {
    if (from < 0 || to < 0 || from >= static_cast<int>(pages_.size()) ||
        to >= static_cast<int>(pages_.size()) || from == to) {
        return;
    }
    auto item = pages_[static_cast<size_t>(from)];
    pages_.erase(pages_.begin() + from);
    pages_.insert(pages_.begin() + to, item);
}

void Assembly::remove(int index) {
    if (index < 0 || index >= static_cast<int>(pages_.size())) {
        return;
    }
    pages_.erase(pages_.begin() + index);
}

void Assembly::rotate(int index, int deltaDegrees) {
    if (index < 0 || index >= static_cast<int>(pages_.size())) {
        return;
    }
    pages_[static_cast<size_t>(index)].rotation =
        normalizeRotation(pages_[static_cast<size_t>(index)].rotation + deltaDegrees);
}

void Assembly::setIncluded(int index, bool included) {
    if (index < 0 || index >= static_cast<int>(pages_.size())) {
        return;
    }
    pages_[static_cast<size_t>(index)].included = included;
}

int Assembly::count() const { return static_cast<int>(pages_.size()); }

int Assembly::includedCount() const {
    return static_cast<int>(
        std::count_if(pages_.begin(), pages_.end(), [](const PageSpec& p) { return p.included; }));
}

void Assembly::clear() {
    pages_.clear();
    passwords_.clear();
}

Result<void> Assembly::write(const std::filesystem::path& output,
                             const WriteOptions& options) const {
    try {
        if (includedCount() <= 0) {
            return Error{"no pages to write"};
        }

        QPDF out;
        out.emptyPDF();
        QPDFPageDocumentHelper dest(out);

        std::map<std::filesystem::path, std::unique_ptr<QPDF>> open;
        auto sourceFor = [&](const std::filesystem::path& path) -> QPDF& {
            auto it = open.find(path);
            if (it != open.end()) {
                return *it->second;
            }
            auto pdf = std::make_unique<QPDF>();
            pdf->setSuppressWarnings(true);
            std::string pw;
            auto pwIt = passwords_.find(path);
            if (pwIt != passwords_.end()) {
                pw = pwIt->second;
            }
            qpdfProcessFile(*pdf, path, pw);
            QPDF& ref = *pdf;
            open.emplace(path, std::move(pdf));
            return ref;
        };

        for (const auto& spec : pages_) {
            if (!spec.included) {
                continue;
            }
            QPDF& src = sourceFor(spec.file);
            auto srcPages = QPDFPageDocumentHelper(src).getAllPages();
            if (spec.index < 0 || spec.index >= static_cast<int>(srcPages.size())) {
                return Error{"page index out of range in " + spec.file.string()};
            }
            dest.addPage(srcPages[static_cast<size_t>(spec.index)], false);
            if (spec.rotation != 0) {
                auto destPages = dest.getAllPages();
                destPages.back().rotatePage(spec.rotation, false);
            }
        }

        QPDFWriter writer(out);
        writer.setOutputFilename(pathToUtf8(output).c_str());
        applyWriterFlags(writer, options);
        applyEncryption(writer, options);
        writer.write();
        return {};
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

Result<void> optimizePdf(const std::filesystem::path& input, const std::filesystem::path& output,
                         const std::string& password) {
    try {
        QPDF pdf;
        pdf.setSuppressWarnings(true);
        qpdfProcessFile(pdf, input, password);
        QPDFWriter writer(pdf);
        writer.setOutputFilename(pathToUtf8(output).c_str());
        WriteOptions opt;
        opt.optimizeStreams = true;
        opt.objectStreams = true;
        applyWriterFlags(writer, opt);
        writer.write();
        return {};
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

Result<void> decryptPdf(const std::filesystem::path& input, const std::filesystem::path& output,
                        const std::string& password) {
    try {
        QPDF pdf;
        pdf.setSuppressWarnings(true);
        qpdfProcessFile(pdf, input, password);
        QPDFWriter writer(pdf);
        writer.setOutputFilename(pathToUtf8(output).c_str());
        writer.setCompressStreams(true);
        writer.write();
        return {};
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

Result<void> encryptPdf(const std::filesystem::path& input, const std::filesystem::path& output,
                        const std::string& userPassword, const std::string& ownerPassword,
                        const std::string& currentPassword) {
    try {
        QPDF pdf;
        pdf.setSuppressWarnings(true);
        qpdfProcessFile(pdf, input, currentPassword);
        QPDFWriter writer(pdf);
        writer.setOutputFilename(pathToUtf8(output).c_str());
        WriteOptions opt;
        opt.userPassword = userPassword;
        opt.ownerPassword = ownerPassword.empty() ? userPassword : ownerPassword;
        applyWriterFlags(writer, opt);
        applyEncryption(writer, opt);
        writer.write();
        return {};
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

} // namespace drpdf::core
