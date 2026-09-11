# macOS packaging

## Disk image (CPack)

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cpack -G DragNDrop -B build
```

Produces `DrPdf-1.0.0.dmg`.

## Notarized Developer ID build

1. Archive `DrPdf.app` from `build/src`.
2. `codesign --deep --force --options runtime --sign "Developer ID Application: …" DrPdf.app`
3. Zip the app and submit:

```
xcrun notarytool submit DrPdf.zip --apple-id … --team-id … --wait
xcrun stapler staple DrPdf.app
```

Hardened runtime is required for notarization. The app does not use the network; no entitlement for outgoing connections.

Apple Silicon + Intel: configure a universal Qt kit or ship two DMGs.
