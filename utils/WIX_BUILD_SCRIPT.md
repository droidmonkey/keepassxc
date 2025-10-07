# WiX Installer Build Script

This Python script provides a direct interface to build KeePassXC Windows installers using WiX Toolset v4+ without relying on CPack.

## Requirements

- Python 3.6 or later
- WiX Toolset v4.0 or later installed
- Compiled KeePassXC binaries in the build directory

## Installation

The WiX Toolset can be installed from:
- Official website: https://wixtoolset.org/
- Via dotnet: `dotnet tool install --global wix`
- Via winget: `winget install WixToolset.WiX`

## Usage

### Basic Usage

```bash
python utils/build-wix-installer.py --version 2.7.9
```

### Advanced Usage

```bash
python utils/build-wix-installer.py \
    --build-dir ./build \
    --source-dir . \
    --output-dir ./release \
    --version 2.7.9 \
    --arch x64 \
    --verbose
```

### Parameters

- `--build-dir DIR`: Build directory containing compiled binaries (default: current directory)
- `--source-dir DIR`: Source directory containing WiX templates (default: parent of build dir)
- `--output-dir DIR`: Output directory for the MSI installer (default: build dir)
- `--version VERSION`: Version string (e.g., 2.7.9) - **Required**
- `--arch {x64,x86,arm64}`: Target architecture (default: x64)
- `-v, --verbose`: Enable verbose output for debugging

### Example Workflow

1. **Build KeePassXC:**
   ```bash
   mkdir build && cd build
   cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
   cmake --build .
   ```

2. **Build Installer:**
   ```bash
   python ../utils/build-wix-installer.py --version 2.7.9 --verbose
   ```

3. **Find Output:**
   The MSI installer will be created as `KeePassXC-2.7.9-Win64.msi` in the build directory.

## How It Works

The script performs the following steps:

1. **Locates WiX Tools**: Searches for `wix.exe` in:
   - `%WIX%\bin` environment variable
   - Common Program Files locations
   - System PATH

2. **Verifies Required Files**: Checks for:
   - WiX template files (.wxs)
   - Resource files (icons, images)
   - Compiled binaries
   - License files

3. **Harvests Files**: 
   - Uses `heat.exe` if available to automatically include all files
   - Falls back to a simplified manual approach if heat.exe is not found

4. **Generates Variables**: Creates WiX include files with:
   - Product information
   - Version numbers
   - File paths
   - GUIDs

5. **Builds MSI**: Executes `wix build` with:
   - Main template file
   - Custom dialog files
   - Harvested file fragments
   - Resource bindings
   - Extension references

## Advantages Over CPack

- **Direct Control**: Full control over the WiX build process
- **Better Debugging**: Clear error messages and verbose output
- **Flexible**: Easy to customize and extend
- **Independent**: Doesn't require CMake/CPack to be configured
- **Modern**: Uses WiX v4+ features directly

## Troubleshooting

### "Could not find wix.exe"

Ensure WiX Toolset v4+ is installed and either:
- Set the `WIX` environment variable to the installation directory
- Add the WiX `bin` directory to your PATH

### "WiX source not found"

Verify you're running from the correct directory and the source files exist in `share/windows/`.

### "Install directory not found"

Build KeePassXC first using CMake before running this script.

### "WARNING: Creating simplified file fragment"

The script couldn't find `heat.exe` and is using a simplified file harvesting approach. For production builds, ensure `heat.exe` is available in the same directory as `wix.exe`.

## Integration with release-tool

This script can be integrated into the existing release workflow:

```bash
# In release-tool or build scripts
python utils/build-wix-installer.py \
    --build-dir "$BUILD_DIR" \
    --version "$RELEASE_VERSION" \
    --arch x64
```

## License

Copyright (C) 2024 KeePassXC team <https://keepassxc.org/>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 or (at your option)
version 3 of the License.
