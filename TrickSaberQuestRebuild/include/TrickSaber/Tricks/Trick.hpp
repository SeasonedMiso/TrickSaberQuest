#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "TrickSaber/Enums.hpp"

// Forward declarations
namespace TrickSaber { 
    class SaberTrickManager; 
    class MovementController; 
    class SaberTrickModel; 
}

DECLARE_CLASS_CODEGEN(TrickSaber::Tricks, Trick, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_METHOD(bool, StartTrick, float value);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, EndTrick);
    DECLARE_INSTANCE_METHOD(void, EndTrickImmediately);
    DECLARE_INSTANCE_METHOD(bool, IsActive);
    DECLARE_INSTANCE_METHOD(bool, IsTrickInState, int state);
    
public:
    // Initialize method with proper types
    void Initialize(SaberTrickManager* manager, SaberTrickModel* saberTrickModel);
    
    SaberTrickManager* manager = nullptr;
    SaberTrickModel* saberTrickModel = nullptr;
    bool active = false;
);