#pragma once

#include <functional>
#include <atomic>
#include <mutex>
#include <memory>

namespace TrickSaber::Utils {
    
    template<typename T>
    class LazyInitializer {
    private:
        mutable std::atomic<bool> initialized{false};
        mutable std::mutex initMutex;
        mutable std::unique_ptr<T> instance;
        std::function<std::unique_ptr<T>()> factory;
        
    public:
        explicit LazyInitializer(std::function<std::unique_ptr<T>()> factoryFunc)
            : factory(std::move(factoryFunc)) {}
        
        const T& Get() const {
            if (!initialized.load(std::memory_order_acquire)) {
                std::lock_guard<std::mutex> lock(initMutex);
                if (!initialized.load(std::memory_order_relaxed)) {
                    instance = factory();
                    initialized.store(true, std::memory_order_release);
                }
            }
            return *instance;
        }
        
        T* GetPtr() {
            return &GetMutable();
        }
        
        const T* GetPtr() const {
            return &Get();
        }
        
        T& GetMutable() {
            if (!initialized.load(std::memory_order_acquire)) {
                std::lock_guard<std::mutex> lock(initMutex);
                if (!initialized.load(std::memory_order_relaxed)) {
                    instance = factory();
                    initialized.store(true, std::memory_order_release);
                }
            }
            return *instance;
        }
        
        bool IsInitialized() const {
            return initialized.load(std::memory_order_acquire);
        }
        
        void Reset() {
            std::lock_guard<std::mutex> lock(initMutex);
            instance.reset();
            initialized.store(false, std::memory_order_release);
        }
    };
    
    // Specialized lazy initializer for singletons
    template<typename T>
    class LazySingleton {
    private:
        static std::atomic<T*> instance;
        static std::mutex initMutex;
        static std::function<std::unique_ptr<T>()> factory;
        
    public:
        static void SetFactory(std::function<std::unique_ptr<T>()> factoryFunc) {
            factory = std::move(factoryFunc);
        }
        
        static T* GetInstance() {
            T* inst = instance.load(std::memory_order_acquire);
            if (!inst) {
                std::lock_guard<std::mutex> lock(initMutex);
                inst = instance.load(std::memory_order_relaxed);
                if (!inst) {
                    auto newInstance = factory();
                    inst = newInstance.release();
                    instance.store(inst, std::memory_order_release);
                }
            }
            return inst;
        }
        
        static bool IsInitialized() {
            return instance.load(std::memory_order_acquire) != nullptr;
        }
        
        static void Reset() {
            std::lock_guard<std::mutex> lock(initMutex);
            T* inst = instance.exchange(nullptr, std::memory_order_acq_rel);
            delete inst;
        }
    };
    
    template<typename T>
    std::atomic<T*> LazySingleton<T>::instance{nullptr};
    
    template<typename T>
    std::mutex LazySingleton<T>::initMutex;
    
    template<typename T>
    std::function<std::unique_ptr<T>()> LazySingleton<T>::factory;
    
    // Lazy component initializer for Unity components
    class LazyComponentInitializer {
    private:
        std::atomic<bool> initialized{false};
        std::mutex initMutex;
        std::function<void()> initFunc;
        
    public:
        explicit LazyComponentInitializer(std::function<void()> initFunction)
            : initFunc(std::move(initFunction)) {}
        
        void Initialize() {
            if (!initialized.load(std::memory_order_acquire)) {
                std::lock_guard<std::mutex> lock(initMutex);
                if (!initialized.load(std::memory_order_relaxed)) {
                    initFunc();
                    initialized.store(true, std::memory_order_release);
                }
            }
        }
        
        bool IsInitialized() const {
            return initialized.load(std::memory_order_acquire);
        }
        
        void Reset() {
            std::lock_guard<std::mutex> lock(initMutex);
            initialized.store(false, std::memory_order_release);
        }
    };
}