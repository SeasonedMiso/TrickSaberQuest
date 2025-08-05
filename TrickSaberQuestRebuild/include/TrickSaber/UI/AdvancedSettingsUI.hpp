#pragma once

#include "custom-types/shared/macros.hpp"
#include "HMUI/ViewController.hpp"
#include "UnityEngine/Transform.hpp"

DECLARE_CLASS_CODEGEN(TrickSaber::UI, AdvancedSettingsUI, HMUI::ViewController,
        // Simplified DidActivate without complex override syntax
        DECLARE_INSTANCE_METHOD(void, DidActivate, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
        
        // Basic UI Creation methods
        DECLARE_INSTANCE_METHOD(void, CreateMainSettings, UnityEngine::Transform* parent);
        DECLARE_INSTANCE_METHOD(void, CreateInputSettings, UnityEngine::Transform* parent);
        DECLARE_INSTANCE_METHOD(void, CreateTrickSettings, UnityEngine::Transform* parent);
        
        // Simplified input binding UI
        DECLARE_INSTANCE_METHOD(void, OnInputBindingChanged, int action, int inputType, bool isLeft);
        DECLARE_INSTANCE_METHOD(void, OnInputThresholdChanged, int inputType, float threshold, bool isLeft);
        
        // Basic trick configuration UI
        DECLARE_INSTANCE_METHOD(void, CreateSpinTrickSettings, UnityEngine::Transform* parent);
        DECLARE_INSTANCE_METHOD(void, CreateThrowTrickSettings, UnityEngine::Transform* parent);
        
        // Utility methods with simplified signatures
        DECLARE_INSTANCE_METHOD(void, RefreshAllSettings);
        DECLARE_INSTANCE_METHOD(void, ResetToDefaults);
        
    private:
        bool uiInitialized = false;
);