#include "TrickSaber/AdvancedInputManager.hpp"
#include "TrickSaber/Configuration.hpp"
#include "TrickSaber/SaberTrickManager.hpp"
#include "GlobalNamespace/OVRInput.hpp"
#include "UnityEngine/Mathf.hpp"
#include "UnityEngine/Object.hpp"
#include "main.hpp"

using namespace TrickSaber;

DEFINE_TYPE(TrickSaber, AdvancedInputManager);

void AdvancedInputManager::Awake() {
    enabled = true;
    
    // Initialize input state
    leftTriggerPressed = false;
    rightTriggerPressed = false;
    leftGripPressed = false;
    rightGripPressed = false;
    leftThumbstickActive = false;
    rightThumbstickActive = false;
}

void AdvancedInputManager::Update() {
    if (!enabled) return;
    
    CheckTriggerInput();
    CheckGripInput();
    CheckThumbstickInput();
}

void AdvancedInputManager::CheckTriggerInput() {
    using namespace TrickSaber::Configuration;
    
    // Left trigger
    bool leftPressed = GlobalNamespace::OVRInput::Get(
        GlobalNamespace::OVRInput::Button::PrimaryIndexTrigger, 
        GlobalNamespace::OVRInput::Controller::LTouch);
    
    float leftValue = GlobalNamespace::OVRInput::Get(
        GlobalNamespace::OVRInput::Axis1D::PrimaryIndexTrigger,
        GlobalNamespace::OVRInput::Controller::LTouch);
    
    if (leftPressed && !leftTriggerPressed && leftValue > GetTriggerThreshold()) {
        OnInputActivated(GetTriggerAction(), leftValue, true);
    } else if (!leftPressed && leftTriggerPressed) {
        OnInputDeactivated(GetTriggerAction(), true);
    }
    leftTriggerPressed = leftPressed;
    
    // Right trigger
    bool rightPressed = GlobalNamespace::OVRInput::Get(
        GlobalNamespace::OVRInput::Button::PrimaryIndexTrigger,
        GlobalNamespace::OVRInput::Controller::RTouch);
    
    float rightValue = GlobalNamespace::OVRInput::Get(
        GlobalNamespace::OVRInput::Axis1D::PrimaryIndexTrigger,
        GlobalNamespace::OVRInput::Controller::RTouch);
    
    if (rightPressed && !rightTriggerPressed && rightValue > GetTriggerThreshold()) {
        OnInputActivated(GetTriggerAction(), rightValue, false);
    } else if (!rightPressed && rightTriggerPressed) {
        OnInputDeactivated(GetTriggerAction(), false);
    }
    rightTriggerPressed = rightPressed;
}

void AdvancedInputManager::CheckGripInput() {
    using namespace TrickSaber::Configuration;
    
    if (GetGripAction() == TrickAction::None) return;
    
    // Left grip
    bool leftPressed = GlobalNamespace::OVRInput::Get(
        GlobalNamespace::OVRInput::Button::PrimaryHandTrigger,
        GlobalNamespace::OVRInput::Controller::LTouch);
    
    float leftValue = GlobalNamespace::OVRInput::Get(
        GlobalNamespace::OVRInput::Axis1D::PrimaryHandTrigger,
        GlobalNamespace::OVRInput::Controller::LTouch);
    
    if (leftPressed && !leftGripPressed && leftValue > GetGripThreshold()) {
        OnInputActivated(GetGripAction(), leftValue, true);
    } else if (!leftPressed && leftGripPressed) {
        OnInputDeactivated(GetGripAction(), true);
    }
    leftGripPressed = leftPressed;
    
    // Right grip
    bool rightPressed = GlobalNamespace::OVRInput::Get(
        GlobalNamespace::OVRInput::Button::PrimaryHandTrigger,
        GlobalNamespace::OVRInput::Controller::RTouch);
    
    float rightValue = GlobalNamespace::OVRInput::Get(
        GlobalNamespace::OVRInput::Axis1D::PrimaryHandTrigger,
        GlobalNamespace::OVRInput::Controller::RTouch);
    
    if (rightPressed && !rightGripPressed && rightValue > GetGripThreshold()) {
        OnInputActivated(GetGripAction(), rightValue, false);
    } else if (!rightPressed && rightGripPressed) {
        OnInputDeactivated(GetGripAction(), false);
    }
    rightGripPressed = rightPressed;
}

void AdvancedInputManager::CheckThumbstickInput() {
    using namespace TrickSaber::Configuration;
    
    if (GetThumbstickAction() == TrickAction::None) return;
    
    // Left thumbstick
    auto leftStick = GlobalNamespace::OVRInput::Get(
        GlobalNamespace::OVRInput::Axis2D::PrimaryThumbstick,
        GlobalNamespace::OVRInput::Controller::LTouch);
    
    float leftValue = (GetThumbstickDirection() == ThumbstickDir::Horizontal) ? 
        UnityEngine::Mathf::Abs(leftStick.x) : UnityEngine::Mathf::Abs(leftStick.y);
    bool leftActive = leftValue > GetThumbstickThreshold();
    
    if (leftActive && !leftThumbstickActive) {
        OnInputActivated(GetThumbstickAction(), leftValue, true);
    } else if (!leftActive && leftThumbstickActive) {
        OnInputDeactivated(GetThumbstickAction(), true);
    } else if (leftActive && leftThumbstickActive) {
        OnInputUpdated(GetThumbstickAction(), leftValue, true);
    }
    leftThumbstickActive = leftActive;
    
    // Right thumbstick
    auto rightStick = GlobalNamespace::OVRInput::Get(
        GlobalNamespace::OVRInput::Axis2D::PrimaryThumbstick,
        GlobalNamespace::OVRInput::Controller::RTouch);
    
    float rightValue = (GetThumbstickDirection() == ThumbstickDir::Horizontal) ?
        UnityEngine::Mathf::Abs(rightStick.x) : UnityEngine::Mathf::Abs(rightStick.y);
    bool rightActive = rightValue > GetThumbstickThreshold();
    
    if (rightActive && !rightThumbstickActive) {
        OnInputActivated(GetThumbstickAction(), rightValue, false);
    } else if (!rightActive && rightThumbstickActive) {
        OnInputDeactivated(GetThumbstickAction(), false);
    } else if (rightActive && rightThumbstickActive) {
        OnInputUpdated(GetThumbstickAction(), rightValue, false);
    }
    rightThumbstickActive = rightActive;
}

void AdvancedInputManager::OnInputActivated(TrickAction action, float value, bool isLeft) {
    if (action == TrickAction::None) return;
    
    // Find managers using simple search
    auto foundManagers = UnityEngine::Object::FindObjectsOfType<SaberTrickManager*>();
    if (!foundManagers) return;
    
    for (int i = 0; i < foundManagers->get_Length(); i++) {
        auto manager = foundManagers->_values[i];
        if (!manager || !manager->saber) continue;
        
        bool managerIsLeft = (manager->saber->get_saberType() == GlobalNamespace::SaberType::SaberA);
        if (managerIsLeft == isLeft) {
            manager->OnTrickActivated(action, value);
            break;
        }
    }
}

void AdvancedInputManager::OnInputDeactivated(TrickAction action, bool isLeft) {
    if (action == TrickAction::None) return;
    
    auto foundManagers = UnityEngine::Object::FindObjectsOfType<SaberTrickManager*>();
    if (!foundManagers) return;
    
    for (int i = 0; i < foundManagers->get_Length(); i++) {
        auto manager = foundManagers->_values[i];
        if (!manager || !manager->saber) continue;
        
        bool managerIsLeft = (manager->saber->get_saberType() == GlobalNamespace::SaberType::SaberA);
        if (managerIsLeft == isLeft) {
            manager->OnTrickDeactivated(action);
            break;
        }
    }
}

void AdvancedInputManager::OnInputUpdated(TrickAction action, float value, bool isLeft) {
    if (action == TrickAction::None) return;
    
    auto foundManagers = UnityEngine::Object::FindObjectsOfType<SaberTrickManager*>();
    if (!foundManagers) return;
    
    for (int i = 0; i < foundManagers->get_Length(); i++) {
        auto manager = foundManagers->_values[i];
        if (!manager || !manager->saber) continue;
        
        bool managerIsLeft = (manager->saber->get_saberType() == GlobalNamespace::SaberType::SaberA);
        if (managerIsLeft == isLeft && manager->IsDoingTrick() && action == TrickAction::Spin) {
            manager->OnTrickActivated(action, value);
            break;
        }
    }
}

void AdvancedInputManager::Initialize() {
    Logger.info("AdvancedInputManager initialized");
}