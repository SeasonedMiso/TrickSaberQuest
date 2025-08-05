#pragma once

#include "GlobalNamespace/Saber.hpp"
#include "TrickSaber/SaberTrickManager.hpp"

namespace TrickSaber {
    class TrickManager {
    public:
        static void Initialize(GlobalNamespace::Saber* leftSaber, GlobalNamespace::Saber* rightSaber);
        static void Cleanup();
        
        static SaberTrickManager* leftSaberTrickManager;
        static SaberTrickManager* rightSaberTrickManager;
        
        static bool CanDoTrick();
        static void OnTrickStarted(TrickAction action);
        static void OnTrickEnded(TrickAction action);
    };
}