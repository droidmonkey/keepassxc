# WiX Toolset Migration Guide

This document explains the WiX Toolset support in KeePassXC and migration from WiX v3 to WiX v6.

## Current Status

**WiX v6 Preparation**: KeePassXC's installer templates (.wxs files) have been updated to support the WiX v6 schema and syntax. However, CMake's CPack WIX generator currently only supports WiX v3.

**WiX v3 Support**: Fully functional for building Windows installers.

## Supported WiX Versions

| WiX Version | Support Status | Tools | Schema |
|-------------|----------------|-------|---------|
| **WiX v6**  | Prepared (awaiting CPack support) | `wix.exe` | v4 schema |
| **WiX v5**  | Prepared (awaiting CPack support) | `wix.exe` | v4 schema |
| **WiX v4**  | Prepared (awaiting CPack support) | `wix.exe` | v4 schema |
| **WiX v3**  | ✅ Fully supported | `candle.exe` + `light.exe` | v3 schema |

## Installation

### For WiX v3 (Current Recommended)
1. Download from: [WiX v3 Releases](https://github.com/wixtoolset/wix3/releases)
2. Install to standard location (typically `%ProgramFiles(x86)%\WiX Toolset v3.11\`)
3. Add `bin` directory to PATH or set `WIX` environment variable

### For WiX v6 (Future)
1. Download from: [WiX Toolset Website](https://wixtoolset.org/)
2. Install using MSI installer
3. The unified `wix.exe` tool will be available

## Building Installers

### Using Release Tool
```bash
# Windows (PowerShell)
.\release-tool.ps1 build -CPackGenerators "WIX;ZIP"

# Linux/macOS (when cross-compiling)
./release-tool build
```

### Using CMake/CPack Directly
```bash
# Configure
cmake -G Ninja -DWITH_XC_ALL=ON ..

# Build
cmake --build .

# Package (WiX v3 required)
cpack -G "WIX"
```

## Changes Made for WiX v6

### Template Updates (.wxs files)
- **Namespace**: Updated from `http://schemas.microsoft.com/wix/2006/wi` to `http://wixtoolset.org/schemas/v4/wxs`
- **Util Extension**: Updated namespace to `http://wixtoolset.org/schemas/v4/wxs/util`
- **RequiredVersion**: Removed (not needed in WiX v6)
- **CustomActions**: Removed `BinaryKey="WixCA"` attributes
- **InstallerVersion**: Updated to 500 (Windows Installer 5.0)

### CMake Configuration
- **Extensions**: Updated from `WixUtilExtension.dll` to `WixToolset.Util.wixext`
- **Detection**: Added `FindWiX.cmake` module for version detection
- **Compatibility**: Added `WiXCompat.cmake` for version-specific configuration

## Migration Timeline

1. **Current**: Use WiX v3 for production builds
2. **Preparation**: Install templates support both WiX v3 and v6 schemas
3. **Future**: When CMake/CPack adds WiX v6 support, switch to WiX v6
4. **Long-term**: Deprecate WiX v3 support

## Troubleshooting

### "Could not find the WiX candle executable"
- Install WiX v3 from the official releases
- Ensure `WIX` environment variable points to installation directory
- Add WiX `bin` directory to PATH

### "WiX v6 found but CPack doesn't support it yet"
- This is expected - install WiX v3 alongside WiX v6
- CMake/CPack will use WiX v3 tools automatically
- Monitor CMake releases for WiX v6 support

### Template Compatibility
- Templates use WiX v4 schema which is forward-compatible
- WiX v3 tools ignore unknown namespaces gracefully
- No changes needed to existing builds

## Future Improvements

When CMake/CPack adds WiX v6 support:
1. Remove compatibility layer
2. Switch to native WiX v6 extension names
3. Take advantage of new WiX v6 features
4. Update documentation for WiX v6 as primary version