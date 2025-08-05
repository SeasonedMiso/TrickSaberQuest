#include "TrickSaber/UI/SettingsViewController.hpp"
#include "TrickSaber/UI/DebugOverlay.hpp"
#include "TrickSaber/Configuration.hpp"
#include "TrickSaber/Config.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "main.hpp"

namespace TrickSaber::UI {
    
    void SettingsViewControllerDidActivate(
        HMUI::ViewController* self,
        bool firstActivation,
        bool addedToHierarchy,
        bool screenSystemEnabling
    ) {
        if (!firstActivation) return;
        
        auto* container = BSML::Lite::CreateScrollableSettingsContainer(self->get_transform());
        auto* parent = container->get_transform().ptr();
        
        // Header
        BSML::Lite::CreateText(parent, "<size=5><color=#00ff00>TrickSaber Quest Settings</color></size>");
        
        // Main toggle
        BSML::Lite::CreateToggle(parent, "Enable TrickSaber",
            Configuration::IsModEnabled(),
            [](bool value) {
                Configuration::config.enabled = value;
                TrickSaber::SaveConfig();
            }
        );
        
        // Trick Type Toggles
        BSML::Lite::CreateText(parent, "<size=4><color=#ffff00>Trick Modes</color></size>");
        
        BSML::Lite::CreateToggle(parent, "Enable Spin Tricks",
            Configuration::AreSpinTricksEnabled(),
            [](bool value) {
                Configuration::SetTrickEnabled(TrickAction::Spin, value);
                TrickSaber::SaveConfig();
            }
        );
        
        BSML::Lite::CreateToggle(parent, "Enable Throw Tricks",
            Configuration::AreThrowTricksEnabled(),
            [](bool value) {
                Configuration::SetTrickEnabled(TrickAction::Throw, value);
                TrickSaber::SaveConfig();
            }
        );
        
        // Per-Saber Settings
        BSML::Lite::CreateText(parent, "<size=4><color=#aaffaa>Per-Saber Settings</color></size>");
        
        BSML::Lite::CreateToggle(parent, "Left Saber Enabled",
            Configuration::IsLeftSaberEnabled(),
            [](bool value) {
                Configuration::config.leftSaberEnabled = value;
                TrickSaber::SaveConfig();
            }
        );
        
        BSML::Lite::CreateToggle(parent, "Right Saber Enabled",
            Configuration::IsRightSaberEnabled(),
            [](bool value) {
                Configuration::config.rightSaberEnabled = value;
                TrickSaber::SaveConfig();
            }
        );
        
        // Input Thresholds
        BSML::Lite::CreateText(parent, "<size=4><color=#ff88ff>Input Sensitivity</color></size>");
        
        BSML::Lite::CreateIncrementSetting(parent, "Trigger Threshold", 2, 0.05f,
            Configuration::GetTriggerThreshold(),
            0.1f, 1.0f,
            [](float value) {
                Configuration::SetInputThreshold(0, value);
                TrickSaber::SaveConfig();
            }
        );
        
        BSML::Lite::CreateIncrementSetting(parent, "Thumbstick Threshold", 2, 0.05f,
            Configuration::GetThumbstickThreshold(),
            0.1f, 1.0f,
            [](float value) {
                Configuration::SetInputThreshold(2, value);
                TrickSaber::SaveConfig();
            }
        );
        
        // Physics Settings
        BSML::Lite::CreateText(parent, "<size=4><color=#88ffff>Physics Settings</color></size>");
        
        BSML::Lite::CreateIncrementSetting(parent, "Spin Speed", 1, 0.1f,
            Configuration::GetSpinSpeed(),
            0.1f, 5.0f,
            [](float value) {
                Configuration::SetSpinSettings(Configuration::IsSpeedVelocityDependent(), 
                    value, Configuration::GetSpinDirection());
                TrickSaber::SaveConfig();
            }
        );
        
        BSML::Lite::CreateIncrementSetting(parent, "Throw Velocity", 1, 0.1f,
            Configuration::GetThrowVelocity(),
            0.1f, 5.0f,
            [](float value) {
                Configuration::SetThrowSettings(value, Configuration::GetReturnSpeed());
                TrickSaber::SaveConfig();
            }
        );
        
        BSML::Lite::CreateIncrementSetting(parent, "Return Speed", 1, 1.0f,
            Configuration::GetReturnSpeed(),
            1.0f, 50.0f,
            [](float value) {
                Configuration::SetThrowSettings(Configuration::GetThrowVelocity(), value);
                TrickSaber::SaveConfig();
            }
        );
        
        // Advanced Options
        BSML::Lite::CreateText(parent, "<size=4><color=#ffaa88>Advanced Features</color></size>");
        
        BSML::Lite::CreateToggle(parent, "Slowmo During Throw",
            Configuration::IsSlowmoDuringThrow(),
            [](bool value) {
                Configuration::SetAdvancedFeature("slowmoDuringThrow", value);
                TrickSaber::SaveConfig();
            }
        );
        
        BSML::Lite::CreateToggle(parent, "Disable If Notes On Screen",
            Configuration::ShouldDisableIfNotesOnScreen(),
            [](bool value) {
                Configuration::SetAdvancedFeature("disableIfNotesOnScreen", value);
                TrickSaber::SaveConfig();
            }
        );
        
        BSML::Lite::CreateToggle(parent, "Vibrate On Return",
            Configuration::ShouldVibrateOnReturn(),
            [](bool value) {
                Configuration::config.vibrateOnReturn = value;
                TrickSaber::SaveConfig();
            }
        );
        
        // Debug Options
        BSML::Lite::CreateText(parent, "<size=4><color=#ff8888>Debug Options</color></size>");
        
        BSML::Lite::CreateToggle(parent, "Show Debug Overlay",
            TrickSaber::config.showDebugOverlay,
            [](bool value) {
                TrickSaber::config.showDebugOverlay = value;
                TrickSaber::SaveConfig();
                // Update overlay visibility
                if (value) {
                    TrickSaber::UI::DebugOverlay::CreateOverlay();
                } else {
                    TrickSaber::UI::DebugOverlay::DestroyOverlay();
                }
            }
        );
        
        PaperLogger.info("TrickSaber minimal settings UI created");
    }
}