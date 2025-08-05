#pragma once

#include "UnityEngine/Object.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

namespace TrickSaber {
    template<typename T>
    class SafePtrUnity {
    private:
        T* _ptr = nullptr;
        
        bool IsUnityObjectValid() const {
            if (!_ptr) return false;
            
            // Check if the Unity object is still valid (not destroyed)
            auto* unityObj = reinterpret_cast<UnityEngine::Object*>(_ptr);
            return unityObj && !UnityEngine::Object::op_Equality(unityObj, nullptr);
        }
        
    public:
        SafePtrUnity() = default;
        
        SafePtrUnity(T* ptr) : _ptr(ptr) {}
        
        SafePtrUnity(const SafePtrUnity& other) : _ptr(other._ptr) {}
        
        SafePtrUnity& operator=(T* ptr) {
            _ptr = ptr;
            return *this;
        }
        
        SafePtrUnity& operator=(const SafePtrUnity& other) {
            _ptr = other._ptr;
            return *this;
        }
        
        T* ptr() const {
            return IsValid() ? _ptr : nullptr;
        }
        
        T* operator->() const {
            return ptr();
        }
        
        T& operator*() const {
            return *ptr();
        }
        
        operator bool() const {
            return IsValid();
        }
        
        bool operator==(const SafePtrUnity& other) const {
            return _ptr == other._ptr;
        }
        
        bool operator!=(const SafePtrUnity& other) const {
            return _ptr != other._ptr;
        }
        
        bool operator==(T* ptr) const {
            return _ptr == ptr;
        }
        
        bool operator!=(T* ptr) const {
            return _ptr != ptr;
        }
        
        bool IsValid() const {
            return _ptr != nullptr && IsUnityObjectValid();
        }
        
        void Reset() {
            _ptr = nullptr;
        }
        
        T* UnsafePtr() const {
            return _ptr;
        }
    };
    
    // Helper function to create SafePtrUnity
    template<typename T>
    SafePtrUnity<T> MakeSafe(T* ptr) {
        return SafePtrUnity<T>(ptr);
    }
}