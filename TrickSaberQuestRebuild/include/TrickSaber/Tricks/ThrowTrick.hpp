#pragma once

#include "TrickSaber/Tricks/Trick.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Rigidbody.hpp"

DECLARE_CLASS_CODEGEN(TrickSaber::Tricks, ThrowTrick, Trick,
    DECLARE_INSTANCE_METHOD(bool, StartTrick, float value);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, EndTrick);
    DECLARE_INSTANCE_METHOD(void, EndTrickImmediately);
    DECLARE_INSTANCE_METHOD(void, FixedUpdate);
    
private:
    enum class ThrowState {
        Preparing,
        Thrown,
        Returning,
        Snapping
    };
    
    ThrowState state = ThrowState::Preparing;
    
    // Transform state
    UnityEngine::Vector3 originalLocalPosition;
    UnityEngine::Quaternion originalLocalRotation;
    UnityEngine::Transform* originalParent = nullptr;
    
    // Simple physics state
    UnityEngine::Vector3 throwVelocity;
    UnityEngine::Vector3 currentVelocity;
    float saberRotSpeed = 0.0f;
    
    // Return state
    UnityEngine::Vector3 throwReleasePosition;
    UnityEngine::Quaternion throwReleaseRotation;
    float returnTime = 0.0f;
    float returnDuration = 0.5f;
    
    // Simple collision detection
    float snapBackDistance = 8.0f;
    
    void ApplyThrowForces();
    void UpdatePhysics();
    void UpdateReturn();
    void CalculateThrowForces();
    void ThrowEnd();
    bool ShouldSnapBack();
    
    // Simplified physics
    void UpdatePhysicsSimplified();
    void UpdateReturnSimplified();
);