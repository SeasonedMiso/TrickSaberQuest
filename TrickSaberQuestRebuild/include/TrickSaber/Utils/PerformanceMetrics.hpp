#pragma once

#include "TrickSaber/Utils/LazyInitializer.hpp"
#include <chrono>
#include <unordered_map>
#include <string>
#include <memory>

namespace TrickSaber::Utils {
    
    struct FrameMetrics {
        float frameTime = 0.0f;
        float fps = 0.0f;
        int droppedFrames = 0;
        float cpuTime = 0.0f;
        float gpuTime = 0.0f;
    };

    struct MemoryMetrics {
        size_t totalMemory = 0;
        size_t usedMemory = 0;
        size_t peakMemory = 0;
        size_t allocations = 0;
        size_t deallocations = 0;
    };

    struct TrickMetrics {
        int throwsPerformed = 0;
        int spinsPerformed = 0;
        float avgThrowVelocity = 0.0f;
        float avgReturnTime = 0.0f;
        int failedTricks = 0;
        float trickDuration = 0.0f;
    };

    class PerformanceMetrics {
    private:
        static LazyInitializer<PerformanceMetrics> lazyInstance;
        
        std::chrono::high_resolution_clock::time_point lastFrameTime;
        std::chrono::high_resolution_clock::time_point startTime;
        
        FrameMetrics frameMetrics{};
        MemoryMetrics memoryMetrics{};
        TrickMetrics trickMetrics{};
        
        std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> timers;
        std::unordered_map<std::string, float> averageTimes;
        
        float frameTimeBuffer[60] = {0}; // 1 second at 60fps
        int frameBufferIndex = 0;
        bool bufferFull = false;
        
        bool enabled = true;
        float reportInterval = 5.0f; // Report every 5 seconds
        std::chrono::high_resolution_clock::time_point lastReport;

    public:
        static PerformanceMetrics* GetInstance();
        static bool IsInitialized();
        
        // Frame performance
        void UpdateFrameMetrics();
        void RecordFrameDrop();
        float GetAverageFPS() const;
        float GetFrameTime() const;
        
        // Memory tracking
        void UpdateMemoryMetrics();
        void RecordAllocation(size_t size);
        void RecordDeallocation(size_t size);
        size_t GetMemoryUsage() const;
        
        // Trick performance
        void RecordThrowTrick(float velocity, float returnTime);
        void RecordSpinTrick(float duration);
        void RecordFailedTrick();
        
        // Timing utilities
        void StartTimer(const std::string& name);
        float EndTimer(const std::string& name);
        
        // Reporting
        void LogPerformanceReport();
        void SetReportInterval(float seconds);
        void SetEnabled(bool enabled);
        
        // Getters
        const FrameMetrics& GetFrameMetrics() const { return frameMetrics; }
        const MemoryMetrics& GetMemoryMetrics() const { return memoryMetrics; }
        const TrickMetrics& GetTrickMetrics() const { return trickMetrics; }
        
        // Quest-specific metrics
        float GetBatteryUsage() const;
        float GetThermalState() const;
        bool IsPerformanceThrottled() const;
        
        PerformanceMetrics() = default;
        
    private:
        friend class LazyInitializer<PerformanceMetrics>;
    };
    
    // Lazy performance metrics initializer
    class LazyPerformanceSetup {
    private:
        static LazyComponentInitializer setupInitializer;
        
    public:
        static void Setup();
        static bool IsSetup();
    };

    // Lazy convenience macros for performance measurement
    #define PERF_TIMER_START(name) do { \
        if (TrickSaber::Utils::PerformanceMetrics::IsInitialized()) { \
            TrickSaber::Utils::PerformanceMetrics::GetInstance()->StartTimer(name); \
        } \
    } while(0)
    
    #define PERF_TIMER_END(name) do { \
        if (TrickSaber::Utils::PerformanceMetrics::IsInitialized()) { \
            TrickSaber::Utils::PerformanceMetrics::GetInstance()->EndTimer(name); \
        } \
    } while(0)
    
    // RAII timer for automatic measurement
    class ScopedTimer {
    private:
        std::string name;
        std::chrono::high_resolution_clock::time_point startTime;
        
    public:
        ScopedTimer(const std::string& timerName) : name(timerName) {
            startTime = std::chrono::high_resolution_clock::now();
        }
        
        ~ScopedTimer() {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
            // Log or store the timing result
        }
    };
    
    #define PERF_SCOPE_TIMER(name) TrickSaber::Utils::ScopedTimer _timer(name)
}