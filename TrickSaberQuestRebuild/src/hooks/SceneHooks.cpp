#include "main.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "TrickSaber/Core/StateManager.hpp"
#include "TrickSaber/GlobalTrickManager.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "UnityEngine/SceneManagement/Scene.hpp"
#include "UnityEngine/SceneManagement/LoadSceneMode.hpp"

extern bool SafeExecute(const std::function<void()>& func, const char* context);
extern void PerformComprehensiveCleanup();

MAKE_HOOK_MATCH(SceneManager_Internal_ActiveSceneChanged, &UnityEngine::SceneManagement::SceneManager::Internal_ActiveSceneChanged, void, 
    UnityEngine::SceneManagement::Scene previousActiveScene, UnityEngine::SceneManagement::Scene newActiveScene) {
    
    auto stateManager = TrickSaber::Core::StateManager::GetInstance();
    if (stateManager->IsInitialized()) {
        SafeExecute([]() {
            auto globalManager = TrickSaber::GlobalTrickManager::GetInstance();
            if (globalManager) {
                globalManager->EndAllTricks();
            }
            PerformComprehensiveCleanup();
        }, "Active scene change cleanup");
    }
    
    SceneManager_Internal_ActiveSceneChanged(previousActiveScene, newActiveScene);
}

MAKE_HOOK_MATCH(SceneManager_SetActiveScene, &UnityEngine::SceneManagement::SceneManager::SetActiveScene, bool, UnityEngine::SceneManagement::Scene scene) {
    SafeExecute([]() {
        auto stateManager = TrickSaber::Core::StateManager::GetInstance();
        stateManager->Reset();
    }, "Scene change cleanup");
    
    return SceneManager_SetActiveScene(scene);
}

MAKE_HOOK_MATCH(SceneManager_Internal_SceneLoaded, &UnityEngine::SceneManagement::SceneManager::Internal_SceneLoaded, void, UnityEngine::SceneManagement::Scene scene, UnityEngine::SceneManagement::LoadSceneMode mode) {
    SafeExecute([]() {
        auto stateManager = TrickSaber::Core::StateManager::GetInstance();
        stateManager->Reset();
    }, "Scene load cleanup");
    
    SceneManager_Internal_SceneLoaded(scene, mode);
}

void InstallSceneHooks() {
    INSTALL_HOOK(Logger, SceneManager_Internal_ActiveSceneChanged);
    INSTALL_HOOK(Logger, SceneManager_SetActiveScene);
    INSTALL_HOOK(Logger, SceneManager_Internal_SceneLoaded);
    Logger.info("Scene hooks installed");
}