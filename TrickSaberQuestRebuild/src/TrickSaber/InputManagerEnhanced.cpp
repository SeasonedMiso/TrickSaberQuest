#include "TrickSaber/InputManager_Enhanced.hpp"
#include "TrickSaber/Config.hpp"
#include "TrickSaber/Configuration.hpp"
#include "main.hpp"

#include "UnityEngine/Vector2.hpp"
#include "GlobalNamespace/SaberType.hpp"
#include "GlobalNamespace/OVRInput.hpp"
#include <cmath>
#include <algorithm>

DEFINE_TYPE(TrickSaber, InputManagerEnhanced);

using namespace TrickSaber;
using namespace GlobalNamespace;

// Enhanced input state methods
float InputManagerEnhanced::EnhancedInputState::GetSmoothedValue() const {
    return (valueHistory[0] + valueHistory[1] + valueHistory[2]) / 3.0f;
}

void InputManagerEnhanced::EnhancedInputState::UpdateHistory(float value) {
    valueHistory[historyIndex] = value;
    historyIndex = (historyIndex + 1) % 3;
}

bool InputManagerEnhanced::EnhancedInputState::ValidateStateChange(bool newState) {
    if (newState == pendingState) {
        consecutiveFrames++;
        return consecutiveFrames >= VALIDATION_FRAMES;
    } else {
        pendingState = newState;
        consecutiveFrames = 1;
        return false;
    }
}

void InputManagerEnhanced::Awake() {
    // Initialize enhanced input states
    triggerState = {};
    gripState = {};
    thumbstickState = {};
    
    auto now = std::chrono::steady_clock::now();
    triggerState.lastChangeTime = now;
    gripState.lastChangeTime = now;
    thumbstickState.lastChangeTime = now;
    lastConnectionCheck = now;
    
    wasConnected = false;
}

void InputManagerEnhanced::Update() {
    if ((!controller) || (!controller->get_enabled())) {
        return;
    }
    
    try {
        // Check controller connection periodically
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastConnectionCheck);
        
        if (elapsed.count() >= CONNECTION_CHECK_INTERVAL_MS) {
            bool connected = IsControllerDetected();
            if (connected != wasConnected) {
                HandleControllerConnectionChange(connected);
                wasConnected = connected;
            }
            lastConnectionCheck = now;
        }
        
        if (wasConnected) {
            CheckInputs();
        }
    } catch (const std::bad_alloc& e) {
        PaperLogger.error("Memory allocation failed in InputManagerEnhanced::Update: {}", e.what());
        // Reset input states to prevent memory issues
        triggerState = {};
        gripState = {};
        thumbstickState = {};
    } catch (const std::runtime_error& e) {
        PaperLogger.error("Runtime error in InputManagerEnhanced::Update: {}", e.what());
        // Log controller state for debugging
        PaperLogger.error("Controller connected: {}, enabled: {}", 
            wasConnected, controller ? controller->get_enabled() : false);
    } catch (const std::exception& e) {
        PaperLogger.error("Exception in InputManagerEnhanced::Update: {}", e.what());
    } catch (...) {
        PaperLogger.error("Unknown error in InputManagerEnhanced::Update - resetting input states");
        // Reset states as recovery measure
        triggerState.pressed = false;
        gripState.pressed = false;
        thumbstickState.pressed = false;
    }
}

void InputManagerEnhanced::Initialize(GlobalNamespace::VRController* vrController, GlobalNamespace::SaberType saberType) {
    if (!vrController) {
        PaperLogger.error("VRController is null in InputManagerEnhanced::Initialize");
        return;
    }
    
    controller = vrController;
    this->saberType = static_cast<int>(saberType);
    
    // Set OVR controller based on saber type
    ovrController = (saberType == GlobalNamespace::SaberType::SaberA) ? 
        OVRInput::Controller::LTouch : OVRInput::Controller::RTouch;
    
    PaperLogger.info("InputManagerEnhanced initialized for {} saber", 
        saberType == GlobalNamespace::SaberType::SaberA ? "left" : "right");
}

void InputManagerEnhanced::CheckInputs() {
    // Check each configured input type with enhanced processing
    CheckEnhancedInput([this](float& v) { return GetTriggerValue(v); }, triggerState, config.triggerAction);
    CheckEnhancedInput([this](float& v) { return GetGripValue(v); }, gripState, config.gripAction);
    CheckEnhancedInput([this](float& v) { return GetThumbstickValue(v); }, thumbstickState, config.thumbstickAction);
}

void InputManagerEnhanced::CheckEnhancedInput(std::function<bool(float&)> getValue, EnhancedInputState& state, TrickAction action) {
    if (action == TrickAction::None || !getValue) return;
    
    try {
        float rawValue = 0.0f;
        bool rawPressed = getValue(rawValue);
        
        // Validate value is finite
        if (!std::isfinite(rawValue)) {
            rawValue = 0.0f;
            rawPressed = false;
        }
        
        // Update smoothing history
        state.UpdateHistory(rawValue);
        float smoothedValue = state.GetSmoothedValue();
        
        // Apply hysteresis for state determination
        bool hysteresisPressed;
        if (state.pressed) {
            hysteresisPressed = smoothedValue >= state.releaseThreshold;
        } else {
            hysteresisPressed = smoothedValue >= state.pressThreshold;
        }
        
        // Validate state change (require 2 consecutive frames)
        if (state.ValidateStateChange(hysteresisPressed) && 
            hysteresisPressed != state.pressed && 
            IsDebounceTimeElapsed(state)) {
            
            state.pressed = hysteresisPressed;
            state.lastValue = smoothedValue;
            state.lastChangeTime = std::chrono::steady_clock::now();
            
#ifdef DEBUG
            LogInputDiagnostics(state, action, smoothedValue);
#endif
            
            if (hysteresisPressed) {
                if (onTrickActivated) {
                    onTrickActivated(action, smoothedValue);
                }
            } else {
                if (onTrickDeactivated) {
                    onTrickDeactivated(action);
                }
            }
        }
        
        // Update value for continuous input
        if (state.pressed) {
            state.lastValue = smoothedValue;
        }
        
    } catch (const std::runtime_error& e) {
        PaperLogger.error("Runtime error checking input for action {}: {}", static_cast<int>(action), e.what());
        state.pressed = false;
        state.lastChangeTime = std::chrono::steady_clock::now();
    } catch (const std::exception& e) {
        PaperLogger.error("Exception checking input for action {}: {}", static_cast<int>(action), e.what());
        state.pressed = false;
        state.lastChangeTime = std::chrono::steady_clock::now();
    } catch (...) {
        PaperLogger.error("Unknown error checking enhanced input for action {} - resetting state", static_cast<int>(action));
        state.pressed = false;
        state.lastChangeTime = std::chrono::steady_clock::now();
    }
}

bool InputManagerEnhanced::GetTriggerValue(float& value) {
    if (!IsControllerDetected()) {
        value = 0.0f;
        return false;
    }
    
    try {
        value = OVRInput::Get(OVRInput::Axis1D::PrimaryIndexTrigger, ovrController);
        value = std::clamp(value, 0.0f, 1.0f);
        
        if (config.reverseTrigger) {
            value = 1.0f - value;
        }
        
        return value >= Configuration::GetTriggerThreshold();
    } catch (const std::exception& e) {
        PaperLogger.debug("Exception getting trigger value: {}", e.what());
        value = 0.0f;
        return false;
    } catch (...) {
        value = 0.0f;
        return false;
    }
}

bool InputManagerEnhanced::GetGripValue(float& value) {
    if (!IsControllerDetected()) {
        value = 0.0f;
        return false;
    }
    
    try {
        value = OVRInput::Get(OVRInput::Axis1D::PrimaryHandTrigger, ovrController);
        value = std::clamp(value, 0.0f, 1.0f);
        
        if (config.reverseGrip) {
            value = 1.0f - value;
        }
        
        return value >= Configuration::GetTriggerThreshold();
    } catch (const std::exception& e) {
        PaperLogger.debug("Exception getting grip value: {}", e.what());
        value = 0.0f;
        return false;
    } catch (...) {
        value = 0.0f;
        return false;
    }
}

bool InputManagerEnhanced::GetThumbstickValue(float& value) {
    if (!IsControllerDetected()) {
        value = 0.0f;
        return false;
    }
    
    try {
        auto stick = OVRInput::Get(OVRInput::Axis2D::PrimaryThumbstick, ovrController);
        
        if (stick.get_magnitude() < config.thumbstickDeadzone) {
            value = 0.0f;
            return false;
        }
        
        // Use horizontal component for traditional spin
        value = stick.x;
        
        if (config.reverseThumbstick) {
            value = -value;
        }
        
        return std::abs(value) >= Configuration::GetThumbstickThreshold();
        
    } catch (const std::exception& e) {
        PaperLogger.debug("Exception getting thumbstick value: {}", e.what());
        value = 0.0f;
        return false;
    } catch (...) {
        value = 0.0f;
        return false;
    }
}

UnityEngine::Vector2 InputManagerEnhanced::GetThumbstickVector2() const {
    if (!const_cast<InputManagerEnhanced*>(this)->IsControllerDetected()) {
        return UnityEngine::Vector2::get_zero();
    }
    
    try {
        auto stick = OVRInput::Get(OVRInput::Axis2D::PrimaryThumbstick, ovrController);
        
        if (stick.get_magnitude() < config.thumbstickDeadzone) {
            return UnityEngine::Vector2::get_zero();
        }
        
        return stick;
    } catch (const std::exception& e) {
        PaperLogger.debug("Exception getting thumbstick vector: {}", e.what());
        return UnityEngine::Vector2::get_zero();
    } catch (...) {
        return UnityEngine::Vector2::get_zero();
    }
}

bool InputManagerEnhanced::IsDebounceTimeElapsed(const EnhancedInputState& state) const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - state.lastChangeTime);
    return elapsed.count() >= DEBOUNCE_TIME_MS;
}

bool InputManagerEnhanced::IsControllerConnected() const {
    return wasConnected;
}

bool InputManagerEnhanced::IsControllerDetected() const {
    if (OVRInput::IsControllerConnected(ovrController)) {
        return true;
    }
    
    if (controller && controller->get_enabled()) {
        try {
            float testValue = OVRInput::Get(OVRInput::Axis1D::PrimaryIndexTrigger, ovrController);
            return std::isfinite(testValue);
        } catch (...) {
            // Controller not responding
        }
    }
    
    return false;
}

void InputManagerEnhanced::HandleControllerConnectionChange(bool connected) {
    if (connected && !wasConnected) {
        PaperLogger.info("Enhanced controller {} connected", 
            saberType == 0 ? "left" : "right");
        
        // Reset enhanced input states
        triggerState = {};
        gripState = {};
        thumbstickState = {};
        
        auto now = std::chrono::steady_clock::now();
        triggerState.lastChangeTime = now;
        gripState.lastChangeTime = now;
        thumbstickState.lastChangeTime = now;
        
    } else if (!connected && wasConnected) {
        PaperLogger.warn("Enhanced controller {} disconnected", 
            saberType == 0 ? "left" : "right");
        
        // End active tricks
        if (triggerState.pressed && onTrickDeactivated) {
            onTrickDeactivated(config.triggerAction);
        }
        if (gripState.pressed && onTrickDeactivated) {
            onTrickDeactivated(config.gripAction);
        }
        if (thumbstickState.pressed && onTrickDeactivated) {
            onTrickDeactivated(config.thumbstickAction);
        }
        
        // Reset states
        triggerState.pressed = false;
        gripState.pressed = false;
        thumbstickState.pressed = false;
    }
}

#ifdef DEBUG
void InputManagerEnhanced::LogInputDiagnostics(const EnhancedInputState& state, TrickAction action, float value) {
    PaperLogger.debug("Input {} - Action: {}, Value: {:.3f}, Smoothed: {:.3f}, State: {}", 
        saberType == 0 ? "L" : "R",
        static_cast<int>(action),
        state.lastValue,
        value,
        state.pressed ? "ON" : "OFF");
}
#endif