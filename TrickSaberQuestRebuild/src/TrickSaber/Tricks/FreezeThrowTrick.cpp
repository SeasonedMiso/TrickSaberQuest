#include "TrickSaber/Tricks/FreezeThrowTrick.hpp"
#include "TrickSaber/SaberTrickManager.hpp"
#include "TrickSaber/Core/TrickSaberManager.hpp"
#include "TrickSaber/Config.hpp"
#include "main.hpp"

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Mathf.hpp"
#include "GlobalNamespace/Saber.hpp"

DEFINE_TYPE(TrickSaber::Tricks, FreezeThrowTrick);

using namespace TrickSaber::Tricks;
using namespace TrickSaber;

void FreezeThrowTrick::Awake() {
    isFrozen = false;
    rigidbody = nullptr;
}

void FreezeThrowTrick::Update() {
    if (active && isFrozen && saberTrickManager && saberTrickManager->saber) {
        // Keep saber frozen in position
        auto transform = saberTrickManager->saber->get_transform();
        transform->set_position(frozenPosition);
        transform->set_rotation(frozenRotation);
        
        // Also freeze rigidbody if present
        if (rigidbody) {
            rigidbody->set_velocity(UnityEngine::Vector3::get_zero());
            rigidbody->set_angularVelocity(UnityEngine::Vector3::get_zero());
        }
    }
}

void FreezeThrowTrick::Initialize(SaberTrickManager* manager) {
    this->manager = manager;
    saberTrickManager = manager;
    active = false;
}

void FreezeThrowTrick::Initialize(SaberTrickManager* manager, SaberTrickModel* saberTrickModel) {
    Trick::Initialize(manager, saberTrickModel);
    saberTrickManager = manager;
}

TrickAction FreezeThrowTrick::GetTrickAction() {
    return TrickAction::FreezeThrow;
}

void FreezeThrowTrick::OnTrickStart() {
    FreezeSaber();
    
    // Apply slowmo if enabled
    if (config.slowmoDuringThrow) {
        auto manager = Core::TrickSaberManager::GetInstance();
        if (manager) {
            manager->ApplySlowmo(config.slowmoAmount);
        }
    }
    
    PaperLogger.debug("Freeze throw trick started at position: ({:.2f}, {:.2f}, {:.2f})", 
        frozenPosition.x, frozenPosition.y, frozenPosition.z);
}

void FreezeThrowTrick::OnTrickEndRequested() {
    active = false;
    StartCoroutine(custom_types::Helpers::CoroutineHelper::New(ReturnSaber()));
}

void FreezeThrowTrick::OnTrickEndImmediately() {
    StopAllCoroutines();
    UnfreezeSaber();
    
    // Remove slowmo
    if (config.slowmoDuringThrow) {
        auto manager = Core::TrickSaberManager::GetInstance();
        if (manager) {
            manager->RemoveSlowmo();
        }
    }
    
    active = false;
    PaperLogger.debug("Freeze throw trick ended immediately");
}

void FreezeThrowTrick::OnInit() {
    PaperLogger.debug("Freeze throw trick initialized");
}

void FreezeThrowTrick::FreezeSaber() {
    if (!saberTrickManager || !saberTrickManager->saber) return;
    
    auto saberGO = saberTrickManager->saber->get_gameObject();
    auto transform = saberGO->get_transform();
    
    // Store frozen position and rotation
    frozenPosition = transform->get_position();
    frozenRotation = transform->get_rotation();
    
    // Get or add rigidbody for physics control
    rigidbody = saberGO->GetComponent<UnityEngine::Rigidbody*>();
    if (!rigidbody) {
        rigidbody = saberGO->AddComponent<UnityEngine::Rigidbody*>();
        rigidbody->set_mass(0.5f);
        rigidbody->set_drag(0.0f);
        rigidbody->set_angularDrag(0.0f);
    }
    
    // Freeze physics
    rigidbody->set_isKinematic(true);
    rigidbody->set_velocity(UnityEngine::Vector3::get_zero());
    rigidbody->set_angularVelocity(UnityEngine::Vector3::get_zero());
    
    isFrozen = true;
}

void FreezeThrowTrick::UnfreezeSaber() {
    isFrozen = false;
    
    if (rigidbody) {
        rigidbody->set_isKinematic(true); // Keep kinematic for normal saber behavior
    }
}

custom_types::Helpers::Coroutine FreezeThrowTrick::ReturnSaber() {
    if (!saberTrickManager || !saberTrickManager->vrController) {
        OnTrickEndImmediately();
        co_return;
    }
    
    // Remove slowmo effect
    if (config.slowmoDuringThrow) {
        auto manager = Core::TrickSaberManager::GetInstance();
        if (manager) {
            manager->RemoveSlowmo();
        }
    }
    
    auto transform = saberTrickManager->saber->get_transform();
    auto startPos = transform->get_position();
    auto targetPos = saberTrickManager->vrController->get_position();
    
    float returnTime = 0.0f;
    const float returnDuration = 1.0f / config.returnSpeed;
    
    // Smooth return to controller
    while (returnTime < returnDuration) {
        if (!saberTrickManager || !saberTrickManager->saber) break;
        
        returnTime += UnityEngine::Time::get_deltaTime();
        float t = returnTime / returnDuration;
        t = t * t * (3.0f - 2.0f * t); // Smoothstep
        
        // Update target position
        if (saberTrickManager->vrController) {
            targetPos = saberTrickManager->vrController->get_position();
        }
        
        auto currentPos = UnityEngine::Vector3::Lerp(startPos, targetPos, t);
        frozenPosition = currentPos; // Update frozen position for Update() method
        
        co_yield nullptr;
    }
    
    UnfreezeSaber();
    active = false;
    PaperLogger.debug("Freeze throw trick return completed");
}