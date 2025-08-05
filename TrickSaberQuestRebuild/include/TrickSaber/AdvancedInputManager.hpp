#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "TrickSaber/Enums.hpp"

DECLARE_CLASS_CODEGEN(TrickSaber, AdvancedInputManager, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, Update);
    
    DECLARE_INSTANCE_FIELD(bool, enabled);
    
    // Input state tracking
    DECLARE_INSTANCE_FIELD(bool, leftTriggerPressed);
    DECLARE_INSTANCE_FIELD(bool, rightTriggerPressed);
    DECLARE_INSTANCE_FIELD(bool, leftGripPressed);
    DECLARE_INSTANCE_FIELD(bool, rightGripPressed);
    DECLARE_INSTANCE_FIELD(bool, leftThumbstickActive);
    DECLARE_INSTANCE_FIELD(bool, rightThumbstickActive);
    
    // Input checking methods
    DECLARE_INSTANCE_METHOD(void, CheckTriggerInput);
    DECLARE_INSTANCE_METHOD(void, CheckGripInput);
    DECLARE_INSTANCE_METHOD(void, CheckThumbstickInput);
    
    // Static initialization
    DECLARE_STATIC_METHOD(void, Initialize);
    
public:
    void OnInputActivated(TrickAction action, float value, bool isLeft);
    void OnInputDeactivated(TrickAction action, bool isLeft);
    void OnInputUpdated(TrickAction action, float value, bool isLeft);
)