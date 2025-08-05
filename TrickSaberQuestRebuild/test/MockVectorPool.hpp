#pragma once

#include <queue>
#include <mutex>

namespace TrickSaber::Utils {

// Mock Vector3 for testing
struct MockVector3 {
    float x, y, z;
    MockVector3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    static MockVector3 get_zero() { return MockVector3(0, 0, 0); }
};

// Mock Quaternion for testing
struct MockQuaternion {
    float x, y, z, w;
    MockQuaternion(float x = 0, float y = 0, float z = 0, float w = 1) : x(x), y(y), z(z), w(w) {}
    static MockQuaternion get_identity() { return MockQuaternion(0, 0, 0, 1); }
};

class MockVectorPool {
private:
    static std::queue<MockVector3> vector3Pool;
    static std::queue<MockQuaternion> quaternionPool;
    static std::mutex vector3Mutex;
    static std::mutex quaternionMutex;
    static constexpr size_t MAX_POOL_SIZE = 50;

public:
    static MockVector3 GetVector3() {
        std::lock_guard<std::mutex> lock(vector3Mutex);
        
        if (vector3Pool.empty()) {
            return MockVector3::get_zero();
        }
        
        auto vec = vector3Pool.front();
        vector3Pool.pop();
        return vec;
    }

    static void ReturnVector3(const MockVector3& vec) {
        std::lock_guard<std::mutex> lock(vector3Mutex);
        
        if (vector3Pool.size() < MAX_POOL_SIZE) {
            vector3Pool.push(vec);
        }
    }

    static MockQuaternion GetQuaternion() {
        std::lock_guard<std::mutex> lock(quaternionMutex);
        
        if (quaternionPool.empty()) {
            return MockQuaternion::get_identity();
        }
        
        auto quat = quaternionPool.front();
        quaternionPool.pop();
        return quat;
    }

    static void ReturnQuaternion(const MockQuaternion& quat) {
        std::lock_guard<std::mutex> lock(quaternionMutex);
        
        if (quaternionPool.size() < MAX_POOL_SIZE) {
            quaternionPool.push(quat);
        }
    }

    static void WarmUp() {
        // Pre-populate pools
        for (int i = 0; i < 20; ++i) {
            ReturnVector3(MockVector3::get_zero());
            ReturnQuaternion(MockQuaternion::get_identity());
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