#pragma once

#ifdef HOST_TESTS
#include "HostMocks.hpp"
#else
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#endif

#include <chrono>
#include <cmath>

// Simple 2D vector for thumbstick input
struct Vector2 {
    float x, y;
    Vector2(float x = 0, float y = 0) : x(x), y(y) {}
};

// Mock Quest 3 TouchPlus Controller
class MockQuest3Controller {
public:
    struct ControllerState {
        // Pose data
        UnityEngine::Vector3 position{0, 0, 0};
        UnityEngine::Quaternion rotation = UnityEngine::Quaternion::get_identity();
        UnityEngine::Vector3 velocity{0, 0, 0};
        UnityEngine::Vector3 angularVelocity{0, 0, 0};
        
        // Analog inputs (0-1)
        float triggerValue = 0.0f;
        float gripValue = 0.0f;
        Vector2 thumbstick{0, 0};
        
        // Digital inputs
        bool triggerPressed = false;
        bool triggerTouched = false;
        bool gripPressed = false;
        bool primaryButton = false;  // A/X
        bool secondaryButton = false; // B/Y
        bool thumbstickClick = false;
        
        // Touch sensors
        bool primaryTouch = false;
        bool secondaryTouch = false;
        bool thumbstickTouch = false;
        bool thumbRest = false;
        
        // Haptics
        float hapticIntensity = 0.0f;
        float hapticDuration = 0.0f;
    };
    
private:
    ControllerState state;
    bool isRightHand;
    std::chrono::steady_clock::time_point lastUpdate;
    
    // Physics constraints
    static constexpr float MAX_VELOCITY = 10.0f;
    static constexpr float MAX_ACCELERATION = 50.0f;
    static constexpr float TRIGGER_THRESHOLD = 0.1f;
    static constexpr float GRIP_THRESHOLD = 0.1f;
    
public:
    MockQuest3Controller(bool rightHand = true);
    
    // Simulate realistic controller movement
    void SimulateMovement(const UnityEngine::Vector3& targetPos, float deltaTime);
    
    // Simulate trigger input with two-stage detection
    void SetTrigger(float value);
    
    // Simulate grip input
    void SetGrip(float value);
    
    // Simulate thumbstick with deadzone
    void SetThumbstick(float x, float y);
    
    // Simulate face buttons (A/B for right, X/Y for left)
    void SetPrimaryButton(bool pressed, bool touched = false);
    void SetSecondaryButton(bool pressed, bool touched = false);
    
    // Simulate haptic feedback
    void TriggerHaptic(float intensity, float duration);
    
    // Getters
    const ControllerState& GetState() const { return state; }
    bool IsRightHand() const { return isRightHand; }
    
    // Simulate realistic throw motion
    void SimulateThrowMotion(float progress);
};