#!/usr/bin/env bash
# Build a portable AppImage (Linux). Run from the repo root after a Release cmake build.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD="${ROOT}/build"
APPDIR="${BUILD}/AppDir"
BIN="${BUILD}/src/DrPdf"

if [[ ! -x "$BIN" ]]; then
  echo "Build DrPdf first: cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build"
  exit 1
fi

rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/share/applications" "$APPDIR/usr/share/icons/hicolor/scalable/apps"
cp "$BIN" "$APPDIR/usr/bin/DrPdf"
cp "$ROOT/packaging/linux/drpdf.desktop" "$APPDIR/usr/share/applications/"
cp "$ROOT/packaging/linux/drpdf.svg" "$APPDIR/usr/share/icons/hicolor/scalable/apps/drpdf.svg"
ln -sf usr/share/applications/drpdf.desktop "$APPDIR/drpdf.desktop"
ln -sf usr/share/icons/hicolor/scalable/apps/drpdf.svg "$APPDIR/drpdf.svg"

# Optional: linuxdeploy if present
if command -v linuxdeploy >/dev/null 2>&1; then
  linuxdeploy --appdir "$APPDIR" --executable "$APPDIR/usr/bin/DrPdf" --desktop-file "$APPDIR/usr/share/applications/drpdf.desktop" --icon-file "$APPDIR/usr/share/icons/hicolor/scalable/apps/drpdf.svg" --output appimage
else
  echo "AppDir staged at $APPDIR"
  echo "Install linuxdeploy to wrap it as an AppImage: https://github.com/linuxdeploy/linuxdeploy"
fi
