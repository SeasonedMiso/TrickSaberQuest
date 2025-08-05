#pragma once

#include "GlobalNamespace/SaberType.hpp"

namespace TrickSaber::Utils {
    class HapticFeedbackHelper {
    public:
        enum class HapticType {
            TrickStart,
            TrickEnd,
            SaberReturn,
            SpinStart,
            SpinEnd
        };
        
        struct HapticParams {
            float duration;
            float strength;
        };
        
        static void TriggerHaptic(GlobalNamespace::SaberType saberType, HapticType type);
        static void TriggerHaptic(GlobalNamespace::SaberType saberType, float duration, float strength);
        static HapticParams GetHapticParams(HapticType type);
    };
}