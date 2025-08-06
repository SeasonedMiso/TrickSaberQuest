#include "main.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "TrickSaber/Core/StateManager.hpp"
#include "TrickSaber/Core/TrickSaberManager.hpp"
#include "TrickSaber/GlobalTrickManager.hpp"
#include "TrickSaber/EnhancedSaberManager.hpp"
#include "TrickSaber/AdvancedInputSystem.hpp"
#include "TrickSaber/AdvancedTrickFeatures.hpp"
#include "TrickSaber/BurnMarkHandler.hpp"
#include "TrickSaber/Utils/MemoryManager.hpp"
#include "TrickSaber/Utils/ObjectCache.hpp"
#include "GlobalNamespace/SaberManager.hpp"
#include "GlobalNamespace/SaberModelController.hpp"
#include "GlobalNamespace/SaberClashChecker.hpp"
#include "beatsaber-hook/shared/utils/byref.hpp"

extern bool SafeExecute(const std::function<void()>& func, const char* context);

MAKE_HOOK_MATCH(SaberModelController_Init, &GlobalNamespace::SaberModelController::Init, void, 
    GlobalNamespace::SaberModelController* self, UnityEngine::Transform* parent, GlobalNamespace::Saber* saber) {
    
    SaberModelController_Init(self, parent, saber);
    
    if (saber && self) {
        auto saberTransform = saber->get_transform();
        if (saberTransform) {
            auto stateManager = TrickSaber::Core::StateManager::GetInstance();
            stateManager->AddSaberTransform(saber, saberTransform);
            Logger.debug("Captured transform for {} saber", 
                saber->get_saberType() == GlobalNamespace::SaberType::SaberA ? "left" : "right");
        }
    }
}

MAKE_HOOK_MATCH(SaberManager_Start, &GlobalNamespace::SaberManager::Start, void, GlobalNamespace::SaberManager* self) {
    SaberManager_Start(self);
    
    auto stateManager = TrickSaber::Core::StateManager::GetInstance();
    stateManager->SetSaberManager(self);
    
    if (!self || !TrickSaber::config.trickSaberEnabled || stateManager->IsInitialized()) {
        return;
    }
    
    SafeExecute([self, stateManager]() {
        auto audioController = TrickSaber::Utils::ObjectCache::GetAudioController();
        if (!audioController) {
            Logger.error("AudioTimeSyncController not found - TrickSaber initialization failed");
            return;
        }
        
        // Initialize core TrickSaber systems without custom components
        // Custom components will be initialized later when needed
        
        TrickSaber::Core::TrickSaberManager::Initialize(self, audioController);
        TrickSaber::GlobalTrickManager::Initialize(audioController);
        TrickSaber::BurnMarkHandler::Initialize();
        TrickSaber::Utils::MemoryManager::Initialize();
        
        stateManager->SetInitialized(true);
        Logger.info("TrickSaber core initialized - expensive operations will be lazy loaded");
    }, "SaberManager initialization");
}

MAKE_HOOK_MATCH(SaberClashChecker_AreSabersClashing, &GlobalNamespace::SaberClashChecker::AreSabersClashing, bool, 
    GlobalNamespace::SaberClashChecker* self, ByRef<UnityEngine::Vector3> clashingPoint) {
    
    if (!self) return false;
    
    auto globalManager = TrickSaber::GlobalTrickManager::GetInstance();
    if (globalManager && globalManager->IsDoingTrick()) {
        return false;
    }
    
    return SaberClashChecker_AreSabersClashing(self, clashingPoint);
}

void InstallSaberHooks() {
    INSTALL_HOOK(Logger, SaberModelController_Init);
    INSTALL_HOOK(Logger, SaberManager_Start);
    INSTALL_HOOK(Logger, SaberClashChecker_AreSabersClashing);
    Logger.info("Saber hooks installed");
}