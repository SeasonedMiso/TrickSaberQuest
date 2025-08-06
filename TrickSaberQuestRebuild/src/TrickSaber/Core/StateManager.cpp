#include "TrickSaber/Core/StateManager.hpp"
#include "TrickSaber/Constants.hpp"
#include "TrickSaber/Utils/ObjectCache.hpp"
#include "TrickSaber/BurnMarkHandler.hpp"
#include "TrickSaber/MovementController.hpp"
#include "main.hpp"

namespace TrickSaber::Core {
    std::unique_ptr<StateManager> StateManager::instance = nullptr;
    std::mutex StateManager::instanceMutex;

    StateManager::~StateManager() {
        Reset();
    }

    StateManager* StateManager::GetInstance() {
        std::lock_guard<std::mutex> lock(instanceMutex);
        if (!instance) {
            instance = std::unique_ptr<StateManager>(new StateManager());
        }
        return instance.get();
    }

    void StateManager::DestroyInstance() {
        std::lock_guard<std::mutex> lock(instanceMutex);
        instance.reset();
    }

    void StateManager::AddSaberTransform(GlobalNamespace::Saber* saber, UnityEngine::Transform* transform) {
        if (saber && transform) {
            saberTransforms[saber] = transform;
        }
    }

    UnityEngine::Transform* StateManager::GetSaberTransform(GlobalNamespace::Saber* saber) const {
        auto it = saberTransforms.find(saber);
        return it != saberTransforms.end() ? it->second : nullptr;
    }

    bool StateManager::ShouldValidateCache() const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastCacheValidation);
        return elapsed.count() > Constants::CACHE_VALIDATION_INTERVAL_SEC;
    }

    void StateManager::UpdateCacheValidationTime() {
        lastCacheValidation = std::chrono::steady_clock::now();
    }

    void StateManager::Reset() {
        Logger.debug("Resetting StateManager");
        
        saberManager = nullptr;
        leftSaber = nullptr;
        rightSaber = nullptr;
        saberTransforms.clear();
        noteCount = 0;
        isInitialized = false;
        perfMetrics = nullptr;
        
        Utils::ObjectCache::ClearCache();
        BurnMarkHandler::ClearCache();
        MovementController::ClearBuffers();
        
        Logger.debug("StateManager reset completed");
    }
}