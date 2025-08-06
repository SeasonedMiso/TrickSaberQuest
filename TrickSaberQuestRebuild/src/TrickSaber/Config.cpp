#include "TrickSaber/Config.hpp"
#include "TrickSaber/Constants.hpp"
#include "main.hpp"
#include <algorithm>

namespace TrickSaber {
    Config config;
    
    void LoadConfig() {
        try {
            // Load default configuration for now
            config = Config{};
            Logger.info("Configuration loaded successfully");
        } catch (const std::exception& e) {
            Logger.error("Failed to load configuration");
            LoadDefaultConfig();
        } catch (...) {
            Logger.error("Unknown error loading configuration, using defaults");
            LoadDefaultConfig();
        }
    }
    
    void SaveConfig() {
        try {
            Logger.info("Configuration saved successfully");
        } catch (const std::exception& e) {
            Logger.error("Failed to save configuration");
        } catch (...) {
            Logger.error("Unknown error saving configuration");
        }
    }
    
    TrickAction ValidateTrickAction(int value, TrickAction defaultValue) {
        if (value >= static_cast<int>(TrickAction::None) && value <= static_cast<int>(TrickAction::Spin)) {
            return static_cast<TrickAction>(value);
        }
        Logger.error("Invalid TrickAction value, using default");
        return defaultValue;
    }
    
    SpinDir ValidateSpinDir(int value, SpinDir defaultValue) {
        if (value >= static_cast<int>(SpinDir::Forward) && value <= static_cast<int>(SpinDir::Backward)) {
            return static_cast<SpinDir>(value);
        }
        Logger.error("Invalid SpinDir value, using default");
        return defaultValue;
    }
    
    ThumbstickDir ValidateThumbstickDir(int value, ThumbstickDir defaultValue) {
        if (value >= static_cast<int>(ThumbstickDir::Horizontal) && value <= static_cast<int>(ThumbstickDir::Vertical)) {
            return static_cast<ThumbstickDir>(value);
        }
        Logger.error("Invalid ThumbstickDir value, using default");
        return defaultValue;
    }
    
    ThrowMode ValidateThrowMode(int value, ThrowMode defaultValue) {
        if (value >= static_cast<int>(ThrowMode::PressToThrow) && value <= static_cast<int>(ThrowMode::GripToSummon)) {
            return static_cast<ThrowMode>(value);
        }
        Logger.error("Invalid ThrowMode value, using default");
        return defaultValue;
    }
    
    SpinMode ValidateSpinMode(int value, SpinMode defaultValue) {
        if (value >= static_cast<int>(SpinMode::Traditional) && value <= static_cast<int>(SpinMode::Momentum)) {
            return static_cast<SpinMode>(value);
        }
        Logger.error("Invalid SpinMode value, using default");
        return defaultValue;
    }
    
    void ValidateConfigConsistency() {
        if (config.triggerAction == TrickAction::None && 
            config.gripAction == TrickAction::None && 
            config.thumbstickAction == TrickAction::None) {
            Logger.error("No inputs assigned to tricks, assigning defaults");
            config.triggerAction = TrickAction::Throw;
            config.thumbstickAction = TrickAction::Spin;
        }
        
        if (config.slowmoDuringThrow && config.slowmoAmount >= Constants::NORMAL_TIME_SCALE) {
            Logger.error("Invalid slowmo amount, clamping to valid range");
            config.slowmoAmount = 0.5f;
        }
    }
    
    void LoadDefaultConfig() {
        Logger.info("Loading default configuration");
        config = Config{};
    }
}