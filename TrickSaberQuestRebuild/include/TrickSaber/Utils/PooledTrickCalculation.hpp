#pragma once

#include "MemoryManager.hpp"
#include <memory>

namespace TrickSaber::Utils {

// RAII wrapper for pooled trick calculations
class PooledTrickCalculation {
private:
    std::unique_ptr<TrickCalculation> calculation;

public:
    PooledTrickCalculation() : calculation(MemoryManager::GetTrickCalculation()) {}
    
    ~PooledTrickCalculation() {
        if (calculation) {
            MemoryManager::ReturnTrickCalculation(std::move(calculation));
        }
    }

    // Non-copyable but movable
    PooledTrickCalculation(const PooledTrickCalculation&) = delete;
    PooledTrickCalculation& operator=(const PooledTrickCalculation&) = delete;
    
    PooledTrickCalculation(PooledTrickCalculation&& other) noexcept 
        : calculation(std::move(other.calculation)) {}
    
    PooledTrickCalculation& operator=(PooledTrickCalculation&& other) noexcept {
        if (this != &other) {
            if (calculation) {
                MemoryManager::ReturnTrickCalculation(std::move(calculation));
            }
            calculation = std::move(other.calculation);
        }
        return *this;
    }

    TrickCalculation* operator->() { return calculation.get(); }
    const TrickCalculation* operator->() const { return calculation.get(); }
    
    TrickCalculation& operator*() { return *calculation; }
    const TrickCalculation& operator*() const { return *calculation; }
    
    bool IsValid() const { return calculation != nullptr; }
};

} // namespace TrickSaber::Utils