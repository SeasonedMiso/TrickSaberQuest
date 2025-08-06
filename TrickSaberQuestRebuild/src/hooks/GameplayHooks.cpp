#include "main.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "TrickSaber/Core/StateManager.hpp"
#include "TrickSaber/GlobalTrickManager.hpp"
#include "TrickSaber/MovementController.hpp"
#include "TrickSaber/Utils/PerformanceMetrics.hpp"
#include "TrickSaber/Utils/LazyInitializer.hpp"
#include "TrickSaber/Constants.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/GamePause.hpp"
#include "GlobalNamespace/OculusVRHelper.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/NoteCutInfo.hpp"
#include "GlobalNamespace/VRController.hpp"
#include "beatsaber-hook/shared/utils/byref.hpp"
#include "UnityEngine/Time.hpp"

extern bool SafeExecute(const std::function<void()>& func, const char* context);

MAKE_HOOK_MATCH(BeatmapObjectSpawnController_HandleNoteDataCallback, 
    &GlobalNamespace::BeatmapObjectSpawnController::HandleNoteDataCallback, void, 
    GlobalNamespace::BeatmapObjectSpawnController* self, GlobalNamespace::NoteData* noteData) {
    
    BeatmapObjectSpawnController_HandleNoteDataCallback(self, noteData);
    
    if (TrickSaber::config.disableIfNotesOnScreen && noteData) {
        auto stateManager = TrickSaber::Core::StateManager::GetInstance();
        stateManager->IncrementNoteCount();
        
        auto globalManager = TrickSaber::GlobalTrickManager::GetInstance();
        if (globalManager && globalManager->IsDoingTrick()) {
            SafeExecute([globalManager]() {
                globalManager->OnNoteSpawned();
            }, "Note spawn handling");
        }
    }
}

MAKE_HOOK_MATCH(BeatmapObjectManager_HandleNoteControllerNoteWasCut, 
    &GlobalNamespace::BeatmapObjectManager::HandleNoteControllerNoteWasCut, void, 
    GlobalNamespace::BeatmapObjectManager* self, GlobalNamespace::NoteController* noteController, 
    ByRef<GlobalNamespace::NoteCutInfo> noteCutInfo) {
    
    BeatmapObjectManager_HandleNoteControllerNoteWasCut(self, noteController, noteCutInfo);
    
    if (TrickSaber::config.disableIfNotesOnScreen) {
        auto stateManager = TrickSaber::Core::StateManager::GetInstance();
        if (stateManager->GetNoteCount() > 0) {
            stateManager->DecrementNoteCount();
        }
    }
}

MAKE_HOOK_MATCH(BeatmapObjectManager_HandleNoteControllerNoteWasMissed, 
    &GlobalNamespace::BeatmapObjectManager::HandleNoteControllerNoteWasMissed, void, 
    GlobalNamespace::BeatmapObjectManager* self, GlobalNamespace::NoteController* noteController) {
    
    BeatmapObjectManager_HandleNoteControllerNoteWasMissed(self, noteController);
    
    if (TrickSaber::config.disableIfNotesOnScreen) {
        auto stateManager = TrickSaber::Core::StateManager::GetInstance();
        if (stateManager->GetNoteCount() > 0) {
            stateManager->DecrementNoteCount();
        }
    }
}

MAKE_HOOK_MATCH(GamePause_Pause, &GlobalNamespace::GamePause::Pause, void, GlobalNamespace::GamePause* self) {
    SafeExecute([]() {
        auto globalManager = TrickSaber::GlobalTrickManager::GetInstance();
        if (globalManager) {
            globalManager->EndAllTricks();
        }
    }, "Pause trick cleanup");
    
    GamePause_Pause(self);
}

MAKE_HOOK_MATCH(GamePause_Resume, &GlobalNamespace::GamePause::Resume, void, GlobalNamespace::GamePause* self) {
    SafeExecute([]() {
        auto stateManager = TrickSaber::Core::StateManager::GetInstance();
        if (stateManager) {
            // Reset time scale through global manager
            auto globalManager = TrickSaber::GlobalTrickManager::GetInstance();
            if (globalManager) {
                globalManager->EndAllTricks();
            }
        }
    }, "Resume time scale reset");
    
    GamePause_Resume(self);
}

MAKE_HOOK_MATCH(OculusVRHelper_FixedUpdate, &GlobalNamespace::OculusVRHelper::FixedUpdate, void, GlobalNamespace::OculusVRHelper* self) {
    OculusVRHelper_FixedUpdate(self);
    
    auto stateManager = TrickSaber::Core::StateManager::GetInstance();
    auto saberManager = stateManager->GetSaberManager();
    if (!TrickSaber::config.trickSaberEnabled || !saberManager) return;
    
    static int fixedUpdateCounter = 0;
    fixedUpdateCounter++;
    
    auto globalManager = TrickSaber::GlobalTrickManager::GetInstance();
    if (!globalManager || (!globalManager->IsDoingTrick() && fixedUpdateCounter % 3 != 0)) {
        return;
    }
    
    float deltaTime = UnityEngine::Time::get_fixedDeltaTime();
    if (deltaTime <= TrickSaber::Constants::MIN_DELTA_TIME) deltaTime = TrickSaber::Constants::FALLBACK_DELTA_TIME;
    
    auto leftSaber = saberManager->get_leftSaber();
    auto rightSaber = saberManager->get_rightSaber();
    
    UnityEngine::Transform* leftHand = nullptr;
    UnityEngine::Transform* rightHand = nullptr;
    
    if (leftSaber) {
        auto leftController = leftSaber->get_transform()->GetComponentInParent<GlobalNamespace::VRController*>();
        if (leftController) leftHand = leftController->get_transform();
    }
    
    if (rightSaber) {
        auto rightController = rightSaber->get_transform()->GetComponentInParent<GlobalNamespace::VRController*>();
        if (rightController) rightHand = rightController->get_transform();
    }
    
    TrickSaber::MovementController::UpdateVelocities(leftHand, rightHand, deltaTime);
}

MAKE_HOOK_MATCH(AudioTimeSyncController_Update, &GlobalNamespace::AudioTimeSyncController::Update, void, GlobalNamespace::AudioTimeSyncController* self) {
    AudioTimeSyncController_Update(self);
    
    auto stateManager = TrickSaber::Core::StateManager::GetInstance();
    auto saberManager = stateManager->GetSaberManager();
    if (!TrickSaber::config.trickSaberEnabled || !saberManager) return;
    
    static int updateCounter = 0;
    static float lastActiveTime = 0.0f;
    static bool wasDoingTrick = false;
    
    updateCounter++;
    float currentTime = UnityEngine::Time::get_time();
    
    auto globalManager = TrickSaber::GlobalTrickManager::GetInstance();
    if (!globalManager) return;
    
    bool isDoingTrick = globalManager->IsDoingTrick();
    bool trickStateChanged = isDoingTrick != wasDoingTrick;
    
    if (isDoingTrick || trickStateChanged) {
        lastActiveTime = currentTime;
        SafeExecute([globalManager]() {
            globalManager->UpdateTricks();
        }, "Trick update execution");
    }
    else {
        float idleTime = currentTime - lastActiveTime;
        int skipInterval = idleTime > 2.0f ? 15 : TrickSaber::Constants::TRICK_UPDATE_IDLE_INTERVAL;
        
        if (updateCounter % skipInterval == 0) {
            SafeExecute([globalManager]() {
                globalManager->UpdateTricks();
            }, "Trick update execution");
        }
    }
    
    if (updateCounter % TrickSaber::Constants::PERFORMANCE_UPDATE_INTERVAL_FRAMES == 0) {
        if (TrickSaber::Utils::PerformanceMetrics::IsInitialized()) {
            auto perfMetrics = TrickSaber::Utils::PerformanceMetrics::GetInstance();
            perfMetrics->UpdateFrameMetrics();
            perfMetrics->UpdateMemoryMetrics();
        } else if (isDoingTrick) {
            TrickSaber::Utils::LazyPerformanceSetup::Setup();
        }
    }
    
    wasDoingTrick = isDoingTrick;
}

void InstallGameplayHooks() {
    INSTALL_HOOK(Logger, BeatmapObjectSpawnController_HandleNoteDataCallback);
    INSTALL_HOOK(Logger, BeatmapObjectManager_HandleNoteControllerNoteWasCut);
    INSTALL_HOOK(Logger, BeatmapObjectManager_HandleNoteControllerNoteWasMissed);
    INSTALL_HOOK(Logger, GamePause_Pause);
    INSTALL_HOOK(Logger, GamePause_Resume);
    INSTALL_HOOK(Logger, OculusVRHelper_FixedUpdate);
    INSTALL_HOOK(Logger, AudioTimeSyncController_Update);
    Logger.info("Gameplay hooks installed");
}