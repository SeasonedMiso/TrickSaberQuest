#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "GlobalNamespace/SaberManager.hpp"
#include "GlobalNamespace/Saber.hpp"
#include "TrickSaber/SaberState.hpp"
#include "TrickSaber/BurnMarkHandler.hpp"
#include "TrickSaber/PhysicsHandler.hpp"

DECLARE_CLASS_CODEGEN(TrickSaber, EnhancedSaberManager, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, FixedUpdate);
    DECLARE_INSTANCE_METHOD(void, Initialize, GlobalNamespace::SaberManager* saberManager);
    
public:
    static EnhancedSaberManager* GetInstance() { return instance; }
    
    void ProcessSaberInput(int saberIndex);
    void UpdateSaberPhysics(int saberIndex, float deltaTime);
    void ResetSaberStates();
    
    SaberPhysicsState& GetSaberState(int saberIndex) {
        return saberIndex == 0 ? leftSaberState : rightSaberState;
    }
    
private:
    static EnhancedSaberManager* instance;
    
    GlobalNamespace::SaberManager* saberManager = nullptr;
    BurnMarkHandler* burnMarkHandler = nullptr;
    PhysicsHandler* physicsHandler = nullptr;
    
    SaberPhysicsState leftSaberState;
    SaberPhysicsState rightSaberState;
    
    bool initialized = false;
    
    void InitializeSaberState(SaberPhysicsState& state, GlobalNamespace::Saber* saber);
    void HandleThrowInput(SaberPhysicsState& state, int saberIndex, bool inputPressed);
    void HandleSpinInput(SaberPhysicsState& state, int saberIndex, bool inputPressed);
    int GetOVRButtonForConfig(int configuredButtonIndex, bool isLeftController);
);