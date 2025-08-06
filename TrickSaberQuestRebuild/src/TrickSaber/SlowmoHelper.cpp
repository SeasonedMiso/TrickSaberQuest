#include "TrickSaber/Configuration.hpp"
#include "TrickSaber/Config.hpp"
#include "TrickSaber/LogSanitizer.hpp"
#include "main.hpp"
#include <algorithm>

namespace TrickSaber {
    
    namespace SlowmoHelper {
        
        void EnableSlowmo() {
            Configuration::SetAdvancedFeature("slowmoDuringThrow", true);
            Logger.info("Slowmo enabled for throw tricks");
        }
        
        void DisableSlowmo() {
            Configuration::SetAdvancedFeature("slowmoDuringThrow", false);
            Logger.info("Slowmo disabled");
        }
        
        void SetSlowmoAmount(float amount) {
            amount = std::clamp(amount, 0.1f, 0.9f);
            config.slowmoAmount = amount;
            Logger.info("Slowmo amount set to {:.1f} ({:.0f}% speed)", 
                amount, (1.0f - amount) * 100.0f);
        }
        
        void SetSlowmoPreset(const std::string& preset) {
            if (preset == "subtle") {
                EnableSlowmo();
                SetSlowmoAmount(0.1f);
            } else if (preset == "moderate") {
                EnableSlowmo();
                SetSlowmoAmount(0.2f);
            } else if (preset == "dramatic") {
                EnableSlowmo();
                SetSlowmoAmount(0.4f);
            } else if (preset == "extreme") {
                EnableSlowmo();
                SetSlowmoAmount(0.6f);
            }
            Logger.info("Slowmo preset '{}' applied", Utils::SanitizeForLog(preset));
        }
        
        bool IsSlowmoEnabled() {
            return Configuration::IsSlowmoDuringThrow();
        }
        
        float GetSlowmoAmount() {
            return Configuration::GetSlowmoAmount();
        }
    }
}