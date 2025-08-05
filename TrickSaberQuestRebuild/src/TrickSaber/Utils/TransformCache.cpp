#include "TrickSaber/Utils/TransformCache.hpp"
#include "UnityEngine/Time.hpp"

namespace TrickSaber::Utils {

// Static member definitions
std::unordered_map<UnityEngine::Transform*, CachedTransform> TransformCache::cache;
std::mutex TransformCache::cacheMutex;

UnityEngine::Vector3 TransformCache::GetPosition(UnityEngine::Transform* transform) {
    if (!transform) return UnityEngine::Vector3::get_zero();
    
    std::lock_guard<std::mutex> lock(cacheMutex);
    
    auto it = cache.find(transform);
    float currentTime = UnityEngine::Time::get_time();
    
    if (it == cache.end() || !IsCacheValid(it->second, currentTime)) {
        CachedTransform& cached = cache[transform];
        UpdateCache(transform, cached, currentTime);
        return cached.position;
    }
    
    return it->second.position;
}

UnityEngine::Quaternion TransformCache::GetRotation(UnityEngine::Transform* transform) {
    if (!transform) return UnityEngine::Quaternion::get_identity();
    
    std::lock_guard<std::mutex> lock(cacheMutex);
    
    auto it = cache.find(transform);
    float currentTime = UnityEngine::Time::get_time();
    
    if (it == cache.end() || !IsCacheValid(it->second, currentTime)) {
        CachedTransform& cached = cache[transform];
        UpdateCache(transform, cached, currentTime);
        return cached.rotation;
    }
    
    return it->second.rotation;
}

UnityEngine::Vector3 TransformCache::GetLocalPosition(UnityEngine::Transform* transform) {
    if (!transform) return UnityEngine::Vector3::get_zero();
    
    std::lock_guard<std::mutex> lock(cacheMutex);
    
    auto it = cache.find(transform);
    float currentTime = UnityEngine::Time::get_time();
    
    if (it == cache.end() || !IsCacheValid(it->second, currentTime)) {
        CachedTransform& cached = cache[transform];
        UpdateCache(transform, cached, currentTime);
        return cached.localPosition;
    }
    
    return it->second.localPosition;
}

UnityEngine::Quaternion TransformCache::GetLocalRotation(UnityEngine::Transform* transform) {
    if (!transform) return UnityEngine::Quaternion::get_identity();
    
    std::lock_guard<std::mutex> lock(cacheMutex);
    
    auto it = cache.find(transform);
    float currentTime = UnityEngine::Time::get_time();
    
    if (it == cache.end() || !IsCacheValid(it->second, currentTime)) {
        CachedTransform& cached = cache[transform];
        UpdateCache(transform, cached, currentTime);
        return cached.localRotation;
    }
    
    return it->second.localRotation;
}

void TransformCache::InvalidateCache(UnityEngine::Transform* transform) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    cache.erase(transform);
}

void TransformCache::ClearCache() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    cache.clear();
}

size_t TransformCache::GetCacheSize() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    return cache.size();
}

bool TransformCache::IsCacheValid(const CachedTransform& cached, float currentTime) {
    return (currentTime - cached.lastUpdateTime) < CACHE_TIMEOUT;
}

void TransformCache::UpdateCache(UnityEngine::Transform* transform, CachedTransform& cached, float currentTime) {
    cached.position = transform->get_position();
    cached.rotation = transform->get_rotation();
    cached.localPosition = transform->get_localPosition();
    cached.localRotation = transform->get_localRotation();
    cached.lastUpdateTime = currentTime;
}

} // namespace TrickSaber::Utils