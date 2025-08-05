#pragma once

#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Transform.hpp"
#include <vector>

namespace TrickSaber {
    class MovementController {
    private:
        // Velocity buffers for smoothing (PC parity)
        static std::vector<UnityEngine::Vector3> leftVelocityBuffer;
        static std::vector<UnityEngine::Vector3> rightVelocityBuffer;
        static std::vector<UnityEngine::Vector3> leftAngularVelocityBuffer;
        static std::vector<UnityEngine::Vector3> rightAngularVelocityBuffer;
        
        // Buffer indices
        static int leftBufferIndex;
        static int rightBufferIndex;
        static bool initialized;
        
        // Previous transform data
        static UnityEngine::Vector3 prevLeftHandPos;
        static UnityEngine::Vector3 prevRightHandPos;
        static UnityEngine::Quaternion prevLeftHandRot;
        static UnityEngine::Quaternion prevRightHandRot;
        
        // Helper methods
        static UnityEngine::Vector3 CalculateAngularVelocity(UnityEngine::Quaternion prevRot, UnityEngine::Quaternion currentRot, float deltaTime);
        static void AddVelocityProbe(UnityEngine::Vector3 velocity, UnityEngine::Vector3 angularVelocity, bool isLeft);
        
    public:
        // Current velocity values
        static UnityEngine::Vector3 leftControllerVelocity;
        static UnityEngine::Vector3 rightControllerVelocity;
        static UnityEngine::Vector3 leftAngularVelocity;
        static UnityEngine::Vector3 rightAngularVelocity;
        
        // Initialization
        static void Initialize();
        
        // Main update method
        static void UpdateVelocities(UnityEngine::Transform* leftHand, UnityEngine::Transform* rightHand, float deltaTime);
        
        // PC-compatible methods with velocity averaging
        static UnityEngine::Vector3 GetAverageVelocity(bool isLeft);
        static UnityEngine::Vector3 GetAverageAngularVelocity(bool isLeft);
        
        // Legacy compatibility methods
        static UnityEngine::Vector3 GetLeftVelocity();
        static UnityEngine::Vector3 GetRightVelocity();
        static UnityEngine::Vector3 GetLeftAngularVelocity();
        static UnityEngine::Vector3 GetRightAngularVelocity();
        
        // Debug helper methods
        static UnityEngine::Vector3 GetVelocity(bool isRight);
        static float GetAngularVelocity(bool isRight);
        
        // Cleanup method for static variables
        static void ClearBuffers();
    };
}