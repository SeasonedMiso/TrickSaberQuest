#pragma once

#include <string>

namespace TrickSaber {
    namespace SlowmoHelper {
        
        // Enable slowmo with default settings
        void EnableSlowmo();
        
        // Disable slowmo
        void DisableSlowmo();
        
        // Set custom slowmo amount (0.1f = 90% speed, 0.5f = 50% speed)
        void SetSlowmoAmount(float amount);
        
        // Quick preset configurations
        void SetSlowmoPreset(const std::string& preset);
        
        // Check if slowmo is currently enabled
        bool IsSlowmoEnabled();
        
        // Get current slowmo amount
        float GetSlowmoAmount();
    }
}