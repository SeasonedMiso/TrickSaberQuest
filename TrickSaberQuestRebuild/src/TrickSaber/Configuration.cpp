#include "TrickSaber/Configuration.hpp"
#include "TrickSaber/Config.hpp"
#include "TrickSaber/LogSanitizer.hpp"
#include "main.hpp"
#include <algorithm>

namespace TrickSaber::Configuration {
    
    ModConfig config;
    
    void Initialize() {
        // Sync with legacy config system
        TrickSaber::config.trickSaberEnabled = config.enabled;
        TrickSaber::config.triggerThreshold = config.triggerThreshold;
        TrickSaber::config.thumbstickThreshold = config.thumbstickThreshold;
        TrickSaber::config.enableSpinTricks = config.enableSpinTricks;
        TrickSaber::config.enableThrowTricks = config.enableThrowTricks;
        TrickSaber::config.leftSaberEnabled = config.leftSaberEnabled;
        TrickSaber::config.rightSaberEnabled = config.rightSaberEnabled;
        TrickSaber::config.returnSpeed = config.returnSpeed;
        TrickSaber::config.throwVelocityMultiplier = config.throwVelocityMultiplier;
        TrickSaber::config.vibrateOnReturn = config.vibrateOnReturn;
        TrickSaber::config.disableIfNotesOnScreen = config.disableIfNotesOnScreen;
        TrickSaber::config.spinMode = config.spinMode;
        TrickSaber::config.throwMode = config.throwMode;
        
        Logger.info("TrickSaber configuration initialized with full PC parity and mode switching");
    }
    
    // Core accessors
    bool IsModEnabled() { return config.enabled; }
    float GetTriggerThreshold() { return config.triggerThreshold; }
    float GetGripThreshold() { return config.gripThreshold; }
    float GetThumbstickThreshold() { return config.thumbstickThreshold; }
    float GetVelocityThreshold() { return config.velocityThreshold; }
    
    // Input binding accessors
    TrickAction GetTriggerAction() { return config.triggerAction; }
    TrickAction GetGripAction() { return config.gripAction; }
    TrickAction GetThumbstickAction() { return config.thumbstickAction; }
    ThumbstickDir GetThumbstickDirection() { return config.thumbstickDirection; }
    
    // Trick settings
    bool AreSpinTricksEnabled() { return config.enableSpinTricks; }
    bool AreThrowTricksEnabled() { return config.enableThrowTricks; }
    bool IsLeftSaberEnabled() { return config.leftSaberEnabled; }
    bool IsRightSaberEnabled() { return config.rightSaberEnabled; }
    bool IsSpeedVelocityDependent() { return config.isSpeedVelocityDependent; }
    bool IsCompleteRotationMode() { return config.completeRotationMode; }
    
    // Physics settings
    float GetSpinSpeed() { return config.spinSpeed; }
    SpinDir GetSpinDirection() { return config.spinDirection; }
    float GetThrowVelocity() { return config.throwVelocity; }
    float GetReturnSpeed() { return config.returnSpeed; }
    float GetReturnSpinMultiplier() { return config.returnSpinMultiplier; }
    
    // Advanced features
    bool IsSlowmoDuringThrow() { return config.slowmoDuringThrow; }
    float GetSlowmoAmount() { return config.slowmoAmount; }
    bool ShouldDisableIfNotesOnScreen() { return config.disableIfNotesOnScreen; }
    bool CanHitNotesDuringTrick() { return config.hitNotesDuringTrick; }
    bool ShouldVibrateOnReturn() { return config.vibrateOnReturn; }
    
    // Legacy compatibility
    float GetThrowVelocityMultiplier() { return config.throwVelocityMultiplier; }
    
    void SetTrickEnabled(TrickAction action, bool enabled) {
        switch (action) {
            case TrickAction::Spin:
                config.enableSpinTricks = enabled;
                TrickSaber::config.enableSpinTricks = enabled;
                break;
            case TrickAction::Throw:
                config.enableThrowTricks = enabled;
                TrickSaber::config.enableThrowTricks = enabled;
                break;
            default:
                break;
        }
        Logger.info("Trick {} {}", static_cast<int>(action), enabled ? "enabled" : "disabled");
    }
    
    void SetInputThreshold(int inputType, float threshold) {
        threshold = std::clamp(threshold, 0.1f, 1.0f);
        
        switch (inputType) {
            case 0: // Trigger
                config.triggerThreshold = threshold;
                TrickSaber::config.triggerThreshold = threshold;
                break;
            case 1: // Grip
                config.gripThreshold = threshold;
                break;
            case 2: // Thumbstick
                config.thumbstickThreshold = threshold;
                TrickSaber::config.thumbstickThreshold = threshold;
                break;
            default:
                break;
        }
        Logger.info("Input threshold {} set to {:.2f}", inputType, threshold);
    }
    
    void SetInputBinding(int inputType, TrickAction action) {
        switch (inputType) {
            case 0: // Trigger
                config.triggerAction = action;
                break;
            case 1: // Grip
                config.gripAction = action;
                break;
            case 2: // Thumbstick
                config.thumbstickAction = action;
                break;
            default:
                break;
        }
        Logger.info("Input {} bound to trick {}", inputType, static_cast<int>(action));
    }
    
    void SetSpinSettings(bool velocityDependent, float speed, SpinDir direction) {
        config.isSpeedVelocityDependent = velocityDependent;
        config.spinSpeed = std::clamp(speed, 0.1f, 5.0f);
        config.spinDirection = direction;
        Logger.info("Spin settings updated: velocity-dependent={}, speed={:.1f}, direction={}", 
            velocityDependent, speed, static_cast<int>(direction));
    }
    
    void SetThrowSettings(float velocity, float returnSpeed) {
        config.throwVelocity = std::clamp(velocity, 0.1f, 5.0f);
        config.returnSpeed = std::clamp(returnSpeed, 1.0f, 50.0f);
        config.throwVelocityMultiplier = velocity;
        TrickSaber::config.throwVelocityMultiplier = velocity;
        TrickSaber::config.returnSpeed = returnSpeed;
        Logger.info("Throw settings updated: velocity={:.1f}, return speed={:.1f}", velocity, returnSpeed);
    }
    
    void SetAdvancedFeature(const std::string& feature, bool enabled) {
        if (feature == "slowmoDuringThrow") {
            config.slowmoDuringThrow = enabled;
        } else if (feature == "disableIfNotesOnScreen") {
            config.disableIfNotesOnScreen = enabled;
            TrickSaber::config.disableIfNotesOnScreen = enabled;
        } else if (feature == "hitNotesDuringTrick") {
            config.hitNotesDuringTrick = enabled;
        } else if (feature == "completeRotationMode") {
            config.completeRotationMode = enabled;
        }
        Logger.info("Advanced feature '{}' {}", Utils::SanitizeForLog(feature), enabled ? "enabled" : "disabled");
    }
    
    bool IsSimplifiedInputEnabled() { return config.useSimplifiedInput; }
    
    void SetSimplifiedInput(bool enabled) {
        config.useSimplifiedInput = enabled;
        Logger.info("Input mode switched to: {}", enabled ? "Simplified" : "PC Complex");
    }
    
    // Mode switching implementations
    void SetSpinMode(SpinMode mode) {
        config.spinMode = mode;
        TrickSaber::config.spinMode = mode;
        Logger.info("Spin mode set to: {}", static_cast<int>(mode));
    }
    
    void SetThrowMode(ThrowMode mode) {
        config.throwMode = mode;
        TrickSaber::config.throwMode = mode;
        Logger.info("Throw mode set to: {}", static_cast<int>(mode));
    }
    
    SpinMode GetSpinMode() { return config.spinMode; }
    ThrowMode GetThrowMode() { return config.throwMode; }
}