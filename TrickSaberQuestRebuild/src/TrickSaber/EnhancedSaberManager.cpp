#include "TrickSaber/EnhancedSaberManager.hpp"
#include "TrickSaber/Config.hpp"
#include "main.hpp"

#include "GlobalNamespace/OVRInput.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/GameObject.hpp"

DEFINE_TYPE(TrickSaber, EnhancedSaberManager);

using namespace TrickSaber;
using namespace GlobalNamespace;

EnhancedSaberManager* EnhancedSaberManager::instance = nullptr;

void EnhancedSaberManager::Awake() {
    instance = this;
    
    // BurnMarkHandler is now a static utility class
    physicsHandler = get_gameObject()->AddComponent<PhysicsHandler*>();
}

void EnhancedSaberManager::Initialize(GlobalNamespace::SaberManager* manager) {
    if (!manager) return;
    
    saberManager = manager;
    
    InitializeSaberState(leftSaberState, manager->get_leftSaber());
    InitializeSaberState(rightSaberState, manager->get_rightSaber());
    
    initialized = true;
    PaperLogger.info("EnhancedSaberManager initialized");
}

void EnhancedSaberManager::InitializeSaberState(SaberPhysicsState& state, GlobalNamespace::Saber* saber) {
    if (!saber) return;
    
    auto transform = saber->get_transform();
    state.saberTransform = transform;
    state.originalParent = transform->get_parent();
    state.handTransform = state.originalParent.ptr();
    state.originalLocalPosition = transform->get_localPosition();
    state.originalLocalRotation = transform->get_localRotation();
    state.prevControllerPos = state.handTransform->get_position();
    state.Reset();
}

void EnhancedSaberManager::FixedUpdate() {
    if (!initialized || !config.trickSaberEnabled) return;
    
    float deltaTime = UnityEngine::Time::get_fixedDeltaTime();
    if (deltaTime <= 0.00001f) deltaTime = 1.0f / 90.0f;
    
    // Update controller velocities
    PhysicsHandler::UpdateControllerVelocity(leftSaberState, deltaTime);
    PhysicsHandler::UpdateControllerVelocity(rightSaberState, deltaTime);
    
    // Process input and physics
    if (config.leftSaberEnabled) {
        ProcessSaberInput(0);
        UpdateSaberPhysics(0, deltaTime);
    }
    
    if (config.rightSaberEnabled) {
        ProcessSaberInput(1);
        UpdateSaberPhysics(1, deltaTime);
    }
}

void EnhancedSaberManager::ProcessSaberInput(int saberIndex) {
    auto& state = GetSaberState(saberIndex);
    if (!state.saberTransform) return;
    
    bool isLeft = saberIndex == 0;
    auto controller = isLeft ? OVRInput::Controller::LTouch : OVRInput::Controller::RTouch;
    
    // Handle throw input
    bool throwPressed = false;
    if (config.triggerAction == TrickAction::Throw) {
        float triggerValue = OVRInput::Get(OVRInput::Axis1D::PrimaryIndexTrigger, controller);
        throwPressed = triggerValue >= config.triggerThreshold;
    }
    
    HandleThrowInput(state, saberIndex, throwPressed);
    
    // Handle spin input
    bool spinPressed = false;
    if (config.thumbstickAction == TrickAction::Spin) {
        auto thumbstick = OVRInput::Get(OVRInput::Axis2D::PrimaryThumbstick, controller);
        float magnitude = config.thumbstickDirection == ThumbstickDir::Horizontal ? 
            abs(thumbstick.x) : abs(thumbstick.y);
        spinPressed = magnitude >= config.thumbstickThreshold;
    }
    
    HandleSpinInput(state, saberIndex, spinPressed);
}

void EnhancedSaberManager::HandleThrowInput(SaberPhysicsState& state, int saberIndex, bool inputPressed) {
    if (inputPressed && state.state == SaberInteractionState::Held && !state.throwButtonPressed) {
        // Initiate throw
        state.state = SaberInteractionState::Thrown;
        state.saberTransform->SetParent(nullptr, true);
        
        PhysicsHandler::CalculateThrowPhysics(state, config.throwVelocityMultiplier);
        
        if (config.enableTrickCutting) {
            BurnMarkHandler::DisableBurnMarks(saberIndex);
        }
        
        state.spinActive = false;
        PaperLogger.debug("Saber {} throw initiated", saberIndex);
        
    } else if (!inputPressed && state.state == SaberInteractionState::Thrown && state.throwButtonPressed) {
        // Initiate return
        state.state = SaberInteractionState::Returning;
        state.returnTime = 0.0f;
        state.throwReleasePosition = state.saberTransform->get_position();
        state.throwReleaseRotation = state.saberTransform->get_rotation();
        
        PaperLogger.debug("Saber {} return initiated", saberIndex);
    }
    
    state.throwButtonPressed = inputPressed;
}

void EnhancedSaberManager::HandleSpinInput(SaberPhysicsState& state, int saberIndex, bool inputPressed) {
    if (state.state != SaberInteractionState::Held) return;
    
    if (inputPressed && !state.spinActive) {
        state.spinActive = true;
        PaperLogger.debug("Saber {} spin activated", saberIndex);
    } else if (!inputPressed && state.spinActive) {
        state.spinActive = false;
        // Restore original position
        if (state.saberTransform) {
            state.saberTransform->set_localPosition(state.originalLocalPosition);
            state.saberTransform->set_localRotation(state.originalLocalRotation);
        }
        PaperLogger.debug("Saber {} spin deactivated", saberIndex);
    }
}

void EnhancedSaberManager::UpdateSaberPhysics(int saberIndex, float deltaTime) {
    auto& state = GetSaberState(saberIndex);
    
    switch (state.state) {
        case SaberInteractionState::Held:
            if (state.spinActive) {
                PhysicsHandler::ApplySaberSpin(state, config.spinSpeed, 
                    config.spinDirection == SpinDir::Forward, config.spinAnchorZOffset, deltaTime);
            } else {
                PhysicsHandler::UpdateSaberPhysics(state, deltaTime);
            }
            break;
            
        case SaberInteractionState::Thrown:
            if (config.moveWhileThrown) {
                PhysicsHandler::UpdateSaberPhysics(state, deltaTime);
            }
            break;
            
        case SaberInteractionState::Returning:
            PhysicsHandler::UpdateReturnMotion(state, config.returnDuration, deltaTime);
            if (state.state == SaberInteractionState::Held) {
                // Return completed
                if (config.enableTrickCutting) {
                    BurnMarkHandler::EnableBurnMarks(saberIndex);
                }
                if (config.vibrateOnReturn) {
                    // Basic haptic feedback on return
                    auto controller = saberIndex == 0 ? OVRInput::Controller::LTouch : OVRInput::Controller::RTouch;
                    OVRInput::SetControllerVibration(0.3f, 0.1f, controller);
                }
            }
            break;
            
        default:
            break;
    }
}

void EnhancedSaberManager::ResetSaberStates() {
    leftSaberState.Reset();
    rightSaberState.Reset();
    
    // Restore saber positions
    if (leftSaberState.saberTransform && leftSaberState.originalParent) {
        leftSaberState.saberTransform->SetParent(leftSaberState.originalParent.ptr(), false);
        leftSaberState.saberTransform->set_localPosition(leftSaberState.originalLocalPosition);
        leftSaberState.saberTransform->set_localRotation(leftSaberState.originalLocalRotation);
    }
    
    if (rightSaberState.saberTransform && rightSaberState.originalParent) {
        rightSaberState.saberTransform->SetParent(rightSaberState.originalParent.ptr(), false);
        rightSaberState.saberTransform->set_localPosition(rightSaberState.originalLocalPosition);
        rightSaberState.saberTransform->set_localRotation(rightSaberState.originalLocalRotation);
    }
    
    if (config.enableTrickCutting) {
        BurnMarkHandler::EnableBurnMarks(0, true);
        BurnMarkHandler::EnableBurnMarks(1, true);
    }
}