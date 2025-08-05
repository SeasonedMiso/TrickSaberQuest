#pragma once

#include "TrickSaber/Tricks/Trick.hpp"
#include "TrickSaber/MovementController.hpp"
#include "TrickSaber/SaberTrickModel.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Rigidbody.hpp"
#include "custom-types/shared/coroutine.hpp"

DECLARE_CLASS_CODEGEN(TrickSaber::Tricks, FreezeThrowTrick, TrickSaber::Tricks::Trick,
    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, Update);
    
public:
    void Initialize(TrickSaber::SaberTrickManager* manager);
    void Initialize(TrickSaber::SaberTrickManager* manager, TrickSaber::SaberTrickModel* saberTrickModel);
    TrickSaber::TrickAction GetTrickAction();
    void OnTrickStart();
    void OnTrickEndRequested();
    void OnTrickEndImmediately();
    void OnInit();
    
private:
    TrickSaber::SaberTrickManager* saberTrickManager = nullptr;
    UnityEngine::Vector3 frozenPosition;
    UnityEngine::Quaternion frozenRotation;
    UnityEngine::Rigidbody* rigidbody = nullptr;
    bool isFrozen = false;
    
    void FreezeSaber();
    void UnfreezeSaber();
    custom_types::Helpers::Coroutine ReturnSaber();
);