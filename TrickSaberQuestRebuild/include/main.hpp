#pragma once

#include "scotland2/shared/modloader.h"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"

// Use beatsaber-hook logger instead to avoid paper conflicts
#include "beatsaber-hook/shared/utils/logging.hpp"

// Simple logger wrapper to avoid conflicts
struct SimpleLogger {
    static constexpr const char* tag = "TrickSaber";
    
    template<typename... Args>
    static void info(const char* fmt, Args&&... args) {
        printf("[TrickSaber] %s\n", fmt);
    }
    
    template<typename... Args>
    static void debug(const char* fmt, Args&&... args) {
        printf("[TrickSaber DEBUG] %s\n", fmt);
    }
    
    template<typename... Args>
    static void warn(const char* fmt, Args&&... args) {
        printf("[TrickSaber WARN] %s\n", fmt);
    }
    
    template<typename... Args>
    static void error(const char* fmt, Args&&... args) {
        printf("[TrickSaber ERROR] %s\n", fmt);
    }
    
    template<typename... Args>
    static void critical(const char* fmt, Args&&... args) {
        printf("[TrickSaber CRITICAL] %s\n", fmt);
    }
};

static constexpr auto Logger = SimpleLogger{};

extern modloader::ModInfo modInfo;

// Simple Configuration wrapper
struct Configuration {
    Configuration(const modloader::ModInfo& info) {}
    void Write() {}
};

Configuration& getConfig();

#include "TrickSaber/Config.hpp"
#include "TrickSaber/Enums.hpp"

// Forward declarations
namespace GlobalNamespace {
    class SaberManager;
}

// Global variables
extern GlobalNamespace::SaberManager* saberManager;
extern Configuration globalConfig;

// Utility functions
bool SafeExecute(const std::function<void()>& func, const char* context);
void PerformComprehensiveCleanup();

namespace TrickSaber {
    void LoadConfig();
    void SaveConfig();
}