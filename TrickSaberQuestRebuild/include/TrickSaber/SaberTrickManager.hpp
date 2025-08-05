#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "GlobalNamespace/Saber.hpp"
#include "GlobalNamespace/VRController.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "TrickSaber/Enums.hpp"

#include <unordered_map>
#include <functional>
#include <chrono>

namespace TrickSaber::Tricks { class Trick; }
namespace TrickSaber { class InputManager; class MovementController; class SaberTrickModel; class TrailHandler; }

DECLARE_CLASS_CODEGEN(TrickSaber, SaberTrickManager, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);
    
    DECLARE_INSTANCE_METHOD(void, Initialize, GlobalNamespace::Saber* saber);
    DECLARE_INSTANCE_METHOD(bool, IsDoingTrick);
    DECLARE_INSTANCE_METHOD(void, EndAllTricks);
    // DECLARE_INSTANCE_METHOD(bool, IsTrickInState, TrickSaber::TrickAction action, TrickSaber::TrickState state);
    
public:
    GlobalNamespace::Saber* saber = nullptr;
    
    bool IsTrickInState(TrickAction action, TrickState state);
    GlobalNamespace::VRController* vrController = nullptr;
    // No InputManager needed - using direct OVRInput
    // MovementController is now static - no instance needed
    SaberTrickModel* saberTrickModel = nullptr;
    TrailHandler* trailHandler = nullptr;
    
    bool enabled = true;
    
    // Event callbacks
    std::function<void(TrickAction)> onTrickStarted;
    std::function<void(TrickAction)> onTrickEnding;
    std::function<void(TrickAction)> onTrickEnded;
    
    // Public method for tricks to call when they end
    void OnTrickEnded(TrickAction action);
    void UpdateActiveTricks();
    
    // Public input callbacks
    void OnTrickActivated(TrickAction action, float value);
    void OnTrickDeactivated(TrickAction action);
    
private:
    std::unordered_map<TrickAction, Tricks::Trick*> tricks;
    TrickAction currentTrick = TrickAction::None;
    
    // Performance tracking
    std::chrono::high_resolution_clock::time_point trickStartTime;
    
    void InitializeComponents();
    void InitializeTricks();
    void ConnectInputEvents();
    bool CanDoTrick(TrickAction action) const;
    bool IsAnyTrickActive() const;
    
    void OnTrickStarted(TrickAction action);
    void OnTrickEnding(TrickAction action);
    
    void ValidateComponents();
    void Cleanup();
    void CheckDirectInput();
    
    // Input state tracking
    bool triggerWasPressed = false;
    bool thumbstickWasActive = false;
);