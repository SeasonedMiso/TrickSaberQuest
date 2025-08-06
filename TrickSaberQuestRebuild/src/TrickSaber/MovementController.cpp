#include "TrickSaber/MovementController.hpp"
#include "TrickSaber/Configuration.hpp"
#include "TrickSaber/Utils/MemoryManager.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Mathf.hpp"
#include "UnityEngine/Time.hpp"
#include "main.hpp"
#include <cmath>

using namespace TrickSaber;

// Static variable definitions with velocity buffering
std::vector<UnityEngine::Vector3> MovementController::leftVelocityBuffer;
std::vector<UnityEngine::Vector3> MovementController::rightVelocityBuffer;
std::vector<UnityEngine::Vector3> MovementController::leftAngularVelocityBuffer;
std::vector<UnityEngine::Vector3> MovementController::rightAngularVelocityBuffer;

UnityEngine::Vector3 MovementController::leftControllerVelocity = UnityEngine::Vector3::get_zero();
UnityEngine::Vector3 MovementController::rightControllerVelocity = UnityEngine::Vector3::get_zero();
UnityEngine::Vector3 MovementController::leftAngularVelocity = UnityEngine::Vector3::get_zero();
UnityEngine::Vector3 MovementController::rightAngularVelocity = UnityEngine::Vector3::get_zero();

UnityEngine::Vector3 MovementController::prevLeftHandPos = UnityEngine::Vector3::get_zero();
UnityEngine::Vector3 MovementController::prevRightHandPos = UnityEngine::Vector3::get_zero();
UnityEngine::Quaternion MovementController::prevLeftHandRot = UnityEngine::Quaternion::get_identity();
UnityEngine::Quaternion MovementController::prevRightHandRot = UnityEngine::Quaternion::get_identity();

int MovementController::leftBufferIndex = 0;
int MovementController::rightBufferIndex = 0;
bool MovementController::initialized = false;

void MovementController::Initialize() {
    if (initialized) return;
    
    int bufferSize = Configuration::config.velocityBufferSize;
    
    leftVelocityBuffer.resize(bufferSize, UnityEngine::Vector3::get_zero());
    rightVelocityBuffer.resize(bufferSize, UnityEngine::Vector3::get_zero());
    leftAngularVelocityBuffer.resize(bufferSize, UnityEngine::Vector3::get_zero());
    rightAngularVelocityBuffer.resize(bufferSize, UnityEngine::Vector3::get_zero());
    
    initialized = true;
    Logger.debug("MovementController initialized with buffer size: {}", bufferSize);
}

void MovementController::UpdateVelocities(UnityEngine::Transform* leftHand, UnityEngine::Transform* rightHand, float deltaTime) {
    if (!initialized) Initialize();
    
    if (deltaTime <= 0.0001f) deltaTime = 1.0f / 90.0f; // Fallback for invalid deltaTime
    
    if (leftHand) {
        // Calculate linear velocity
        auto currentPos = Utils::MemoryManager::GetCachedPosition(leftHand);
        auto velocity = UnityEngine::Vector3::op_Division(
            UnityEngine::Vector3::op_Subtraction(currentPos, prevLeftHandPos), deltaTime);
        
        // Calculate angular velocity
        auto currentRot = Utils::MemoryManager::GetCachedRotation(leftHand);
        auto angularVel = CalculateAngularVelocity(prevLeftHandRot, currentRot, deltaTime);
        
        // Add to circular buffers
        AddVelocityProbe(velocity, angularVel, true);
        
        // Update cached values
        leftControllerVelocity = GetAverageVelocity(true);
        leftAngularVelocity = GetAverageAngularVelocity(true);
        
        prevLeftHandPos = currentPos;
        prevLeftHandRot = currentRot;
    }
    
    if (rightHand) {
        // Calculate linear velocity
        auto currentPos = Utils::MemoryManager::GetCachedPosition(rightHand);
        auto velocity = UnityEngine::Vector3::op_Division(
            UnityEngine::Vector3::op_Subtraction(currentPos, prevRightHandPos), deltaTime);
        
        // Calculate angular velocity
        auto currentRot = Utils::MemoryManager::GetCachedRotation(rightHand);
        auto angularVel = CalculateAngularVelocity(prevRightHandRot, currentRot, deltaTime);
        
        // Add to circular buffers
        AddVelocityProbe(velocity, angularVel, false);
        
        // Update cached values
        rightControllerVelocity = GetAverageVelocity(false);
        rightAngularVelocity = GetAverageAngularVelocity(false);
        
        prevRightHandPos = currentPos;
        prevRightHandRot = currentRot;
    }
}

UnityEngine::Vector3 MovementController::CalculateAngularVelocity(UnityEngine::Quaternion prevRot, UnityEngine::Quaternion currentRot, float deltaTime) {
    auto q = UnityEngine::Quaternion::op_Multiply(currentRot, UnityEngine::Quaternion::Inverse(prevRot));
    
    if (std::abs(q.w) > 1023.5f / 1024.0f) {
        return UnityEngine::Vector3::get_zero();
    }
    
    float gain;
    if (q.w < 0.0f) {
        float angle = std::acos(-q.w);
        gain = -2.0f * angle / (std::sin(angle) * deltaTime);
    } else {
        float angle = std::acos(q.w);
        gain = 2.0f * angle / (std::sin(angle) * deltaTime);
    }
    
    return UnityEngine::Vector3(q.x * gain, q.y * gain, q.z * gain);
}

void MovementController::AddVelocityProbe(UnityEngine::Vector3 velocity, UnityEngine::Vector3 angularVelocity, bool isLeft) {
    if (isLeft) {
        if (leftBufferIndex >= leftVelocityBuffer.size()) leftBufferIndex = 0;
        leftVelocityBuffer[leftBufferIndex] = velocity;
        leftAngularVelocityBuffer[leftBufferIndex] = angularVelocity;
        leftBufferIndex++;
    } else {
        if (rightBufferIndex >= rightVelocityBuffer.size()) rightBufferIndex = 0;
        rightVelocityBuffer[rightBufferIndex] = velocity;
        rightAngularVelocityBuffer[rightBufferIndex] = angularVelocity;
        rightBufferIndex++;
    }
}

UnityEngine::Vector3 MovementController::GetAverageVelocity(bool isLeft) {
    auto& buffer = isLeft ? leftVelocityBuffer : rightVelocityBuffer;
    
    UnityEngine::Vector3 avg = UnityEngine::Vector3::get_zero();
    for (const auto& vel : buffer) {
        avg = UnityEngine::Vector3::op_Addition(avg, vel);
    }
    
    return UnityEngine::Vector3::op_Division(avg, static_cast<float>(buffer.size()));
}

UnityEngine::Vector3 MovementController::GetAverageAngularVelocity(bool isLeft) {
    auto& buffer = isLeft ? leftAngularVelocityBuffer : rightAngularVelocityBuffer;
    
    UnityEngine::Vector3 avg = UnityEngine::Vector3::get_zero();
    for (const auto& vel : buffer) {
        avg = UnityEngine::Vector3::op_Addition(avg, vel);
    }
    
    return UnityEngine::Vector3::op_Division(avg, static_cast<float>(buffer.size()));
}

// Legacy compatibility methods
UnityEngine::Vector3 MovementController::GetLeftVelocity() {
    return leftControllerVelocity;
}

UnityEngine::Vector3 MovementController::GetRightVelocity() {
    return rightControllerVelocity;
}

UnityEngine::Vector3 MovementController::GetLeftAngularVelocity() {
    return leftAngularVelocity;
}

UnityEngine::Vector3 MovementController::GetRightAngularVelocity() {
    return rightAngularVelocity;
}

// Debug helper methods
UnityEngine::Vector3 MovementController::GetVelocity(bool isRight) {
    return isRight ? rightControllerVelocity : leftControllerVelocity;
}

float MovementController::GetAngularVelocity(bool isRight) {
    auto angVel = isRight ? rightAngularVelocity : leftAngularVelocity;
    return angVel.get_magnitude() * 57.2958f; // Rad to Deg conversion
}

void MovementController::ClearBuffers() {
    // Clear all velocity buffers
    leftVelocityBuffer.clear();
    rightVelocityBuffer.clear();
    leftAngularVelocityBuffer.clear();
    rightAngularVelocityBuffer.clear();
    
    // Reset current velocity values
    leftControllerVelocity = UnityEngine::Vector3::get_zero();
    rightControllerVelocity = UnityEngine::Vector3::get_zero();
    leftAngularVelocity = UnityEngine::Vector3::get_zero();
    rightAngularVelocity = UnityEngine::Vector3::get_zero();
    
    // Reset previous transform data
    prevLeftHandPos = UnityEngine::Vector3::get_zero();
    prevRightHandPos = UnityEngine::Vector3::get_zero();
    prevLeftHandRot = UnityEngine::Quaternion::get_identity();
    prevRightHandRot = UnityEngine::Quaternion::get_identity();
    
    // Reset buffer indices and initialization flag
    leftBufferIndex = 0;
    rightBufferIndex = 0;
    initialized = false;
    
    Logger.debug("MovementController buffers cleared");
}