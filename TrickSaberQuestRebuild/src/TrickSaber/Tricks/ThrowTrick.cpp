#include "TrickSaber/Tricks/ThrowTrick.hpp"
#include "TrickSaber/SaberTrickModel.hpp"
#include "TrickSaber/SaberTrickManager.hpp"
#include "TrickSaber/MovementController.hpp"
#include "TrickSaber/Config.hpp"
#include "TrickSaber/Configuration.hpp"
#include "TrickSaber/Core/TrickSaberManager.hpp"
#include "TrickSaber/Utils/UnityMath.hpp"
#include "TrickSaber/Utils/HapticFeedbackHelper.hpp"
#include "TrickSaber/Utils/PooledTrickCalculation.hpp"
#include "TrickSaber/Constants.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Mathf.hpp"
#include "UnityEngine/Physics.hpp"
#include "main.hpp"

DEFINE_TYPE(TrickSaber::Tricks, ThrowTrick);

using namespace TrickSaber::Tricks;

bool ThrowTrick::StartTrick(float value) {
    if (!Trick::StartTrick(value)) return false;
    
    if (!saberTrickModel || !saberTrickModel->saber) {
        Logger.error("ThrowTrick: Missing required components");
        return false;
    }
    
    auto saberTransform = saberTrickModel->saber->get_transform();
    
    // Store original transform and parent
    originalLocalPosition = saberTransform->get_localPosition();
    originalLocalRotation = saberTransform->get_localRotation();
    originalParent = saberTransform->get_parent();
    
    // Switch to trick model with rigidbody (PC parity)
    saberTrickModel->ChangeToTrickModel();
    
    auto rigidbody = saberTrickModel->GetRigidbody();
    if (rigidbody) {
        rigidbody->set_isKinematic(false);
    }
    
    // Calculate throw forces based on controller movement
    CalculateThrowForces();
    
    // Apply physics forces to rigidbody
    ApplyThrowForces();
    
    // Apply slowmo if enabled
    if (TrickSaber::Configuration::IsSlowmoDuringThrow()) {
        auto coreManager = TrickSaber::Core::TrickSaberManager::GetInstance();
        if (coreManager) {
            coreManager->ApplySlowmo(TrickSaber::Configuration::GetSlowmoAmount());
        }
    }
    
    state = ThrowState::Thrown;
    returnDuration = config.returnDuration > 0 ? config.returnDuration : Constants::DEFAULT_RETURN_DURATION;
    
    // Trigger throw start haptic
    TrickSaber::Utils::HapticFeedbackHelper::TriggerHaptic(saberTrickModel->saber->get_saberType(), 
        TrickSaber::Utils::HapticFeedbackHelper::HapticType::TrickStart);
    
    Logger.debug("ThrowTrick started with velocity: ({:.2f}, {:.2f}, {:.2f})", 
        throwVelocity.x, throwVelocity.y, throwVelocity.z);
    return true;
}

void ThrowTrick::ApplyThrowForces() {
    // Store throw parameters for physics simulation
    // No rigidbody needed - we'll handle physics manually like Triick
    auto saberTransform = saberTrickModel->saber->get_transform();
    if (saberTransform) {
        // Detach from parent for free movement
        saberTransform->SetParent(nullptr, true);
        
        Logger.debug("Saber detached for throw: velocity=({:.2f},{:.2f},{:.2f})", 
            throwVelocity.x, throwVelocity.y, throwVelocity.z);
    }
}

void ThrowTrick::CalculateThrowForces() {
    // Use pooled calculation for complex throw physics
    auto calculation = TrickSaber::Utils::PooledTrickCalculation();
    if (!calculation.IsValid()) {
        Logger.error("Failed to get pooled calculation for throw forces");
        return;
    }
    
    // Get averaged velocity from movement controller (PC parity)
    UnityEngine::Vector3 velocity;
    UnityEngine::Vector3 angularVelocity;
    
    if (saberTrickModel && saberTrickModel->saber) {
        bool isLeft = (saberTrickModel->saber->get_saberType() == GlobalNamespace::SaberType::SaberA);
        velocity = TrickSaber::MovementController::GetAverageVelocity(isLeft);
        angularVelocity = TrickSaber::MovementController::GetAverageAngularVelocity(isLeft);
    }
    
    // Store in pooled calculation
    calculation->velocity = velocity;
    calculation->angularVelocity = angularVelocity;
    calculation->isActive = true;
    
    if (TrickSaber::Configuration::IsSpeedVelocityDependent()) {
        // Velocity-dependent mode: use actual controller movement
        float velocityMultiplier = TrickSaber::Configuration::GetThrowVelocity();
        throwVelocity = UnityEngine::Vector3::op_Multiply(velocity, velocityMultiplier * 2.0f);
        
        // Minimum throw velocity based on controller forward direction
        if (throwVelocity.get_magnitude() < TrickSaber::Configuration::GetVelocityThreshold()) {
            auto forward = saberTrickModel->saber->get_transform()->get_forward();
            throwVelocity = UnityEngine::Vector3::op_Multiply(forward, 
                TrickSaber::Configuration::GetVelocityThreshold() * velocityMultiplier);
        }
        
        // Spin speed based on actual angular velocity
        saberRotSpeed = angularVelocity.get_magnitude() * Constants::RADIANS_TO_DEGREES;
        if (angularVelocity.x < 0) saberRotSpeed *= -1;
    } else {
        // Fixed velocity mode: use configured values
        float velocityMultiplier = TrickSaber::config.throwVelocityMultiplier;
        float baseVelocity = TrickSaber::Configuration::GetThrowVelocity();
        
        // Use controller direction but fixed magnitude
        auto direction = velocity.get_normalized();
        if (direction.get_magnitude() < 0.1f) {
            direction = saberTrickModel->saber->get_transform()->get_forward();
        }
        
        throwVelocity = UnityEngine::Vector3::op_Multiply(direction, baseVelocity * velocityMultiplier);
        
        // Fixed spin speed
        saberRotSpeed = baseVelocity * Constants::SPIN_VELOCITY_SCALE;
        if (angularVelocity.x < 0) saberRotSpeed *= -1;
    }
    
    currentVelocity = throwVelocity;
    
    Logger.debug("Throw calculated (velocity-dependent={}): velocity=({:.2f},{:.2f},{:.2f}), rotSpeed={:.1f}", 
        TrickSaber::Configuration::IsSpeedVelocityDependent(),
        throwVelocity.x, throwVelocity.y, throwVelocity.z, saberRotSpeed);
}

void ThrowTrick::Update() {
    if (!active || !saberTrickModel || !saberTrickModel->saber) return;
    
    switch (state) {
        case ThrowState::Preparing:
            // Initialization state, nothing to do
            break;
            
        case ThrowState::Thrown:
            if (TrickSaber::Configuration::IsSimplifiedInputEnabled()) {
                UpdatePhysicsSimplified();
            } else {
                UpdatePhysics();
            }
            if (ShouldSnapBack()) {
                state = ThrowState::Returning;
                throwReleasePosition = saberTrickModel->saber->get_transform()->get_position();
                throwReleaseRotation = saberTrickModel->saber->get_transform()->get_rotation();
                returnTime = 0.0f;
            }
            break;
            
        case ThrowState::Returning:
            if (TrickSaber::Configuration::IsSimplifiedInputEnabled()) {
                UpdateReturnSimplified();
            } else {
                UpdateReturn();
            }
            break;
            
        case ThrowState::Snapping:
            // Immediate snap back
            EndTrickImmediately();
            break;
            
        default:
            break;
    }
}

void ThrowTrick::FixedUpdate() {
    // Physics handled in Update() for simplicity
}

void ThrowTrick::UpdatePhysics() {
    // Manual physics simulation like Triick
    auto saberTransform = saberTrickModel->saber->get_transform();
    if (!saberTransform) return;
    
    float deltaTime = UnityEngine::Time::get_deltaTime();
    
    // Update position based on velocity
    auto currentPos = saberTransform->get_position();
    auto newPos = UnityEngine::Vector3::op_Addition(currentPos, 
        UnityEngine::Vector3::op_Multiply(currentVelocity, deltaTime));
    saberTransform->set_position(newPos);
    
    // Update rotation based on angular velocity
    if (saberRotSpeed != 0.0f) {
        float angle = saberRotSpeed * deltaTime;
        auto axis = saberTransform->get_right(); // Spin around saber's right axis
        auto deltaRotation = UnityEngine::Quaternion::AngleAxis(angle, axis);
        saberTransform->set_rotation(UnityEngine::Quaternion::op_Multiply(
            deltaRotation, saberTransform->get_rotation()));
    }
}

void ThrowTrick::UpdateReturn() {
    if (!originalParent || !saberTrickModel) return;
    
    auto saberTransform = saberTrickModel->saber->get_transform();
    if (!saberTransform) return;
    
    float deltaTime = UnityEngine::Time::get_deltaTime();
    returnTime += deltaTime;
    
    // Simple lerp return like Triick - more reliable
    float t = UnityEngine::Mathf::Clamp01(returnTime / returnDuration);
    
    // Target position in world space
    auto targetPos = originalParent->TransformPoint(originalLocalPosition);
    auto targetRot = UnityEngine::Quaternion::op_Multiply(originalParent->get_rotation(), originalLocalRotation);
    
    // Smooth interpolation
    auto currentPos = UnityEngine::Vector3::Lerp(throwReleasePosition, targetPos, t);
    auto currentRot = UnityEngine::Quaternion::Slerp(throwReleaseRotation, targetRot, t);
    
    // Apply return spin effects if configured
    float returnSpinMultiplier = TrickSaber::Configuration::GetReturnSpinMultiplier();
    if (returnSpinMultiplier > 0.0f) {
        // Add spin during return based on original throw velocity
        float spinSpeed = throwVelocity.get_magnitude() * returnSpinMultiplier * Constants::RETURN_SPIN_SCALE;
        float spinAngle = spinSpeed * deltaTime;
        
        auto spinAxis = UnityEngine::Vector3::get_right(); // Spin around saber's right axis
        auto spinRotation = UnityEngine::Quaternion::AngleAxis(spinAngle, spinAxis);
        currentRot = UnityEngine::Quaternion::op_Multiply(currentRot, spinRotation);
    }
    
    saberTransform->set_position(currentPos);
    saberTransform->set_rotation(currentRot);
    
    // Complete return when close enough
    if (t >= 1.0f) {
        ThrowEnd();
    }
}

bool ThrowTrick::ShouldSnapBack() {
    if (!originalParent || !saberTrickModel) return true;
    
    auto trickTransform = saberTrickModel->GetTrickModelTransform();
    if (!trickTransform) return true;
    
    auto saberPos = trickTransform->get_position();
    auto handPos = originalParent->get_position();
    
    float distance = UnityEngine::Vector3::Distance(saberPos, handPos);
    return distance > (snapBackDistance > 0 ? snapBackDistance : Constants::DEFAULT_SNAP_BACK_DISTANCE);
}



void ThrowTrick::EndTrick() {
    if (!active) return;
    
    if (state == ThrowState::Thrown) {
        // Start return sequence
        state = ThrowState::Returning;
        returnTime = 0.0f;
        
        if (saberTrickModel && saberTrickModel->saber) {
            auto saberTransform = saberTrickModel->saber->get_transform();
            throwReleasePosition = saberTransform->get_position();
            throwReleaseRotation = saberTransform->get_rotation();
        }
        
        // Trigger trick end haptic
        TrickSaber::Utils::HapticFeedbackHelper::TriggerHaptic(saberTrickModel->saber->get_saberType(), 
            TrickSaber::Utils::HapticFeedbackHelper::HapticType::TrickEnd);
        
        // Physics disabled during return
        
        Logger.debug("ThrowTrick: Starting return sequence");
        return; // Don't end the trick yet, let Update() handle the return
    }
    
    // Reset velocity
    currentVelocity = UnityEngine::Vector3::get_zero();
    
    Logger.debug("ThrowTrick ended");
}

void ThrowTrick::EndTrickImmediately() {
    ThrowEnd();
    Logger.debug("ThrowTrick ended immediately");
}

void ThrowTrick::ThrowEnd() {
    // Remove slowmo if it was applied
    if (TrickSaber::Configuration::IsSlowmoDuringThrow()) {
        auto coreManager = TrickSaber::Core::TrickSaberManager::GetInstance();
        if (coreManager) {
            coreManager->RemoveSlowmo();
        }
    }
    
    auto rigidbody = saberTrickModel->GetRigidbody();
    if (rigidbody) {
        rigidbody->set_isKinematic(true);
        rigidbody->set_velocity(UnityEngine::Vector3::get_zero());
    }
    
    // Switch back to actual saber
    saberTrickModel->ChangeToActualSaber();
    
    // Trigger saber return haptic
    if (saberTrickModel && saberTrickModel->saber) {
        TrickSaber::Utils::HapticFeedbackHelper::TriggerHaptic(saberTrickModel->saber->get_saberType(), 
            TrickSaber::Utils::HapticFeedbackHelper::HapticType::SaberReturn);
    }
    
    if (manager) {
        manager->OnTrickEnded(TrickAction::Throw);
    }
    
    Trick::EndTrick();
}

// Simplified physics implementation
void ThrowTrick::UpdatePhysicsSimplified() {
    // Direct transform manipulation - no rigidbody
    auto saberTransform = saberTrickModel->saber->get_transform();
    if (!saberTransform) return;
    
    float deltaTime = UnityEngine::Time::get_deltaTime();
    
    // Simple forward movement with gravity
    currentVelocity.y -= Constants::GRAVITY_ACCELERATION * deltaTime;
    
    auto newPos = UnityEngine::Vector3::op_Addition(saberTransform->get_position(), 
        UnityEngine::Vector3::op_Multiply(currentVelocity, deltaTime));
    saberTransform->set_position(newPos);
    
    // Simple spin
    if (saberRotSpeed != 0.0f) {
        float angle = saberRotSpeed * deltaTime;
        auto axis = UnityEngine::Vector3::get_right();
        auto deltaRotation = UnityEngine::Quaternion::AngleAxis(angle, axis);
        saberTransform->set_rotation(UnityEngine::Quaternion::op_Multiply(
            deltaRotation, saberTransform->get_rotation()));
    }
}

void ThrowTrick::UpdateReturnSimplified() {
    // Direct lerp - immediate and simple
    if (!originalParent || !saberTrickModel) return;
    
    auto saberTransform = saberTrickModel->saber->get_transform();
    if (!saberTransform) return;
    
    float deltaTime = UnityEngine::Time::get_deltaTime();
    returnTime += deltaTime;
    
    // Faster return than PC version
    float t = UnityEngine::Mathf::Clamp01(returnTime / Constants::SIMPLIFIED_RETURN_DURATION);
    
    auto targetPos = originalParent->TransformPoint(originalLocalPosition);
    auto targetRot = UnityEngine::Quaternion::op_Multiply(originalParent->get_rotation(), originalLocalRotation);
    
    // Direct lerp - no easing
    auto currentPos = UnityEngine::Vector3::Lerp(throwReleasePosition, targetPos, t);
    auto currentRot = UnityEngine::Quaternion::Slerp(throwReleaseRotation, targetRot, t);
    
    // Apply return spin effects if configured (simplified version)
    float returnSpinMultiplier = TrickSaber::Configuration::GetReturnSpinMultiplier();
    if (returnSpinMultiplier > 0.0f) {
        // Simpler spin calculation for simplified mode
        float spinSpeed = throwVelocity.get_magnitude() * returnSpinMultiplier * Constants::SIMPLIFIED_RETURN_SPIN_SCALE;
        float spinAngle = spinSpeed * deltaTime;
        
        auto spinRotation = UnityEngine::Quaternion::AngleAxis(spinAngle, UnityEngine::Vector3::get_right());
        currentRot = UnityEngine::Quaternion::op_Multiply(currentRot, spinRotation);
    }
    
    saberTransform->set_position(currentPos);
    saberTransform->set_rotation(currentRot);
    
    if (t >= 1.0f) {
        ThrowEnd();
    }
}