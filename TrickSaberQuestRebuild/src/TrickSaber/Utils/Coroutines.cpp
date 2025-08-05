#include "TrickSaber/Utils/Coroutines.hpp"
#include "TrickSaber/Core/TrickSaberManager.hpp"
#include "UnityEngine/Vector3.hpp"

using namespace TrickSaber::Utils;
using namespace custom_types::Helpers;

Coroutine TrickSaber::Utils::LerpToOriginalRotation(UnityEngine::Transform* transform) {
    auto rot = transform->get_localRotation();
    auto identity = UnityEngine::Quaternion::get_identity();
    
    while (UnityEngine::Quaternion::Angle(rot, identity) > 5.0f) {
        rot = UnityEngine::Quaternion::Lerp(rot, identity, UnityEngine::Time::get_deltaTime() * 20.0f);
        transform->set_localRotation(rot);
        co_yield reinterpret_cast<System::Collections::IEnumerator*>(UnityEngine::WaitForEndOfFrame::New_ctor());
    }
    
    transform->set_localRotation(identity);
}

Coroutine TrickSaber::Utils::CompleteRotation(UnityEngine::Transform* transform, float finalSpeed) {
    auto minSpeed = 8.0f;
    auto speed = std::abs(finalSpeed) < minSpeed ? (finalSpeed < 0 ? -minSpeed : minSpeed) : finalSpeed;
    auto threshold = std::abs(finalSpeed) + 0.1f;
    auto identity = UnityEngine::Quaternion::get_identity();
    
    auto angle = UnityEngine::Quaternion::Angle(transform->get_localRotation(), identity);
    
    while (angle > threshold) {
        auto rotationVector = UnityEngine::Vector3::get_right();
        rotationVector = UnityEngine::Vector3::op_Multiply(rotationVector, speed);
        transform->Rotate(rotationVector);
        angle = UnityEngine::Quaternion::Angle(transform->get_localRotation(), identity);
        co_yield reinterpret_cast<System::Collections::IEnumerator*>(UnityEngine::WaitForEndOfFrame::New_ctor());
    }
    
    transform->set_localRotation(identity);
}

Coroutine TrickSaber::Utils::SmoothSlowmo(float from, float to, float step) {
    auto current = from;
    auto manager = TrickSaber::Core::TrickSaberManager::GetInstance();
    if (!manager) co_return;
    
    if (from > to) {
        while (current > to) {
            current -= step;
            manager->SetTimeScale(current);
            co_yield reinterpret_cast<System::Collections::IEnumerator*>(UnityEngine::WaitForEndOfFrame::New_ctor());
        }
    } else {
        while (current < to) {
            current += step;
            manager->SetTimeScale(current);
            co_yield reinterpret_cast<System::Collections::IEnumerator*>(UnityEngine::WaitForEndOfFrame::New_ctor());
        }
    }
    
    manager->SetTimeScale(to);
}