#pragma once

#include "TrickSaber/Tricks/Trick.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"

DECLARE_CLASS_CODEGEN(TrickSaber::Tricks, SpinTrick, Trick,
    DECLARE_INSTANCE_METHOD(bool, StartTrick, float value);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, EndTrick);
    DECLARE_INSTANCE_METHOD(void, EndTrickImmediately);
    
public:
    // Input tracking for continuous updates
    float inputValue = 0.0f;
    
private:
    enum class SpinState {
        Spinning,
        Stopping,
        Completing
    };
    
    SpinState state = SpinState::Spinning;
    
    // Transform state
    UnityEngine::Vector3 originalLocalPosition;
    UnityEngine::Quaternion originalLocalRotation;
    
    // Spin physics
    float currentSpinSpeed = 0.0f;
    float targetSpinSpeed = 0.0f;
    float largestSpinSpeed = 0.0f;
    
    void UpdateSpinPhysics();
    void ApplySpinRotation(float deltaTime);
    void LerpToOriginalRotation(float deltaTime);
    float CalculateTargetSpeed(float input);
);