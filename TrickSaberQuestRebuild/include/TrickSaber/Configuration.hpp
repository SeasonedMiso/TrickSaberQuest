#pragma once

#include "TrickSaber/Enums.hpp"
#include <string>

namespace TrickSaber::Configuration {
    
    // Full configuration structure matching PC version
    struct ModConfig {
        // Core settings
        bool enabled = true;
        
        // Input bindings
        TrickAction triggerAction = TrickAction::Throw;
        TrickAction gripAction = TrickAction::None;
        TrickAction thumbstickAction = TrickAction::Spin;
        
        // Input options
        bool reverseTrigger = false;
        bool reverseGrip = false;
        bool reverseThumbstick = false;
        ThumbstickDir thumbstickDirection = ThumbstickDir::Horizontal;
        
        // Input thresholds
        float triggerThreshold = 0.8f;
        float gripThreshold = 0.8f;
        float thumbstickThreshold = 0.8f;
        float controllerSnapThreshold = 0.3f;
        
        // Spin settings
        bool isSpeedVelocityDependent = false;
        float spinSpeed = 1.0f;
        SpinDir spinDirection = SpinDir::Backward;
        SpinMode spinMode = SpinMode::OmniDirectional;
        bool completeRotationMode = false;
        
        // Throw settings
        float throwVelocity = 1.0f;
        float returnSpeed = 10.0f;
        float returnSpinMultiplier = 1.0f;
        ThrowMode throwMode = ThrowMode::PressToThrow;
        
        // Advanced features
        bool slowmoDuringThrow = false;
        float slowmoAmount = 0.2f;
        bool disableIfNotesOnScreen = false;
        bool hitNotesDuringTrick = false;
        
        // Legacy compatibility
        float velocityThreshold = 0.5f;
        bool enableSpinTricks = true;
        bool enableThrowTricks = true;
        bool leftSaberEnabled = true;
        bool rightSaberEnabled = true;
        float throwVelocityMultiplier = 3.0f;
        bool vibrateOnReturn = true;
        
        // Advanced settings
        int velocityBufferSize = 5;
        float slowmoStepAmount = 0.02f;
        
        // A/B Testing toggle
        bool useSimplifiedInput = false; // false = PC complex, true = simplified
        
        // Debug overlay
        bool showDebugOverlay = false;
    };
    
    // Global config instance
    extern ModConfig config;
    
    // Initialize configuration
    void Initialize();
    
    // Core accessors
    bool IsModEnabled();
    float GetTriggerThreshold();
    float GetGripThreshold();
    float GetThumbstickThreshold();
    float GetVelocityThreshold();
    
    // Input binding accessors
    TrickAction GetTriggerAction();
    TrickAction GetGripAction();
    TrickAction GetThumbstickAction();
    ThumbstickDir GetThumbstickDirection();
    
    // Trick settings
    bool AreSpinTricksEnabled();
    bool AreThrowTricksEnabled();
    bool IsLeftSaberEnabled();
    bool IsRightSaberEnabled();
    bool IsSpeedVelocityDependent();
    bool IsCompleteRotationMode();
    
    // Physics settings
    float GetSpinSpeed();
    SpinDir GetSpinDirection();
    float GetThrowVelocity();
    float GetReturnSpeed();
    float GetReturnSpinMultiplier();
    
    // Advanced features
    bool IsSlowmoDuringThrow();
    float GetSlowmoAmount();
    bool ShouldDisableIfNotesOnScreen();
    bool CanHitNotesDuringTrick();
    bool ShouldVibrateOnReturn();
    
    // Runtime configuration changes
    void SetTrickEnabled(TrickAction action, bool enabled);
    void SetInputThreshold(int inputType, float threshold);
    void SetInputBinding(int inputType, TrickAction action);
    void SetSpinSettings(bool velocityDependent, float speed, SpinDir direction);
    void SetThrowSettings(float velocity, float returnSpeed);
    void SetAdvancedFeature(const std::string& feature, bool enabled);
    
    // Mode switching functions
    void SetSpinMode(SpinMode mode);
    void SetThrowMode(ThrowMode mode);
    SpinMode GetSpinMode();
    ThrowMode GetThrowMode();
    
    // A/B Testing
    bool IsSimplifiedInputEnabled();
    void SetSimplifiedInput(bool enabled);
}