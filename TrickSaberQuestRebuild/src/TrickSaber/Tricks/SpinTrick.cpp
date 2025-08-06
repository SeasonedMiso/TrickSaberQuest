#include "TrickSaber/Tricks/SpinTrick.hpp"
#include "TrickSaber/SaberTrickModel.hpp"
#include "TrickSaber/SaberTrickManager.hpp"
#include "TrickSaber/MovementController.hpp"
#include "TrickSaber/Config.hpp"
#include "TrickSaber/Configuration.hpp"
#include "TrickSaber/Utils/HapticFeedbackHelper.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Mathf.hpp"
#include "main.hpp"

DEFINE_TYPE(TrickSaber::Tricks, SpinTrick);

using namespace TrickSaber::Tricks;

bool SpinTrick::StartTrick(float value) {
    if (!Trick::StartTrick(value)) return false;
    
    if (!saberTrickModel || !saberTrickModel->saber) {
        Logger.error("SpinTrick: Missing saber or trick model");
        return false;
    }
    
    // Store original transform
    auto saberTransform = saberTrickModel->saber->get_transform();
    originalLocalPosition = saberTransform->get_localPosition();
    originalLocalRotation = saberTransform->get_localRotation();
    
    // Initialize spin state
    state = SpinState::Spinning;
    inputValue = value;
    currentSpinSpeed = 0.0f;
    
    targetSpinSpeed = CalculateTargetSpeed(value);
    
    // Trigger spin start haptic
    if (saberTrickModel && saberTrickModel->saber) {
        TrickSaber::Utils::HapticFeedbackHelper::TriggerHaptic(saberTrickModel->saber->get_saberType(), 
            TrickSaber::Utils::HapticFeedbackHelper::HapticType::SpinStart);
    }
    
    Logger.debug("SpinTrick started with value: {:.2f}, target speed: {:.1f}", value, targetSpinSpeed);
    return true;
}

void SpinTrick::Update() {
    if (!active || !saberTrickModel || !saberTrickModel->saber) return;
    
    float deltaTime = UnityEngine::Time::get_deltaTime();
    
    switch (state) {
        case SpinState::Spinning:
            UpdateSpinPhysics();
            
            // Track largest spin speed for complete rotation mode
            if (UnityEngine::Mathf::Abs(currentSpinSpeed) > UnityEngine::Mathf::Abs(largestSpinSpeed)) {
                largestSpinSpeed = currentSpinSpeed;
            }
            
            ApplySpinRotation(deltaTime);
            break;
            
        case SpinState::Stopping:
            // Decelerate to stop
            if (UnityEngine::Mathf::Abs(currentSpinSpeed) > 0.1f) {
                currentSpinSpeed = UnityEngine::Mathf::MoveTowards(currentSpinSpeed, 0.0f, 
                    300.0f * deltaTime);
                ApplySpinRotation(deltaTime);
            } else {
                // Lerp back to original rotation
                LerpToOriginalRotation(deltaTime);
            }
            break;
            
        case SpinState::Completing:
            // Complete rotation mode - spin until aligned with original rotation
            if (saberTrickModel && saberTrickModel->saber) {
                auto saberTransform = saberTrickModel->saber->get_transform();
                float angle = UnityEngine::Quaternion::Angle(saberTransform->get_localRotation(), originalLocalRotation);
                
                // Continue spinning until we're close to original rotation
                if (angle > 5.0f) {
                    // Maintain consistent spin speed for completion
                    currentSpinSpeed = largestSpinSpeed;
                    ApplySpinRotation(deltaTime);
                } else {
                    // Close enough - snap to original rotation and end
                    saberTransform->set_localRotation(originalLocalRotation);
                    if (manager) manager->OnTrickEnded(TrickAction::Spin);
                    Trick::EndTrick();
                }
            }
            break;
            
        default:
            break;
    }
}

void SpinTrick::UpdateSpinPhysics() {
    float deltaTime = UnityEngine::Time::get_deltaTime();
    
    // Simple target speed calculation
    targetSpinSpeed = CalculateTargetSpeed(inputValue);
    
    // Linear interpolation to target speed
    currentSpinSpeed = UnityEngine::Mathf::MoveTowards(currentSpinSpeed, targetSpinSpeed, 
        300.0f * deltaTime); // 300 deg/s acceleration
}

void SpinTrick::ApplySpinRotation(float deltaTime) {
    if (!saberTrickModel || !saberTrickModel->saber || currentSpinSpeed < 0.1f) return;
    
    auto saberTransform = saberTrickModel->saber->get_transform();
    
    // Simple rotation around local axis
    float rotationAngle = currentSpinSpeed * deltaTime;
    auto deltaRotation = UnityEngine::Quaternion::AngleAxis(rotationAngle, UnityEngine::Vector3::get_forward());
    
    auto currentRotation = saberTransform->get_localRotation();
    auto newRotation = UnityEngine::Quaternion::op_Multiply(currentRotation, deltaRotation);
    saberTransform->set_localRotation(newRotation);
}



float SpinTrick::CalculateTargetSpeed(float input) {
    using namespace Configuration;
    
    if (IsSpeedVelocityDependent()) {
        // Use controller angular velocity like PC version
        bool isLeft = (saberTrickModel && saberTrickModel->saber && 
                      saberTrickModel->saber->get_saberType() == GlobalNamespace::SaberType::SaberA);
        
        auto angularVelocity = isLeft ? 
            TrickSaber::MovementController::GetAverageAngularVelocity(true) :
            TrickSaber::MovementController::GetAverageAngularVelocity(false);
        
        // Calculate speed based on dominant axis
        float speed = 0.0f;
        if (GetSpinMode() == SpinMode::OmniDirectional) {
            // Use both X and Y components for omni-directional
            speed = UnityEngine::Mathf::Sqrt(angularVelocity.x * angularVelocity.x + 
                                           angularVelocity.y * angularVelocity.y);
            
            // Determine direction from input
            if (input < 0) speed *= -1;
        } else {
            // Use primary axis based on spin direction
            speed = (GetSpinDirection() == SpinDir::Forward) ? angularVelocity.x : -angularVelocity.x;
        }
        
        // Apply velocity scaling with input magnitude
        float velocityScale = GetSpinSpeed() * UnityEngine::Mathf::Abs(input);
        float finalSpeed = speed * velocityScale * 57.2958f; // Convert to degrees
        
        // Minimum speed threshold to prevent jitter
        if (UnityEngine::Mathf::Abs(finalSpeed) < 5.0f && UnityEngine::Mathf::Abs(input) > 0.1f) {
            finalSpeed = (finalSpeed < 0 ? -5.0f : 5.0f) * velocityScale;
        }
        
        return finalSpeed;
    } else {
        // Fixed speed mode with input scaling
        float baseSpeed = 60.0f * GetSpinSpeed(); // Increased base speed for better feel
        if (GetSpinDirection() == SpinDir::Backward) baseSpeed *= -1;
        
        // Cubic scaling for more responsive control
        float inputScale = UnityEngine::Mathf::Pow(UnityEngine::Mathf::Abs(input), 2.0f);
        if (input < 0) inputScale *= -1;
        
        return baseSpeed * inputScale;
    }
}



void SpinTrick::EndTrick() {
    if (!active) return;
    
    using namespace Configuration;
    
    if (state == SpinState::Spinning) {
        if (IsCompleteRotationMode()) {
            // Complete rotation mode - finish the current rotation
            state = SpinState::Completing;
            
            // Ensure minimum speed for completion
            float minSpeed = 30.0f;
            if (UnityEngine::Mathf::Abs(largestSpinSpeed) < minSpeed) {
                largestSpinSpeed = largestSpinSpeed < 0 ? -minSpeed : minSpeed;
            }
            
            currentSpinSpeed = largestSpinSpeed;
            Logger.debug("SpinTrick: Starting complete rotation mode with speed {:.1f}", largestSpinSpeed);
            return;
        } else if (UnityEngine::Mathf::Abs(currentSpinSpeed) > 60.0f) {
            // Gradual stop for smooth transition
            state = SpinState::Stopping;
            targetSpinSpeed = 0.0f;
            Logger.debug("SpinTrick: Starting gradual stop");
            return;
        }
    }
    
    // Restore original position and rotation
    if (saberTrickModel && saberTrickModel->saber) {
        auto saberTransform = saberTrickModel->saber->get_transform();
        saberTransform->set_localPosition(originalLocalPosition);
        saberTransform->set_localRotation(originalLocalRotation);
    }
    
    currentSpinSpeed = 0.0f;
    
    // Trigger spin end haptic
    if (saberTrickModel && saberTrickModel->saber) {
        TrickSaber::Utils::HapticFeedbackHelper::TriggerHaptic(saberTrickModel->saber->get_saberType(), 
            TrickSaber::Utils::HapticFeedbackHelper::HapticType::SpinEnd);
    }
    
    if (manager) {
        manager->OnTrickEnded(TrickAction::Spin);
    }
    
    Trick::EndTrick();
    Logger.debug("SpinTrick ended");
}

void SpinTrick::EndTrickImmediately() {
    // Force immediate stop
    state = SpinState::Stopping;
    currentSpinSpeed = 0.0f;
    targetSpinSpeed = 0.0f;
    
    if (saberTrickModel && saberTrickModel->saber) {
        auto saberTransform = saberTrickModel->saber->get_transform();
        saberTransform->set_localPosition(originalLocalPosition);
        saberTransform->set_localRotation(originalLocalRotation);
    }
    
    Trick::EndTrick();
    Logger.debug("SpinTrick ended immediately");
}

void SpinTrick::LerpToOriginalRotation(float deltaTime) {
    if (!saberTrickModel || !saberTrickModel->saber) return;
    
    auto saberTransform = saberTrickModel->saber->get_transform();
    auto currentRot = saberTransform->get_localRotation();
    
    float angle = UnityEngine::Quaternion::Angle(currentRot, originalLocalRotation);
    if (angle > 5.0f) {
        auto newRot = UnityEngine::Quaternion::Lerp(currentRot, originalLocalRotation, deltaTime * 20.0f);
        saberTransform->set_localRotation(newRot);
    } else {
        // Close enough - snap to original and end
        saberTransform->set_localRotation(originalLocalRotation);
        if (manager) manager->OnTrickEnded(TrickAction::Spin);
        Trick::EndTrick();
    }
}