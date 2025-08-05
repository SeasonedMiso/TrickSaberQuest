#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"

DECLARE_CLASS_CODEGEN(TrickSaber, AdvancedInputSystem, UnityEngine::MonoBehaviour,
        DECLARE_INSTANCE_METHOD(void, Awake);
        DECLARE_INSTANCE_METHOD(void, Update);
        
        // Simplified configuration methods using int instead of enums
        DECLARE_INSTANCE_METHOD(void, SetInputBinding, int inputType, int action, bool isLeft);
        DECLARE_INSTANCE_METHOD(void, SetInputThreshold, int inputType, float threshold, bool isLeft);
        DECLARE_INSTANCE_METHOD(void, SetInputEnabled, int inputType, bool enabled, bool isLeft);
        
        // Simplified query methods
        DECLARE_INSTANCE_METHOD(bool, IsInputPressed, int inputType, bool isLeft);
        DECLARE_INSTANCE_METHOD(bool, WasInputJustPressed, int inputType, bool isLeft);
        DECLARE_INSTANCE_METHOD(float, GetInputValue, int inputType, bool isLeft);
        
        // Basic features
        DECLARE_INSTANCE_METHOD(bool, IsVelocityDependentSpinEnabled);
        DECLARE_INSTANCE_METHOD(float, GetVelocityBasedSpinSpeed, bool isLeft);
        
        DECLARE_STATIC_METHOD(AdvancedInputSystem*, GetInstance);
        
    private:
        static AdvancedInputSystem* instance;
);