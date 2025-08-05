# Beat Saber Mod Porting Guide: PC to Quest

## Overview

Porting a Beat Saber mod from PC to Quest is a complex process that requires understanding fundamental differences between the two platforms. Development for PC and Quest standalone are two vastly different workflows, making direct ports impossible without significant code changes.

## Key Platform Differences

### Architecture Differences
- **PC**: Uses BSIPA (Beat Saber Illusion Plugin Architecture) for mod injection
- **Quest**: Uses native Android/ARM64 architecture with different hooking mechanisms
- **Runtime**: PC runs on .NET Framework/Mono while Quest uses IL2CPP compilation

### Development Environments
- **PC**: C# development with Visual Studio, full .NET API access
- **Quest**: C++ development with Android NDK, limited to specific APIs

### Mod Loading Systems
- **PC**: BSIPA handles plugin loading and dependency injection
- **Quest**: Uses ModsBeforeFriday (MBF) or similar Quest-specific mod loaders

## Prerequisites

Before starting the porting process, ensure you have:

### Development Tools
- **Android NDK**: For compiling native Quest libraries
- **Visual Studio or CLion**: For C++ development
- **Quest Development Environment**: Set up per BSMG wiki guidelines
- **ADB (Android Debug Bridge)**: For Quest debugging
- **Il2CppDumper**: For analyzing game structures

### Knowledge Requirements
- C++ programming (Quest mods are primarily C++)
- Understanding of hooking mechanisms (Dobby, MSHook)
- Android development basics
- IL2CPP internals knowledge
- Beat Saber game structure familiarity

## Step-by-Step Porting Process

### 1. Analyze the Original PC Mod

**Code Structure Analysis:**
- Identify all BSIPA-specific code and dependencies
- Document all game hooks and patches used
- List external library dependencies
- Map out UI components and interactions

**Functionality Breakdown:**
- Core mod features and logic
- Configuration systems used
- UI/UX components
- Integration points with other mods

### 2. Set Up Quest Development Environment

**Environment Setup:**
```bash
# Install Android NDK
# Set up Quest development per BSMG wiki
# Configure build tools for Quest modding
```

**Project Structure:**
- Create new C++ project structure
- Set up CMakeLists.txt for Quest compilation
- Configure mod.json for Quest mod loader compatibility

### 3. Rewrite Core Logic

**Language Translation:**
- Convert C# logic to C++ equivalents
- Replace .NET-specific APIs with Android/Quest alternatives
- Implement memory management (C++ vs C# garbage collection)

**Game Hooking:**
- Replace BSIPA hooks with Dobby or MSHook equivalents
- Update method signatures for IL2CPP calling conventions
- Implement proper hook initialization and cleanup

### 4. Handle Platform-Specific Features

**Memory Management:**
```cpp
// Example: Convert C# automatic memory management
// From C#: var gameObject = new GameObject();
// To C++: 
auto* gameObject = UnityEngine::GameObject::New_ctor();
// Remember to handle cleanup appropriately
```

**Configuration Systems:**
- Replace PC config libraries with Quest alternatives
- Implement config-utils for Quest-compatible settings
- Ensure configuration persists correctly on Quest storage

**UI Components:**
- Convert PC UI elements to BSML (Beat Saber Markup Language)
- Adapt layouts for Quest's different screen resolution/aspect ratio
- Implement touch-friendly interfaces where applicable

### 5. Dependencies and Libraries

**Library Mapping:**
- Identify Quest equivalents for PC-only libraries
- Remove or replace Windows-specific dependencies
- Use Quest-compatible versions of common libraries

**Mod Dependencies:**
- Update dependency declarations in mod.json
- Ensure all required Quest mods are available
- Test compatibility with popular Quest mod combinations

### 6. Testing and Debugging

**Development Testing:**
- Use ADB for log monitoring during development
- Test on actual Quest hardware (not just emulation)
- Validate memory usage and performance impacts

**Compatibility Testing:**
- Test with different Beat Saber versions supported on Quest
- Verify compatibility with common Quest mods
- Test multiplayer functionality if applicable

### 7. Optimization for Quest Hardware

**Performance Considerations:**
- Optimize for Quest's mobile CPU/GPU limitations
- Reduce memory allocations and garbage collection pressure
- Implement efficient rendering for Quest's display requirements

**Battery Life:**
- Minimize background processing
- Implement proper sleep/wake handling
- Optimize graphics and effects for power efficiency

## Common Challenges and Solutions

### Challenge 1: BSIPA to Quest Mod Loader Migration
**Problem**: PC mods rely heavily on BSIPA's plugin architecture
**Solution**: Rewrite initialization code for Quest mod loaders like MBF, implementing proper mod lifecycle management

### Challenge 2: Harmony Patches
**Problem**: PC mods use Harmony for runtime patching
**Solution**: Convert to native C++ hooks using Dobby, ensuring proper IL2CPP method resolution

### Challenge 3: UI Framework Differences
**Problem**: PC UI frameworks don't exist on Quest
**Solution**: Rebuild UI using BSML, adapting layouts for VR-specific interactions and Quest's interface guidelines

### Challenge 4: File System Access
**Problem**: Different file system permissions and paths
**Solution**: Use Quest-appropriate storage locations and handle Android permission requirements

## Best Practices

### Code Organization
- Maintain clean separation between game logic and platform-specific code
- Use abstract interfaces to isolate platform dependencies
- Implement proper error handling for Quest-specific edge cases

### Performance
- Profile memory usage regularly during development
- Minimize allocations in performance-critical paths
- Use Quest-optimized graphics and rendering techniques

### User Experience
- Design for VR interaction patterns, not desktop metaphors
- Ensure mod settings are easily accessible in VR
- Provide clear visual feedback for all mod actions

### Documentation
- Document all platform-specific implementation decisions
- Maintain compatibility notes for different Quest firmware versions
- Provide clear installation and troubleshooting guides

## Release and Distribution

### Testing Checklist
- [ ] Basic functionality works on Quest
- [ ] No crashes during typical gameplay
- [ ] Settings and configuration persist correctly
- [ ] Compatible with popular Quest mod combinations
- [ ] Performance meets Quest hardware limitations
- [ ] UI is usable in VR environment

### Distribution
- Package according to Quest mod standards
- Provide clear installation instructions
- Include compatibility information
- Submit to appropriate Quest mod repositories

## Additional Resources

### Communities and Support
- **BSMG Discord**: Primary community for Beat Saber modding support
- **Quest Modding Channels**: Platform-specific help and resources
- **GitHub Repositories**: Source code examples and libraries

### Documentation
- BSMG Wiki Quest Modding sections
- Quest mod development tutorials
- IL2CPP and Unity documentation for Quest development

### Tools and Libraries
- config-utils: Quest configuration management
- BSML: UI framework for Quest mods  
- Questboard: Quest mod distribution platform

## Conclusion

Porting a Beat Saber mod from PC to Quest requires substantial code rewriting and platform-specific knowledge. While challenging, successful ports can bring popular PC mod functionality to the Quest platform. The key to success is understanding the fundamental architectural differences and taking a systematic approach to the conversion process.

Remember that not all PC mod features may be feasible on Quest due to hardware limitations, and some may need to be redesigned or simplified for the VR environment. Always prioritize user experience and performance on the Quest platform.

---

# Technical Beat Saber Mod Porting Guide: PC to Quest Implementation

## Architecture Deep Dive

### Core Differences Between PC and Quest

**PC Beat Saber Modding Stack:**
```
Beat Saber (C# Managed Code)
        ↓
BSIPA (Beat Saber Illusion Plugin Architecture)
        ↓
Harmony (Runtime C# Patching)
        ↓
.NET Framework/Mono Runtime
        ↓
Windows/Linux/macOS
```

**Quest Beat Saber Modding Stack:**
```
Beat Saber (IL2CPP Compiled C++)
        ↓
beatsaber-hook (Native C++ Hooking)
        ↓
Dobby/MSHook (Native Function Hooking)
        ↓
Android Runtime (ART)
        ↓
Android/ARM64
```

### IL2CPP Compilation Impact

il2cpp is the mechanism that Unity uses to compile game code into C++. This fundamental difference means:

- **PC**: Direct access to managed C# objects and reflection
- **Quest**: Access through IL2CPP metadata and native pointers
- **Memory Management**: PC uses garbage collection, Quest requires manual pointer management
- **Method Calls**: PC uses direct C# invocation, Quest uses IL2CPP function pointers

## Development Environment Setup

### Prerequisites and Tools

**Essential Dependencies:**
```bash
# Android NDK (required for C++ compilation)
export ANDROID_NDK_ROOT="/path/to/android-ndk-r25c"

# Android SDK (for ADB and debugging)
export ANDROID_SDK_ROOT="/path/to/android-sdk"
export PATH="$ANDROID_SDK_ROOT/platform-tools:$PATH"

# QPM (Quest Package Manager)
# Download from GitHub Actions artifacts
qpm --version  # Verify installation
```

**Project Template Setup:**
```bash
# Clone the project template
git clone https://github.com/Lauriethefish/quest-mod-template.git my-mod

# Configure template variables
# Replace #{ndkpath} with your NDK path
# Replace #{id} with mod ID (no spaces)
# Replace #{name} with readable mod name
# Replace #{author} with your name
# Replace #{description} with mod description

# Initialize dependencies
cd my-mod
qpm restore
```

### CMakeLists.txt Configuration

**Essential CMake Setup:**
```cmake
cmake_minimum_required(VERSION 3.21)
project(${COMPILE_ID})

# Include beatsaber-hook and codegen
find_package(beatsaber-hook REQUIRED)
find_package(codegen REQUIRED)

# Create shared library
add_library(
    ${COMPILE_ID}
    SHARED
    src/main.cpp
    # Add your source files here
)

# Link essential libraries
target_link_libraries(${COMPILE_ID} 
    beatsaber-hook::beatsaber-hook
    codegen::codegen
    # Add other dependencies
)

# Compiler flags for optimization
target_compile_options(${COMPILE_ID} PRIVATE 
    -O3
    -fvisibility=hidden
    -ffunction-sections
    -fdata-sections
)
```

## Code Translation Patterns

### 1. Hook System Translation

**PC BSIPA Hook (C#):**
```csharp
[HarmonyPatch(typeof(ScoreController), "HandleNoteWasCut")]
public static class ScoreControllerPatch
{
    static void Postfix(ScoreController __instance, NoteController noteController)
    {
        // Mod logic here
        Console.WriteLine($"Note cut with score: {__instance.multipliedScore}");
    }
}
```

**Quest beatsaber-hook Translation (C++):**
```cpp
#include "GlobalNamespace/ScoreController.hpp"
#include "GlobalNamespace/NoteController.hpp"

MAKE_HOOK_MATCH(
    ScoreController_HandleNoteWasCut,
    &GlobalNamespace::ScoreController::HandleNoteWasCut,
    void,
    GlobalNamespace::ScoreController* self,
    GlobalNamespace::NoteController* noteController,
    GlobalNamespace::NoteCutInfo* noteCutInfo
) {
    // Call original method first
    ScoreController_HandleNoteWasCut(self, noteController, noteCutInfo);
    
    // Mod logic here
    int multipliedScore = self->get_multipliedScore();
    getLogger().info("Note cut with score: %d", multipliedScore);
}

extern "C" void load() {
    il2cpp_functions::Init();
    INSTALL_HOOK(getLogger(), ScoreController_HandleNoteWasCut);
}
```

### 2. Property Access Translation

**PC Property Access (C#):**
```csharp
// Direct property access
var playerController = FindObjectOfType<PlayerController>();
float energy = playerController.energy;
playerController.energy = 0.5f;
```

**Quest Property Access (C++):**
```cpp
// Through IL2CPP utilities or codegen
#include "GlobalNamespace/GameEnergyCounter.hpp"

auto* energyCounter = UnityEngine::Object::FindObjectOfType<GlobalNamespace::GameEnergyCounter*>();
if (energyCounter != nullptr) {
    // Get property value
    float energy = energyCounter->get_energy();
    
    // Set property value  
    energyCounter->set_energy(0.5f);
}
```

### 3. GameObject Manipulation

**PC GameObject Creation (C#):**
```csharp
var myObject = new GameObject("MyCustomObject");
myObject.transform.SetParent(parentTransform);
var textComponent = myObject.AddComponent<TextMeshProUGUI>();
textComponent.text = "Hello World";
```

**Quest GameObject Creation (C++):**
```cpp
#include "UnityEngine/GameObject.hpp"
#include "HMUI/CurvedTextMeshPro.hpp"

// Create GameObject
auto* myObject = UnityEngine::GameObject::New_ctor(
    il2cpp_utils::newcsstr("MyCustomObject")
);

// Set parent
myObject->get_transform()->SetParent(parentTransform, false);

// Add component
auto* textComponent = myObject->AddComponent<HMUI::CurvedTextMeshPro*>();
textComponent->set_text(il2cpp_utils::newcsstr("Hello World"));
```

### 4. Configuration System Translation

**PC Configuration (C#):**
```csharp
[Serializable]
public class ModConfig
{
    public bool EnableFeature { get; set; } = true;
    public float SpeedMultiplier { get; set; } = 1.0f;
}

// Usage
var config = Configuration.Load<ModConfig>();
config.EnableFeature = false;
Configuration.Save(config);
```

**Quest Configuration (C++):**
```cpp
#include "config-utils/shared/config-utils.hpp"

// Define configuration structure
DECLARE_CONFIG(ModConfig,
    CONFIG_VALUE(EnableFeature, bool, "Enable Feature", true);
    CONFIG_VALUE(SpeedMultiplier, float, "Speed Multiplier", 1.0f);
)

// Usage in code
void load() {
    getModConfig().Init(modInfo);
    
    bool featureEnabled = getModConfig().EnableFeature.GetValue();
    if (featureEnabled) {
        float speed = getModConfig().SpeedMultiplier.GetValue();
        // Use configuration values
    }
}
```

## Advanced Implementation Patterns

### 1. Custom Types Creation

**Creating Custom MonoBehaviour:**
```cpp
#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"

DECLARE_CLASS_CODEGEN(MyNamespace, CustomBehaviour, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_METHOD(void, Start);
    DECLARE_INSTANCE_METHOD(void, Update);
    
    DECLARE_INSTANCE_FIELD(float, timer);
    DECLARE_INSTANCE_FIELD(bool, isActive);
)

// Implementation
DEFINE_TYPE(MyNamespace, CustomBehaviour);

void MyNamespace::CustomBehaviour::Start() {
    timer = 0.0f;
    isActive = true;
}

void MyNamespace::CustomBehaviour::Update() {
    if (isActive) {
        timer += UnityEngine::Time::get_deltaTime();
        if (timer > 5.0f) {
            // Do something after 5 seconds
            isActive = false;
        }
    }
}

// Register in load()
extern "C" void load() {
    custom_types::Register::AutoRegister();
}
```

### 2. UI Creation with QuestUI

**Settings Menu Implementation:**
```cpp
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

void DidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (firstActivation) {
        auto* container = QuestUI::BeatSaberUI::CreateScrollableSettingsContainer(self->get_transform());
        
        // Toggle setting
        auto* toggle = QuestUI::BeatSaberUI::CreateToggle(
            container->get_transform(),
            "Enable Feature",
            getModConfig().EnableFeature.GetValue(),
            [](bool value) {
                getModConfig().EnableFeature.SetValue(value);
            }
        );
        
        // Slider setting
        auto* slider = QuestUI::BeatSaberUI::CreateSliderSetting(
            container->get_transform(),
            "Speed Multiplier",
            0.1f, 5.0f,
            getModConfig().SpeedMultiplier.GetValue(),
            [](float value) {
                getModConfig().SpeedMultiplier.SetValue(value);
            }
        );
    }
}

extern "C" void load() {
    QuestUI::Register::RegisterModSettingsViewController(modInfo, DidActivate);
}
```

### 3. Memory Management Patterns

**Safe Pointer Handling:**
```cpp
// Always check for null pointers
template<typename T>
bool IsValidPointer(T* ptr) {
    return ptr != nullptr && 
           reinterpret_cast<uintptr_t>(ptr) > 0x1000; // Basic sanity check
}

// Safe method invocation
template<typename T, typename... Args>
auto SafeInvoke(T* obj, void(T::*method)(Args...), Args... args) -> bool {
    if (!IsValidPointer(obj)) {
        getLogger().error("Invalid object pointer");
        return false;
    }
    
    try {
        (obj->*method)(args...);
        return true;
    } catch (...) {
        getLogger().error("Exception during method invocation");
        return false;
    }
}
```

### 4. Asset Loading and Management

**Loading Custom Assets:**
```cpp
#include "assets.hpp" // Generated by build process

void LoadCustomAssets() {
    // Load embedded asset data
    auto assetData = Assets::MyCustomTexture_png;
    
    // Create Unity texture from raw data
    auto* texture = QuestUI::BeatSaberUI::ArrayToTexture(assetData);
    
    // Create sprite from texture
    auto* sprite = QuestUI::BeatSaberUI::ArrayToSprite(assetData);
    
    // Use sprite in UI
    auto* image = QuestUI::BeatSaberUI::CreateImage(
        parentTransform,
        sprite,
        UnityEngine::Vector2(0.0f, 0.0f),
        UnityEngine::Vector2(50.0f, 50.0f)
    );
}
```

## Debugging and Development Workflow

### 1. Logging System

**Comprehensive Logging:**
```cpp
#include "beatsaber-hook/shared/utils/logging.hpp"

// Different log levels
getLogger().debug("Debug information: %s", debugString.c_str());
getLogger().info("General information: %d", value);
getLogger().warning("Warning: Invalid state detected");
getLogger().error("Error: Failed to load resource");
getLogger().critical("Critical: System failure imminent");

// Conditional logging
#ifdef DEBUG
    getLogger().debug("Debug build information");
#endif

// Object inspection logging
void LogObjectInfo(Il2CppObject* obj) {
    if (!IsValidPointer(obj)) {
        getLogger().error("Invalid object pointer");
        return;
    }
    
    auto* klass = obj->klass;
    if (IsValidPointer(klass)) {
        getLogger().info("Object type: %s::%s", 
                        klass->namespaze, 
                        klass->name);
    }
}
```

### 2. Real-time Debugging

**ADB Logcat Commands:**
```bash
# Filter for your mod specifically
adb logcat QuestHook[YourModID*]:* AndroidRuntime:E *:S

# Monitor all Quest mod logs
adb logcat | grep QuestHook

# Save logs to file for analysis
adb logcat QuestHook[YourModID*]:* > debug_log.txt

# Clear log buffer before testing
adb logcat -c
```

### 3. Error Handling Patterns

**Robust Error Handling:**
```cpp
class ModSafetyWrapper {
private:
    static bool initialized;
    
public:
    template<typename Func>
    static auto SafeExecute(const std::string& operation, Func&& func) -> decltype(func()) {
        if (!initialized) {
            getLogger().error("Attempting to execute %s before initialization", operation.c_str());
            return {};
        }
        
        try {
            return func();
        } catch (const std::exception& e) {
            getLogger().error("Exception in %s: %s", operation.c_str(), e.what());
            return {};
        } catch (...) {
            getLogger().error("Unknown exception in %s", operation.c_str());
            return {};
        }
    }
};

// Usage
auto result = ModSafetyWrapper::SafeExecute("ScoreUpdate", [&]() {
    return UpdatePlayerScore(newScore);
});
```

## Performance Optimization for Quest

### 1. Memory Pool Management

**Object Pooling Pattern:**
```cpp
template<typename T>
class ObjectPool {
private:
    std::queue<std::unique_ptr<T>> available;
    std::vector<std::unique_ptr<T>> all;
    
public:
    T* Acquire() {
        if (available.empty()) {
            auto obj = std::make_unique<T>();
            T* ptr = obj.get();
            all.push_back(std::move(obj));
            return ptr;
        }
        
        auto obj = std::move(available.front());
        available.pop();
        T* ptr = obj.get();
        obj.release(); // Don't delete when unique_ptr goes out of scope
        return ptr;
    }
    
    void Release(T* obj) {
        available.push(std::unique_ptr<T>(obj));
    }
};

// Usage for frequently created objects
static ObjectPool<CustomEffect> effectPool;
```

### 2. Efficient Update Patterns

**Optimized Update Loops:**
```cpp
class OptimizedUpdater {
private:
    static constexpr float UPDATE_INTERVAL = 0.016f; // ~60 FPS
    float lastUpdateTime = 0.0f;
    
public:
    bool ShouldUpdate() {
        float currentTime = UnityEngine::Time::get_time();
        if (currentTime - lastUpdateTime >= UPDATE_INTERVAL) {
            lastUpdateTime = currentTime;
            return true;
        }
        return false;
    }
};

// In your update hook
MAKE_HOOK_MATCH(Update_Hook, &SomeClass::Update, void, SomeClass* self) {
    Update_Hook(self);
    
    static OptimizedUpdater updater;
    if (updater.ShouldUpdate()) {
        // Your expensive update logic here
    }
}
```

### 3. Battery Life Considerations

**Power-Efficient Implementations:**
```cpp
class PowerManager {
private:
    bool isInBackground = false;
    float backgroundUpdateInterval = 1.0f; // Slower updates when not active
    
public:
    void OnApplicationPause(bool pauseStatus) {
        isInBackground = pauseStatus;
        
        if (isInBackground) {
            // Reduce update frequency
            // Disable expensive visual effects
            // Save state
        } else {
            // Resume normal operation
        }
    }
    
    bool ShouldPerformExpensiveOperation() {
        return !isInBackground && 
               UnityEngine::Time::get_deltaTime() < 0.02f; // Good framerate
    }
};
```

## Platform-Specific Implementation Details

### 1. File System Access

**Quest Storage Patterns:**
```cpp
#include <android/asset_manager.h>
#include <fstream>

class QuestFileManager {
public:
    static std::string GetModDataPath() {
        return "/sdcard/Android/data/com.beatgames.beatsaber/files/mods/YourModID/";
    }
    
    static bool EnsureDirectoryExists(const std::string& path) {
        return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
    }
    
    static bool WriteConfigFile(const std::string& filename, const std::string& data) {
        std::string fullPath = GetModDataPath() + filename;
        EnsureDirectoryExists(GetModDataPath());
        
        std::ofstream file(fullPath);
        if (file.is_open()) {
            file << data;
            return true;
        }
        return false;
    }
    
    static std::string ReadConfigFile(const std::string& filename) {
        std::string fullPath = GetModDataPath() + filename;
        std::ifstream file(fullPath);
        
        if (file.is_open()) {
            return std::string((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
        }
        return "";
    }
};
```

### 2. Input System Differences

**VR Controller Input:**
```cpp
#include "UnityEngine/XR/XRNode.hpp"
#include "UnityEngine/XR/InputTracking.hpp"

class VRInputHandler {
public:
    static UnityEngine::Vector3 GetControllerPosition(bool rightHand) {
        auto node = rightHand ? UnityEngine::XR::XRNode::RightHand 
                             : UnityEngine::XR::XRNode::LeftHand;
        return UnityEngine::XR::InputTracking::GetLocalPosition(node);
    }
    
    static UnityEngine::Quaternion GetControllerRotation(bool rightHand) {
        auto node = rightHand ? UnityEngine::XR::XRNode::RightHand 
                             : UnityEngine::XR::XRNode::LeftHand;
        return UnityEngine::XR::InputTracking::GetLocalRotation(node);
    }
    
    static bool IsControllerConnected(bool rightHand) {
        auto node = rightHand ? UnityEngine::XR::XRNode::RightHand 
                             : UnityEngine::XR::XRNode::LeftHand;
        return UnityEngine::XR::InputTracking::GetNodeStates(node);
    }
};
```

## Build System and Distribution

### 1. Build Scripts

**PowerShell Build Script (build.ps1):**
```powershell
param(
    [Parameter(Mandatory=$false)]
    [Switch] $clean
)

if ($clean.IsPresent) {
    if (Test-Path -Path "build") {
        Remove-Item -Path build -Force -Recurse
    }
}

if (!(Test-Path -Path "build")) {
    New-Item -ItemType Directory -Path build
}

Set-Location build
& cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
& cmake --build . -j 8
Set-Location ..

if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed"
    exit 1
}

Write-Host "Build completed successfully" -ForegroundColor Green
```

### 2. QMOD Packaging

**Automated QMOD Creation:**
```powershell
# buildQMOD.ps1
& .\build.ps1
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$qmodName = "YourModID_v1.0.0.qmod"

# Create temporary packaging directory
$tempDir = "temp_qmod"
if (Test-Path $tempDir) { Remove-Item $tempDir -Recurse -Force }
New-Item -ItemType Directory $tempDir

# Copy mod files
Copy-Item "build/libYourModID.so" "$tempDir/"
Copy-Item "mod.json" "$tempDir/"

# Copy any additional resources
if (Test-Path "assets") {
    Copy-Item "assets" "$tempDir/" -Recurse
}

# Create QMOD zip
Compress-Archive -Path "$tempDir/*" -DestinationPath $qmodName -Force
Remove-Item $tempDir -Recurse -Force

Write-Host "QMOD created: $qmodName" -ForegroundColor Green
```

### 3. mod.json Configuration

**Complete mod.json Template:**
```json
{
    "_QPVersion": "0.1.1",
    "name": "Your Mod Name",
    "id": "YourModID",
    "author": "YourName",
    "version": "1.0.0",
    "packageId": "com.beatgames.beatsaber",
    "packageVersion": "1.29.1",
    "description": "Detailed description of your mod functionality",
    "dependencies": [
        {
            "id": "beatsaber-hook",
            "version": "^3.14.0",
            "downloadIfMissing": "https://github.com/sc2ad/beatsaber-hook/releases/latest"
        },
        {
            "id": "codegen",
            "version": "^0.23.0",
            "downloadIfMissing": "https://github.com/sc2ad/BeatSaber-Quest-Codegen/releases/latest"
        }
    ],
    "modFiles": [
        "libYourModID.so"
    ],
    "libraryFiles": [],
    "fileCopies": []
}
```

## Testing and Validation

### 1. Automated Testing Framework

**Unit Test Structure:**
```cpp
class ModTestRunner {
public:
    static void RunAllTests() {
        TestConfigurationSystem();
        TestHookInstallation();
        TestUICreation();
        TestMemoryManagement();
    }
    
private:
    static void TestConfigurationSystem() {
        getLogger().info("Testing configuration system...");
        
        // Test default values
        assert(getModConfig().EnableFeature.GetValue() == true);
        
        // Test value changes
        getModConfig().EnableFeature.SetValue(false);
        assert(getModConfig().EnableFeature.GetValue() == false);
        
        getLogger().info("Configuration system tests passed");
    }
    
    static void TestHookInstallation() {
        getLogger().info("Testing hook installation...");
        
        // Verify hooks are installed
        // This would require more sophisticated testing infrastructure
        
        getLogger().info("Hook installation tests passed");
    }
};

// Run tests in development builds
extern "C" void load() {
    #ifdef DEBUG
        ModTestRunner::RunAllTests();
    #endif
    
    // Normal mod initialization
}
```

### 2. Performance Profiling

**Memory Usage Monitoring:**
```cpp
class PerformanceProfiler {
private:
    static size_t peakMemoryUsage;
    static std::chrono::high_resolution_clock::time_point lastProfileTime;
    
public:
    static void ProfileMemoryUsage() {
        // Get current memory usage (platform-specific implementation needed)
        size_t currentUsage = GetCurrentMemoryUsage();
        
        if (currentUsage > peakMemoryUsage) {
            peakMemoryUsage = currentUsage;
            getLogger().info("New peak memory usage: %zu bytes", peakMemoryUsage);
        }
        
        // Log periodic memory reports
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - lastProfileTime);
        
        if (duration.count() >= 60) { // Every minute
            getLogger().info("Current memory usage: %zu bytes (Peak: %zu)", 
                           currentUsage, peakMemoryUsage);
            lastProfileTime = now;
        }
    }
};
```

## Conclusion

This technical guide provides the specific implementation details needed to successfully port Beat Saber mods from PC to Quest. The key challenges involve:

1. **Architecture Translation**: Converting from managed C# to native C++ with IL2CPP
2. **Hook System Migration**: Replacing Harmony patches with beatsaber-hook implementations  
3. **Memory Management**: Moving from garbage collection to manual pointer management
4. **Platform Optimization**: Accounting for Quest's mobile hardware limitations
5. **Development Workflow**: Adapting to Android-based development and debugging

Success requires understanding both platforms deeply and taking a systematic approach to the conversion process. The code examples and patterns provided here should serve as a foundation for tackling specific porting challenges you'll encounter with your particular mod.