#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UnityEngine/Canvas.hpp"
#include "TrickSaber/Utils/LazyInitializer.hpp"

DECLARE_CLASS_CODEGEN(TrickSaber::UI, DebugOverlay, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, debugText);
    DECLARE_INSTANCE_FIELD(UnityEngine::Canvas*, canvas);
    DECLARE_INSTANCE_FIELD(float, updateTimer);
    
    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, UpdateStats);
    DECLARE_INSTANCE_METHOD(void, SetVisible, bool visible);
    
    public:
        static TrickSaber::UI::DebugOverlay* instance;
        static void CreateOverlay();
        static void DestroyOverlay();
        static void Toggle();
        static bool IsOverlayCreated();
)

namespace TrickSaber::UI {
    struct DebugStats {
        float fps = 0.0f;
        int activeThrows = 0;
        int activeSpins = 0;
        float memoryUsage = 0.0f;
        bool tricksEnabled = false;
        int noteCount = 0;
        
        // Left saber data
        UnityEngine::Vector3 leftSaberPos = UnityEngine::Vector3::get_zero();
        UnityEngine::Vector3 leftSaberVel = UnityEngine::Vector3::get_zero();
        float leftSaberAngularVel = 0.0f;
        
        // Right saber data
        UnityEngine::Vector3 rightSaberPos = UnityEngine::Vector3::get_zero();
        UnityEngine::Vector3 rightSaberVel = UnityEngine::Vector3::get_zero();
        float rightSaberAngularVel = 0.0f;
        
        bool anyTrickActive = false;
    };
    
    extern DebugStats currentStats;
    void UpdateDebugStats();
    
    // Lazy debug overlay initializer
    class LazyDebugOverlayCreator {
    private:
        static Utils::LazyComponentInitializer overlayCreator;
        
    public:
        static void CreateOverlayIfNeeded();
        static bool IsCreated();
    };
}