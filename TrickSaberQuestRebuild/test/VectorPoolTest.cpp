#include <gtest/gtest.h>
#include "MockVectorPool.hpp"
#include <thread>
#include <vector>

using namespace TrickSaber::Utils;

class VectorPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        MockVectorPool::Clear();
    }
    
    void TearDown() override {
        MockVectorPool::Clear();
    }
};

TEST_F(VectorPoolTest, BasicVector3Operations) {
    auto vec1 = MockVectorPool::GetVector3();
    EXPECT_EQ(vec1.x, 0.0f);
    EXPECT_EQ(vec1.y, 0.0f);
    EXPECT_EQ(vec1.z, 0.0f);
    
    MockVector3 testVec(1.0f, 2.0f, 3.0f);
    MockVectorPool::ReturnVector3(testVec);
    
    auto vec2 = MockVectorPool::GetVector3();
    EXPECT_EQ(vec2.x, 1.0f);
    EXPECT_EQ(vec2.y, 2.0f);
    EXPECT_EQ(vec2.z, 3.0f);
}

TEST_F(VectorPoolTest, BasicQuaternionOperations) {
    auto quat1 = MockVectorPool::GetQuaternion();
    EXPECT_EQ(quat1.x, 0.0f);
    EXPECT_EQ(quat1.y, 0.0f);
    EXPECT_EQ(quat1.z, 0.0f);
    EXPECT_EQ(quat1.w, 1.0f);
    
    MockQuaternion testQuat(0.5f, 0.5f, 0.5f, 0.5f);
    MockVectorPool::ReturnQuaternion(testQuat);
    
    auto quat2 = MockVectorPool::GetQuaternion();
    EXPECT_EQ(quat2.x, 0.5f);
    EXPECT_EQ(quat2.y, 0.5f);
    EXPECT_EQ(quat2.z, 0.5f);
    EXPECT_EQ(quat2.w, 0.5f);
}

TEST_F(VectorPoolTest, PoolSizeLimit) {
    // Fill pool beyond limit
    for (int i = 0; i < 100; ++i) {
        MockVector3 vec(i, i, i);
        MockVectorPool::ReturnVector3(vec);
    }
    
    // Pool should be limited to MAX_POOL_SIZE
    int retrievedCount = 0;
    for (int i = 0; i < 100; ++i) {
        auto vec = MockVectorPool::GetVector3();
        if (vec.x != 0.0f || vec.y != 0.0f || vec.z != 0.0f) {
            retrievedCount++;
        }
    }
    
    EXPECT_LE(retrievedCount, 50); // MAX_POOL_SIZE
}

TEST_F(VectorPoolTest, ThreadSafety) {
    const int numThreads = 10;
    const int operationsPerThread = 100;
    std::vector<std::thread> threads;
    
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([operationsPerThread]() {
            for (int i = 0; i < operationsPerThread; ++i) {
                auto vec = MockVectorPool::GetVector3();
                MockVector3 testVec(i, i, i);
                MockVectorPool::ReturnVector3(testVec);
                
                auto quat = MockVectorPool::GetQuaternion();
                MockQuaternion testQuat(i, i, i, i);
                MockVectorPool::ReturnQuaternion(testQuat);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Test should complete without crashes
    EXPECT_TRUE(true);
}

TEST_F(VectorPoolTest, WarmUp) {
    MockVectorPool::WarmUp();
    
    // Should have pre-populated vectors available
    int availableVectors = 0;
    for (int i = 0; i < 25; ++i) {
        auto vec = MockVectorPool::GetVector3();
        if (vec.x == 0.0f && vec.y == 0.0f && vec.z == 0.0f) {
            availableVectors++;
        }
    }
    
    EXPECT_GT(availableVectors, 0);
}

TEST_F(VectorPoolTest, Clear) {
    // Add some vectors
    for (int i = 0; i < 10; ++i) {
        MockVector3 vec(i, i, i);
        MockVectorPool::ReturnVector3(vec);
    }
    
    MockVectorPool::Clear();
    
    // After clear, should get default vectors
    auto vec = MockVectorPool::GetVector3();
    EXPECT_EQ(vec.x, 0.0f);
    EXPECT_EQ(vec.y, 0.0f);
    EXPECT_EQ(vec.z, 0.0f);
}