#include "TrickSaber/SaberTrickManager.hpp"
#include "TrickSaber/Core/TrickSaberManager.hpp"
#include "TrickSaber/GlobalTrickManager.hpp"
#include "TrickSaber/SaberTrickModel.hpp"
#include "TrickSaber/TrailHandler.hpp"
#include "TrickSaber/MovementController.hpp"
#include "TrickSaber/Tricks/Trick.hpp"
#include "TrickSaber/Tricks/SpinTrick.hpp"
#include "TrickSaber/Tricks/ThrowTrick.hpp"
#include "TrickSaber/Utils/HapticFeedbackHelper.hpp"
#include "TrickSaber/Utils/PerformanceMetrics.hpp"
#include "TrickSaber/BurnMarkHandler.hpp"
#include "TrickSaber/Config.hpp"
#include "TrickSaber/Constants.hpp"
#include "GlobalNamespace/OVRInput.hpp"
#include "main.hpp"

#include "UnityEngine/Time.hpp"
#include "UnityEngine/Object.hpp"
#include "custom-types/shared/register.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "System/Type.hpp"

DEFINE_TYPE(TrickSaber, SaberTrickManager);

using namespace TrickSaber;
using namespace GlobalNamespace;

void SaberTrickManager::Awake() {
    enabled = true;
    currentTrick = TrickAction::None;
}

void SaberTrickManager::Update() {
    if (!enabled || !config.trickSaberEnabled) return;
    
    ValidateComponents();
    CheckDirectInput();
}

void SaberTrickManager::CheckDirectInput() {
    if (!saber) return;
    
    // Determine controller based on saber type
    auto ovrController = (saber->get_saberType() == GlobalNamespace::SaberType::SaberA) ? 
        GlobalNamespace::OVRInput::Controller::LTouch : GlobalNamespace::OVRInput::Controller::RTouch;
    
    // Check trigger for throw trick
    bool triggerPressed = GlobalNamespace::OVRInput::Get(
        GlobalNamespace::OVRInput::Button::PrimaryIndexTrigger, ovrController);
    
    if (triggerPressed && !triggerWasPressed && CanDoTrick(TrickAction::Throw)) {
        OnTrickActivated(TrickAction::Throw, 1.0f);
    } else if (!triggerPressed && triggerWasPressed && currentTrick == TrickAction::Throw) {
        OnTrickDeactivated(TrickAction::Throw);
    }
    triggerWasPressed = triggerPressed;
    
    // Check thumbstick for spin trick
    auto stick = GlobalNamespace::OVRInput::Get(
        GlobalNamespace::OVRInput::Axis2D::PrimaryThumbstick, ovrController);
    
    bool isLeftSaber = (saber->get_saberType() == GlobalNamespace::SaberType::SaberA);
    float thumbstickValue = isLeftSaber ? -stick.x : stick.x;
    bool thumbstickActive = (thumbstickValue > TrickSaber::Constants::SIMPLIFIED_THUMBSTICK_THRESHOLD);
    
    if (thumbstickActive && !thumbstickWasActive && CanDoTrick(TrickAction::Spin)) {
        OnTrickActivated(TrickAction::Spin, thumbstickValue);
    } else if (!thumbstickActive && thumbstickWasActive && currentTrick == TrickAction::Spin) {
        OnTrickDeactivated(TrickAction::Spin);
    } else if (thumbstickActive && currentTrick == TrickAction::Spin) {
        // Update spin trick with new input value
        auto it = tricks.find(TrickAction::Spin);
        if (it != tricks.end() && it->second) {
            auto spinTrick = static_cast<Tricks::SpinTrick*>(it->second);
            if (spinTrick) {
                spinTrick->inputValue = thumbstickValue;
            }
        }
    }
    thumbstickWasActive = thumbstickActive;
}

void SaberTrickManager::OnDestroy() {
    EndAllTricks();
    Cleanup();
}

void SaberTrickManager::Initialize(Saber* saber) {
    if (!saber) {
        PaperLogger.error("Saber is null in SaberTrickManager::Initialize");
        enabled = false;
        return;
    }
    
    this->saber = saber;
    this->vrController = saber->get_transform()->GetComponentInParent<VRController*>();
    
    if (!vrController) {
        PaperLogger.error("No VRController found for saber");
        enabled = false;
        return;
    }
    
    try {
        InitializeComponents();
        InitializeTricks();
        ConnectInputEvents();
        
        PaperLogger.info("SaberTrickManager initialized for {} saber", 
            saber->get_saberType() == GlobalNamespace::SaberType::SaberA ? "left" : "right");
    } catch (const std::exception& e) {
        PaperLogger.error("Failed to initialize SaberTrickManager: {}", e.what());
        enabled = false;
        Cleanup();
    }
}

void SaberTrickManager::InitializeComponents() {
    auto gameObject = get_gameObject();
    
    // No InputManager needed - using direct OVRInput
    PaperLogger.debug("Using direct OVRInput - no InputManager needed");
    
    // MovementController is now static - no component needed
    
    saberTrickModel = gameObject->AddComponent<SaberTrickModel*>();
    if (!saberTrickModel) {
        throw std::runtime_error("Failed to create SaberTrickModel");
    }
    saberTrickModel->Initialize(saber);
    
    trailHandler = gameObject->AddComponent<TrailHandler*>();
    if (trailHandler) {
        trailHandler->Initialize(saber);
    }
    
    PaperLogger.debug("Components initialized");
}

void SaberTrickManager::InitializeTricks() {
    auto gameObject = get_gameObject();
    
    // Create SpinTrick using template AddComponent
    auto spinTrick = gameObject->AddComponent<Tricks::SpinTrick*>();
    if (spinTrick) {
        spinTrick->Initialize(this, saberTrickModel);
        tricks[TrickAction::Spin] = spinTrick;
    }
    
    // Create ThrowTrick using template AddComponent
    auto throwTrick = gameObject->AddComponent<Tricks::ThrowTrick*>();
    if (throwTrick) {
        throwTrick->Initialize(this, saberTrickModel);
        tricks[TrickAction::Throw] = throwTrick;
    }
    
    PaperLogger.debug("Tricks initialized: {}", tricks.size());
}

void SaberTrickManager::ConnectInputEvents() {
    // Direct input - no callbacks needed
    PaperLogger.debug("Using direct OVRInput - no callbacks needed");
}

void SaberTrickManager::OnTrickActivated(TrickAction action, float value) {
    PERF_TIMER_START("TrickActivation");
    
    if (!enabled || !saber || action == TrickAction::None) {
        PaperLogger.debug("Trick activation ignored - enabled: {}, saber: {}, action: {}", 
            enabled, saber != nullptr, static_cast<int>(action));
        return;
    }
    
    if (!CanDoTrick(action)) {
        PaperLogger.debug("Cannot start trick {} - conditions not met (current: {})", 
            static_cast<int>(action), static_cast<int>(currentTrick));
        
        // Record failed trick attempt
        auto perfMetrics = Utils::PerformanceMetrics::GetInstance();
        if (perfMetrics) {
            perfMetrics->RecordFailedTrick();
        }
        return;
    }
    
    // Handle special case for spin trick continuous input
    if (action == TrickAction::Spin && currentTrick == TrickAction::Spin) {
        // Update existing spin trick with new input value
        auto it = tricks.find(action);
        if (it != tricks.end() && it->second) {
            auto spinTrick = static_cast<Tricks::SpinTrick*>(it->second);
            if (spinTrick) {
                spinTrick->inputValue = value;
                PaperLogger.debug("Updated spin trick input: {:.2f}", value);
            }
        }
        return;
    }
    
    auto it = tricks.find(action);
    if (it != tricks.end() && it->second) {
        try {
            PaperLogger.debug("Starting trick {} with value {:.2f}", static_cast<int>(action), value);
            if (it->second->StartTrick(value)) {
                currentTrick = action;
                OnTrickStarted(action);
                PaperLogger.info("Trick {} started successfully", static_cast<int>(action));
            } else {
                PaperLogger.error("Trick {} failed to start", static_cast<int>(action));
                
                // Record failed trick
                auto perfMetrics = Utils::PerformanceMetrics::GetInstance();
                if (perfMetrics) {
                    perfMetrics->RecordFailedTrick();
                }
            }
        } catch (const std::exception& e) {
            PaperLogger.error("Error starting trick {}: {}", static_cast<int>(action), e.what());
            
            // Record failed trick
            auto perfMetrics = Utils::PerformanceMetrics::GetInstance();
            if (perfMetrics) {
                perfMetrics->RecordFailedTrick();
            }
        }
    } else {
        PaperLogger.error("Trick {} not found or null", static_cast<int>(action));
    }
    
    PERF_TIMER_END("TrickActivation");
}

void SaberTrickManager::OnTrickDeactivated(TrickAction action) {
    if (!enabled || !saber || action == TrickAction::None) return;
    
    // Only end trick if it's currently active
    if (currentTrick != action) {
        PaperLogger.debug("Trick deactivation ignored - not current trick (current: {}, deactivating: {})", 
            static_cast<int>(currentTrick), static_cast<int>(action));
        return;
    }
    
    auto it = tricks.find(action);
    if (it != tricks.end() && it->second && it->second->IsActive()) {
        try {
            OnTrickEnding(action);
            it->second->EndTrick();
            PaperLogger.debug("Trick {} deactivated", static_cast<int>(action));
        } catch (const std::exception& e) {
            PaperLogger.error("Error ending trick {}: {}", static_cast<int>(action), e.what());
            it->second->EndTrickImmediately();
            OnTrickEnded(action); // Ensure state is cleaned up
        }
    }
}

void SaberTrickManager::OnTrickStarted(TrickAction action) {
    // Record trick start time for performance metrics
    trickStartTime = std::chrono::high_resolution_clock::now();
    
    // Disable burn marks during trick
    BurnMarkHandler::DisableBurnMarks(static_cast<int>(saber->get_saberType()));
    
    // Trigger haptic feedback
    Utils::HapticFeedbackHelper::TriggerHaptic(saber->get_saberType(), 
        Utils::HapticFeedbackHelper::HapticType::TrickStart);
    
    // Notify global manager
    auto globalManager = GlobalTrickManager::GetInstance();
    if (globalManager) {
        globalManager->OnTrickStarted(action);
    }
    
    if (onTrickStarted) {
        onTrickStarted(action);
    }
    
    PaperLogger.debug("Trick started: {}", static_cast<int>(action));
}

void SaberTrickManager::OnTrickEnding(TrickAction action) {
    if (onTrickEnding) {
        onTrickEnding(action);
    }
    
    PaperLogger.debug("Trick ending: {}", static_cast<int>(action));
}

void SaberTrickManager::OnTrickEnded(TrickAction action) {
    // Calculate trick duration and record performance metrics
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - trickStartTime).count() / 1000.0f;
    
    auto perfMetrics = Utils::PerformanceMetrics::GetInstance();
    if (perfMetrics) {
        if (action == TrickAction::Throw) {
            // Get throw velocity from static movement controller
            float velocity = 0.0f;
            if (saber->get_saberType() == GlobalNamespace::SaberType::SaberA) {
                velocity = MovementController::GetLeftVelocity().get_magnitude();
            } else {
                velocity = MovementController::GetRightVelocity().get_magnitude();
            }
            perfMetrics->RecordThrowTrick(velocity, duration);
        } else if (action == TrickAction::Spin) {
            perfMetrics->RecordSpinTrick(duration);
        }
    }
    
    currentTrick = TrickAction::None;
    
    // Re-enable burn marks
    BurnMarkHandler::EnableBurnMarks(static_cast<int>(saber->get_saberType()));
    
    // Trigger return haptic feedback
    Utils::HapticFeedbackHelper::TriggerHaptic(saber->get_saberType(), 
        Utils::HapticFeedbackHelper::HapticType::SaberReturn);
    
    // Notify global manager
    auto globalManager = GlobalTrickManager::GetInstance();
    if (globalManager) {
        globalManager->OnTrickEnded(action);
    }
    
    if (onTrickEnded) {
        onTrickEnded(action);
    }
    
    PaperLogger.debug("Trick ended: {} (duration: {:.2f}s)", static_cast<int>(action), duration);
}

bool SaberTrickManager::CanDoTrick(TrickAction action) const {
    if (!enabled || !config.trickSaberEnabled || !saber) return false;
    
    // Controller connection is handled internally by OVRInput
    
    // Special case: allow spin trick updates while spinning
    if (action == TrickAction::Spin && currentTrick == TrickAction::Spin) {
        return true;
    }
    
    // Only one trick at a time (except for spin updates)
    if (IsAnyTrickActive()) return false;
    
    // Check saber-specific configuration
    bool saberEnabled = (saber->get_saberType() == GlobalNamespace::SaberType::SaberA) ? 
        config.leftSaberEnabled : config.rightSaberEnabled;
    if (!saberEnabled) return false;
    
    // Check global conditions
    auto globalManager = GlobalTrickManager::GetInstance();
    if (globalManager) {
        int saberType = static_cast<int>(saber->get_saberType());
        return globalManager->CanStartTrick(action, saberType);
    }
    
    return true;
}

bool SaberTrickManager::IsAnyTrickActive() const {
    return currentTrick != TrickAction::None;
}

bool SaberTrickManager::IsDoingTrick() {
    return IsAnyTrickActive();
}

void SaberTrickManager::EndAllTricks() {
    for (auto& [action, trick] : tricks) {
        if (trick && trick->IsActive()) {
            OnTrickEnding(action);
            trick->EndTrickImmediately();
            OnTrickEnded(action);
        }
    }
    currentTrick = TrickAction::None;
}

bool SaberTrickManager::IsTrickInState(TrickAction action, TrickState state) {
    auto it = tricks.find(action);
    if (it != tricks.end() && it->second) {
        return it->second->IsTrickInState(static_cast<int>(state));
    }
    return false;
}

void SaberTrickManager::ValidateComponents() {
    if (!saber || !vrController) {
        PaperLogger.error("Critical components missing - saber: {}, vrController: {}", 
            saber != nullptr, vrController != nullptr);
        enabled = false;
        return;
    }
    
    // Controller connection is handled internally by OVRInput
    
    // Direct OVRInput - no validation needed
}

void SaberTrickManager::UpdateActiveTricks() {
    for (auto& [action, trick] : tricks) {
        if (trick && trick->IsActive()) {
            trick->Update();
        }
    }
}

void SaberTrickManager::Cleanup() {
    // No InputManager to clean up
    
    onTrickStarted = nullptr;
    onTrickEnding = nullptr;
    onTrickEnded = nullptr;
}