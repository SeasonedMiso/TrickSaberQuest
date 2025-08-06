#include "TrickSaber/GlobalTrickManager.hpp"
#include "TrickSaber/SaberTrickManager.hpp"
#include "TrickSaber/SaberTrickModel.hpp"
#include "TrickSaber/Tricks/Trick.hpp"
#include "TrickSaber/Config.hpp"
#include "TrickSaber/Configuration.hpp"
#include "main.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/AudioSource.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include <algorithm>

DEFINE_TYPE(TrickSaber, GlobalTrickManager);

using namespace TrickSaber;

void GlobalTrickManager::Awake() {
    instance = this;
    saberClashEnabled = true;
    timeSinceLastNote = 0.0f;
    lastCacheUpdate = std::chrono::steady_clock::now();
    RefreshManagerCache();
}

void GlobalTrickManager::Update() {
    ValidateManagerCache();
    
    if (config.disableIfNotesOnScreen) {
        timeSinceLastNote += UnityEngine::Time::get_deltaTime();
    }
}

void GlobalTrickManager::OnDestroy() {
    if (instance == this) {
        instance = nullptr;
    }
}

GlobalTrickManager* GlobalTrickManager::GetInstance() {
    return instance;
}

void GlobalTrickManager::Initialize(GlobalNamespace::AudioTimeSyncController* audioController) {
    if (!audioController) return;
    
    auto go = UnityEngine::GameObject::New_ctor("GlobalTrickManager");
    UnityEngine::Object::DontDestroyOnLoad(go);
    
    auto manager = go->AddComponent<GlobalTrickManager*>();
    manager->audioController = audioController;
}

void GlobalTrickManager::Cleanup() {
    if (instance) {
        UnityEngine::Object::Destroy(instance->get_gameObject());
        instance = nullptr;
    }
}

void GlobalTrickManager::OnTrickStarted(TrickAction action) {
    saberClashEnabled = false;
    
    // Apply slowmo for throw tricks if enabled
    if (action == TrickAction::Throw && 
        Configuration::IsSlowmoDuringThrow() && 
        audioController && !slowmoApplied) {
        
        float currentTimeScale = audioController->get_timeScale();
        originalTimeScale = currentTimeScale;
        float targetTimeScale = currentTimeScale - Configuration::GetSlowmoAmount();
        if (targetTimeScale < 0.1f) targetTimeScale = 0.1f;
        
        // Start slowmo coroutine
        StartSlowmo(targetTimeScale);
        slowmoApplied = true;
        
        Logger.debug("Slowmo applied: {} -> {}", currentTimeScale, targetTimeScale);
    }
    
    Logger.debug("Trick started: {}, clash disabled", static_cast<int>(action));
}

void GlobalTrickManager::OnTrickEndRequested(TrickAction action) {
    // End slowmo for throw tricks
    if (action == TrickAction::Throw && 
        Configuration::IsSlowmoDuringThrow() && 
        audioController && slowmoApplied && 
        !IsTrickInState(TrickAction::Throw, TrickState::Started)) {
        
        EndSlowmo();
        slowmoApplied = false;
        
        Logger.debug("Slowmo ended for throw trick");
    }
    
    Logger.debug("Trick end requested: {}", static_cast<int>(action));
}

void GlobalTrickManager::OnTrickEnded(TrickAction action) {
    // Re-enable clash only when no tricks are active
    if (!IsDoingTrick()) {
        saberClashEnabled = true;
        
        // Clean up any lingering effects using cached managers
        auto managers = GetCachedManagers();
        for (auto manager : managers) {
            if (manager && manager->saberTrickModel) {
                // Ensure proper cleanup of trick models
                if (manager->saberTrickModel->IsUsingTrickModel()) {
                    manager->saberTrickModel->ChangeToActualSaber();
                }
            }
        }
        
        Logger.debug("All tricks ended, clash enabled, cleanup completed");
    }
}

bool GlobalTrickManager::IsTrickInState(TrickAction action, TrickState state) {
    auto managers = GetCachedManagers();
    
    for (auto manager : managers) {
        if (manager && manager->IsTrickInState(action, state)) {
            return true;
        }
    }
    return false;
}

bool GlobalTrickManager::IsDoingTrick() {
    auto managers = GetCachedManagers();
    
    for (auto manager : managers) {
        if (manager && manager->IsDoingTrick()) {
            return true;
        }
    }
    return false;
}

bool GlobalTrickManager::CanDoTrick() {
    if (!config.disableIfNotesOnScreen) return true;
    return timeSinceLastNote > 1.0f;
}

bool GlobalTrickManager::CanStartTrick(TrickAction action, int saberType) {
    if (!CanDoTrick()) return false;
    
    // Prevent conflicting tricks on same saber
    auto managers = GetCachedManagers();
    
    for (auto manager : managers) {
        if (!manager || !manager->saber) continue;
        
        int managerSaberType = static_cast<int>(manager->saber->get_saberType());
        if (managerSaberType == saberType && manager->IsDoingTrick()) {
            Logger.debug("Cannot start trick - saber {} already doing trick", saberType);
            return false;
        }
    }
    
    return true;
}

void GlobalTrickManager::EndAllTricks() {
    auto managers = GetCachedManagers();
    
    for (auto manager : managers) {
        if (manager) {
            manager->EndAllTricks();
            
            // Force cleanup of trick models
            if (manager->saberTrickModel && 
                manager->saberTrickModel->IsUsingTrickModel()) {
                manager->saberTrickModel->ChangeToActualSaber();
            }
        }
    }
    
    // Reset global state
    saberClashEnabled = true;
    timeSinceLastNote = 0.0f;
    
    Logger.debug("All tricks force ended with complete cleanup");
}

void GlobalTrickManager::UpdateNoteTimer(float deltaTime) {
    timeSinceLastNote += deltaTime;
}

void GlobalTrickManager::OnNoteSpawned() {
    timeSinceLastNote = 0.0f;
}

void GlobalTrickManager::RefreshManagerCache() {
    cachedManagers.clear();
    
    // Use FindObjectsOfType as fallback when cache needs refresh
    auto managers = UnityEngine::Object::FindObjectsOfType<SaberTrickManager*>();
    if (managers) {
        cachedManagers.reserve(managers->get_Length());
        for (int i = 0; i < managers->get_Length(); i++) {
            if (managers->_values[i]) {
                cachedManagers.push_back(managers->_values[i]);
            }
        }
    }
    
    lastCacheUpdate = std::chrono::steady_clock::now();
    Logger.debug("Manager cache refreshed: {} managers", cachedManagers.size());
}

void GlobalTrickManager::ValidateManagerCache() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastCacheUpdate);
    
    if (elapsed.count() > 5) { // Refresh every 5 seconds
        RefreshManagerCache();
    } else {
        // Quick validation - remove null managers
        cachedManagers.erase(
            std::remove_if(cachedManagers.begin(), cachedManagers.end(),
                [](SaberTrickManager* manager) { return !manager; }),
            cachedManagers.end());
    }
}

std::vector<TrickSaber::SaberTrickManager*> GlobalTrickManager::GetCachedManagers() {
    ValidateManagerCache();
    return cachedManagers;
}

void GlobalTrickManager::UpdateTricks() {
    auto managers = GetCachedManagers();
    
    for (auto manager : managers) {
        if (manager && manager->enabled) {
            manager->UpdateActiveTricks();
        }
    }
}
void GlobalTrickManager::StartSlowmo(float targetTimeScale) {
    if (!audioController) return;
    
    // Simple immediate slowmo application (coroutines would be more complex)
    SetTimeScale(targetTimeScale);
}

void GlobalTrickManager::EndSlowmo() {
    if (!audioController) return;
    
    // Restore original time scale
    SetTimeScale(originalTimeScale);
}

void GlobalTrickManager::SetTimeScale(float timeScale) {
    if (!audioController) return;
    
    // Simple audio pitch adjustment (basic slowmo effect)
    auto audioSource = audioController->GetComponent<UnityEngine::AudioSource*>();
    if (audioSource) {
        audioSource->set_pitch(timeScale);
        Logger.debug("Audio pitch set to: {:.2f}", timeScale);
    } else {
        Logger.debug("No audio source found for slowmo effect");
    }
}

int GlobalTrickManager::GetActiveThrowCount() {
    int count = 0;
    auto managers = GetCachedManagers();
    for (auto manager : managers) {
        if (manager && manager->IsTrickInState(TrickAction::Throw, TrickState::Started)) {
            count++;
        }
    }
    return count;
}

int GlobalTrickManager::GetActiveSpinCount() {
    int count = 0;
    auto managers = GetCachedManagers();
    for (auto manager : managers) {
        if (manager && manager->IsTrickInState(TrickAction::Spin, TrickState::Started)) {
            count++;
        }
    }
    return count;
}