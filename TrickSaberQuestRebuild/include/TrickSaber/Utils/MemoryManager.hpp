#pragma once

#include "ObjectPool.hpp"
#include "VectorPool.hpp"
#include "TransformCache.hpp"
#include <memory>

namespace TrickSaber::Utils {

struct TrickCalculation {
    UnityEngine::Vector3 velocity;
    UnityEngine::Vector3 angularVelocity;
    UnityEngine::Vector3 targetPosition;
    UnityEngine::Quaternion targetRotation;
    float duration;
    bool isActive;
    
    void Reset() {
        velocity = UnityEngine::Vector3::get_zero();
        angularVelocity = UnityEngine::Vector3::get_zero();
        targetPosition = UnityEngine::Vector3::get_zero();
        targetRotation = UnityEngine::Quaternion::get_identity();
        duration = 0.0f;
        isActive = false;
    }
};

class MemoryManager {
private:
    static std::unique_ptr<ObjectPool<TrickCalculation>> trickCalculationPool;
    static bool initialized;

public:
    static void Initialize();
    static void Shutdown();
    
    // Trick calculation pooling
    static std::unique_ptr<TrickCalculation> GetTrickCalculation();
    static void ReturnTrickCalculation(std::unique_ptr<TrickCalculation> calc);
    
    // Vector pooling shortcuts
    static UnityEngine::Vector3 GetVector3() { return VectorPool::GetVector3(); }
    static void ReturnVector3(const UnityEngine::Vector3& vec) { VectorPool::ReturnVector3(vec); }
    
    // Transform caching shortcuts
    static UnityEngine::Vector3 GetCachedPosition(UnityEngine::Transform* transform) {
        return TransformCache::GetPosition(transform);
    }
    static UnityEngine::Quaternion GetCachedRotation(UnityEngine::Transform* transform) {
        return TransformCache::GetRotation(transform);
    }
    
    // Memory statistics
    static size_t GetTotalPooledObjects();
    static void ClearAllPools();
};

} // namespace TrickSaber::Utils