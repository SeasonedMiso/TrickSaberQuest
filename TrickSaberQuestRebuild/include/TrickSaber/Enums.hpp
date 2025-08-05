#pragma once

namespace TrickSaber {
    enum class VRSystem {
        Oculus,
        SteamVR
    };

    enum class ThumbstickDir {
        Horizontal,
        Vertical
    };

    enum class SpinDir {
        Forward,
        Backward
    };

    enum class TrickAction {
        None,
        Throw,
        Spin,
        FreezeThrow
    };

    enum class TrickState {
        Inactive,
        Started,
        Ending
    };

    enum class ThrowMode {
        PressToThrow,
        GripToSummon
    };

    enum class SpinMode {
        Traditional,      // Left/right only per saber
        OmniDirectional, // All 4 directions based on dominant axis
        AngleSpeed,      // Use thumbstick angle and magnitude for speed
        Momentum         // Use controller momentum
    };

    enum class SaberType {
        SaberA,  // Left saber
        SaberB   // Right saber
    };
}