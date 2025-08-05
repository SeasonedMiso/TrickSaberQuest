#pragma once

#include "custom-types/shared/macros.hpp"
#include "HMUI/ViewController.hpp"
#include "UnityEngine/Transform.hpp"

namespace TrickSaber::UI {
    void SettingsViewControllerDidActivate(
        HMUI::ViewController* self,
        bool firstActivation,
        bool addedToHierarchy,
        bool screenSystemEnabling
    );
}