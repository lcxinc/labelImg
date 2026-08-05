# labelImgCpp

Qt 6 C++ / Qt Widgets port of the Python labelImg application.

See [docs/feature-parity-design.md](docs/feature-parity-design.md) for the parity matrix, implementation boundaries, and regression checklist.

## Windows MSVC Build

Install:

- Visual Studio Build Tools with MSVC C++ workload
- Qt 6 MSVC package
- CMake 3.24+

Configure and build:

```powershell
bin\build.bat -QtPrefix "D:\Qt\6.11.0\msvc2022_64" -RunTests
```

Deploy manually when needed:

```powershell
windeployqt target\cpp-build\Release\labelImgCpp.exe
```

Build the Windows deployment directory, portable zip, and per-user installer:

```powershell
powershell -ExecutionPolicy Bypass -File cpp\packaging\build_windows_installer.ps1
```

Build artifacts are written under `target/`; the repository `bin/` directory contains only developer entry scripts.

The installer writes to `%LOCALAPPDATA%\Programs\labelImgCpp` and creates Start Menu/Desktop shortcuts, so it does not require administrator rights.

The optional `labelme_ai_bridge.py` is included beside the executable in both the portable zip and installer payload, so AI point/box modes keep the same runtime lookup path after installation.

The C++ app loads resources from the installed application directory first, then falls back to the repository root during development:

- `resources/icons`
- `resources/strings`
- `data/predefined_classes.txt`
