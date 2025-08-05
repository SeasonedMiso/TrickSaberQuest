#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "GlobalNamespace/Saber.hpp"
#include "GlobalNamespace/SaberManager.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "UnityEngine/Coroutine.hpp"

#include <memory>
#include <unordered_map>
#include <coroutine>

DECLARE_CLASS_CODEGEN(TrickSaber::Core, TrickSaberManager, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);
    
    DECLARE_STATIC_METHOD(TrickSaberManager*, GetInstance);
    DECLARE_STATIC_METHOD(void, Initialize, GlobalNamespace::SaberManager* saberManager, GlobalNamespace::AudioTimeSyncController* audioController);
    DECLARE_STATIC_METHOD(void, Cleanup);
    
public:
    void InitializeSabers(GlobalNamespace::Saber* leftSaber, GlobalNamespace::Saber* rightSaber);
    void ApplySlowmo(float amount);
    void RemoveSlowmo();
    void SetTimeScale(float scale);
    void DisableBurnMarks(int saberType);
    void EnableBurnMarks(int saberType);
    
private:
    static inline TrickSaberManager* instance = nullptr;
    GlobalNamespace::AudioTimeSyncController* audioController = nullptr;
    float originalTimeScale = 1.0f;
    bool slowmoActive = false;
    
    std::unordered_map<Il2CppClass*, FieldInfo*> burnMarkFields;
    
    UnityEngine::Coroutine* ApplySlowmoSmooth(float targetScale);
    UnityEngine::Coroutine* RemoveSlowmoSmooth();
);