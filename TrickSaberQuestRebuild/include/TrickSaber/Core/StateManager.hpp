#pragma once

#include <unordered_map>
#include <chrono>
#include <memory>
#include <mutex>

#include "GlobalNamespace/SaberManager.hpp"
#include "GlobalNamespace/Saber.hpp"
#include "UnityEngine/Transform.hpp"
#include "TrickSaber/SaberTrickManager.hpp"
#include "TrickSaber/Utils/PerformanceMetrics.hpp"

namespace TrickSaber::Core {
    class StateManager {
    private:
        static std::unique_ptr<StateManager> instance;
        static std::mutex instanceMutex;

        GlobalNamespace::SaberManager* saberManager = nullptr;
        SaberTrickManager* leftSaber = nullptr;
        SaberTrickManager* rightSaber = nullptr;
        std::unordered_map<GlobalNamespace::Saber*, UnityEngine::Transform*> saberTransforms;
        int noteCount = 0;
        bool isInitialized = false;
        std::chrono::steady_clock::time_point lastCacheValidation;
        Utils::PerformanceMetrics* perfMetrics = nullptr;

        StateManager() = default;

    public:
        ~StateManager();
        
        static StateManager* GetInstance();
        static void DestroyInstance();

        // Getters
        GlobalNamespace::SaberManager* GetSaberManager() const { return saberManager; }
        SaberTrickManager* GetLeftSaber() const { return leftSaber; }
        SaberTrickManager* GetRightSaber() const { return rightSaber; }
        const auto& GetSaberTransforms() const { return saberTransforms; }
        int GetNoteCount() const { return noteCount; }
        bool IsInitialized() const { return isInitialized; }
        Utils::PerformanceMetrics* GetPerformanceMetrics() const { return perfMetrics; }

        // Setters
        void SetSaberManager(GlobalNamespace::SaberManager* manager) { saberManager = manager; }
        void SetLeftSaber(SaberTrickManager* saber) { leftSaber = saber; }
        void SetRightSaber(SaberTrickManager* saber) { rightSaber = saber; }
        void SetInitialized(bool initialized) { isInitialized = initialized; }
        void SetPerformanceMetrics(Utils::PerformanceMetrics* metrics) { perfMetrics = metrics; }

        // Transform management
        void AddSaberTransform(GlobalNamespace::Saber* saber, UnityEngine::Transform* transform);
        UnityEngine::Transform* GetSaberTransform(GlobalNamespace::Saber* saber) const;

        // Note counting
        void IncrementNoteCount() { noteCount++; }
        void DecrementNoteCount() { noteCount = std::max(0, noteCount - 1); }
        void ResetNoteCount() { noteCount = 0; }

        // Cache validation
        bool ShouldValidateCache() const;
        void UpdateCacheValidationTime();

        // Cleanup
        void Reset();
    };
}