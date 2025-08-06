#include "TrickSaber/Utils/PerformanceMetrics.hpp"
#include "main.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/SystemInfo.hpp"
#include <algorithm>
#include <numeric>

using namespace TrickSaber::Utils;

LazyInitializer<PerformanceMetrics> PerformanceMetrics::lazyInstance(
    []() -> std::unique_ptr<PerformanceMetrics> {
        Logger.debug("Initializing PerformanceMetrics on first access");
        auto instance = std::make_unique<PerformanceMetrics>();
        instance->startTime = std::chrono::high_resolution_clock::now();
        instance->lastFrameTime = instance->startTime;
        instance->lastReport = instance->startTime;
        return instance;
    }
);

// Constructor is defaulted in header

PerformanceMetrics* PerformanceMetrics::GetInstance() {
    return lazyInstance.GetPtr();
}

bool PerformanceMetrics::IsInitialized() {
    return lazyInstance.IsInitialized();
}

void PerformanceMetrics::UpdateFrameMetrics() {
    if (!enabled) return;
    
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastFrameTime);
    
    frameMetrics.frameTime = frameDuration.count() / 1000.0f; // Convert to milliseconds
    frameMetrics.fps = 1000.0f / frameMetrics.frameTime;
    
    // Update circular buffer
    frameTimeBuffer[frameBufferIndex] = frameMetrics.frameTime;
    frameBufferIndex = (frameBufferIndex + 1) % 60;
    if (frameBufferIndex == 0) bufferFull = true;
    
    // Check for dropped frames (assuming 90fps target for Quest)
    if (frameMetrics.frameTime > 11.11f) { // 1000ms / 90fps = 11.11ms
        frameMetrics.droppedFrames++;
    }
    
    lastFrameTime = currentTime;
    
    // Auto-report if interval passed
    auto timeSinceReport = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastReport);
    if (timeSinceReport.count() >= reportInterval) {
        LogPerformanceReport();
        lastReport = currentTime;
    }
}

void PerformanceMetrics::RecordFrameDrop() {
    frameMetrics.droppedFrames++;
}

float PerformanceMetrics::GetAverageFPS() const {
    if (!bufferFull && frameBufferIndex == 0) return frameMetrics.fps;
    
    int count = bufferFull ? 60 : frameBufferIndex;
    float sum = std::accumulate(frameTimeBuffer, frameTimeBuffer + count, 0.0f);
    float avgFrameTime = sum / count;
    
    return avgFrameTime > 0 ? 1000.0f / avgFrameTime : 0.0f;
}

float PerformanceMetrics::GetFrameTime() const {
    return frameMetrics.frameTime;
}

void PerformanceMetrics::UpdateMemoryMetrics() {
    if (!enabled) return;
    
    // Quest-specific memory monitoring would require Android NDK calls
    // For now, track our own allocations
    memoryMetrics.totalMemory = UnityEngine::SystemInfo::get_systemMemorySize() * 1024 * 1024; // MB to bytes
    
    if (memoryMetrics.usedMemory > memoryMetrics.peakMemory) {
        memoryMetrics.peakMemory = memoryMetrics.usedMemory;
    }
}

void PerformanceMetrics::RecordAllocation(size_t size) {
    memoryMetrics.usedMemory += size;
    memoryMetrics.allocations++;
    UpdateMemoryMetrics();
}

void PerformanceMetrics::RecordDeallocation(size_t size) {
    memoryMetrics.usedMemory = (size > memoryMetrics.usedMemory) ? 0 : memoryMetrics.usedMemory - size;
    memoryMetrics.deallocations++;
}

size_t PerformanceMetrics::GetMemoryUsage() const {
    return memoryMetrics.usedMemory;
}

void PerformanceMetrics::RecordThrowTrick(float velocity, float returnTime) {
    trickMetrics.throwsPerformed++;
    
    // Update running average for throw velocity
    float totalVelocity = trickMetrics.avgThrowVelocity * (trickMetrics.throwsPerformed - 1) + velocity;
    trickMetrics.avgThrowVelocity = totalVelocity / trickMetrics.throwsPerformed;
    
    // Update running average for return time
    float totalReturnTime = trickMetrics.avgReturnTime * (trickMetrics.throwsPerformed - 1) + returnTime;
    trickMetrics.avgReturnTime = totalReturnTime / trickMetrics.throwsPerformed;
}

void PerformanceMetrics::RecordSpinTrick(float duration) {
    trickMetrics.spinsPerformed++;
    trickMetrics.trickDuration += duration;
}

void PerformanceMetrics::RecordFailedTrick() {
    trickMetrics.failedTricks++;
}

void PerformanceMetrics::StartTimer(const std::string& name) {
    timers[name] = std::chrono::high_resolution_clock::now();
}

float PerformanceMetrics::EndTimer(const std::string& name) {
    auto it = timers.find(name);
    if (it == timers.end()) return 0.0f;
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - it->second);
    float durationMs = duration.count() / 1000.0f;
    
    // Update running average
    if (averageTimes.find(name) == averageTimes.end()) {
        averageTimes[name] = durationMs;
    } else {
        averageTimes[name] = (averageTimes[name] + durationMs) / 2.0f;
    }
    
    timers.erase(it);
    return durationMs;
}

void PerformanceMetrics::LogPerformanceReport() {
    if (!enabled) return;
    
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
    
    Logger.info("=== TrickSaber Performance Report (Uptime: {}s) ===", uptime.count());
    
    // Frame metrics
    Logger.info("Frame Performance:");
    Logger.info("  Current FPS: {:.1f}", frameMetrics.fps);
    Logger.info("  Average FPS: {:.1f}", GetAverageFPS());
    Logger.info("  Frame Time: {:.2f}ms", frameMetrics.frameTime);
    Logger.info("  Dropped Frames: {}", frameMetrics.droppedFrames);
    
    // Memory metrics
    Logger.info("Memory Usage:");
    Logger.info("  Current: {:.2f}MB", memoryMetrics.usedMemory / (1024.0f * 1024.0f));
    Logger.info("  Peak: {:.2f}MB", memoryMetrics.peakMemory / (1024.0f * 1024.0f));
    Logger.info("  Total System: {:.0f}MB", memoryMetrics.totalMemory / (1024.0f * 1024.0f));
    Logger.info("  Allocations: {} | Deallocations: {}", memoryMetrics.allocations, memoryMetrics.deallocations);
    
    // Trick metrics
    Logger.info("Trick Performance:");
    Logger.info("  Throws: {} | Spins: {} | Failed: {}", 
                     trickMetrics.throwsPerformed, trickMetrics.spinsPerformed, trickMetrics.failedTricks);
    Logger.info("  Avg Throw Velocity: {:.2f}m/s", trickMetrics.avgThrowVelocity);
    Logger.info("  Avg Return Time: {:.2f}s", trickMetrics.avgReturnTime);
    
    // Quest-specific metrics
    Logger.info("Quest Hardware:");
    Logger.info("  Battery Usage: {:.1f}%", GetBatteryUsage());
    Logger.info("  Thermal State: {:.1f}°C", GetThermalState());
    Logger.info("  Performance Throttled: {}", IsPerformanceThrottled() ? "Yes" : "No");
    
    // Timer averages
    if (!averageTimes.empty()) {
        Logger.info("Average Timings:");
        for (const auto& [name, time] : averageTimes) {
            Logger.info("  {}: {:.3f}ms", name, time);
        }
    }
    
    Logger.info("=== End Performance Report ===");
}

void PerformanceMetrics::SetReportInterval(float seconds) {
    reportInterval = seconds;
}

void PerformanceMetrics::SetEnabled(bool enabled) {
    this->enabled = enabled;
}

float PerformanceMetrics::GetBatteryUsage() const {
    // Quest-specific battery monitoring would require Android API calls
    // Placeholder implementation
    return 50.0f; // Return mock value for now
}

float PerformanceMetrics::GetThermalState() const {
    // Quest-specific thermal monitoring would require Android API calls
    // Placeholder implementation
    return 35.0f; // Return mock temperature
}

bool PerformanceMetrics::IsPerformanceThrottled() const {
    // Check if performance is being throttled due to thermal or battery constraints
    // This would require Android API integration
    return GetAverageFPS() < 85.0f; // Simple heuristic for now
}

// LazyPerformanceSetup implementation
LazyComponentInitializer LazyPerformanceSetup::setupInitializer([]() {
    Logger.debug("Setting up PerformanceMetrics with default configuration");
    
    auto* metrics = PerformanceMetrics::GetInstance();
    metrics->SetEnabled(true);
    metrics->SetReportInterval(5.0f); // 5 second intervals
    
    Logger.debug("PerformanceMetrics setup completed");
});

void LazyPerformanceSetup::Setup() {
    setupInitializer.Initialize();
}

bool LazyPerformanceSetup::IsSetup() {
    return setupInitializer.IsInitialized();
}