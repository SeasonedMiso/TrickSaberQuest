#pragma once

#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include <unordered_map>
#include <mutex>

namespace TrickSaber::Utils {

struct CachedTransform {
    UnityEngine::Vector3 position;
    UnityEngine::Quaternion rotation;
    UnityEngine::Vector3 localPosition;
    UnityEngine::Quaternion localRotation;
    float lastUpdateTime;
    
    CachedTransform() : 
        position(UnityEngine::Vector3::get_zero()),
        rotation(UnityEngine::Quaternion::get_identity()),
        localPosition(UnityEngine::Vector3::get_zero()),
        localRotation(UnityEngine::Quaternion::get_identity()),
        lastUpdateTime(0.0f) {}
};

class TransformCache {
private:
    static std::unordered_map<UnityEngine::Transform*, CachedTransform> cache;
    static std::mutex cacheMutex;
    static constexpr float CACHE_TIMEOUT = 0.016f; // ~60fps

public:
    static UnityEngine::Vector3 GetPosition(UnityEngine::Transform* transform);
    static UnityEngine::Quaternion GetRotation(UnityEngine::Transform* transform);
    static UnityEngine::Vector3 GetLocalPosition(UnityEngine::Transform* transform);
    static UnityEngine::Quaternion GetLocalRotation(UnityEngine::Transform* transform);
    
    static void InvalidateCache(UnityEngine::Transform* transform);
    static void ClearCache();
    static size_t GetCacheSize();

private:
    static bool IsCacheValid(const CachedTransform& cached, float currentTime);
    static void UpdateCache(UnityEngine::Transform* transform, CachedTransform& cached, float currentTime);
};

} // namespace TrickSaber::Utils