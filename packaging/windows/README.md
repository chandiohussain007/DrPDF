# Windows packaging

## NSIS installer (CPack)

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cpack -G NSIS -B build
```

Produces `DrPdf-1.0.0-win64.exe`.

## ZIP portable

```
cpack -G ZIP -B build
```

## MSIX (store / sideload)

1. Install the [Windows App SDK / MSIX packaging tools](https://learn.microsoft.com/en-us/windows/msix/packaging-tool/tool-overview).
2. Point the packaging tool at `build/src/Release/DrPdf.exe` (or the CPack install prefix).
3. Use `packaging/linux/drpdf.svg` converted to PNG 256 as the store icon.
4. Identity: `DrPdf.Desktop`, publisher matches your code-signing cert.
5. Capabilities: none. This app is local-first — do not request `internetClient`.
6. Sign with your Authenticode certificate before sideload.

Notarization / EV signing is done on your machine with your cert — this repo does not embed secrets.
