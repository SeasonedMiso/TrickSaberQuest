#include "TrickSaber/Utils/ObjectCache.hpp"
#include "main.hpp"

using namespace TrickSaber::Utils;

// Static member definitions
LazyInitializer<std::unordered_map<std::type_index, ObjectCache::CacheEntry>> ObjectCache::lazyCache(
    []() -> std::unique_ptr<std::unordered_map<std::type_index, ObjectCache::CacheEntry>> {
        PaperLogger.debug("Initializing ObjectCache on first access");
        return std::make_unique<std::unordered_map<std::type_index, ObjectCache::CacheEntry>>();
    }
);
std::mutex ObjectCache::cacheMutex;

bool ObjectCache::IsObjectValid(UnityEngine::Object* obj) {
    return obj && obj->m_CachedPtr.m_value;
}

void ObjectCache::InitializeCache() {
    // Force initialization of lazy cache
    lazyCache.Get();
    PaperLogger.debug("ObjectCache initialized");
}

void ObjectCache::CleanupExpiredEntries() {
    if (!lazyCache.IsInitialized()) return;
    
    auto& cache = lazyCache.GetMutable();
    auto now = std::chrono::steady_clock::now();
    size_t removedCount = 0;
    
    for (auto it = cache.begin(); it != cache.end();) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.timestamp).count();
        
        if (elapsed >= CACHE_TIMEOUT_SECONDS || !IsObjectValid(it->second.object)) {
            it = cache.erase(it);
            removedCount++;
        } else {
            ++it;
        }
    }
    
    if (removedCount > 0) {
        PaperLogger.debug("ObjectCache cleaned {} expired entries", removedCount);
    }
}

GlobalNamespace::AudioTimeSyncController* ObjectCache::GetAudioController() {
    return GetCachedObject<GlobalNamespace::AudioTimeSyncController>();
}

GlobalNamespace::HapticFeedbackManager* ObjectCache::GetHapticController() {
    return GetCachedObject<GlobalNamespace::HapticFeedbackManager>();
}

GlobalNamespace::SaberManager* ObjectCache::GetSaberManager() {
    return GetCachedObject<GlobalNamespace::SaberManager>();
}

GlobalNamespace::GameScenesManager* ObjectCache::GetGameScenesManager() {
    return GetCachedObject<GlobalNamespace::GameScenesManager>();
}

GlobalNamespace::PauseController* ObjectCache::GetPauseController() {
    return GetCachedObject<GlobalNamespace::PauseController>();
}

void ObjectCache::ValidateCache() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    CleanupExpiredEntries();
}

void ObjectCache::ClearCache() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    if (!lazyCache.IsInitialized()) return;
    
    auto& cache = lazyCache.GetMutable();
    size_t clearedCount = cache.size();
    cache.clear();
    PaperLogger.debug("ObjectCache cleared {} entries", clearedCount);
}

size_t ObjectCache::GetCacheSize() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    if (!lazyCache.IsInitialized()) return 0;
    
    return lazyCache.Get().size();
}

bool ObjectCache::IsCacheInitialized() {
    return lazyCache.IsInitialized();
}

// LazyCacheWarmer implementation
LazyComponentInitializer LazyCacheWarmer::warmer([]() {
    PaperLogger.debug("Warming ObjectCache with commonly used objects");
    
    // Warm cache with essential objects
    ObjectCache::GetAudioController();
    ObjectCache::GetHapticController();
    ObjectCache::GetSaberManager();
    ObjectCache::GetGameScenesManager();
    ObjectCache::GetPauseController();
    
    PaperLogger.debug("ObjectCache warming completed");
});

void LazyCacheWarmer::WarmCache() {
    warmer.Initialize();
}

bool LazyCacheWarmer::IsWarmed() {
    return warmer.IsInitialized();
}