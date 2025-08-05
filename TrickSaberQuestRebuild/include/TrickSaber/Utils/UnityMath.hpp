#pragma once

#include "UnityEngine/Mathf.hpp"

namespace TrickSaber::Utils {
    class UnityMath {
    public:
        // Unity math constants
        static constexpr float Deg2Rad = 0.017453292519943295f;
        static constexpr float Rad2Deg = 57.29577951308232f;
        
        // SmoothStep implementation (missing from Unity Mathf)
        static float SmoothStep(float from, float to, float t) {
            t = UnityEngine::Mathf::Clamp01(t);
            t = t * t * (3.0f - 2.0f * t);
            return UnityEngine::Mathf::Lerp(from, to, t);
        }
        
        // Additional utility functions
        static float SmoothDamp(float current, float target, float& currentVelocity, float smoothTime, float deltaTime) {
            float omega = 2.0f / smoothTime;
            float x = omega * deltaTime;
            float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
            float change = current - target;
            float originalTo = target;
            
            float maxChange = 1000.0f * smoothTime;
            change = UnityEngine::Mathf::Clamp(change, -maxChange, maxChange);
            target = current - change;
            
            float temp = (currentVelocity + omega * change) * deltaTime;
            currentVelocity = (currentVelocity - omega * temp) * exp;
            float output = target + (change + temp) * exp;
            
            if (originalTo - current > 0.0f == output > originalTo) {
                output = originalTo;
                currentVelocity = (output - originalTo) / deltaTime;
            }
            
            return output;
        }
    };
}