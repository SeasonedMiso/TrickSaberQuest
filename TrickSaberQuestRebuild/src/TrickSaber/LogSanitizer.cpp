#include "TrickSaber/LogSanitizer.hpp"
#include <algorithm>

namespace TrickSaber::Utils {
    std::string SanitizeForLog(const std::string& input) {
        std::string sanitized = input;
        // Remove newlines and carriage returns to prevent log injection
        std::replace(sanitized.begin(), sanitized.end(), '\n', ' ');
        std::replace(sanitized.begin(), sanitized.end(), '\r', ' ');
        // Remove null bytes
        sanitized.erase(std::remove(sanitized.begin(), sanitized.end(), '\0'), sanitized.end());
        return sanitized;
    }
}