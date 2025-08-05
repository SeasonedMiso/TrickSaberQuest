#pragma once

#include <queue>
#include <memory>
#include <mutex>
#include <functional>

namespace TrickSaber::Utils {

template<typename T>
class ObjectPool {
private:
    std::queue<std::unique_ptr<T>> available;
    mutable std::mutex poolMutex;
    std::function<std::unique_ptr<T>()> factory;
    std::function<void(T*)> reset;
    size_t maxSize;

public:
    ObjectPool(std::function<std::unique_ptr<T>()> factory, 
               std::function<void(T*)> reset = nullptr,
               size_t maxSize = 100) 
        : factory(factory), reset(reset), maxSize(maxSize) {}

    std::unique_ptr<T> Acquire() {
        std::lock_guard<std::mutex> lock(poolMutex);
        
        if (available.empty()) {
            return factory();
        }
        
        auto obj = std::move(available.front());
        available.pop();
        return obj;
    }

    void Release(std::unique_ptr<T> obj) {
        if (!obj) return;
        
        std::lock_guard<std::mutex> lock(poolMutex);
        
        if (available.size() >= maxSize) {
            return; // Pool full, let object be destroyed
        }
        
        if (reset) {
            reset(obj.get());
        }
        
        available.push(std::move(obj));
    }

    size_t Size() const {
        std::lock_guard<std::mutex> lock(poolMutex);
        return available.size();
    }

    void Clear() {
        std::lock_guard<std::mutex> lock(poolMutex);
        while (!available.empty()) {
            available.pop();
        }
    }
};

} // namespace TrickSaber::Utils