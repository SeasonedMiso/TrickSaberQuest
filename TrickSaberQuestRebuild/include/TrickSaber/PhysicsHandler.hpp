#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "TrickSaber/SaberState.hpp"

DECLARE_CLASS_CODEGEN(TrickSaber, PhysicsHandler, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, FixedUpdate);
    
public:
    static void UpdateSaberPhysics(SaberPhysicsState& state, float deltaTime);
    static void CalculateThrowPhysics(SaberPhysicsState& state, float throwMultiplier);
    static void UpdateControllerVelocity(SaberPhysicsState& state, float deltaTime);
    static void ApplySaberSpin(SaberPhysicsState& state, float speed, bool clockwise, float zOffset, float deltaTime);
    static void UpdateReturnMotion(SaberPhysicsState& state, float returnDuration, float deltaTime);
    
private:
    static constexpr float MIN_NATURAL_ROTATION_RAD_PER_SEC = 3.14f;
    static constexpr float THROW_VELOCITY_TO_ROTATION_SCALE = 2.5f;
    static constexpr float MAX_NATURAL_ROTATION_FROM_VELOCITY_RAD_PER_SEC = 70.0f;
    static constexpr float MIN_THROW_SPEED_FOR_CROSS_PRODUCT_ROTATION = 0.2f;
    static constexpr float DEG2RAD_CONSTANT = 0.017453292f;
    static constexpr float RAD2DEG_CONSTANT = 57.29577951f;
);