#include "TrickSaber/TrickManager.hpp"
#include "TrickSaber/SaberTrickManager.hpp"
#include "main.hpp"

#include "UnityEngine/GameObject.hpp"
#include "GlobalNamespace/SaberType.hpp"

using namespace TrickSaber;
using namespace GlobalNamespace;

SaberTrickManager* TrickManager::leftSaberTrickManager = nullptr;
SaberTrickManager* TrickManager::rightSaberTrickManager = nullptr;

void TrickManager::Initialize(Saber* leftSaber, Saber* rightSaber) {
    Logger.info("Initializing TrickManager");
    
    if (leftSaber) {
        auto leftGameObject = leftSaber->get_gameObject();
        leftSaberTrickManager = leftGameObject->AddComponent<SaberTrickManager*>();
        leftSaberTrickManager->Initialize(leftSaber);
    }
    
    if (rightSaber) {
        auto rightGameObject = rightSaber->get_gameObject();
        rightSaberTrickManager = rightGameObject->AddComponent<SaberTrickManager*>();
        rightSaberTrickManager->Initialize(rightSaber);
    }
    
    Logger.info("TrickManager initialized successfully");
}

void TrickManager::Cleanup() {
    if (leftSaberTrickManager) {
        leftSaberTrickManager->EndAllTricks();
        leftSaberTrickManager = nullptr;
    }
    
    if (rightSaberTrickManager) {
        rightSaberTrickManager->EndAllTricks();
        rightSaberTrickManager = nullptr;
    }
}

bool TrickManager::CanDoTrick() {
    return config.trickSaberEnabled;
}

void TrickManager::OnTrickStarted(const TrickAction action) {
    Logger.debug("Trick started: {}", static_cast<int>(action));
}

void TrickManager::OnTrickEnded(const TrickAction action) {
    Logger.debug("Trick ended: {}", static_cast<int>(action));
}