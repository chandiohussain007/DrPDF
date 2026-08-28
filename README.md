# Dr PDF

Local-first, offline PDF editor. C++20 + Qt 6. No accounts, no uploads, no telemetry.

> Everything a PDF tool should be. Nothing it shouldn't.

This is **Milestone 1** of the v1 roadmap (core engine + shell + the structural tools). Copy this folder onto your machine and build it there.

## What works now

| Tool | Status |
| --- | --- |
| Create PDF (rich text → PDF) | Working |
| Images → PDF | Working |
| Merge (reorder, rotate, multi-file) | Working |
| Split / extract (ranges, every N, selected pages) | Working |
| Organize (rotate / reorder / delete) | Working |
| Compress (lossless stream + object-stream optimize) | Working — image downsample is Milestone 4 |
| Protect (AES-256 lock / unlock) | Working |
| Viewer (PDFium via Qt PDF) | Working |
| Dark / light theme | Working |
| Command palette (`Ctrl+K`) | Working |
| Edit existing text, Sign, OCR, Annotate, Watermark | Shell in place, engine next |

All processing is in-process. The only network the OS might see is whatever *you* initiate (opening a file from a network drive, etc.). There is no update check, no analytics, no license server.

## Architecture

```
src/core          headless PDF engine (QPDF) — no Qt, unit-tested
src/services      Qt helpers: image→PDF, rich-text→PDF, thumbnails
src/ui            Qt Widgets shell, theme tokens, tool views
```

- **Structure / merge / split / encrypt:** [QPDF](https://github.com/qpdf/qpdf) (Apache-2.0)
- **Render / viewer:** Qt PDF (PDFium, BSD-style) — this is the PDFium layer the PRD asked for, without vendoring depot_tools
- **Create-from-scratch:** `QTextDocument` + `QPdfWriter`
- **Settings / recents:** local `QSettings` ini file (not document content)

GPL/AGPL libraries (Poppler, MuPDF) are intentionally not linked.

## Prerequisites

- CMake 3.24+
- A C++20 compiler (MSVC 2022, Apple Clang, or GCC 12+)
- **Qt 6.5+** with modules: `Core Gui Widgets Pdf PdfWidgets Concurrent PrintSupport`

- **QPDF 11+** (library + headers)

### Qt

Install from [qt.io](https://www.qt.io/download-qt-installer) and tick **Qt PDF**.

Or:

```bash
# macOS
brew install qt

# Ubuntu / Debian (names vary by release)
sudo apt install qt6-base-dev qt6-tools-dev libqt6pdf6 libqt6pdfwidgets6 qt6-svg-dev libqpdf-dev
```

If `find_package(Qt6 COMPONENTS Pdf)` fails, the installer’s **Qt PDF** box was unchecked.

### QPDF

```bash
# macOS
brew install qpdf

# Ubuntu / Debian
sudo apt install libqpdf-dev

# Windows (vcpkg, from this folder)
vcpkg install qpdf:x64-windows
```

## Build

### macOS / Linux

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/src/DrPdf
```

If CMake cannot see Qt:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
```

### Windows (MSVC + vcpkg)

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_PREFIX_PATH=C:\Qt\6.7.2\msvc2019_64 ^
  -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
build\src\Release\DrPdf.exe
```

Point `CMAKE_PREFIX_PATH` at *your* Qt kit. The `Pdf` module must be in that kit.

## Layout

```
src/core/assembly.*     merge / split / rotate / encrypt / optimize
src/core/ranges.h       "1-3,5,8-z" parser
src/ui/shell            main window, drop routing, palette
src/ui/views            one screen per tool
src/ui/theme            light/dark design tokens + stylesheet
tests/                  core tests (no Qt)
```

## Roadmap (from the PRD)

1. **M1 (this tree)** — engine, shell, merge/split/organize/create/images/protect/compress-lossless
2. **M2** — block-level text edit, images in existing PDFs, watermark / page numbers, annotations
3. **M3** — richer Word-like creator (tables, headers)
4. **M4** — Tesseract OCR, image-downsample compress, signatures + PKCS#12
5. **M5** — installers (MSIX / notarized dmg / AppImage), accessibility pass, perf

## License

Application code: all rights reserved to you (the project owner).

Third-party: QPDF (Apache-2.0), Qt (LGPL/commercial — use a dynamic Qt build unless you have a Qt commercial license), PDFium via Qt PDF (BSD-style).
