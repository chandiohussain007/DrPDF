# Dr PDF

> **Everything a PDF tool should be. Nothing it shouldn't.**  
> *A high-performance, local-first, offline PDF editor built with C++20 and Qt 6.*

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Qt 6.5+](https://img.shields.io/badge/Qt-6.5%2B-green.svg)](https://www.qt.io/)
[![QPDF 11+](https://img.shields.io/badge/Engine-QPDF%2011%2B-orange.svg)](https://github.com/qpdf/qpdf)
[![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)](#building-from-source)

---

## Overview

**Dr PDF** is a lightweight, privacy-focused desktop application designed to manipulate, edit, convert, and secure PDF documents completely offline. Built from the ground up for speed and zero network footprint: **no user accounts, no cloud processing, no analytics, and no telemetry.**

### Key Features at a Glance

* 🔒 **Complete Data Sovereignty:** All operations (OCR, PDF compilation, signature processing) happen strictly in-process on your host hardware.
* ⚡ **Blazing Fast Engine:** Modular architecture powered by a headless C++20 core wrapping native QPDF and rendering via PDFium.
* 🎨 **Modern Shell & UI:** Foxit-style productivity ribbon, collapsible sidebar, fluid thumbnail grids, dark/light theme tokens (*Midnight* and *Champagne Gold*), and quick actions palette (`Ctrl+K`).
* ♿ **Built for Accessibility:** Full keyboard navigation, screen reader support, accessible action tags, and high-contrast focus rings.

---

## Feature Matrix

| Domain | Capabilities | Engine / Stack |
| :--- | :--- | :--- |
| **Document Creation** | Word-like document builder (headings, multi-column tables, inline images, custom page breaks, repeating headers/footers) & Image-to-PDF compilation with aspect ratio locking. | `QPdfWriter` / `QTextDocument` |
| **Content Editing** | Block-level text editing, text run search & replace, custom text insertion, image stamping, and permanent black-box redaction. | `src/core/text_edit` |
| **Annotations & Markups** | Highlights, sticky comments, freehand ink drawing, watermarking, custom dynamic headers, footers, and automatic page numbering. | `src/core/annotations` & `overlay` |
| **Page Assembly** | Reorder, rotate, split (by ranges, interval $N$, or selection), merge multi-file queues, and delete arbitrary pages. | QPDF 11+ |
| **OCR & Signatures** | Offline optical character recognition for searchable text layers. Visual canvas signatures, typed signatures, and cryptographic PKCS#12 certificate signatures. | Tesseract OCR & OpenSSL |
| **Security & Optimization** | Hardware-accelerated AES-256 encryption/decryption, permission flags, lossless object stream compression, and intelligent image downsampling. | QPDF Stream Optimizer |
| **Viewer & Navigation** | Continuous scrolling viewer, variable step zoom, thumbnail strip previews, drag-and-drop file routing, and single-keystroke action command palette. | Qt PDF (PDFium) |

---

## System Architecture

Dr PDF enforces a clear separation between headless domain logic and graphical client layers:

```
┌─────────────────────────────────────────────────────────────┐
│                       src/ui (Shell)                        │
│   Qt Widgets Ribbon · Collapsible Sidebar · Palette (Ctrl+K)   │
└──────────────────────────────┬──────────────────────────────┘
                               │
┌──────────────────────────────▼──────────────────────────────┐
│                    src/services (Helpers)                   │
│   Image/Rich-Text Converters · Async Thumbnails · UI Bridges│
└──────────────────────────────┬──────────────────────────────┘
                               │
┌──────────────────────────────▼──────────────────────────────┐
│                     src/core (Engine)                       │
│    Headless C++20 Engine · QPDF wrappers · Zero Qt coupling │
└─────────────────────────────────────────────────────────────┘
```

* **`src/core/`**: Pure C++20 logic handling low-level object parsing, page re-ordering, stream compression, text redaction, and encryption pipelines without linking to UI frameworks.
* **`src/services/`**: Asynchronous Qt-backed worker tasks bridge non-blocking desktop events to core processing units.
* **`src/ui/`**: Modern Foxit-style ribbon layout, stylesheet tokens, customized tool views, and keyboard-first accessibility routes.

---

## Prerequisites

Ensure your environment meets the minimum compiler and dependency versions before building:

* **CMake** `3.24+`
* **C++20 Compiler** (MSVC 2022, Apple Clang 14+, or GCC 12+)
* **Qt 6.5+** with required modules:
  * `Core`, `Gui`, `Widgets`, `Pdf`, `PdfWidgets`, `Concurrent`, `PrintSupport`
* **QPDF 11+** (development headers + library)
* **Tesseract OCR** (runtime executable + language packs for OCR feature)
* **OpenSSL** (optional, required for PKCS#12 cryptographic signatures)

### Installing Dependencies

**macOS (Homebrew)**
```bash
brew install cmake qt qpdf tesseract openssl
```

**Ubuntu / Debian**
```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev qt6-tools-dev \
                 libqt6pdf6 libqt6pdfwidgets6 qt6-svg-dev \
                 libqpdf-dev tesseract-ocr tesseract-ocr-eng libssl-dev
```

**Windows (vcpkg)**
```powershell
vcpkg install qpdf:x64-windows openssl:x64-windows
# Install Tesseract from UB-Mannheim binary distributions or vcpkg
# Ensure Qt 6.5+ (including Qt PDF module) is installed via the Qt Online Installer
```

---

## Building from Source

### macOS & Linux

```bash
# 1. Configure build tree
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Note: If Qt is non-standard, pass CMAKE_PREFIX_PATH:
# cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"

# 2. Compile project
cmake --build build -j$(nproc)

# 3. Execute unit tests
ctest --test-dir build --output-on-failure

# 4. Launch Dr PDF
./build/src/DrPdf
```

### Windows (MSVC 2022 + vcpkg)

```cmd
:: Configure Visual Studio x64 Build
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_PREFIX_PATH=C:\Qt\6.7.2\msvc2019_64 ^
  -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake

:: Build Release Target
cmake --build build --config Release

:: Launch Executable
build\src\Release\DrPdf.exe
```

---

## Packaging & Distribution

Dr PDF includes cross-platform CPack configurations for streamlined distribution:

| Target Platform | Command | Generated Package Format |
| :--- | :--- | :--- |
| **Linux AppImage** | `packaging/linux/build-appimage.sh` | Standalone `.AppImage` |
| **Debian / Ubuntu** | `cpack -G DEB` | Standard `.deb` package |
| **Windows NSIS** | `cpack -G NSIS` | Win32 Installer `.exe` |
| **Windows Zip** | `cpack -G ZIP` | Portable `.zip` bundle |
| **macOS Bundle** | `cpack -G DragNDrop` | Drag-and-Drop `.dmg` volume |

---

## Shortcuts & Accessibility

Designed for maximum keyboard efficiency and compliance with system accessibility services:

* **`Ctrl + K`**: Open Global Command Palette
* **`Ctrl + O`**: Open PDF Document
* **`Ctrl + +` / `Ctrl + -`**: Zoom Viewer In / Out
* **`Ctrl + 0` / `Ctrl + 1`**: Fit Page Width / Actual Size
* **`Esc`**: Dismiss overlay dialogs & return to main dashboard
* **Screen Readers**: High-contrast outline focus rings, explicit `QAccessible` tooltips, and explicit ARIA-equivalent tags across drop zones and page thumbnail strips.

---

## Code Base Layout

```
dr-pdf/
├── CMakeLists.txt           # Master CMake build configuration
├── src/
│   ├── core/                # Pure C++20 PDF manipulation logic
│   │   ├── assembly.cpp     # Merge, split, rotate, lock engine
│   │   ├── text_edit.cpp    # Search/replace text & redaction engine
│   │   ├── overlay.cpp      # Watermarking & dynamic page numbering
│   │   └── annotations.cpp  # Annotations, ink drawing & comments
│   ├── services/            # Qt processing bridges & converters
│   ├── ui/                  # Qt Widgets application
│   │   ├── shell/           # Ribbon bar, menus, command palette
│   │   ├── views/           # Tool-specific user screens
│   │   └── theme/           # Design tokens, SVG assets, CSS rules
├── packaging/               # Platform deployment scripts (NSIS, DMG, AppImage)
└── tests/                   # Core unit test suite (GTest/QtTest)
```

---

## Privacy & Security

PDF documents often contain sensitive personal or enterprise data. Dr PDF guarantees:

1. **Zero External Requests:** No telemetry, update checkers, or license pinging.
2. **Local Memory Safety:** Modern C++ RAII paradigms minimize memory leak risks and arbitrary code execution vectors when parsing untrusted PDF streams.
3. **Private Vulnerability Disclosure:** Please report security findings privately via repository security advisories rather than public issue trackers.

---

## License

Application codebase is distributed under the **MIT License**. See [LICENSE](LICENSE) for terms.

* **Qt**: Dual-licensed under LGPLv3 and Commercial terms. Ensure binary releases dynamically link Qt libraries when distributing under LGPL.
* **QPDF**: Licensed under Apache-2.0.
* **PDFium (Qt PDF)**: BSD-style license.
