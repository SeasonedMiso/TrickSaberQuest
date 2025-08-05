#pragma once

#include "scotland2/shared/modloader.h"
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"

#include "paper/shared/logger.hpp"

static constexpr auto PaperLogger = Paper::ConstLoggerContext("TrickSaber");

extern modloader::ModInfo modInfo;

Configuration& getConfig();

#include "TrickSaber/Config.hpp"
#include "TrickSaber/Enums.hpp"

// Forward declarations
namespace GlobalNamespace {
    class SaberManager;
}

// Global variables
extern GlobalNamespace::SaberManager* saberManager;

// Utility functions
bool SafeExecute(const std::function<void()>& func, const char* context);
void PerformComprehensiveCleanup();

namespace TrickSaber {
    void LoadConfig();
    void SaveConfig();
}