# Build Tools Locations

## System Paths (macOS)
- **CMake**: `/opt/homebrew/bin/cmake`
- **Android NDK**: `/opt/homebrew/Caskroom/android-ndk/28c/AndroidNDK13676358.app/Contents/NDK`
- **QPM**: `qpm` (should be in PATH)
- **Make**: `/usr/bin/make`
- **GCC**: `/usr/bin/gcc` 
- **Clang**: `/usr/bin/clang`

## Build Commands
```bash
# Restore dependencies
qpm restore

# Configure build
/opt/homebrew/bin/cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="RelWithDebInfo" -B build

# Build project (verbose) (Don't use unless you need to)
/opt/homebrew/bin/cmake --build ./build

# Build project (quiet - less verbose)
/opt/homebrew/bin/cmake --build ./build --config RelWithDebInfo -- -j$(nproc) VERBOSE=0

# Build project (minimal output)
make -C build -s

# Clean rebuild
rm -rf build && /opt/homebrew/bin/cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="RelWithDebInfo" -B build && /opt/homebrew/bin/cmake --build ./build
```

## Working Directory
All commands should be run from: `/Users/mac83/dev/TrickSaber/TrickSaberQuestRebuild`