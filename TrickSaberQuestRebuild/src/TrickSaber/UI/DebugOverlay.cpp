#include "TrickSaber/UI/DebugOverlay.hpp"
#include "TrickSaber/Configuration.hpp"
#include "TrickSaber/GlobalTrickManager.hpp"
#include "TrickSaber/Constants.hpp"
#include "TrickSaber/MovementController.hpp"
#include "TrickSaber/Core/StateManager.hpp"
#include "GlobalNamespace/SaberManager.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/Application.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "main.hpp"

DEFINE_TYPE(TrickSaber::UI, DebugOverlay);

namespace TrickSaber::UI {
    DebugStats currentStats;
    TrickSaber::UI::DebugOverlay* DebugOverlay::instance = nullptr;
    
    void DebugOverlay::Awake() {
        updateTimer = 0.0f;
        instance = this;
        
        // Create canvas
        auto* canvasGO = UnityEngine::GameObject::New_ctor();
        canvasGO->get_transform()->SetParent(get_transform(), false);
        canvas = canvasGO->AddComponent<UnityEngine::Canvas*>();
        canvas->set_sortingOrder(100);
        
        // Create debug text
        debugText = BSML::Lite::CreateText(canvas->get_transform(), 
            "Debug Stats Loading...", 
            UnityEngine::Vector2(10, -10));
        
        auto rectTransform = debugText->get_rectTransform().ptr();
        rectTransform->set_anchorMin(UnityEngine::Vector2(0, 1));
        rectTransform->set_anchorMax(UnityEngine::Vector2(0, 1));
        rectTransform->set_pivot(UnityEngine::Vector2(0, 1));
        
        debugText->set_fontSize(12.0f);
        debugText->set_color(UnityEngine::Color::get_green());
        
        SetVisible(TrickSaber::config.showDebugOverlay);
    }
    
    void DebugOverlay::Update() {
        if (!canvas || !canvas->get_enabled()) return;
        
        updateTimer += UnityEngine::Time::get_deltaTime();
        if (updateTimer >= 0.5f) {
            UpdateStats();
            updateTimer = 0.0f;
        }
    }
    
    void DebugOverlay::UpdateStats() {
        if (!debugText) return;
        
        UpdateDebugStats();
        
        float leftVelMag = currentStats.leftSaberVel.get_magnitude();
        float rightVelMag = currentStats.rightSaberVel.get_magnitude();
        
        std::string statsText = fmt::format(
            "<color=#00ff00>TrickSaber Debug</color>\n"
            "<color=#ffff00>FPS:</color> {:.1f} <color=#aaaaaa>Tricks:</color> {}\n"
            "<color=#ff8800>Throws:</color> {} <color=#8888ff>Spins:</color> {}\n"
            "\n<color=#88ff88>Left Saber</color>\n"
            "Pos: {:.2f},{:.2f},{:.2f}\n"
            "Vel: {:.2f},{:.2f},{:.2f} |{:.2f}|\n"
            "RotVel: {:.1f}°/s\n"
            "\n<color=#ff8888>Right Saber</color>\n"
            "Pos: {:.2f},{:.2f},{:.2f}\n"
            "Vel: {:.2f},{:.2f},{:.2f} |{:.2f}|\n"
            "RotVel: {:.1f}°/s",
            currentStats.fps,
            currentStats.anyTrickActive ? "<color=#00ff00>ON</color>" : "<color=#666666>OFF</color>",
            currentStats.activeThrows,
            currentStats.activeSpins,
            currentStats.leftSaberPos.x, currentStats.leftSaberPos.y, currentStats.leftSaberPos.z,
            currentStats.leftSaberVel.x, currentStats.leftSaberVel.y, currentStats.leftSaberVel.z, leftVelMag,
            currentStats.leftSaberAngularVel,
            currentStats.rightSaberPos.x, currentStats.rightSaberPos.y, currentStats.rightSaberPos.z,
            currentStats.rightSaberVel.x, currentStats.rightSaberVel.y, currentStats.rightSaberVel.z, rightVelMag,
            currentStats.rightSaberAngularVel
        );
        
        debugText->set_text(statsText);
    }
    
    void DebugOverlay::SetVisible(bool visible) {
        if (canvas) {
            canvas->set_enabled(visible);
        }
    }
    
    void DebugOverlay::CreateOverlay() {
        if (instance) return;
        
        PaperLogger.debug("Creating DebugOverlay on demand");
        auto* go = UnityEngine::GameObject::New_ctor("TrickSaberDebugOverlay");
        UnityEngine::Object::DontDestroyOnLoad(go);
        go->AddComponent<TrickSaber::UI::DebugOverlay*>();
    }
    
    bool DebugOverlay::IsOverlayCreated() {
        return instance != nullptr;
    }
    
    void DebugOverlay::DestroyOverlay() {
        if (instance && instance->get_gameObject()) {
            UnityEngine::Object::Destroy(instance->get_gameObject());
            instance = nullptr;
        }
    }
    
    void DebugOverlay::Toggle() {
        TrickSaber::config.showDebugOverlay = !TrickSaber::config.showDebugOverlay;
        if (instance) {
            instance->SetVisible(TrickSaber::config.showDebugOverlay);
        } else if (TrickSaber::config.showDebugOverlay) {
            CreateOverlay();
        }
        TrickSaber::SaveConfig();
    }
    
    void UpdateDebugStats() {
        currentStats.fps = 1.0f / UnityEngine::Time::get_deltaTime();
        currentStats.tricksEnabled = Configuration::IsModEnabled();
        
        // Get active trick counts from GlobalTrickManager
        if (auto* manager = GlobalTrickManager::GetInstance()) {
            currentStats.activeThrows = manager->GetActiveThrowCount();
            currentStats.activeSpins = manager->GetActiveSpinCount();
            currentStats.anyTrickActive = manager->IsDoingTrick();
        }
        
        // Get saber data from MovementController
        auto leftVel = TrickSaber::MovementController::GetVelocity(false);
        auto rightVel = TrickSaber::MovementController::GetVelocity(true);
        auto leftAngVel = TrickSaber::MovementController::GetAngularVelocity(false);
        auto rightAngVel = TrickSaber::MovementController::GetAngularVelocity(true);
        
        currentStats.leftSaberVel = leftVel;
        currentStats.rightSaberVel = rightVel;
        currentStats.leftSaberAngularVel = leftAngVel * TrickSaber::Constants::RAD_TO_DEG;
        currentStats.rightSaberAngularVel = rightAngVel * TrickSaber::Constants::RAD_TO_DEG;
        
        // Get saber positions from StateManager
        auto stateManager = TrickSaber::Core::StateManager::GetInstance();
        auto saberManager = stateManager->GetSaberManager();
        if (saberManager) {
            auto leftSaber = saberManager->get_leftSaber();
            if (leftSaber) {
                currentStats.leftSaberPos = leftSaber->get_transform()->get_position();
            }
            auto rightSaber = saberManager->get_rightSaber();
            if (rightSaber) {
                currentStats.rightSaberPos = rightSaber->get_transform()->get_position();
            }
        }
        
        // Log detailed data only when tricks are active
        if (currentStats.anyTrickActive) {
            float leftVelMag = currentStats.leftSaberVel.get_magnitude();
            float rightVelMag = currentStats.rightSaberVel.get_magnitude();
            
            PaperLogger.debug("[TRICK_ACTIVE] L_Pos({:.2f},{:.2f},{:.2f}) L_Vel({:.2f},{:.2f},{:.2f})|{:.2f}| L_RotVel({:.1f}°/s) R_Pos({:.2f},{:.2f},{:.2f}) R_Vel({:.2f},{:.2f},{:.2f})|{:.2f}| R_RotVel({:.1f}°/s)",
                currentStats.leftSaberPos.x, currentStats.leftSaberPos.y, currentStats.leftSaberPos.z,
                currentStats.leftSaberVel.x, currentStats.leftSaberVel.y, currentStats.leftSaberVel.z, leftVelMag,
                currentStats.leftSaberAngularVel,
                currentStats.rightSaberPos.x, currentStats.rightSaberPos.y, currentStats.rightSaberPos.z,
                currentStats.rightSaberVel.x, currentStats.rightSaberVel.y, currentStats.rightSaberVel.z, rightVelMag,
                currentStats.rightSaberAngularVel
            );
        }
    }
    
    // LazyDebugOverlayCreator implementation
    Utils::LazyComponentInitializer LazyDebugOverlayCreator::overlayCreator([]() {
        if (TrickSaber::config.showDebugOverlay) {
            PaperLogger.debug("Lazy creating DebugOverlay");
            DebugOverlay::CreateOverlay();
        }
    });
    
    void LazyDebugOverlayCreator::CreateOverlayIfNeeded() {
        overlayCreator.Initialize();
    }
    
    bool LazyDebugOverlayCreator::IsCreated() {
        return overlayCreator.IsInitialized();
    }
}