# TrickSaber Quest
![](TrickSaberPC/Resources/LogoSmall.png)

TrickSaber Quest is a native C++ port of the popular PC TrickSaber mod for Oculus Quest devices.
This project contains multiple versions and development stages of the Quest port.

**Target Platform:** Oculus Quest 2/3 with Beat Saber 1.37

## Project Structure

- **TrickSaberPC/**: Original PC mod source (C#)
- **TrickSaberQuestRebuild/**: Current working Quest port (C++)
- **TrickSaberQuest/**: Previous Quest implementation
- **OldVersionTrickSaberQuest/**: Legacy Quest version

## Features

- **Spin Tricks**: Rotate sabers with thumbstick input
- **Throw Tricks**: Throw and recall sabers with trigger control  
- **Freeze Throw**: Hold sabers in mid-air while thrown
- **Advanced Physics**: Realistic throw mechanics with velocity and torque
- **Slowmo Effects**: Smooth time dilation during tricks
- **Haptic Feedback**: Controller vibration on saber return
- **Note Collision**: Optional trick disable when notes are on screen
- **Complete Rotation**: Full 360° spin completion mode
- **Velocity-Dependent**: Speed based on controller movement
- **Multi-Input Support**: Trigger, grip, thumbstick, and button mapping
- **Burn Mark Management**: Automatic visual effect handling
- **Quest UI**: Native settings integration

## Installation

### Prerequisites
- Oculus Quest 2 or Quest 3
- Beat Saber 1.37 (modded with BMBF or QuestPatcher)
- Developer mode enabled on Quest

### Install Steps
1. Download the latest `.qmod` file from releases
2. Install via BMBF or QuestPatcher:
   - **BMBF**: Upload tab → drag and drop `.qmod` → Sync to Beat Saber
   - **QuestPatcher**: Mods tab → Install from file
3. Launch Beat Saber and verify mod is loaded
4. Configure in Mods > TrickSaber settings

## Usage

### Default Controls
- **Trigger**: Throw trick (pull and hold, release to throw, pull again to recall)
- **Thumbstick**: Spin trick (horizontal movement for roll axis)
- **Grip**: Configurable input
- **Buttons**: Configurable input

### Advanced Features
- **Reverse Controls**: Invert any input direction
- **Threshold Adjustment**: Customize activation sensitivity
- **Speed Control**: Adjust trick speeds and physics
- **Slowmo Effects**: Time dilation during tricks
- **Haptic Feedback**: Controller vibration on saber return

### Configuration
- In-game settings via Mods > TrickSaber
- Customizable input mapping and thresholds
- Physics and visual effect options

## Development

### Build Requirements
- Android NDK r25+ (path: `/opt/homebrew/Caskroom/android-ndk/28c/AndroidNDK13676358.app/Contents/NDK`)
- QPM-Rust package manager
- CMake 3.21+ (path: `/opt/homebrew/bin/cmake`)
- Visual Studio Code (recommended)

### Build Commands (from TrickSaberQuestRebuild/)

**Windows (PowerShell):**
```bash
# Restore dependencies
qpm restore

# Build and copy to Quest
pwsh ./scripts/build.ps1
pwsh ./scripts/copy.ps1

# Build release package
pwsh ./scripts/createqmod.ps1
```

**macOS/Linux (Bash):**
```bash
# Restore dependencies
qpm restore

# Build and copy to Quest
./scripts/build.sh
./scripts/copy.sh

# Build release package
./scripts/createqmod.sh
```

**Manual Build:**
```bash
# Configure build
/opt/homebrew/bin/cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="RelWithDebInfo" -B build

# Build mod
/opt/homebrew/bin/cmake --build ./build --config RelWithDebInfo -- -j$(nproc) VERBOSE=0
```

### Performance Monitoring

TrickSaber includes performance monitoring tools for development:

```bash
# Performance monitoring (from TrickSaberQuestRebuild/scripts/)
pwsh ./performance-monitor.ps1 -Duration 60 -RealTime
pwsh ./benchmark.ps1 -TestType quick
```

### Architecture

The Quest port uses modern C++20 patterns:

- **Component System**: Modular trick implementations
- **Memory Safe**: RAII patterns and proper cleanup
- **Performance Optimized**: Efficient input handling and physics
- **Type Safe**: Strong typing with codegen integration
- **Coroutine-Based**: Smooth animations and state management

### Debug and Testing

```bash
# View logs
adb logcat QuestHook[TrickSaber*]:* *:S

# Performance monitoring
adb logcat | grep "Performance\|FPS\|Memory\|Trick"

# Error debugging
adb logcat | grep "ERROR.*TrickSaber"
```

## Testing

### Unit Tests
**⚠️ Current Status: Limited Testing Available**

The test suite has architectural limitations due to Unity/IL2CPP dependencies:

**Working Tests:**
```bash
# Basic C++ functionality tests (currently working)
./scripts/test.sh --filter "*SimpleConfigTest*"
./scripts/test.sh --filter "*VectorPoolTest*"
```

**Windows (PowerShell):**
```bash
# Run working tests
pwsh ./scripts/test.ps1 -Filter "*SimpleConfigTest*"
pwsh ./scripts/test.ps1 -Filter "*VectorPoolTest*"
```

**macOS/Linux (Bash):**
```bash
# Run working tests
./scripts/test.sh --filter "*SimpleConfigTest*"
./scripts/test.sh --filter "*VectorPoolTest*"
```

**Test Coverage (Limited):**
- ✅ **Basic C++ Logic**: Data structures, algorithms
- ✅ **Memory Pool**: Object pooling (with mocks)
- ❌ **Unity Components**: StateManager, Physics (IL2CPP dependent)
- ❌ **Custom Types**: Hooks and Quest integrations
- ❌ **Device Tests**: Requires Quest hardware

**Known Issues:**
- Most tests require Unity runtime (not available in test environment)
- IL2CPP dependencies prevent linking on host platform
- Tests are configured for Android/ARM64 but run on macOS

### Integration Tests
1. **Installation**: Check mod loads via `adb logcat | grep "TrickSaber"`
2. **Throw Tricks**: Hold trigger → throw motion → release → saber flies and returns
3. **Spin Tricks**: Move thumbstick → saber spins → release → stops spinning
4. **Settings**: Access Mods > TrickSaber for configuration
5. **Performance**: Monitor FPS and memory usage during gameplay



## Compatibility
- **Beat Saber**: 1.37 (Quest)
- **BMBF/QuestPatcher**: Latest stable versions
- **Other Mods**: Compatible with most gameplay mods
- **Controllers**: Quest 2/3 Touch controllers
- **Dependencies**: custom-types ^0.17.10, questui ^0.25.0

## Project Status

### TrickSaberQuestRebuild (Current)
- ✅ Core trick system implemented
- ✅ Advanced input handling
- ✅ Physics and effects system
- ✅ Quest UI integration
- ✅ Performance monitoring
- 🔄 Final testing and optimization

### Known Limitations
- Requires specific Beat Saber version (1.37)
- Performance monitoring requires ADB for detailed metrics
- Some advanced PC features still being ported
- Unit testing limited due to Unity/IL2CPP dependencies
- Test suite requires architectural improvements for full coverage

## Contributing
PRs welcome! See implementation plan in `.amazonq/rules/` for current priorities.

## Credits
- **Original PC Mod**: ToniMacaroni (TrickSaber)
- **Quest Port**: Development team
- **Community**: BSMG Quest modding community
- **Tools**: sc2ad (beatsaber-hook, codegen), Lauriethefish (quest-mod-template)
