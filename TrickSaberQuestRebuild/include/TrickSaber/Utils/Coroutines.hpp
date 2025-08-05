#pragma once

#include "custom-types/shared/coroutine.hpp"
#include "UnityEngine/WaitForEndOfFrame.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Quaternion.hpp"

namespace TrickSaber::Utils {
    custom_types::Helpers::Coroutine LerpToOriginalRotation(UnityEngine::Transform* transform);
    custom_types::Helpers::Coroutine CompleteRotation(UnityEngine::Transform* transform, float finalSpeed);
    custom_types::Helpers::Coroutine SmoothSlowmo(float from, float to, float step);
}