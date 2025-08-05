#pragma once
#include <string>

namespace TrickSaber::Utils {
    // Sanitize input for safe logging to prevent log injection attacks
    std::string SanitizeForLog(const std::string& input);
}