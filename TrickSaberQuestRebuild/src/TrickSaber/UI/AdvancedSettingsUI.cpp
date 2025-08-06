#include "TrickSaber/UI/AdvancedSettingsUI.hpp"
#include "TrickSaber/Configuration.hpp"
#include "TrickSaber/Config.hpp"
#include "main.hpp"
#include "bsml/shared/BSML.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "beatsaber-hook/shared/utils/utils.h"

DEFINE_TYPE(TrickSaber::UI, AdvancedSettingsUI);

using namespace TrickSaber::UI;

void AdvancedSettingsUI::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (firstActivation) {
        auto container = BSML::Lite::CreateScrollableSettingsContainer(get_transform());
        
        // === MAIN TOGGLES ===
        BSML::Lite::CreateText(container->get_transform(), "<size=5><color=#00ff00>TrickSaber Quest Settings</color></size>");
        
        BSML::Lite::CreateToggle(container->get_transform(), "Enable TrickSaber", 
            Configuration::IsModEnabled(),
            [](bool value) { 
                TrickSaber::config.trickSaberEnabled = value;
                Configuration::config.enabled = value;
                TrickSaber::SaveConfig();
            });
        
        // === TRICK MODE TOGGLES ===
        BSML::Lite::CreateText(container->get_transform(), "<size=4><color=#ffff00>Trick Modes</color></size>");
        
        BSML::Lite::CreateToggle(container->get_transform(), "Enable Spin Tricks", 
            Configuration::AreSpinTricksEnabled(),
            [](bool value) { 
                Configuration::SetTrickEnabled(TrickAction::Spin, value);
                TrickSaber::SaveConfig();
            });
            
        BSML::Lite::CreateToggle(container->get_transform(), "Enable Throw Tricks", 
            Configuration::AreThrowTricksEnabled(),
            [](bool value) { 
                Configuration::SetTrickEnabled(TrickAction::Throw, value);
                TrickSaber::SaveConfig();
            });
        
        // === SPIN MODE SELECTION ===
        BSML::Lite::CreateText(container->get_transform(), "<size=4><color=#ff8800>Spin Mode</color></size>");
        
        // Spin Mode Dropdown
        std::array<std::string_view, 4> spinModeOptions = {
            "Traditional", "Omni-Directional", "Angle Speed", "Momentum"
        };
        BSML::Lite::CreateDropdown(container->get_transform(), "Spin Mode", 
            spinModeOptions[static_cast<int>(TrickSaber::config.spinMode)], std::span(spinModeOptions),
            [](StringW value) {
                std::string val = static_cast<std::string>(value);
                if (val == "Traditional") TrickSaber::config.spinMode = SpinMode::Traditional;
                else if (val == "Omni-Directional") TrickSaber::config.spinMode = SpinMode::OmniDirectional;
                else if (val == "Angle Speed") TrickSaber::config.spinMode = SpinMode::AngleSpeed;
                else if (val == "Momentum") TrickSaber::config.spinMode = SpinMode::Momentum;
                TrickSaber::SaveConfig();
            });
        
        // === THROW MODE SELECTION ===
        BSML::Lite::CreateText(container->get_transform(), "<size=4><color=#ff8800>Throw Mode</color></size>");
        
        std::array<std::string_view, 2> throwModeOptions = {
            "Press to Throw", "Grip to Summon"
        };
        BSML::Lite::CreateDropdown(container->get_transform(), "Throw Mode", 
            throwModeOptions[static_cast<int>(TrickSaber::config.throwMode)], std::span(throwModeOptions),
            [](StringW value) {
                std::string val = static_cast<std::string>(value);
                if (val == "Press to Throw") TrickSaber::config.throwMode = ThrowMode::PressToThrow;
                else if (val == "Grip to Summon") TrickSaber::config.throwMode = ThrowMode::GripToSummon;
                TrickSaber::SaveConfig();
            });
        
        // === INPUT BINDINGS ===
        BSML::Lite::CreateText(container->get_transform(), "<size=4><color=#8888ff>Input Bindings</color></size>");
        
        // Trigger Action
        std::array<std::string_view, 4> actionOptions = {
            "None", "Throw", "Spin", "Freeze Throw"
        };
        BSML::Lite::CreateDropdown(container->get_transform(), "Trigger Action", 
            actionOptions[static_cast<int>(TrickSaber::config.triggerAction)], std::span(actionOptions),
            [](StringW value) {
                std::string val = static_cast<std::string>(value);
                if (val == "None") TrickSaber::config.triggerAction = TrickAction::None;
                else if (val == "Throw") TrickSaber::config.triggerAction = TrickAction::Throw;
                else if (val == "Spin") TrickSaber::config.triggerAction = TrickAction::Spin;
                else if (val == "Freeze Throw") TrickSaber::config.triggerAction = TrickAction::FreezeThrow;
                TrickSaber::SaveConfig();
            });
        
        // Thumbstick Action
        BSML::Lite::CreateDropdown(container->get_transform(), "Thumbstick Action", 
            actionOptions[static_cast<int>(TrickSaber::config.thumbstickAction)], std::span(actionOptions),
            [](StringW value) {
                std::string val = static_cast<std::string>(value);
                if (val == "None") TrickSaber::config.thumbstickAction = TrickAction::None;
                else if (val == "Throw") TrickSaber::config.thumbstickAction = TrickAction::Throw;
                else if (val == "Spin") TrickSaber::config.thumbstickAction = TrickAction::Spin;
                else if (val == "Freeze Throw") TrickSaber::config.thumbstickAction = TrickAction::FreezeThrow;
                TrickSaber::SaveConfig();
            });
        
        // === INPUT THRESHOLDS ===
        BSML::Lite::CreateText(container->get_transform(), "<size=4><color=#ff88ff>Input Sensitivity</color></size>");
        
        BSML::Lite::CreateSliderSetting(container->get_transform(), "Trigger Threshold", 
            0.01f, Configuration::GetTriggerThreshold(), 0.1f, 1.0f, 0.0f, UnityEngine::Vector2::get_zero(),
            [](float value) { 
                Configuration::SetInputThreshold(0, value);
                TrickSaber::SaveConfig();
            });
            
        BSML::Lite::CreateSliderSetting(container->get_transform(), "Thumbstick Threshold", 
            0.01f, Configuration::GetThumbstickThreshold(), 0.1f, 1.0f, 0.0f, UnityEngine::Vector2::get_zero(),
            [](float value) { 
                Configuration::SetInputThreshold(2, value);
                TrickSaber::SaveConfig();
            });
        
        // === PHYSICS SETTINGS ===
        BSML::Lite::CreateText(container->get_transform(), "<size=4><color=#88ffff>Physics Settings</color></size>");
        
        BSML::Lite::CreateSliderSetting(container->get_transform(), "Spin Speed", 
            0.1f, Configuration::GetSpinSpeed(), 0.1f, 5.0f, 0.0f, UnityEngine::Vector2::get_zero(),
            [](float value) { 
                Configuration::SetSpinSettings(Configuration::IsSpeedVelocityDependent(), 
                    value, Configuration::GetSpinDirection()); 
                TrickSaber::SaveConfig();
            });
            
        BSML::Lite::CreateSliderSetting(container->get_transform(), "Throw Velocity", 
            0.1f, Configuration::GetThrowVelocity(), 0.1f, 5.0f, 0.0f, UnityEngine::Vector2::get_zero(),
            [](float value) { 
                Configuration::SetThrowSettings(value, Configuration::GetReturnSpeed()); 
                TrickSaber::SaveConfig();
            });
            
        BSML::Lite::CreateSliderSetting(container->get_transform(), "Return Speed", 
            1.0f, Configuration::GetReturnSpeed(), 1.0f, 50.0f, 0.0f, UnityEngine::Vector2::get_zero(),
            [](float value) { 
                Configuration::SetThrowSettings(Configuration::GetThrowVelocity(), value); 
                TrickSaber::SaveConfig();
            });
        
        // === ADVANCED FEATURES ===
        BSML::Lite::CreateText(container->get_transform(), "<size=4><color=#ffaa88>Advanced Features</color></size>");
        
        BSML::Lite::CreateToggle(container->get_transform(), "Slowmo During Throw", 
            Configuration::IsSlowmoDuringThrow(),
            [](bool value) { 
                Configuration::SetAdvancedFeature("slowmoDuringThrow", value);
                TrickSaber::SaveConfig();
            });
            
        BSML::Lite::CreateToggle(container->get_transform(), "Complete Rotation Mode", 
            Configuration::IsCompleteRotationMode(),
            [](bool value) { 
                Configuration::SetAdvancedFeature("completeRotationMode", value);
                TrickSaber::SaveConfig();
            });
            
        BSML::Lite::CreateToggle(container->get_transform(), "Disable If Notes On Screen", 
            Configuration::ShouldDisableIfNotesOnScreen(),
            [](bool value) { 
                Configuration::SetAdvancedFeature("disableIfNotesOnScreen", value);
                TrickSaber::SaveConfig();
            });
        
        BSML::Lite::CreateToggle(container->get_transform(), "Vibrate On Return", 
            Configuration::ShouldVibrateOnReturn(),
            [](bool value) { 
                TrickSaber::config.vibrateOnReturn = value;
                Configuration::config.vibrateOnReturn = value;
                TrickSaber::SaveConfig();
            });
        
        // === SABER TOGGLES ===
        BSML::Lite::CreateText(container->get_transform(), "<size=4><color=#aaffaa>Per-Saber Settings</color></size>");
        
        BSML::Lite::CreateToggle(container->get_transform(), "Left Saber Enabled", 
            Configuration::IsLeftSaberEnabled(),
            [](bool value) { 
                TrickSaber::config.leftSaberEnabled = value;
                Configuration::config.leftSaberEnabled = value;
                TrickSaber::SaveConfig();
            });
            
        BSML::Lite::CreateToggle(container->get_transform(), "Right Saber Enabled", 
            Configuration::IsRightSaberEnabled(),
            [](bool value) { 
                TrickSaber::config.rightSaberEnabled = value;
                Configuration::config.rightSaberEnabled = value;
                TrickSaber::SaveConfig();
            });
        
        uiInitialized = true;
        Logger.info("Advanced TrickSaber UI initialized with complete mode switching");
    }
}

void AdvancedSettingsUI::CreateMainSettings(UnityEngine::Transform* parent) {
    // Placeholder implementation
}

void AdvancedSettingsUI::CreateInputSettings(UnityEngine::Transform* parent) {
    // Placeholder implementation
}

void AdvancedSettingsUI::CreateTrickSettings(UnityEngine::Transform* parent) {
    // Placeholder implementation
}

void AdvancedSettingsUI::OnInputBindingChanged(int action, int inputType, bool isLeft) {
    // Placeholder implementation
}

void AdvancedSettingsUI::OnInputThresholdChanged(int inputType, float threshold, bool isLeft) {
    // Placeholder implementation
}

void AdvancedSettingsUI::CreateSpinTrickSettings(UnityEngine::Transform* parent) {
    // Placeholder implementation
}

void AdvancedSettingsUI::CreateThrowTrickSettings(UnityEngine::Transform* parent) {
    // Placeholder implementation
}

void AdvancedSettingsUI::RefreshAllSettings() {
    // Placeholder implementation
}

void AdvancedSettingsUI::ResetToDefaults() {
    // Placeholder implementation
}