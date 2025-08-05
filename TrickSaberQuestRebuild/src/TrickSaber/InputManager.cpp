#include "TrickSaber/InputManager.hpp"
#include "TrickSaber/Config.hpp"
#include "TrickSaber/Configuration.hpp"
#include "TrickSaber/Constants.hpp"
#include "main.hpp"

#include "UnityEngine/Vector2.hpp"
#include "GlobalNamespace/SaberType.hpp"
#include "GlobalNamespace/OVRInput.hpp"
#include <cmath>

DEFINE_TYPE(TrickSaber, InputManager);

using namespace TrickSaber;
using namespace GlobalNamespace;

void InputManager::Awake() {
    // Initialize input states
    triggerState = {};
    gripState = {};
    thumbstickState = {};
    buttonOneState = {};
    buttonTwoState = {};
    
    auto now = std::chrono::steady_clock::now();
    triggerState.lastChangeTime = now;
    gripState.lastChangeTime = now;
    thumbstickState.lastChangeTime = now;
    buttonOneState.lastChangeTime = now;
    buttonTwoState.lastChangeTime = now;
    lastConnectionCheck = now;
    
    wasConnected = false;
    activeCombination = {};
}

void InputManager::Update() {
    if ((!controller) || (!controller->get_enabled())) {
        return;
    }
    
    try {
        // Check controller connection periodically
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastConnectionCheck);
        
        if (elapsed.count() >= Constants::CONNECTION_CHECK_INTERVAL_MS) {
            bool connected = IsControllerDetected();
            if (connected != wasConnected) {
                HandleControllerConnectionChange(connected);
                wasConnected = connected;
            }
            lastConnectionCheck = now;
        }
        
        if (wasConnected) {
            if (TrickSaber::Configuration::IsSimplifiedInputEnabled()) {
                CheckInputsSimplified();
            } else {
                CheckInputs();
            }
        }
    } catch (const std::exception& e) {
        PaperLogger.error("Error in InputManager::Update: {}", e.what());
    } catch (...) {
        PaperLogger.error("Unknown error in InputManager::Update");
    }
}

void InputManager::Initialize(GlobalNamespace::VRController* vrController, GlobalNamespace::SaberType saberType) {
    if (!vrController) {
        PaperLogger.error("VRController is null in InputManager::Initialize");
        return;
    }
    
    controller = vrController;
    this->saberType = static_cast<int>(saberType);
    
    // Set OVR controller based on saber type
    ovrController = (saberType == GlobalNamespace::SaberType::SaberA) ? 
        OVRInput::Controller::LTouch : OVRInput::Controller::RTouch;
    
    PaperLogger.info("InputManager initialized for {} saber", 
        saberType == GlobalNamespace::SaberType::SaberA ? "left" : "right");
}

void InputManager::CheckInputs() {
    // Check each configured input type with debouncing
    CheckInput([this](float& v) { return GetTriggerValue(v); }, triggerState, config.triggerAction);
    CheckInput([this](float& v) { return GetGripValue(v); }, gripState, config.gripAction);
    CheckInput([this](float& v) { return GetThumbstickValue(v); }, thumbstickState, config.thumbstickAction);
    CheckInput([this](float& v) { return GetButtonOneValue(v); }, buttonOneState, config.buttonOneAction);
    CheckInput([this](float& v) { return GetButtonTwoValue(v); }, buttonTwoState, config.buttonTwoAction);
    
    // Check for multi-input combinations
    CheckMultiInputCombinations();
}

void InputManager::CheckInput(std::function<bool(float&)> getValue, InputState& state, TrickAction action) {
    if (action == TrickAction::None || !getValue) return;
    
    try {
        float value = 0.0f;
        bool isPressed = getValue(value);
        
        // Validate value is finite
        if (!std::isfinite(value)) {
            value = 0.0f;
            isPressed = false;
        }
        
        // Check for state change with debouncing
        if (isPressed != state.pressed && IsDebounceTimeElapsed(state)) {
            state.pressed = isPressed;
            state.lastValue = value;
            state.lastChangeTime = std::chrono::steady_clock::now();
            
            if (isPressed) {
                if (onTrickActivated) {
                    onTrickActivated(action, value);
                }
            } else {
                if (onTrickDeactivated) {
                    onTrickDeactivated(action);
                }
            }
        }
        
        // Update value even if no state change for continuous input
        if (state.pressed) {
            state.lastValue = value;
        }
        
    } catch (...) {
        PaperLogger.error("Error checking input for action {}", static_cast<int>(action));
        state.pressed = false; // Reset to safe state
        state.lastChangeTime = std::chrono::steady_clock::now();
    }
}

bool InputManager::GetTriggerValue(float& value) {
    if (!IsControllerDetected()) {
        value = 0.0f;
        return false;
    }
    
    try {
        value = OVRInput::Get(OVRInput::Axis1D::PrimaryIndexTrigger, ovrController);
        
        // Clamp value to valid range
        value = std::clamp(value, 0.0f, 1.0f);
        
        if (config.reverseTrigger) {
            value = 1.0f - value;
        }
        
        return value >= TrickSaber::Configuration::GetTriggerThreshold();
    } catch (...) {
        PaperLogger.error("Error reading trigger value");
        value = 0.0f;
        return false;
    }
}

bool InputManager::GetGripValue(float& value) {
    if (!IsControllerDetected()) {
        value = 0.0f;
        return false;
    }
    
    try {
        value = OVRInput::Get(OVRInput::Axis1D::PrimaryHandTrigger, ovrController);
        
        // Clamp value to valid range
        value = std::clamp(value, 0.0f, 1.0f);
        
        if (config.reverseGrip) {
            value = 1.0f - value;
        }
        
        return value >= TrickSaber::Configuration::GetTriggerThreshold(); // Using trigger threshold for grip
    } catch (...) {
        PaperLogger.error("Error reading grip value");
        value = 0.0f;
        return false;
    }
}

bool InputManager::GetThumbstickValue(float& value) {
    if (!IsControllerDetected()) {
        value = 0.0f;
        return false;
    }
    
    try {
    
    // Check for momentum spin mode with thumbstick press
    if (config.spinMode == SpinMode::Momentum && config.thumbstickAction == TrickAction::Spin) {
        bool thumbstickPressed = OVRInput::Get(OVRInput::Button::PrimaryThumbstick, ovrController);
        value = thumbstickPressed ? 1.0f : 0.0f;
        return thumbstickPressed;
    }
    
    auto stick = OVRInput::Get(OVRInput::Axis2D::PrimaryThumbstick, ovrController);
    
    // Check if thumbstick is outside deadzone
    if (stick.get_magnitude() < config.thumbstickDeadzone) {
        value = 0.0f;
        return false;
    }
    
    switch (config.spinMode) {
        case SpinMode::OmniDirectional: {
            float xValue = std::abs(stick.x);
            float yValue = std::abs(stick.y);
            
            if (xValue > yValue && xValue >= TrickSaber::Configuration::GetThumbstickThreshold()) {
                value = stick.x;
            } else if (yValue >= TrickSaber::Configuration::GetThumbstickThreshold()) {
                value = stick.y;
            } else {
                value = 0.0f;
                return false;
            }
            break;
        }
        case SpinMode::AngleSpeed:
            value = stick.get_magnitude() >= TrickSaber::Configuration::GetThumbstickThreshold() ? 
                   (std::atan2(stick.y, stick.x) * Constants::RADIANS_TO_DEGREES) : 0.0f;
            break;
            
        case SpinMode::Traditional:
        default: {
            bool correctDirection = (saberType == 0) ? 
                (stick.x < -config.thumbstickDeadzone) : (stick.x > config.thumbstickDeadzone);
            
            if (!correctDirection || std::abs(stick.x) < TrickSaber::Configuration::GetThumbstickThreshold()) {
                value = 0.0f;
                return false;
            }
            
            value = stick.x;
            break;
        }
    }
    
    // Apply reverse if configured
    if (config.reverseThumbstick) {
        value = -value;
    }
    
    return std::abs(value) >= TrickSaber::Configuration::GetThumbstickThreshold();
    
    } catch (...) {
        PaperLogger.error("Error reading thumbstick value");
        value = 0.0f;
        return false;
    }
}

bool InputManager::IsDebounceTimeElapsed(const InputState& state) const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - state.lastChangeTime);
    return elapsed.count() >= Constants::DEBOUNCE_TIME_MS;
}

bool InputManager::IsControllerConnected() const {
    return wasConnected;
}

bool InputManager::IsControllerDetected() {
    // Primary detection method
    if (OVRInput::IsControllerConnected(ovrController)) {
        return true;
    }
    
    // Fallback: Check if VRController is enabled and tracking
    if (controller && controller->get_enabled()) {
        // Additional validation - check if we can get any input
        try {
            float testValue = OVRInput::Get(OVRInput::Axis1D::PrimaryIndexTrigger, ovrController);
            return std::isfinite(testValue);
        } catch (...) {
            // If we can't read input, controller likely not connected
        }
    }
    
    return false;
}

void InputManager::HandleControllerConnectionChange(bool connected) {
    if (connected && !wasConnected) {
        PaperLogger.info("Controller {} connected", 
            saberType == 0 ? "left" : "right");
        
        // Reset input states on reconnection
        triggerState = {};
        gripState = {};
        thumbstickState = {};
        buttonOneState = {};
        buttonTwoState = {};
        
        auto now = std::chrono::steady_clock::now();
        triggerState.lastChangeTime = now;
        gripState.lastChangeTime = now;
        thumbstickState.lastChangeTime = now;
        buttonOneState.lastChangeTime = now;
        buttonTwoState.lastChangeTime = now;
        
        activeCombination = {};
        
    } else if (!connected && wasConnected) {
        PaperLogger.warn("Controller {} disconnected", 
            saberType == 0 ? "left" : "right");
        
        // End any active tricks when controller disconnects
        if (triggerState.pressed && onTrickDeactivated) {
            onTrickDeactivated(config.triggerAction);
        }
        if (gripState.pressed && onTrickDeactivated) {
            onTrickDeactivated(config.gripAction);
        }
        if (thumbstickState.pressed && onTrickDeactivated) {
            onTrickDeactivated(config.thumbstickAction);
        }
        if (buttonOneState.pressed && onTrickDeactivated) {
            onTrickDeactivated(config.buttonOneAction);
        }
        if (buttonTwoState.pressed && onTrickDeactivated) {
            onTrickDeactivated(config.buttonTwoAction);
        }
        
        // End any active combination
        if (activeCombination.active && onTrickDeactivated) {
            onTrickDeactivated(activeCombination.primaryAction);
        }
        
        // Reset all states
        triggerState.pressed = false;
        gripState.pressed = false;
        thumbstickState.pressed = false;
        buttonOneState.pressed = false;
        buttonTwoState.pressed = false;
        activeCombination = {};
    }
}

// Simplified input implementation
void InputManager::CheckInputsSimplified() {
    // Direct input checking without debouncing
    float triggerValue = 0.0f;
    bool triggerPressed = GetTriggerValueSimplified(triggerValue);
    
    if (triggerPressed != triggerState.pressed) {
        triggerState.pressed = triggerPressed;
        if (triggerPressed && onTrickActivated) {
            onTrickActivated(config.triggerAction, triggerValue);
        } else if (!triggerPressed && onTrickDeactivated) {
            onTrickDeactivated(config.triggerAction);
        }
    }
    
    float thumbstickValue = 0.0f;
    bool thumbstickPressed = GetThumbstickValueSimplified(thumbstickValue);
    
    if (thumbstickPressed != thumbstickState.pressed) {
        thumbstickState.pressed = thumbstickPressed;
        if (thumbstickPressed && onTrickActivated) {
            onTrickActivated(config.thumbstickAction, thumbstickValue);
        } else if (!thumbstickPressed && onTrickDeactivated) {
            onTrickDeactivated(config.thumbstickAction);
        }
    }
}

bool InputManager::GetTriggerValueSimplified(float& value) {
    if (!IsControllerDetected()) {
        value = 0.0f;
        return false;
    }
    
    value = OVRInput::Get(OVRInput::Axis1D::PrimaryIndexTrigger, ovrController);
    return value >= Constants::SIMPLIFIED_TRIGGER_THRESHOLD;
}

bool InputManager::GetThumbstickValueSimplified(float& value) {
    if (!IsControllerDetected()) {
        value = 0.0f;
        return false;
    }
    
    auto stick = OVRInput::Get(OVRInput::Axis2D::PrimaryThumbstick, ovrController);
    
    // Simple horizontal check
    bool correctDirection = (saberType == 0) ? (stick.x < -Constants::SIMPLIFIED_THUMBSTICK_THRESHOLD) : (stick.x > Constants::SIMPLIFIED_THUMBSTICK_THRESHOLD);
    value = correctDirection ? stick.x : 0.0f;
    return correctDirection;
}

void InputManager::CheckMultiInputCombinations() {
    auto now = std::chrono::steady_clock::now();
    
    // Check if current combination is still active
    if (activeCombination.active) {
        bool primaryStillPressed = GetInputState(activeCombination.primaryAction).pressed;
        bool secondaryStillPressed = GetInputState(activeCombination.secondaryAction).pressed;
        
        if (!primaryStillPressed || !secondaryStillPressed) {
            // End combination
            if (onTrickDeactivated) {
                onTrickDeactivated(activeCombination.primaryAction);
            }
            activeCombination.active = false;
            PaperLogger.debug("Multi-input combination ended: {} + {}", 
                static_cast<int>(activeCombination.primaryAction), 
                static_cast<int>(activeCombination.secondaryAction));
        }
        return;
    }
    
    // Check for new combinations
    struct InputPair {
        TrickAction action1, action2;
        InputState* state1;
        InputState* state2;
        const char* name;
    };
    
    InputPair combinations[] = {
        {config.triggerAction, config.gripAction, &triggerState, &gripState, "trigger+grip"},
        {config.triggerAction, config.thumbstickAction, &triggerState, &thumbstickState, "trigger+thumbstick"},
        {config.gripAction, config.thumbstickAction, &gripState, &thumbstickState, "grip+thumbstick"},
        {config.triggerAction, config.buttonOneAction, &triggerState, &buttonOneState, "trigger+button1"},
        {config.gripAction, config.buttonOneAction, &gripState, &buttonOneState, "grip+button1"}
    };
    
    for (const auto& combo : combinations) {
        if (combo.action1 == TrickAction::None || combo.action2 == TrickAction::None) continue;
        if (combo.action1 == combo.action2) continue; // Same action on different inputs
        
        if (combo.state1->pressed && combo.state2->pressed) {
            // Check if both inputs were pressed within combination window
            auto timeDiff = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(
                combo.state1->lastChangeTime - combo.state2->lastChangeTime).count());
            
            if (timeDiff <= Constants::COMBINATION_WINDOW_MS) {
                // Start combination - prioritize primary action
                activeCombination.active = true;
                activeCombination.primaryAction = combo.action1;
                activeCombination.secondaryAction = combo.action2;
                activeCombination.startTime = now;
                
                if (onTrickActivated) {
                    float combinedValue = (combo.state1->lastValue + combo.state2->lastValue) / 2.0f;
                    onTrickActivated(combo.action1, combinedValue);
                }
                
                PaperLogger.debug("Multi-input combination started: {} ({})", combo.name, timeDiff);
                break;
            }
        }
    }
}

InputManager::InputState& InputManager::GetInputState(TrickAction action) {
    switch (action) {
        case TrickAction::Throw:
            if (config.triggerAction == TrickAction::Throw) return triggerState;
            if (config.gripAction == TrickAction::Throw) return gripState;
            if (config.buttonOneAction == TrickAction::Throw) return buttonOneState;
            break;
        case TrickAction::Spin:
            if (config.thumbstickAction == TrickAction::Spin) return thumbstickState;
            if (config.triggerAction == TrickAction::Spin) return triggerState;
            if (config.gripAction == TrickAction::Spin) return gripState;
            break;
        default:
            break;
    }
    return triggerState; // Fallback
}

bool InputManager::GetButtonOneValue(float& value) {
    if (!IsControllerDetected()) {
        value = 0.0f;
        return false;
    }
    
    try {
        // Button One is typically the A/X button
        bool pressed = OVRInput::Get(OVRInput::Button::One, ovrController);
        value = pressed ? 1.0f : 0.0f;
        
        if (config.reverseButtonOne) {
            value = 1.0f - value;
        }
        
        return pressed;
    } catch (...) {
        PaperLogger.error("Error reading button one value");
        value = 0.0f;
        return false;
    }
}

bool InputManager::GetButtonTwoValue(float& value) {
    if (!IsControllerDetected()) {
        value = 0.0f;
        return false;
    }
    
    try {
        // Button Two is typically the B/Y button
        bool pressed = OVRInput::Get(OVRInput::Button::Two, ovrController);
        value = pressed ? 1.0f : 0.0f;
        
        if (config.reverseButtonTwo) {
            value = 1.0f - value;
        }
        
        return pressed;
    } catch (...) {
        PaperLogger.error("Error reading button two value");
        value = 0.0f;
        return false;
    }
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif