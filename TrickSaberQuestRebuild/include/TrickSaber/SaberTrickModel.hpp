#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Rigidbody.hpp"
#include "GlobalNamespace/Saber.hpp"
#include "GlobalNamespace/SaberModelController.hpp"

DECLARE_CLASS_CODEGEN(TrickSaber, SaberTrickModel, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(GlobalNamespace::Saber*, saber);
    DECLARE_INSTANCE_FIELD(UnityEngine::Transform*, saberTransform);
    DECLARE_INSTANCE_FIELD(UnityEngine::Transform*, originalParent);
    DECLARE_INSTANCE_FIELD(UnityEngine::Vector3, originalPosition);
    DECLARE_INSTANCE_FIELD(UnityEngine::Quaternion, originalRotation);
    DECLARE_INSTANCE_FIELD(bool, usingTrickModel);
    
    // Trick model components (PC parity)
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, trickModel);
    DECLARE_INSTANCE_FIELD(UnityEngine::Transform*, trickModelTransform);
    DECLARE_INSTANCE_FIELD(UnityEngine::Rigidbody*, rigidbody);
    
    DECLARE_INSTANCE_METHOD(void, OnDestroy);
    
public:
    void Initialize(GlobalNamespace::Saber* saber);
    void CopySaberAppearance();
    void AddRigidbody();
    
    bool IsUsingTrickModel();
    void ChangeToActualSaber();
    void ChangeToTrickModel();
    
    UnityEngine::Transform* GetSaberTransform();
    UnityEngine::Transform* GetTrickModelTransform();
    UnityEngine::Rigidbody* GetRigidbody();
    UnityEngine::Vector3 GetOriginalPosition();
    UnityEngine::Quaternion GetOriginalRotation();
    void SetSaberTransform(UnityEngine::Transform* transform);
)