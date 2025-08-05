#pragma once
#include <chrono>
#include <atomic>

namespace TrickSaber::Utils {
    class ErrorCircuitBreaker {
    private:
        static constexpr int MAX_FAILURES = 5;
        static constexpr int RESET_TIMEOUT_MS = 30000; // 30 seconds
        
        std::atomic<int> failureCount{0};
        std::atomic<std::chrono::steady_clock::time_point> lastFailureTime{std::chrono::steady_clock::time_point{}};
        std::atomic<bool> circuitOpen{false};
        
    public:
        bool ShouldExecute() {
            if (!circuitOpen.load()) return true;
            
            auto now = std::chrono::steady_clock::now();
            auto lastFailure = lastFailureTime.load();
            
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFailure).count() > RESET_TIMEOUT_MS) {
                Reset();
                return true;
            }
            
            return false;
        }
        
        void RecordFailure() {
            failureCount.fetch_add(1);
            lastFailureTime.store(std::chrono::steady_clock::now());
            
            if (failureCount.load() >= MAX_FAILURES) {
                circuitOpen.store(true);
            }
        }
        
        void RecordSuccess() {
            failureCount.store(0);
            circuitOpen.store(false);
        }
        
        void Reset() {
            failureCount.store(0);
            circuitOpen.store(false);
        }
        
        bool IsOpen() const { return circuitOpen.load(); }
        int GetFailureCount() const { return failureCount.load(); }
    };
}