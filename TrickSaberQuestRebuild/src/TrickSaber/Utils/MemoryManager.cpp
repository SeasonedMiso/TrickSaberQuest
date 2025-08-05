#include "TrickSaber/Utils/MemoryManager.hpp"
#include "main.hpp"

namespace TrickSaber::Utils {

// Static member definitions
std::unique_ptr<ObjectPool<TrickCalculation>> MemoryManager::trickCalculationPool;
bool MemoryManager::initialized = false;

void MemoryManager::Initialize() {
    if (initialized) return;
    
    // Initialize trick calculation pool
    trickCalculationPool = std::make_unique<ObjectPool<TrickCalculation>>(
        []() { return std::make_unique<TrickCalculation>(); },
        [](TrickCalculation* calc) { calc->Reset(); },
        30 // Max 30 pooled calculations
    );
    
    // Warm up vector pools
    VectorPool::WarmUp();
    
    initialized = true;
    PaperLogger.info("MemoryManager initialized with object pools");
}

void MemoryManager::Shutdown() {
    if (!initialized) return;
    
    trickCalculationPool.reset();
    VectorPool::Clear();
    TransformCache::ClearCache();
    
    initialized = false;
    PaperLogger.info("MemoryManager shutdown complete");
}

std::unique_ptr<TrickCalculation> MemoryManager::GetTrickCalculation() {
    if (!initialized) Initialize();
    return trickCalculationPool->Acquire();
}

void MemoryManager::ReturnTrickCalculation(std::unique_ptr<TrickCalculation> calc) {
    if (!initialized || !calc) return;
    trickCalculationPool->Release(std::move(calc));
}

size_t MemoryManager::GetTotalPooledObjects() {
    if (!initialized) return 0;
    
    return trickCalculationPool->Size() + TransformCache::GetCacheSize();
}

void MemoryManager::ClearAllPools() {
    if (!initialized) return;
    
    trickCalculationPool->Clear();
    VectorPool::Clear();
    TransformCache::ClearCache();
    
    PaperLogger.debug("All memory pools cleared");
}

} // namespace TrickSaber::Utils