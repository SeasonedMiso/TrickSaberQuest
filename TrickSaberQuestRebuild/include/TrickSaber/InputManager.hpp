#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "GlobalNamespace/VRController.hpp"
#include "GlobalNamespace/SaberType.hpp"
#include "GlobalNamespace/OVRInput.hpp"

#include "TrickSaber/Enums.hpp"

#include <functional>
#include <chrono>

DECLARE_CLASS_CODEGEN(TrickSaber, InputManager, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(GlobalNamespace::VRController*, controller);
    DECLARE_INSTANCE_FIELD(int, saberType);
    
    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, Initialize, GlobalNamespace::VRController* vrController, GlobalNamespace::SaberType saberType);
    
public:
    std::function<void(TrickAction, float)> onTrickActivated;
    std::function<void(TrickAction)> onTrickDeactivated;
    
    bool IsControllerConnected() const;
    
private:
    // Input state tracking
    struct InputState {
        bool pressed = false;
        float lastValue = 0.0f;
        std::chrono::steady_clock::time_point lastChangeTime;
    };
    
    InputState triggerState;
    InputState gripState;
    InputState thumbstickState;
    
    GlobalNamespace::OVRInput::Controller ovrController;
    
    // Controller detection state
    bool wasConnected = false;
    std::chrono::steady_clock::time_point lastConnectionCheck;
    
    static constexpr float DEBOUNCE_TIME_MS = 50.0f;
    static constexpr float CONNECTION_CHECK_INTERVAL_MS = 1000.0f;
    
    void CheckInputs();
    void CheckInput(std::function<bool(float&)> getValue, InputState& state, TrickAction action);
    bool GetTriggerValue(float& value);
    bool GetGripValue(float& value);
    bool GetThumbstickValue(float& value);
    bool IsDebounceTimeElapsed(const InputState& state) const;
    bool IsControllerDetected();
    void HandleControllerConnectionChange(bool connected);
    
    // Simplified input methods
    void CheckInputsSimplified();
    bool GetTriggerValueSimplified(float& value);
    bool GetThumbstickValueSimplified(float& value);
    
    // Multi-input combination checking
    void CheckMultiInputCombinations();
    bool GetButtonOneValue(float& value);
    bool GetButtonTwoValue(float& value);
    
    // Multi-input state tracking
    InputState buttonOneState;
    InputState buttonTwoState;
    
    // Combination detection
    struct CombinationState {
        bool active = false;
        TrickAction primaryAction = TrickAction::None;
        TrickAction secondaryAction = TrickAction::None;
        std::chrono::steady_clock::time_point startTime;
    };
    
    CombinationState activeCombination;
    static constexpr float COMBINATION_WINDOW_MS = 200.0f; // Time window for combination detection
    
    InputState& GetInputState(TrickAction action);
);