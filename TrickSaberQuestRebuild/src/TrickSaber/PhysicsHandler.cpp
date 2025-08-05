#include "TrickSaber/PhysicsHandler.hpp"
#include "TrickSaber/Config.hpp"
#include "TrickSaber/Utils/MemoryManager.hpp"
#include "main.hpp"

#include "UnityEngine/Time.hpp"
#include "UnityEngine/Mathf.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"

DEFINE_TYPE(TrickSaber, PhysicsHandler);

using namespace TrickSaber;

void PhysicsHandler::Awake() {
    // Physics handler initialization
}

void PhysicsHandler::FixedUpdate() {
    // Called by Unity's physics system
}

void PhysicsHandler::UpdateControllerVelocity(SaberPhysicsState& state, float deltaTime) {
    if (!state.handTransform || deltaTime <= 0.00001f) return;
    
    auto currentPos = Utils::MemoryManager::GetCachedPosition(state.handTransform.ptr());
    state.controllerVelocity = UnityEngine::Vector3::op_Division(
        UnityEngine::Vector3::op_Subtraction(currentPos, state.prevControllerPos), deltaTime);
    state.prevControllerPos = currentPos;
}

void PhysicsHandler::CalculateThrowPhysics(SaberPhysicsState& state, float throwMultiplier) {
    if (!state.saberTransform) return;
    
    state.velocity = UnityEngine::Vector3::op_Multiply(state.controllerVelocity, throwMultiplier);
    
    if (state.spinActive) {
        // Use current spin for angular velocity
        float speed = config.spinSpeed * DEG2RAD_CONSTANT;
        auto localSpinAxis = Utils::MemoryManager::GetVector3();
        localSpinAxis = UnityEngine::Vector3::get_right();
        state.angularVelocity = UnityEngine::Vector3::op_Multiply(
            state.saberTransform->TransformDirection(localSpinAxis), speed);
        Utils::MemoryManager::ReturnVector3(localSpinAxis);
    } else {
        // Calculate natural rotation from throw
        float throwSpeed = state.velocity.get_magnitude();
        
        if (throwSpeed < 1.0f) {
            // Gentle throw - minimal rotation
            UnityEngine::Vector3 worldRollAxis = state.saberTransform->TransformDirection(
                UnityEngine::Vector3::get_forward());
            state.angularVelocity = UnityEngine::Vector3::op_Multiply(
                worldRollAxis, throwSpeed * MIN_NATURAL_ROTATION_RAD_PER_SEC);
        } else {
            // Calculate cross-product rotation
            UnityEngine::Vector3 throwDirection = state.velocity.get_normalized();
            UnityEngine::Vector3 saberForward = state.saberTransform->TransformDirection(
                UnityEngine::Vector3::get_forward());
            
            UnityEngine::Vector3 naturalSpinAxis = UnityEngine::Vector3::Cross(
                saberForward, throwDirection);
            
            if (naturalSpinAxis.get_sqrMagnitude() < 0.1f) {
                naturalSpinAxis = saberForward;
            } else {
                naturalSpinAxis = naturalSpinAxis.get_normalized();
            }
            
            float spinSpeed = throwSpeed * THROW_VELOCITY_TO_ROTATION_SCALE;
            spinSpeed = UnityEngine::Mathf::Min(spinSpeed, MAX_NATURAL_ROTATION_FROM_VELOCITY_RAD_PER_SEC);
            float totalSpin = MIN_NATURAL_ROTATION_RAD_PER_SEC + spinSpeed;
            
            state.angularVelocity = UnityEngine::Vector3::op_Multiply(naturalSpinAxis, totalSpin);
        }
    }
}

void PhysicsHandler::ApplySaberSpin(SaberPhysicsState& state, float speed, bool clockwise, float zOffset, float deltaTime) {
    if (!state.saberTransform) return;
    
    float direction = clockwise ? 1.0f : -1.0f;
    UnityEngine::Vector3 localSpinAxis = UnityEngine::Vector3::get_right();
    UnityEngine::Vector3 localPivotOffset = UnityEngine::Vector3::op_Multiply(
        UnityEngine::Vector3::get_forward(), zOffset);
    
    UnityEngine::Vector3 worldPivotPoint = state.saberTransform->TransformPoint(localPivotOffset);
    UnityEngine::Vector3 worldSpinAxis = state.saberTransform->TransformDirection(localSpinAxis);
    
    float rotationAngle = direction * speed * deltaTime;
    state.saberTransform->RotateAround(worldPivotPoint, worldSpinAxis, rotationAngle);
}

void PhysicsHandler::UpdateSaberPhysics(SaberPhysicsState& state, float deltaTime) {
    if (!state.saberTransform) return;
    
    switch (state.state) {
        case SaberInteractionState::Held: {
            auto parent = state.saberTransform->get_parent();
            auto originalParent = state.originalParent.ptr();
            if (parent.ptr() != originalParent) {
                state.saberTransform->SetParent(originalParent, false);
                state.saberTransform->set_localPosition(state.originalLocalPosition);
                state.saberTransform->set_localRotation(state.originalLocalRotation);
            }
            break;
        }
            
        case SaberInteractionState::Thrown: {
            // Update position
            auto currentPos = Utils::MemoryManager::GetCachedPosition(state.saberTransform.ptr());
            auto newPos = UnityEngine::Vector3::op_Addition(
                currentPos, UnityEngine::Vector3::op_Multiply(state.velocity, deltaTime));
            state.saberTransform->set_position(newPos);
            
            // Update rotation
            if (state.angularVelocity.get_sqrMagnitude() > 0.0001f) {
                float angle = state.angularVelocity.get_magnitude() * deltaTime * RAD2DEG_CONSTANT;
                UnityEngine::Vector3 axis = state.angularVelocity.get_normalized();
                UnityEngine::Quaternion deltaRotation = UnityEngine::Quaternion::AngleAxis(angle, axis);
                state.saberTransform->set_rotation(UnityEngine::Quaternion::op_Multiply(
                    deltaRotation, state.saberTransform->get_rotation()));
            }
            break;
        }
            
        case SaberInteractionState::Returning:
            // Handled by UpdateReturnMotion
            break;
            
        default:
            break;
    }
}

void PhysicsHandler::UpdateReturnMotion(SaberPhysicsState& state, float returnDuration, float deltaTime) {
    if (!state.saberTransform || !state.handTransform) return;
    
    state.returnTime += deltaTime;
    if (returnDuration < 0.01f) returnDuration = 0.01f;
    
    float t = UnityEngine::Mathf::Clamp01(state.returnTime / returnDuration);
    
    UnityEngine::Vector3 targetPos = state.handTransform->TransformPoint(state.originalLocalPosition);
    UnityEngine::Quaternion targetRot = UnityEngine::Quaternion::op_Multiply(
        state.handTransform->get_rotation(), state.originalLocalRotation);
    
    state.saberTransform->set_position(UnityEngine::Vector3::Lerp(
        state.throwReleasePosition, targetPos, t));
    
    if (state.angularVelocity.get_sqrMagnitude() > 0.0001f && t < 0.95f) {
        // Continue spinning while returning
        float angle = state.angularVelocity.get_magnitude() * deltaTime * RAD2DEG_CONSTANT;
        UnityEngine::Vector3 axis = state.angularVelocity.get_normalized();
        UnityEngine::Quaternion deltaRotation = UnityEngine::Quaternion::AngleAxis(angle, axis);
        state.saberTransform->set_rotation(UnityEngine::Quaternion::op_Multiply(
            deltaRotation, state.saberTransform->get_rotation()));
        state.saberTransform->set_rotation(UnityEngine::Quaternion::Slerp(
            state.saberTransform->get_rotation(), targetRot, t * t * t));
    } else {
        state.saberTransform->set_rotation(UnityEngine::Quaternion::Slerp(
            state.throwReleaseRotation, targetRot, t));
    }
    
    if (t >= 1.0f) {
        state.state = SaberInteractionState::Held;
        state.returnTime = 0.0f;
    }
}