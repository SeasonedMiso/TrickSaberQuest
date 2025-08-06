# TrickSaber Quest

TrickSaber Quest is a native C++ port of the popular PC TrickSaber mod for Oculus Quest devices, designed for **Beat Saber 1.37** on **Oculus Quest 2/3**.
This repository contains multiple versions and development stages of the Quest port.

---

##  Features

* **Spin Tricks** – Rotate sabers using thumbstick input
* **Throw Tricks** – Throw and recall sabers with trigger control
* **Freeze Throw** – Hold sabers mid-air while thrown
* **Advanced Physics** – Realistic velocity- and torque-based throws
* **Slowmo Effects** – Smooth time dilation during tricks
* **Haptic Feedback** – Controller vibration when sabers return
* **Note Collision Handling** – Optional trick disable when notes are on screen
* **Full 360° Rotation** – Complete spin trick mode
* **Customizable Inputs** – Trigger, grip, thumbstick, and button mapping
* **Momentum-Based Spinning** – Physics-driven saber rotations
* **Quest UI Integration** – Native in-game settings menu
* **Automatic Burn Mark Handling** – Visual effect management

---

##  Installation

### Requirements

* Oculus Quest 2 or Quest 3
* Beat Saber **v1.37** (modded with BMBF or QuestPatcher)
* Developer mode enabled on your Quest

### Steps

1. Download the latest `.qmod` file from [Releases](#).
2. Install via:

   * **BMBF**: Upload tab → drag & drop `.qmod` → Sync to Beat Saber
   * **QuestPatcher**: Mods tab → Install from file
3. Launch Beat Saber and verify TrickSaber is loaded.
4. Configure via **Mods → TrickSaber Settings** in-game.

---

##  Usage

### Default Controls

* **Trigger:** Throw trick (pull & hold → release to throw → pull again to recall)
* **Thumbstick:** Spin trick (horizontal axis for roll)
* **Grip/Buttons:** Configurable inputs

### Advanced Options

* **Reverse Controls:** Invert any input direction
* **Activation Thresholds:** Adjust sensitivity
* **Trick Speed Control:** Fine-tune spin and throw speeds
* **Haptic Feedback:** Enable or disable controller vibration
* **Slowmo Effects:** Optional time dilation during tricks

---

##  Development

### Build Requirements

* Android NDK r25+
* QPM-Rust (package manager)
* CMake 3.21+
* Visual Studio Code (recommended)

> Paths for macOS example:
>
> * NDK: `/opt/homebrew/Caskroom/android-ndk/...`
> * CMake: `/opt/homebrew/bin/cmake`

### Build Commands (from `TrickSaberQuestRebuild/`)

**Windows (PowerShell):**

```bash
qpm restore
pwsh ./scripts/build.ps1
pwsh ./scripts/copy.ps1
pwsh ./scripts/createqmod.ps1
```

**macOS/Linux (Bash):**

```bash
qpm restore
./scripts/build.sh
./scripts/copy.sh
./scripts/createqmod.sh
```

**Manual Build:**

```bash
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="RelWithDebInfo" -B build
cmake --build ./build --config RelWithDebInfo -- -j$(nproc) VERBOSE=0
```

---

##  Debugging & Performance

* **View logs:**

  ```bash
  adb logcat QuestHook[TrickSaber*]:* *:S
  ```
* **Performance monitoring:**

  ```bash
  adb logcat | grep "Performance\|FPS\|Memory\|Trick"
  ```
* **Error debugging:**

  ```bash
  adb logcat | grep "ERROR.*TrickSaber"
  ```

Performance scripts:

```bash
pwsh ./scripts/performance-monitor.ps1 -Duration 60 -RealTime
pwsh ./scripts/benchmark.ps1 -TestType quick
```

---

##  Testing

**Host-native unit tests** now work properly on macOS without Android dependencies.

### Available Tests

* ✅ Basic C++ logic (data structures, algorithms)
* ✅ Memory pooling with thread safety
* ✅ Configuration validation
* ❌ Unity components, custom types, device-dependent tests

**Run host-native tests:**

```bash
# Run all tests
./scripts/test-host.sh

# Run specific test suites
./scripts/test-host.sh --filter "*SimpleConfigTest*"
./scripts/test-host.sh --filter "*VectorPoolTest*"

# Clean build and run tests
./scripts/test-host.sh --clean
```

**PowerShell versions:**

```powershell
pwsh ./scripts/test-host.ps1
pwsh ./scripts/test-host.ps1 -Filter "*SimpleConfigTest*"
pwsh ./scripts/test-host.ps1 -Clean
```

### Manual Integration Tests

1. Install mod → check log (`adb logcat | grep "TrickSaber"`)
2. **Throw Tricks:** Trigger → release → saber returns
3. **Spin Tricks:** Thumbstick movement → spins → release stops spin
4. Settings menu accessible under Mods > TrickSaber
5. Observe FPS and memory usage during gameplay

---

##  Architecture

* **Modern C++20**: Strong typing, RAII, modular components
* **Efficient Input Handling:** Low-latency trick response
* **Physics System:** Velocity- and torque-based calculations
* **Coroutine-Based:** Smooth animations and state transitions
* **Quest-Specific Optimizations:** Tailored for ARM64 and IL2CPP

---

##  Compatibility

* Beat Saber 1.37 (Quest)
* BMBF or QuestPatcher (latest stable)
* Works with most gameplay mods
* Controllers: Quest 2/3 Touch
* Dependencies:

  * `custom-types ^0.17.10`
  * `questui ^0.25.0`

---

##  Project Status

* ✅ Core trick system implemented
* ✅ Advanced input handling
* ✅ Physics, effects, and Quest UI integrated
* ✅ Performance monitoring tools
* 🔄 Final testing and optimization in progress

**Known Limitations:**

* Only supports Beat Saber 1.37
* Advanced PC features not yet fully ported
* Unit tests limited due to IL2CPP
* Performance profiling requires ADB

---

##  Contributing

PRs welcome!

---

##  Credits

* **Original PC Mod:** ToniMacaroni (TrickSaber)
* **Quest Port:** TrickSaber Quest Development Team
* **Community:** BSMG Quest modding community
* **Tools:** sc2ad (beatsaber-hook, codegen), Lauriethefish (quest-mod-template)