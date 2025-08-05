#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Transform.hpp"

DECLARE_CLASS_CODEGEN(TrickSaber, AdvancedTrickFeatures, UnityEngine::MonoBehaviour,
        DECLARE_INSTANCE_METHOD(void, Awake);
        DECLARE_INSTANCE_METHOD(void, Update);
        
        // Simplified slowmo features
        DECLARE_INSTANCE_METHOD(void, StartSlowmo, float amount);
        DECLARE_INSTANCE_METHOD(void, EndSlowmo);
        DECLARE_INSTANCE_METHOD(bool, IsSlowmoActive);
        
        // Simplified physics - no reference parameters
        DECLARE_INSTANCE_METHOD(void, ApplyAdvancedPhysics, UnityEngine::Transform* transform);
        DECLARE_INSTANCE_METHOD(UnityEngine::Vector3, CalculateTorque, UnityEngine::Vector3 velocity, UnityEngine::Vector3 angularVel);
        
        // Basic features
        DECLARE_INSTANCE_METHOD(float, GetVelocityDependentSpinSpeed, float baseSpeed);
        DECLARE_INSTANCE_METHOD(bool, ShouldUseVelocityDependentSpin);
        DECLARE_INSTANCE_METHOD(bool, IsCompleteRotationMode);
        DECLARE_INSTANCE_METHOD(float, GetConfigurableReturnSpeed);
        
        // Note interaction features
        DECLARE_INSTANCE_METHOD(bool, ShouldDisableTricksForNotes);
        DECLARE_INSTANCE_METHOD(bool, CanHitNotesDuringTricks);
        
        DECLARE_STATIC_METHOD(AdvancedTrickFeatures*, GetInstance);
        
    private:
        static AdvancedTrickFeatures* instance;
);