#pragma once

#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/HapticFeedbackManager.hpp"
#include "GlobalNamespace/SaberManager.hpp"
#include "GlobalNamespace/GameScenesManager.hpp"
#include "GlobalNamespace/PauseController.hpp"
#include "UnityEngine/Object.hpp"
#include "TrickSaber/Utils/LazyInitializer.hpp"
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <typeindex>

namespace TrickSaber::Utils {
    class ObjectCache {
    private:
        struct CacheEntry {
            UnityEngine::Object* object;
            std::chrono::steady_clock::time_point timestamp;
            
            CacheEntry(UnityEngine::Object* obj) 
                : object(obj), timestamp(std::chrono::steady_clock::now()) {}
        };
        
        static LazyInitializer<std::unordered_map<std::type_index, CacheEntry>> lazyCache;
        static std::mutex cacheMutex;
        static constexpr float CACHE_TIMEOUT_SECONDS = 15.0f;
        static constexpr size_t MAX_CACHE_SIZE = 16;
        
        static bool IsObjectValid(UnityEngine::Object* obj);
        static void CleanupExpiredEntries();
        static void InitializeCache();
        
    public:
        template<typename T>
        static T* GetCachedObject() {
            std::lock_guard<std::mutex> lock(cacheMutex);
            
            auto& cache = lazyCache.GetMutable();
            auto typeIndex = std::type_index(typeid(T));
            auto it = cache.find(typeIndex);
            
            // Check if cached object exists and is still valid
            if (it != cache.end()) {
                auto now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.timestamp).count();
                
                if (elapsed < CACHE_TIMEOUT_SECONDS && IsObjectValid(it->second.object)) {
                    return reinterpret_cast<T*>(it->second.object);
                } else {
                    cache.erase(it);
                }
            }
            
            // Find new object and cache it
            auto* obj = UnityEngine::Object::FindObjectOfType<T*>();
            if (obj && cache.size() < MAX_CACHE_SIZE) {
                cache.emplace(typeIndex, CacheEntry(reinterpret_cast<UnityEngine::Object*>(obj)));
            }
            
            return obj;
        }
        
        // Lazy getters for commonly used objects
        static GlobalNamespace::AudioTimeSyncController* GetAudioController();
        static GlobalNamespace::HapticFeedbackManager* GetHapticController();
        static GlobalNamespace::SaberManager* GetSaberManager();
        static GlobalNamespace::GameScenesManager* GetGameScenesManager();
        static GlobalNamespace::PauseController* GetPauseController();
        
        static void ValidateCache();
        static void ClearCache();
        static size_t GetCacheSize();
        static bool IsCacheInitialized();
    };
    
    // Lazy cache warming utility
    class LazyCacheWarmer {
    private:
        static LazyComponentInitializer warmer;
        
    public:
        static void WarmCache();
        static bool IsWarmed();
    };
}