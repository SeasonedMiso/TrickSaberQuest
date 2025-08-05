#pragma once

#include "TrickSaber/Enums.hpp"
#include "TrickSaber/Constants.hpp"

namespace TrickSaber {
    struct Config {
        bool trickSaberEnabled = true;
        
        TrickAction triggerAction = TrickAction::Throw;
        TrickAction gripAction = TrickAction::None;
        TrickAction thumbstickAction = TrickAction::Spin;
        TrickAction buttonOneAction = TrickAction::None;
        TrickAction buttonTwoAction = TrickAction::None;
        
        bool reverseTrigger = false;
        bool reverseGrip = false;
        bool reverseThumbstick = false;
        bool reverseButtonOne = false;
        bool reverseButtonTwo = false;
        
        ThumbstickDir thumbstickDirection = ThumbstickDir::Horizontal;
        float thumbstickDeadzone = Constants::DEFAULT_THUMBSTICK_DEADZONE;
        
        float triggerThreshold = Constants::DEFAULT_TRIGGER_THRESHOLD;
        float gripThreshold = Constants::DEFAULT_GRIP_THRESHOLD;
        float thumbstickThreshold = Constants::DEFAULT_THUMBSTICK_THRESHOLD;
        
        float controllerSnapThreshold = Constants::DEFAULT_THUMBSTICK_DEADZONE;
        
        SpinMode spinMode = SpinMode::OmniDirectional;
        bool isSpeedVelocityDependent = false;
        float spinSpeed = 1.0f;
        SpinDir spinDirection = SpinDir::Backward;
        float spinAnchorZOffset = 0.25f;
        
        ThrowMode throwMode = ThrowMode::PressToThrow;
        float throwVelocity = 1.0f;
        float returnSpeed = 10.0f;
        float returnSpinMultiplier = 1.0f;
        float spinMultiplier = 1.0f;
        
        bool slowmoDuringThrow = false;
        float slowmoAmount = 0.2f;
        float slowmoStepAmount = 0.02f;
        
        bool completeRotationMode = false;
        bool disableIfNotesOnScreen = false;
        bool hitNotesDuringTrick = false;
        bool vibrateOnReturn = true;
        float hapticIntensity = 1.0f;
        bool moveWhileThrown = false;
        bool enableTrickCutting = false;
        
        // Enhanced physics options
        bool isVelocityDependent = true;
        float throwVelocityMultiplier = 3.0f;
        float returnDuration = Constants::DEFAULT_RETURN_DURATION;
        
        // Throw physics
        float gravityScale = 1.0f;
        float airResistance = 0.98f;
        float angularDamping = 0.95f;
        float maxThrowDistance = 10.0f;
        float snapBackDistance = Constants::DEFAULT_SNAP_BACK_DISTANCE;
        
        // Spin physics
        float spinAcceleration = 720.0f;  // degrees/sec²
        float spinDeceleration = 1440.0f; // degrees/sec²
        
        // Per-saber configuration
        bool leftSaberEnabled = true;
        bool rightSaberEnabled = true;
        
        // Advanced controller options
        bool noTricksWhileNotes = false;
        
        int velocityBufferSize = Constants::DEFAULT_VELOCITY_BUFFER_SIZE;
        
        // Physics quality settings
        bool useRigidbodyPhysics = true;
        bool smoothSpinTransitions = true;
        bool enableCollisionDetection = true;
        
        // Missing configuration options
        float minThrowVelocity = 0.5f;
        float maxThrowVelocity = 10.0f;
        bool enableSpinTricks = true;
        bool enableThrowTricks = true;
        
        // Input reverse options (already declared above)
        // Performance options
        bool optimizePerformance = true;
        int maxActiveEffects = 10;
        
        // Missing configuration options
        bool joystickAngleSpeedControl = false;
        bool momentumSpinMode = false;
        
        // Debug overlay
        bool showDebugOverlay = false;
    };
    
    extern Config config;
    
    void LoadConfig();
    void SaveConfig();
    void LoadDefaultConfig();
    void ValidateConfigConsistency();
    TrickAction ValidateTrickAction(int value, TrickAction defaultValue);
    SpinDir ValidateSpinDir(int value, SpinDir defaultValue);
    SpinMode ValidateSpinMode(int value, SpinMode defaultValue);
    ThumbstickDir ValidateThumbstickDir(int value, ThumbstickDir defaultValue);
    ThrowMode ValidateThrowMode(int value, ThrowMode defaultValue);
}