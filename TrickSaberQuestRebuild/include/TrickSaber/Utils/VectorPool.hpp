#pragma once

#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include <queue>
#include <mutex>

namespace TrickSaber::Utils {

class VectorPool {
private:
    static std::queue<UnityEngine::Vector3> vector3Pool;
    static std::queue<UnityEngine::Quaternion> quaternionPool;
    static std::mutex vector3Mutex;
    static std::mutex quaternionMutex;
    static constexpr size_t MAX_POOL_SIZE = 50;

public:
    static UnityEngine::Vector3 GetVector3() {
        std::lock_guard<std::mutex> lock(vector3Mutex);
        
        if (vector3Pool.empty()) {
            return UnityEngine::Vector3::get_zero();
        }
        
        auto vec = vector3Pool.front();
        vector3Pool.pop();
        return vec;
    }

    static void ReturnVector3(const UnityEngine::Vector3& vec) {
        std::lock_guard<std::mutex> lock(vector3Mutex);
        
        if (vector3Pool.size() < MAX_POOL_SIZE) {
            vector3Pool.push(vec);
        }
    }

    static UnityEngine::Quaternion GetQuaternion() {
        std::lock_guard<std::mutex> lock(quaternionMutex);
        
        if (quaternionPool.empty()) {
            return UnityEngine::Quaternion::get_identity();
        }
        
        auto quat = quaternionPool.front();
        quaternionPool.pop();
        return quat;
    }

    static void ReturnQuaternion(const UnityEngine::Quaternion& quat) {
        std::lock_guard<std::mutex> lock(quaternionMutex);
        
        if (quaternionPool.size() < MAX_POOL_SIZE) {
            quaternionPool.push(quat);
        }
    }

    static void WarmUp() {
        // Pre-populate pools
        for (int i = 0; i < 20; ++i) {
            ReturnVector3(UnityEngine::Vector3::get_zero());
            ReturnQuaternion(UnityEngine::Quaternion::get_identity());
        }
    }

    static void Clear() {
        std::lock_guard<std::mutex> lock1(vector3Mutex);
        std::lock_guard<std::mutex> lock2(quaternionMutex);
        
        while (!vector3Pool.empty()) vector3Pool.pop();
        while (!quaternionPool.empty()) quaternionPool.pop();
    }
};

} // namespace TrickSaber::Utils