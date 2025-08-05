#include "TrickSaber/SaberTrickModel.hpp"
#include "TrickSaber/Config.hpp"
#include "UnityEngine/Rigidbody.hpp"
#include "UnityEngine/RigidbodyInterpolation.hpp"
#include "UnityEngine/ForceMode.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/MeshFilter.hpp"
#include "main.hpp"

DEFINE_TYPE(TrickSaber, SaberTrickModel);

using namespace TrickSaber;

void SaberTrickModel::Initialize(GlobalNamespace::Saber* saber) {
    this->saber = saber;
    if (saber) {
        saberTransform = saber->get_transform();
        if (saberTransform) {
            originalParent = saberTransform->get_parent();
            originalPosition = saberTransform->get_localPosition();
            originalRotation = saberTransform->get_localRotation();
            
            // Create trick model GameObject (PC parity)
            trickModel = UnityEngine::GameObject::New_ctor("TrickModel");
            trickModelTransform = trickModel->get_transform();
            
            // Copy saber appearance to trick model
            CopySaberAppearance();
            
            // Add rigidbody for physics (PC parity)
            AddRigidbody();
            
            // Initially disable trick model
            trickModel->SetActive(false);
            
            PaperLogger.debug("SaberTrickModel initialized with physics for {} saber", 
                saber->get_saberType() == GlobalNamespace::SaberType::SaberA ? "left" : "right");
        } else {
            PaperLogger.error("Failed to get saber transform in SaberTrickModel::Initialize");
        }
    }
    usingTrickModel = false;
}

bool SaberTrickModel::IsUsingTrickModel() {
    return usingTrickModel;
}

void SaberTrickModel::ChangeToActualSaber() {
    if (!saberTransform || !originalParent) return;
    
    // Restore original saber
    if (saber) {
        auto saberModelController = saber->GetComponentInChildren<GlobalNamespace::SaberModelController*>();
        if (saberModelController) {
            saberModelController->get_gameObject()->SetActive(true);
        }
    }
    
    // Disable trick model
    if (trickModel) {
        trickModel->SetActive(false);
    }
    
    // Reset saber transform
    saberTransform->SetParent(originalParent, false);
    saberTransform->set_localPosition(originalPosition);
    saberTransform->set_localRotation(originalRotation);
    
    usingTrickModel = false;
    PaperLogger.debug("Switched to actual saber");
}

void SaberTrickModel::ChangeToTrickModel() {
    if (!trickModel || !saberTransform) return;
    
    // Position trick model at saber location
    trickModelTransform->set_position(saberTransform->get_position());
    trickModelTransform->set_rotation(saberTransform->get_rotation());
    
    // Enable trick model, disable original
    trickModel->SetActive(true);
    if (saber) {
        auto saberModelController = saber->GetComponentInChildren<GlobalNamespace::SaberModelController*>();
        if (saberModelController) {
            saberModelController->get_gameObject()->SetActive(false);
        }
    }
    
    usingTrickModel = true;
    PaperLogger.debug("Switched to trick model");
}

UnityEngine::Transform* SaberTrickModel::GetSaberTransform() {
    return usingTrickModel ? trickModelTransform : saberTransform;
}

UnityEngine::Vector3 SaberTrickModel::GetOriginalPosition() {
    return originalPosition;
}

UnityEngine::Quaternion SaberTrickModel::GetOriginalRotation() {
    return originalRotation;
}

void SaberTrickModel::SetSaberTransform(UnityEngine::Transform* transform) {
    saberTransform = transform;
}

void SaberTrickModel::CopySaberAppearance() {
    if (!saber || !trickModel) return;
    
    // Find saber model components
    auto saberModelController = saber->GetComponentInChildren<GlobalNamespace::SaberModelController*>();
    if (saberModelController) {
        auto meshRenderer = saberModelController->GetComponentInChildren<UnityEngine::MeshRenderer*>();
        auto meshFilter = saberModelController->GetComponentInChildren<UnityEngine::MeshFilter*>();
        
        if (meshRenderer && meshFilter) {
            // Copy mesh and materials to trick model
            auto trickMeshFilter = trickModel->AddComponent<UnityEngine::MeshFilter*>();
            auto trickMeshRenderer = trickModel->AddComponent<UnityEngine::MeshRenderer*>();
            
            if (trickMeshFilter && trickMeshRenderer) {
                trickMeshFilter->set_mesh(meshFilter->get_mesh());
                // Copy materials with proper type conversion
                auto materials = meshRenderer->get_materials();
                if (materials && materials->get_Length() > 0) {
                    trickMeshRenderer->set_material(materials->_values[0]);
                }
                PaperLogger.debug("Copied saber appearance to trick model");
            }
        }
    }
}

void SaberTrickModel::AddRigidbody() {
    if (!trickModel) return;
    
    rigidbody = trickModel->AddComponent<UnityEngine::Rigidbody*>();
    if (rigidbody) {
        rigidbody->set_useGravity(false);
        rigidbody->set_isKinematic(true);
        rigidbody->set_detectCollisions(false);
        rigidbody->set_maxAngularVelocity(800.0f);
        rigidbody->set_interpolation(UnityEngine::RigidbodyInterpolation::Interpolate);
        rigidbody->set_mass(0.1f); // Light weight for responsive physics
        
        PaperLogger.debug("Rigidbody added to trick model with PC parity settings");
    }
}

UnityEngine::Transform* SaberTrickModel::GetTrickModelTransform() {
    return trickModelTransform;
}

UnityEngine::Rigidbody* SaberTrickModel::GetRigidbody() {
    return rigidbody;
}

void SaberTrickModel::OnDestroy() {
    if (trickModel) {
        UnityEngine::Object::DestroyImmediate(trickModel);
    }
}