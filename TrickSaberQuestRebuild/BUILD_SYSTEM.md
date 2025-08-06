# TrickSaber Build System

## Overview

The build system automatically creates both debug and release versions of the QMOD:

- **Debug Version** (`*_debug_*.qmod`): Contains unstripped `.so` with debug symbols for development
- **Release Version** (`*_v*.qmod`): Contains stripped `.so` optimized for distribution

## Build Commands

### Quick Build
```bash
# macOS/Linux
bash scripts/build-all.sh

# Windows
pwsh scripts/build-all.ps1
```

### Manual Build Steps
```bash
# 1. Restore dependencies
qpm restore

# 2. Configure and build
/opt/homebrew/bin/cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="RelWithDebInfo" -B build
/opt/homebrew/bin/cmake --build ./build --config RelWithDebInfo -- -j$(nproc) VERBOSE=0
```

## Output Structure

```
binaries/
├── tricksaber_debug_v0.0.1.qmod    # Debug version with symbols
└── tricksaber_v0.0.1.qmod          # Release version (stripped)
```

## Build Process

1. **Compilation**: C++ source compiled with debug info (`RelWithDebInfo`)
2. **Debug QMOD**: Created with original `.so` file containing debug symbols
3. **Release QMOD**: Created with stripped `.so` file for smaller size
4. **Automatic Packaging**: Both versions include `mod.json` and all dependencies

## Installation

- **Development**: Use debug version for testing and debugging
- **Distribution**: Use release version for end users

## Scripts

- `build-all.sh/ps1`: Complete build with both QMODs
- `build.sh/ps1`: Build only (no QMOD creation)
- `createqmod.sh/ps1`: Create QMODs from existing build
- `test-build-system.sh`: Verify build system configuration

## Dependencies

The build system automatically handles:
- QPM package restoration
- CMake configuration
- Symbol stripping for release builds
- QMOD packaging with proper structure