#include "TrickSaber/Configuration.hpp"
#include "TrickSaber/Config.hpp"
#include "TrickSaber/LogSanitizer.hpp"
#include "main.hpp"

namespace TrickSaber {
    
    class SlowmoHelper {
    public:
        // Enable slowmo with default settings
        static void EnableSlowmo() {
            Configuration::SetAdvancedFeature("slowmoDuringThrow", true);
            PaperLogger.info("Slowmo enabled for throw tricks");
        }
        
        // Disable slowmo
        static void DisableSlowmo() {
            Configuration::SetAdvancedFeature("slowmoDuringThrow", false);
            PaperLogger.info("Slowmo disabled");
        }
        
        // Set custom slowmo amount (0.1f = 90% speed, 0.5f = 50% speed)
        static void SetSlowmoAmount(float amount) {
            amount = std::clamp(amount, 0.1f, 0.9f);
            config.slowmoAmount = amount;
            PaperLogger.info("Slowmo amount set to {:.1f} ({:.0f}% speed)", 
                amount, (1.0f - amount) * 100.0f);
        }
        
        // Quick preset configurations
        static void SetSlowmoPreset(const std::string& preset) {
            if (preset == "subtle") {
                EnableSlowmo();
                SetSlowmoAmount(0.1f); // 90% speed
            } else if (preset == "moderate") {
                EnableSlowmo();
                SetSlowmoAmount(0.2f); // 80% speed
            } else if (preset == "dramatic") {
                EnableSlowmo();
                SetSlowmoAmount(0.4f); // 60% speed
            } else if (preset == "extreme") {
                EnableSlowmo();
                SetSlowmoAmount(0.6f); // 40% speed
            }
            PaperLogger.info("Slowmo preset '{}' applied", Utils::SanitizeForLog(preset));
        }
        
        // Check if slowmo is currently enabled
        static bool IsSlowmoEnabled() {
            return Configuration::IsSlowmoDuringThrow();
        }
        
        // Get current slowmo amount
        static float GetSlowmoAmount() {
            return Configuration::GetSlowmoAmount();
        }
    };
}