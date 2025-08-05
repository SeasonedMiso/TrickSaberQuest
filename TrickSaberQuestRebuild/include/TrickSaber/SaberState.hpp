#pragma once

#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Transform.hpp"
#include "TrickSaber/SafePtrUnity.hpp"

namespace TrickSaber {
    enum class SaberInteractionState {
        Held,
        Thrown,
        Returning
    };
    
    struct SaberPhysicsState {
        UnityEngine::Vector3 velocity = UnityEngine::Vector3::get_zero();
        UnityEngine::Vector3 angularVelocity = UnityEngine::Vector3::get_zero();
        UnityEngine::Vector3 controllerVelocity = UnityEngine::Vector3::get_zero();
        UnityEngine::Vector3 prevControllerPos = UnityEngine::Vector3::get_zero();
        
        // Return state
        float returnTime = 0.0f;
        UnityEngine::Vector3 throwReleasePosition = UnityEngine::Vector3::get_zero();
        UnityEngine::Quaternion throwReleaseRotation = UnityEngine::Quaternion::get_identity();
        
        // Original pose
        UnityEngine::Vector3 originalLocalPosition = UnityEngine::Vector3::get_zero();
        UnityEngine::Quaternion originalLocalRotation = UnityEngine::Quaternion::get_identity();
        
        // References
        SafePtrUnity<UnityEngine::Transform> saberTransform;
        SafePtrUnity<UnityEngine::Transform> originalParent;
        SafePtrUnity<UnityEngine::Transform> handTransform;
        
        // State
        SaberInteractionState state = SaberInteractionState::Held;
        bool spinActive = false;
        bool throwButtonPressed = false;
        
        void Reset() {
            state = SaberInteractionState::Held;
            spinActive = false;
            throwButtonPressed = false;
            returnTime = 0.0f;
            velocity = UnityEngine::Vector3::get_zero();
            angularVelocity = UnityEngine::Vector3::get_zero();
        }
    };
}