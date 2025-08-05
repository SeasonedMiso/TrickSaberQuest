#include "custom-types/shared/register.hpp"
#include "UnityEngine/GameObject.hpp"

// Include custom-types headers
#include "TrickSaber/EnhancedSaberManager.hpp"
#include "TrickSaber/AdvancedInputSystem.hpp"
#include "TrickSaber/AdvancedTrickFeatures.hpp"
#include "TrickSaber/PhysicsHandler.hpp"

namespace TrickSaber {
    void RegisterCustomTypes() {
        custom_types::Register::AutoRegister();
    }
}