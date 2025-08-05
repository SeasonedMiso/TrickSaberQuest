#pragma once

#include <cstdint>

namespace TrickSaber::Constants {
    // Input System
    constexpr int DEBOUNCE_TIME_MS = 75;
    constexpr int CONNECTION_CHECK_INTERVAL_MS = 1000;
    constexpr int COMBINATION_WINDOW_MS = 200;
    
    // Input Thresholds
    constexpr float DEFAULT_TRIGGER_THRESHOLD = 0.8f;
    constexpr float DEFAULT_GRIP_THRESHOLD = 0.8f;
    constexpr float DEFAULT_THUMBSTICK_THRESHOLD = 0.8f;
    constexpr float DEFAULT_THUMBSTICK_DEADZONE = 0.3f;
    constexpr float SIMPLIFIED_TRIGGER_THRESHOLD = 0.8f;
    constexpr float SIMPLIFIED_THUMBSTICK_THRESHOLD = 0.7f;
    
    // Physics
    constexpr float GRAVITY_ACCELERATION = 9.81f;
    constexpr float DEFAULT_RETURN_DURATION = 0.5f;
    constexpr float SIMPLIFIED_RETURN_DURATION = 0.3f;
    constexpr float DEFAULT_SNAP_BACK_DISTANCE = 0.3f;
    constexpr float VELOCITY_MULTIPLIER_SCALE = 2.0f;
    constexpr float SPIN_VELOCITY_SCALE = 180.0f;  // degrees per second (Quest uses deltaTime)
    constexpr float RETURN_SPIN_SCALE = 20.0f;     // lerp speed multiplier (matches Quest code)
    constexpr float SIMPLIFIED_RETURN_SPIN_SCALE = 30.0f;  // min completion speed degrees/sec
    
    // Performance
    constexpr int CACHE_VALIDATION_INTERVAL_SEC = 20;  // Increased base interval
    constexpr int PERFORMANCE_UPDATE_INTERVAL_FRAMES = 30;
    constexpr int TRICK_UPDATE_IDLE_INTERVAL = 5;
    constexpr int TRICK_UPDATE_DEEP_IDLE_INTERVAL = 15;
    constexpr int VELOCITY_UPDATE_IDLE_SKIP = 3;
    constexpr float IDLE_THRESHOLD_SEC = 2.0f;
    constexpr float PERFORMANCE_REPORT_INTERVAL_SEC = 10.0f;
    constexpr float TARGET_FRAMERATE = 90.0f;
    constexpr float FRAME_TIME_MS = 1000.0f / TARGET_FRAMERATE;
    constexpr float MAX_LOAD_FACTOR = 4.0f;  // Cap for adaptive scaling
    
    // Time Scales
    constexpr float MIN_TIME_SCALE = 0.1f;
    constexpr float NORMAL_TIME_SCALE = 1.0f;
    constexpr float MIN_DELTA_TIME = 0.00001f;
    constexpr float FALLBACK_DELTA_TIME = 1.0f / TARGET_FRAMERATE;
    
    // Velocity Buffer
    constexpr int DEFAULT_VELOCITY_BUFFER_SIZE = 5;
    constexpr int MAX_VELOCITY_BUFFER_SIZE = 20;
    
    // Configuration Limits
    constexpr float MIN_THRESHOLD = 0.1f;
    constexpr float MAX_THRESHOLD = 1.0f;
    constexpr float MIN_DEADZONE = 0.0f;
    constexpr float MAX_DEADZONE = 0.9f;
    constexpr float MIN_SPEED = 0.1f;
    constexpr float MAX_SPEED = 5.0f;
    constexpr float MIN_VELOCITY = 0.1f;
    constexpr float MAX_VELOCITY = 20.0f;
    constexpr float MIN_SLOWMO = 0.1f;
    constexpr float MAX_SLOWMO = 0.8f;
    
    // Math Constants
    constexpr float PI = 3.14159265358979323846f;
    constexpr float DEGREES_TO_RADIANS = PI / 180.0f;
    constexpr float RADIANS_TO_DEGREES = 180.0f / PI;
    constexpr float RAD_TO_DEG = RADIANS_TO_DEGREES;
    constexpr float DEG_TO_RAD = DEGREES_TO_RADIANS;
    
    // Validation
    constexpr float MIN_FINITE_VALUE = 0.1f;
    constexpr uintptr_t MIN_VALID_POINTER = 0x1000;
}