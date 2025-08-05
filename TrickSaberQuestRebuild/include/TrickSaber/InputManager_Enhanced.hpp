#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Vector2.hpp"
#include "GlobalNamespace/VRController.hpp"
#include "GlobalNamespace/SaberType.hpp"
#include "GlobalNamespace/OVRInput.hpp"

#include "TrickSaber/Enums.hpp"

#include <functional>
#include <chrono>
#include <array>

DECLARE_CLASS_CODEGEN(TrickSaber, InputManagerEnhanced, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(GlobalNamespace::VRController*, controller);
    DECLARE_INSTANCE_FIELD(int, saberType);
    
    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, Initialize, GlobalNamespace::VRController* vrController, GlobalNamespace::SaberType saberType);
    
public:
    std::function<void(TrickAction, float)> onTrickActivated;
    std::function<void(TrickAction)> onTrickDeactivated;
    
    bool IsControllerConnected() const;
    UnityEngine::Vector2 GetThumbstickVector2() const;
    
private:
    // Enhanced input state with smoothing and validation
    struct EnhancedInputState {
        bool pressed = false;
        float lastValue = 0.0f;
        std::chrono::steady_clock::time_point lastChangeTime;
        
        // 3-sample moving average for smoothing
        std::array<float, 3> valueHistory = {0.0f, 0.0f, 0.0f};
        int historyIndex = 0;
        
        // Hysteresis thresholds
        float pressThreshold = 0.8f;
        float releaseThreshold = 0.6f;
        
        // State validation (require 2 consecutive frames)
        bool pendingState = false;
        int consecutiveFrames = 0;
        
        float GetSmoothedValue() const;
        void UpdateHistory(float value);
        bool ValidateStateChange(bool newState);
    };
    
    EnhancedInputState triggerState;
    EnhancedInputState gripState;
    EnhancedInputState thumbstickState;
    
    GlobalNamespace::OVRInput::Controller ovrController;
    bool wasConnected = false;
    std::chrono::steady_clock::time_point lastConnectionCheck;
    
    // Enhanced constants
    static constexpr float DEBOUNCE_TIME_MS = 50.0f;
    static constexpr float CONNECTION_CHECK_INTERVAL_MS = 1000.0f;
    static constexpr int VALIDATION_FRAMES = 2;
    
    void CheckInputs();
    void CheckEnhancedInput(std::function<bool(float&)> getValue, EnhancedInputState& state, TrickAction action);
    bool GetTriggerValue(float& value);
    bool GetGripValue(float& value);
    bool GetThumbstickValue(float& value);
    bool IsDebounceTimeElapsed(const EnhancedInputState& state) const;
    bool IsControllerDetected() const;
    void HandleControllerConnectionChange(bool connected);
    
#ifdef DEBUG
    void LogInputDiagnostics(const EnhancedInputState& state, TrickAction action, float value);
#endif
);