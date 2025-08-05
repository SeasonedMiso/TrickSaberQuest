#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "TrickSaber/Enums.hpp"
#include <vector>
#include <chrono>

namespace TrickSaber { class SaberTrickManager; }

DECLARE_CLASS_CODEGEN(TrickSaber, GlobalTrickManager, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);
    DECLARE_INSTANCE_METHOD(void, Update);
    
    DECLARE_STATIC_METHOD(GlobalTrickManager*, GetInstance);
    DECLARE_STATIC_METHOD(void, Initialize, GlobalNamespace::AudioTimeSyncController* audioController);
    DECLARE_STATIC_METHOD(void, Cleanup);
    
public:
    void OnTrickStarted(TrickAction action);
    void OnTrickEndRequested(TrickAction action);
    void OnTrickEnded(TrickAction action);
    bool IsTrickInState(TrickAction action, TrickState state);
    bool IsDoingTrick();
    bool CanDoTrick();
    bool CanStartTrick(TrickAction action, int saberType);
    void EndAllTricks();
    void UpdateNoteTimer(float deltaTime);
    void OnNoteSpawned();
    void UpdateTricks();
    
    // Debug stats
    int GetActiveThrowCount();
    int GetActiveSpinCount();
    
private:
    static inline GlobalTrickManager* instance = nullptr;
    GlobalNamespace::AudioTimeSyncController* audioController = nullptr;
    bool saberClashEnabled = true;
    float timeSinceLastNote = 0.0f;
    
    // Slowmo support
    bool slowmoApplied = false;
    float originalTimeScale = 1.0f;
    
    // Performance optimization - cached managers
    std::vector<TrickSaber::SaberTrickManager*> cachedManagers;
    std::chrono::steady_clock::time_point lastCacheUpdate;
    
    void RefreshManagerCache();
    void ValidateManagerCache();
    std::vector<TrickSaber::SaberTrickManager*> GetCachedManagers();
    
    // Slowmo methods
    void StartSlowmo(float targetTimeScale);
    void EndSlowmo();
    void SetTimeScale(float timeScale);
);