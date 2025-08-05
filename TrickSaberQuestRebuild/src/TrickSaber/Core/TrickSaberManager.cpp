#include "TrickSaber/Core/TrickSaberManager.hpp"
#include "TrickSaber/SaberTrickManager.hpp"
#include "TrickSaber/BurnMarkHandler.hpp"
#include "TrickSaber/Config.hpp"
#include "TrickSaber/Constants.hpp"
#include "main.hpp"

#include "UnityEngine/Object.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/WaitForFixedUpdate.hpp"

DEFINE_TYPE(TrickSaber::Core, TrickSaberManager);

using namespace TrickSaber::Core;
using namespace GlobalNamespace;

void TrickSaberManager::Awake() {
    instance = this;
    originalTimeScale = Constants::NORMAL_TIME_SCALE;
    slowmoActive = false;
}

void TrickSaberManager::OnDestroy() {
    if (instance == this) {
        instance = nullptr;
    }
}

TrickSaberManager* TrickSaberManager::GetInstance() {
    return instance;
}

void TrickSaberManager::Initialize(SaberManager* saberManager, AudioTimeSyncController* audioController) {
    if (!saberManager || !audioController) {
        PaperLogger.error("SaberManager or AudioTimeSyncController is null");
        return;
    }
    
    auto go = UnityEngine::GameObject::New_ctor("TrickSaberManager");
    UnityEngine::Object::DontDestroyOnLoad(go);
    
    auto manager = go->AddComponent<TrickSaberManager*>();
    manager->audioController = audioController;
    manager->originalTimeScale = audioController->get_timeScale();
    
    manager->InitializeSabers(saberManager->get_leftSaber(), saberManager->get_rightSaber());
    
    PaperLogger.info("TrickSaberManager initialized successfully");
}

void TrickSaberManager::Cleanup() {
    if (instance) {
        UnityEngine::Object::Destroy(instance->get_gameObject());
        instance = nullptr;
    }
}

void TrickSaberManager::InitializeSabers(Saber* leftSaber, Saber* rightSaber) {
    if (leftSaber) {
        auto leftManager = leftSaber->get_gameObject()->AddComponent<TrickSaber::SaberTrickManager*>();
        leftManager->Initialize(leftSaber);
        PaperLogger.debug("Left saber trick manager initialized");
    }
    
    if (rightSaber) {
        auto rightManager = rightSaber->get_gameObject()->AddComponent<TrickSaber::SaberTrickManager*>();
        rightManager->Initialize(rightSaber);
        PaperLogger.debug("Right saber trick manager initialized");
    }
}

void TrickSaberManager::ApplySlowmo(float amount) {
    if (!audioController || slowmoActive) return;
    
    float targetScale = std::max(Constants::MIN_TIME_SCALE, originalTimeScale - amount);
    SetTimeScale(targetScale);
    slowmoActive = true;
    
    PaperLogger.debug("Slowmo applied: {:.2f}", targetScale);
}

void TrickSaberManager::RemoveSlowmo() {
    if (!audioController || !slowmoActive) return;
    
    SetTimeScale(originalTimeScale);
    slowmoActive = false;
    
    PaperLogger.debug("Slowmo removed, restored to: {:.2f}", originalTimeScale);
}

void TrickSaberManager::SetTimeScale(float scale) {
    if (!audioController) return;
    
    // Use reflection to set the time scale
    auto timeScaleField = il2cpp_utils::FindField(audioController, "_timeScale");
    if (timeScaleField) {
        il2cpp_utils::SetFieldValue(audioController, timeScaleField, scale);
    }
    
    // Update audio pitch if available
    auto audioSourceField = il2cpp_utils::FindField(audioController, "_audioSource");
    if (audioSourceField) {
        auto audioSource = il2cpp_utils::GetFieldValue<UnityEngine::AudioSource*>(audioController, audioSourceField);
        if (audioSource.has_value() && audioSource.value()) {
            // Use reflection to set pitch to avoid undefined symbol
            auto pitchMethod = il2cpp_utils::FindMethodUnsafe(audioSource.value(), "set_pitch", 1);
            if (pitchMethod) {
                il2cpp_utils::RunMethod(audioSource.value(), pitchMethod, scale);
            }
        }
    }
}

void TrickSaberManager::DisableBurnMarks(int saberType) {
    BurnMarkHandler::DisableBurnMarks(saberType);
}

void TrickSaberManager::EnableBurnMarks(int saberType) {
    BurnMarkHandler::EnableBurnMarks(saberType);
}

UnityEngine::Coroutine* TrickSaberManager::ApplySlowmoSmooth(float targetScale) {
    // Simplified implementation - direct time scale setting
    SetTimeScale(targetScale);
    return nullptr;
}

UnityEngine::Coroutine* TrickSaberManager::RemoveSlowmoSmooth() {
    // Simplified implementation - direct time scale setting
    SetTimeScale(originalTimeScale);
    return nullptr;
}