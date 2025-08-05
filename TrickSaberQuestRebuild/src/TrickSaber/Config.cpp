#include "TrickSaber/Config.hpp"
#include "TrickSaber/Constants.hpp"
#include "main.hpp"
#include <algorithm>

namespace TrickSaber {
    Config config;
    
    // Helper functions for cleaner config loading
    template<typename T>
    T GetConfigValue(const rapidjson::Document& cfg, const char* key, T defaultValue) {
        if constexpr (std::is_same_v<T, bool>) {
            return cfg.HasMember(key) && cfg[key].IsBool() ? cfg[key].GetBool() : defaultValue;
        } else if constexpr (std::is_same_v<T, float>) {
            return cfg.HasMember(key) && cfg[key].IsFloat() ? cfg[key].GetFloat() : defaultValue;
        } else if constexpr (std::is_same_v<T, int>) {
            return cfg.HasMember(key) && cfg[key].IsInt() ? cfg[key].GetInt() : defaultValue;
        }
        return defaultValue;
    }
    
    // Macros for simplified config loading
    #define LOAD_BOOL_CONFIG(name, defaultVal) \
        config.name = GetConfigValue(cfg.config, #name, defaultVal)
    
    #define LOAD_FLOAT_CONFIG(name, defaultVal, minVal, maxVal) \
        config.name = std::clamp(GetConfigValue(cfg.config, #name, defaultVal), minVal, maxVal)
    
    #define LOAD_INT_CONFIG(name, defaultVal, minVal, maxVal) \
        config.name = std::clamp(GetConfigValue(cfg.config, #name, defaultVal), minVal, maxVal)
    
    #define LOAD_ENUM_CONFIG(name, enumType, defaultVal, validator) \
        config.name = validator(GetConfigValue(cfg.config, #name, static_cast<int>(defaultVal)), defaultVal)
    
    void LoadConfig() {
        try {
            auto& cfg = getConfig();
            
            // Basic settings
            LOAD_BOOL_CONFIG(trickSaberEnabled, true);
            LOAD_BOOL_CONFIG(trickSaberEnabled, true);
            
            // Input actions
            LOAD_ENUM_CONFIG(triggerAction, TrickAction, TrickAction::Throw, ValidateTrickAction);
            LOAD_ENUM_CONFIG(gripAction, TrickAction, TrickAction::None, ValidateTrickAction);
            LOAD_ENUM_CONFIG(thumbstickAction, TrickAction, TrickAction::Spin, ValidateTrickAction);
            LOAD_ENUM_CONFIG(buttonOneAction, TrickAction, TrickAction::None, ValidateTrickAction);
            LOAD_ENUM_CONFIG(buttonTwoAction, TrickAction, TrickAction::None, ValidateTrickAction);
            
            // Input thresholds
            LOAD_FLOAT_CONFIG(triggerThreshold, Constants::DEFAULT_TRIGGER_THRESHOLD, Constants::MIN_THRESHOLD, Constants::MAX_THRESHOLD);
            LOAD_FLOAT_CONFIG(gripThreshold, Constants::DEFAULT_GRIP_THRESHOLD, Constants::MIN_THRESHOLD, Constants::MAX_THRESHOLD);
            LOAD_FLOAT_CONFIG(thumbstickThreshold, Constants::DEFAULT_THUMBSTICK_THRESHOLD, Constants::MIN_THRESHOLD, Constants::MAX_THRESHOLD);
            LOAD_FLOAT_CONFIG(thumbstickDeadzone, Constants::DEFAULT_THUMBSTICK_DEADZONE, Constants::MIN_DEADZONE, Constants::MAX_DEADZONE);
            LOAD_FLOAT_CONFIG(controllerSnapThreshold, Constants::DEFAULT_THUMBSTICK_DEADZONE, Constants::MIN_THRESHOLD, 2.0f);
            
            // Spin settings
            LOAD_FLOAT_CONFIG(spinSpeed, 1.0f, Constants::MIN_SPEED, Constants::MAX_SPEED);
            LOAD_ENUM_CONFIG(spinDirection, SpinDir, SpinDir::Backward, ValidateSpinDir);
            LOAD_FLOAT_CONFIG(spinAnchorZOffset, 0.25f, -0.5f, 1.0f);
            LOAD_ENUM_CONFIG(thumbstickDirection, ThumbstickDir, ThumbstickDir::Horizontal, ValidateThumbstickDir);
            
            // Throw settings
            LOAD_ENUM_CONFIG(throwMode, ThrowMode, ThrowMode::PressToThrow, ValidateThrowMode);
            LOAD_FLOAT_CONFIG(throwVelocity, 1.0f, Constants::MIN_VELOCITY, 3.0f);
            LOAD_FLOAT_CONFIG(returnSpeed, 10.0f, 1.0f, 50.0f);
            LOAD_FLOAT_CONFIG(returnSpinMultiplier, 1.0f, 0.0f, Constants::MAX_SPEED);
            LOAD_FLOAT_CONFIG(minThrowVelocity, 0.5f, Constants::MIN_VELOCITY, 2.0f);
            LOAD_FLOAT_CONFIG(maxThrowVelocity, 10.0f, 5.0f, Constants::MAX_VELOCITY);
            
            // Slowmo settings
            LOAD_BOOL_CONFIG(slowmoDuringThrow, false);
            LOAD_FLOAT_CONFIG(slowmoAmount, 0.2f, Constants::MIN_SLOWMO, Constants::MAX_SLOWMO);
            LOAD_FLOAT_CONFIG(slowmoStepAmount, 0.02f, 0.01f, Constants::MIN_SLOWMO);
            
            // Input reverse options
            LOAD_BOOL_CONFIG(reverseTrigger, false);
            LOAD_BOOL_CONFIG(reverseGrip, false);
            LOAD_BOOL_CONFIG(reverseThumbstick, false);
            LOAD_BOOL_CONFIG(reverseButtonOne, false);
            LOAD_BOOL_CONFIG(reverseButtonTwo, false);
            
            // Gameplay options
            LOAD_BOOL_CONFIG(completeRotationMode, false);
            LOAD_BOOL_CONFIG(isSpeedVelocityDependent, false);
            LOAD_BOOL_CONFIG(vibrateOnReturn, true);
            LOAD_BOOL_CONFIG(moveWhileThrown, false);
            LOAD_BOOL_CONFIG(enableTrickCutting, false);
            LOAD_BOOL_CONFIG(disableIfNotesOnScreen, false);
            LOAD_BOOL_CONFIG(hitNotesDuringTrick, false);
            
            // Saber settings
            LOAD_BOOL_CONFIG(leftSaberEnabled, true);
            LOAD_BOOL_CONFIG(rightSaberEnabled, true);
            LOAD_BOOL_CONFIG(enableSpinTricks, true);
            LOAD_BOOL_CONFIG(enableThrowTricks, true);
            
            // Performance settings
            LOAD_BOOL_CONFIG(optimizePerformance, true);
            LOAD_INT_CONFIG(maxActiveEffects, 10, 1, 50);
            LOAD_INT_CONFIG(velocityBufferSize, Constants::DEFAULT_VELOCITY_BUFFER_SIZE, 1, Constants::MAX_VELOCITY_BUFFER_SIZE);
            
            // Advanced settings
            LOAD_FLOAT_CONFIG(hapticIntensity, 1.0f, 0.0f, 2.0f);
            LOAD_BOOL_CONFIG(joystickAngleSpeedControl, false);
            LOAD_BOOL_CONFIG(momentumSpinMode, false);
            
            // Debug settings
            LOAD_BOOL_CONFIG(showDebugOverlay, false);
            
            // Validate configuration consistency
            ValidateConfigConsistency();
            
            PaperLogger.info("Configuration loaded successfully");
        } catch (const std::exception& e) {
            PaperLogger.error("Failed to load configuration: {}", e.what());
            LoadDefaultConfig();
        } catch (...) {
            PaperLogger.error("Unknown error loading configuration, using defaults");
            LoadDefaultConfig();
        }
    }
    
    // Macros for simplified config saving
    #define SAVE_BOOL_CONFIG(name) \
        cfg.config[#name].SetBool(config.name)
    
    #define SAVE_FLOAT_CONFIG(name) \
        cfg.config[#name].SetFloat(config.name)
    
    #define SAVE_INT_CONFIG(name) \
        cfg.config[#name].SetInt(config.name)
    
    #define SAVE_ENUM_CONFIG(name) \
        cfg.config[#name].SetInt(static_cast<int>(config.name))
    
    void SaveConfig() {
        try {
            auto& cfg = getConfig();
            
            // Basic settings
            SAVE_BOOL_CONFIG(trickSaberEnabled);
            SAVE_BOOL_CONFIG(trickSaberEnabled);
            
            // Input actions
            SAVE_ENUM_CONFIG(triggerAction);
            SAVE_ENUM_CONFIG(gripAction);
            SAVE_ENUM_CONFIG(thumbstickAction);
            SAVE_ENUM_CONFIG(buttonOneAction);
            SAVE_ENUM_CONFIG(buttonTwoAction);
            
            // Input thresholds
            SAVE_FLOAT_CONFIG(triggerThreshold);
            SAVE_FLOAT_CONFIG(gripThreshold);
            SAVE_FLOAT_CONFIG(thumbstickThreshold);
            SAVE_FLOAT_CONFIG(thumbstickDeadzone);
            SAVE_FLOAT_CONFIG(controllerSnapThreshold);
            
            // Spin settings
            SAVE_FLOAT_CONFIG(spinSpeed);
            SAVE_ENUM_CONFIG(spinDirection);
            SAVE_FLOAT_CONFIG(spinAnchorZOffset);
            SAVE_ENUM_CONFIG(thumbstickDirection);
            
            // Throw settings
            SAVE_ENUM_CONFIG(throwMode);
            SAVE_FLOAT_CONFIG(throwVelocity);
            SAVE_FLOAT_CONFIG(returnSpeed);
            SAVE_FLOAT_CONFIG(returnSpinMultiplier);
            SAVE_FLOAT_CONFIG(minThrowVelocity);
            SAVE_FLOAT_CONFIG(maxThrowVelocity);
            
            // Slowmo settings
            SAVE_BOOL_CONFIG(slowmoDuringThrow);
            SAVE_FLOAT_CONFIG(slowmoAmount);
            SAVE_FLOAT_CONFIG(slowmoStepAmount);
            
            // Input reverse options
            SAVE_BOOL_CONFIG(reverseTrigger);
            SAVE_BOOL_CONFIG(reverseGrip);
            SAVE_BOOL_CONFIG(reverseThumbstick);
            SAVE_BOOL_CONFIG(reverseButtonOne);
            SAVE_BOOL_CONFIG(reverseButtonTwo);
            
            // Gameplay options
            SAVE_BOOL_CONFIG(completeRotationMode);
            SAVE_BOOL_CONFIG(isSpeedVelocityDependent);
            SAVE_BOOL_CONFIG(vibrateOnReturn);
            SAVE_BOOL_CONFIG(moveWhileThrown);
            SAVE_BOOL_CONFIG(enableTrickCutting);
            SAVE_BOOL_CONFIG(disableIfNotesOnScreen);
            SAVE_BOOL_CONFIG(hitNotesDuringTrick);
            
            // Saber settings
            SAVE_BOOL_CONFIG(leftSaberEnabled);
            SAVE_BOOL_CONFIG(rightSaberEnabled);
            SAVE_BOOL_CONFIG(enableSpinTricks);
            SAVE_BOOL_CONFIG(enableThrowTricks);
            
            // Performance settings
            SAVE_BOOL_CONFIG(optimizePerformance);
            SAVE_INT_CONFIG(maxActiveEffects);
            SAVE_INT_CONFIG(velocityBufferSize);
            
            // Advanced settings
            SAVE_FLOAT_CONFIG(hapticIntensity);
            SAVE_BOOL_CONFIG(joystickAngleSpeedControl);
            SAVE_BOOL_CONFIG(momentumSpinMode);
            
            // Debug settings
            SAVE_BOOL_CONFIG(showDebugOverlay);
            
            cfg.Write();
            PaperLogger.info("Configuration saved successfully");
        } catch (const std::exception& e) {
            PaperLogger.error("Failed to save configuration: {}", e.what());
        } catch (...) {
            PaperLogger.error("Unknown error saving configuration");
        }
    }
    
    TrickAction ValidateTrickAction(int value, TrickAction defaultValue) {
        if (value >= static_cast<int>(TrickAction::None) && value <= static_cast<int>(TrickAction::Spin)) {
            return static_cast<TrickAction>(value);
        }
        PaperLogger.error("Invalid TrickAction value: {}, using default", value);
        return defaultValue;
    }
    
    SpinDir ValidateSpinDir(int value, SpinDir defaultValue) {
        if (value >= static_cast<int>(SpinDir::Forward) && value <= static_cast<int>(SpinDir::Backward)) {
            return static_cast<SpinDir>(value);
        }
        PaperLogger.error("Invalid SpinDir value: {}, using default", value);
        return defaultValue;
    }
    
    ThumbstickDir ValidateThumbstickDir(int value, ThumbstickDir defaultValue) {
        if (value >= static_cast<int>(ThumbstickDir::Horizontal) && value <= static_cast<int>(ThumbstickDir::Vertical)) {
            return static_cast<ThumbstickDir>(value);
        }
        PaperLogger.error("Invalid ThumbstickDir value: {}, using default", value);
        return defaultValue;
    }
    
    ThrowMode ValidateThrowMode(int value, ThrowMode defaultValue) {
        if (value >= static_cast<int>(ThrowMode::PressToThrow) && value <= static_cast<int>(ThrowMode::GripToSummon)) {
            return static_cast<ThrowMode>(value);
        }
        PaperLogger.error("Invalid ThrowMode value: {}, using default", value);
        return defaultValue;
    }
    
    SpinMode ValidateSpinMode(int value, SpinMode defaultValue) {
        if (value >= static_cast<int>(SpinMode::Traditional) && value <= static_cast<int>(SpinMode::Momentum)) {
            return static_cast<SpinMode>(value);
        }
        PaperLogger.error("Invalid SpinMode value: {}, using default", value);
        return defaultValue;
    }
    
    void ValidateConfigConsistency() {
        // Ensure at least one input is assigned to a trick
        if (config.triggerAction == TrickAction::None && 
            config.gripAction == TrickAction::None && 
            config.thumbstickAction == TrickAction::None) {
            PaperLogger.error("No inputs assigned to tricks, assigning defaults");
            config.triggerAction = TrickAction::Throw;
            config.thumbstickAction = TrickAction::Spin;
        }
        
        // Validate slowmo settings
        if (config.slowmoDuringThrow && config.slowmoAmount >= Constants::NORMAL_TIME_SCALE) {
            PaperLogger.error("Invalid slowmo amount, clamping to valid range");
            config.slowmoAmount = 0.5f;
        }
        
        // Validate spin mode combinations
        if (config.joystickAngleSpeedControl && config.thumbstickAction != TrickAction::Spin) {
            PaperLogger.info("joystickAngleSpeedControl enabled but thumbstick not assigned to spin");
        }
        
        if (config.momentumSpinMode && config.thumbstickAction != TrickAction::Spin) {
            PaperLogger.info("momentumSpinMode enabled but thumbstick not assigned to spin");
        }
    }
    
    void LoadDefaultConfig() {
        PaperLogger.info("Loading default configuration");
        config = Config{}; // Reset to default values
    }
    
    // Clean up macros
    #undef LOAD_BOOL_CONFIG
    #undef LOAD_FLOAT_CONFIG
    #undef LOAD_INT_CONFIG
    #undef LOAD_ENUM_CONFIG
    #undef SAVE_BOOL_CONFIG
    #undef SAVE_FLOAT_CONFIG
    #undef SAVE_INT_CONFIG
    #undef SAVE_ENUM_CONFIG
}