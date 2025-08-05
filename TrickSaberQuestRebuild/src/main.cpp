#include "main.hpp"
#include "TrickSaber/Constants.hpp"
#include "TrickSaber/Utils/ErrorCircuitBreaker.hpp"

#include "scotland2/shared/modloader.h"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "custom-types/shared/register.hpp"
#include <unordered_map>

#include "GlobalNamespace/SaberManager.hpp"
#include "GlobalNamespace/SaberModelController.hpp"
#include "GlobalNamespace/GameScenesManager.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/NoteCutInfo.hpp"
#include "GlobalNamespace/GamePause.hpp"
#include "GlobalNamespace/SaberClashChecker.hpp"
#include "System/Collections/IEnumerator.hpp"
#include "System/Collections/Generic/List_1.hpp"
#include "System/Action_1.hpp"
#include "Zenject/DiContainer.hpp"
#include "beatsaber-hook/shared/utils/byref.hpp"

#include "TrickSaber/Core/TrickSaberManager.hpp"
#include "TrickSaber/Core/StateManager.hpp"
#include "TrickSaber/GlobalTrickManager.hpp"
#include "TrickSaber/CustomTypesRegistration.hpp"
#include "TrickSaber/EnhancedSaberManager.hpp"
#include "TrickSaber/BurnMarkHandler.hpp"
#include "TrickSaber/PhysicsHandler.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "TrickSaber/UI/SettingsViewController.hpp"
#include "TrickSaber/UI/AdvancedSettingsUI.hpp"
#include "TrickSaber/UI/DebugOverlay.hpp"
#include "TrickSaber/Configuration.hpp"
#include "TrickSaber/Config.hpp"
#include "TrickSaber/SaberTrickManager.hpp"
#include "TrickSaber/SaberTrickModel.hpp"
#include "TrickSaber/InputManager.hpp"
#include "TrickSaber/AdvancedInputSystem.hpp"
#include "TrickSaber/AdvancedTrickFeatures.hpp"
#include "TrickSaber/MovementController.hpp"
#include "GlobalNamespace/OculusVRHelper.hpp"
#include "UnityEngine/Time.hpp"
#include "TrickSaber/TrailHandler.hpp"
#include "TrickSaber/Tricks/SpinTrick.hpp"
#include "TrickSaber/Tricks/ThrowTrick.hpp"
#include "TrickSaber/Tricks/Trick.hpp"
#include "TrickSaber/Utils/ObjectCache.hpp"
#include "TrickSaber/Utils/PerformanceMetrics.hpp"
#include "TrickSaber/Utils/MemoryManager.hpp"
#include "TrickSaber/Utils/LazyInitializer.hpp"
#include "bsml/shared/BSML.hpp"
#include "System/GC.hpp"
#include <optional>

modloader::ModInfo modInfo{MOD_ID, VERSION, 0};

void PerformComprehensiveCleanup();
void ValidateCacheIfNeeded();
bool SafeExecute(const std::function<void()>& func, const char* context);

template<typename T>
std::optional<T> SafeExecuteWithReturn(const std::function<T()>& func, const char* context) {
    try {
        return func();
    } catch (const std::bad_alloc& e) {
        PaperLogger.error("Memory allocation failed in {}: {}", context, e.what());
        System::GC::Collect();
        TrickSaber::Utils::ObjectCache::ClearCache();
        TrickSaber::Utils::MemoryManager::ClearAllPools();
        return std::nullopt;
    } catch (const std::runtime_error& e) {
        PaperLogger.error("Runtime error in {}: {}", context, e.what());
        return std::nullopt;
    } catch (const std::logic_error& e) {
        PaperLogger.error("Logic error in {}: {}", context, e.what());
        return std::nullopt;
    } catch (const std::exception& e) {
        PaperLogger.error("Standard exception in {}: {}", context, e.what());
        return std::nullopt;
    } catch (...) {
        PaperLogger.error("Unknown exception in {}", context);
        return std::nullopt;
    }
}

// Hook implementations moved to separate files

void ValidateCacheIfNeeded() {
    // Only validate cache if it's been initialized
    if (!TrickSaber::Utils::ObjectCache::IsCacheInitialized()) {
        return;
    }
    
    static float lastValidationTime = 0.0f;
    static float avgFrameTime = TrickSaber::Constants::FRAME_TIME_MS;
    static int validationCounter = 0;
    
    float currentTime = UnityEngine::Time::get_time();
    float deltaTime = UnityEngine::Time::get_deltaTime();
    
    // Smooth frame time averaging for better load detection
    float currentFrameTime = deltaTime * 1000.0f;
    avgFrameTime = avgFrameTime * 0.9f + currentFrameTime * 0.1f;
    
    float loadFactor = std::min(avgFrameTime / TrickSaber::Constants::FRAME_TIME_MS, TrickSaber::Constants::MAX_LOAD_FACTOR);
    
    // Adaptive interval: increase significantly under load, reduce when idle
    auto globalManager = TrickSaber::GlobalTrickManager::GetInstance();
    bool isActive = globalManager && globalManager->IsDoingTrick();
    
    // Warm cache on first trick activation
    if (isActive && !TrickSaber::Utils::LazyCacheWarmer::IsWarmed()) {
        TrickSaber::Utils::LazyCacheWarmer::WarmCache();
    }
    
    float baseInterval = TrickSaber::Constants::CACHE_VALIDATION_INTERVAL_SEC;
    float adaptiveInterval = isActive ? 
        baseInterval * (1.0f + loadFactor * 0.5f) :  // Less aggressive scaling when active
        baseInterval * (1.5f + loadFactor);          // More aggressive when idle
    
    if (currentTime - lastValidationTime >= adaptiveInterval) {
        auto stateManager = TrickSaber::Core::StateManager::GetInstance();
        
        TrickSaber::Utils::ObjectCache::ValidateCache();
        stateManager->UpdateCacheValidationTime();
        
        if (validationCounter % 10 == 0) {  // Reduce debug frequency
            size_t cacheSize = TrickSaber::Utils::ObjectCache::GetCacheSize();
            PaperLogger.debug("Cache validated - size: {}, load: {:.1f}x, interval: {:.1f}s", 
                cacheSize, loadFactor, adaptiveInterval);
        }
        
        lastValidationTime = currentTime;
        validationCounter++;
    }
}

bool SafeExecute(const std::function<void()>& func, const char* context) {
    static TrickSaber::Utils::ErrorCircuitBreaker circuitBreaker;
    
    if (!circuitBreaker.ShouldExecute()) {
        PaperLogger.debug("Circuit breaker open for {}, skipping execution", context);
        return false;
    }
    
    try {
        func();
        circuitBreaker.RecordSuccess();
        return true;
    } catch (const std::bad_alloc& e) {
        PaperLogger.error("Memory allocation failed in {}: {}", context, e.what());
        circuitBreaker.RecordFailure();
        System::GC::Collect();
        TrickSaber::Utils::ObjectCache::ClearCache();
        return false;
    } catch (const std::runtime_error& e) {
        PaperLogger.error("Runtime error in {}: {}", context, e.what());
        circuitBreaker.RecordFailure();
        auto stateManager = TrickSaber::Core::StateManager::GetInstance();
        if (stateManager) {
            PaperLogger.error("State: initialized={}, noteCount={}", 
                stateManager->IsInitialized(), stateManager->GetNoteCount());
        }
        return false;
    } catch (const std::logic_error& e) {
        PaperLogger.error("Logic error in {}: {}", context, e.what());
        circuitBreaker.RecordFailure();
        PaperLogger.error("This indicates a programming error that should be fixed");
        return false;
    } catch (const std::exception& e) {
        PaperLogger.error("Standard exception in {}: {}", context, e.what());
        circuitBreaker.RecordFailure();
        return false;
    } catch (...) {
        PaperLogger.error("Unknown exception in {} - attempting recovery", context);
        circuitBreaker.RecordFailure();
        try {
            auto globalManager = TrickSaber::GlobalTrickManager::GetInstance();
            if (globalManager) {
                globalManager->EndAllTricks();
            }
        } catch (...) {
            PaperLogger.error("Recovery attempt failed in {}", context);
        }
        return false;
    }
}



void PerformComprehensiveCleanup() {
    PaperLogger.debug("Starting comprehensive TrickSaber cleanup");
    
    SafeExecute([]() {
        auto globalManager = TrickSaber::GlobalTrickManager::GetInstance();
        if (globalManager) {
            globalManager->EndAllTricks();
        }
    }, "Global trick cleanup");
    
    SafeExecute([]() {
        auto coreManager = TrickSaber::Core::TrickSaberManager::GetInstance();
        if (coreManager) {
            coreManager->RemoveSlowmo();
            coreManager->SetTimeScale(TrickSaber::Constants::NORMAL_TIME_SCALE);
        }
    }, "Time scale reset");
    
    SafeExecute([]() {
        auto enhancedManager = TrickSaber::EnhancedSaberManager::GetInstance();
        if (enhancedManager) {
            enhancedManager->ResetSaberStates();
        }
        
        TrickSaber::Core::TrickSaberManager::Cleanup();
        TrickSaber::GlobalTrickManager::Cleanup();
    }, "Manager cleanup");
    
    SafeExecute([]() {
        auto stateManager = TrickSaber::Core::StateManager::GetInstance();
        stateManager->Reset();
    }, "State manager cleanup");
    
    SafeExecute([]() {
        TrickSaber::Utils::MemoryManager::ClearAllPools();
        System::GC::Collect();
    }, "Memory cleanup and garbage collection");
    
    PaperLogger.debug("TrickSaber cleanup completed");
}

Configuration& getConfig() {
    static Configuration config(modInfo);
    return config;
}

extern "C" void setup(CModInfo* info) noexcept {
    *info = modInfo.to_c();
    
    Paper::Logger::RegisterFileContextId(PaperLogger.tag);
    PaperLogger.info("TrickSaber setup starting...");
}

extern "C" void load() noexcept {
    il2cpp_functions::Init();
    
    TrickSaber::RegisterCustomTypes();
    PaperLogger.info("Custom types registered");
    
    TrickSaber::LoadConfig();
    PaperLogger.info("Configuration loaded");
}

// Additional hook implementations moved to separate files

// Hook declarations from separate files
extern void InstallSaberHooks();
extern void InstallGameplayHooks();
extern void InstallSceneHooks();

extern "C" void late_load() noexcept {
    // Install hooks from separate files
    InstallSaberHooks();
    InstallGameplayHooks();
    InstallSceneHooks();
    
    BSML::Register::RegisterSettingsMenu("TrickSaber", TrickSaber::UI::SettingsViewControllerDidActivate, false);
    PaperLogger.info("TrickSaber settings UI registered successfully");
    
    if (TrickSaber::config.showDebugOverlay) {
        TrickSaber::UI::LazyDebugOverlayCreator::CreateOverlayIfNeeded();
    }
    
    PaperLogger.info("TrickSaber loaded successfully with modular hooks!");
}

