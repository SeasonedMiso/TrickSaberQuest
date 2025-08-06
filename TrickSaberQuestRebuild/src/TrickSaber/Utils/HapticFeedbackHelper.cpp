#include "TrickSaber/Utils/HapticFeedbackHelper.hpp"
#include "TrickSaber/Utils/ObjectCache.hpp"
#include "TrickSaber/Config.hpp"
#include "UnityEngine/XR/XRNode.hpp"
#include "main.hpp"

using namespace TrickSaber::Utils;

void HapticFeedbackHelper::TriggerHaptic(GlobalNamespace::SaberType saberType, HapticType type) {
    auto params = GetHapticParams(type);
    TriggerHaptic(saberType, params.duration, params.strength);
}

void HapticFeedbackHelper::TriggerHaptic(GlobalNamespace::SaberType saberType, float duration, float strength) {
    // Use cached haptic controller for performance
    auto hapticManager = ObjectCache::GetHapticController();
    if (!hapticManager || !hapticManager->hapticFeedbackEnabled) {
        // Fallback to FindObjectOfType if cache miss
        hapticManager = UnityEngine::Object::FindObjectOfType<GlobalNamespace::HapticFeedbackManager*>();
        if (!hapticManager || !hapticManager->hapticFeedbackEnabled) {
            return;
        }
    }
    
    // Apply config intensity multiplier
    float finalStrength = strength * config.hapticIntensity;
    if (finalStrength <= 0.0f) return;
    
    UnityEngine::XR::XRNode node = (saberType == GlobalNamespace::SaberType::SaberA) ? 
        UnityEngine::XR::XRNode::LeftHand : UnityEngine::XR::XRNode::RightHand;
    
    try {
        hapticManager->PlayHapticFeedback(node, nullptr);
    } catch (const std::runtime_error& e) {
        Logger.debug("Runtime error playing haptic feedback: {}", e.what());
    } catch (const std::exception& e) {
        Logger.debug("Exception playing haptic feedback: {}", e.what());
    } catch (...) {
        // Silently fail for unknown exceptions
    }
}

HapticFeedbackHelper::HapticParams HapticFeedbackHelper::GetHapticParams(HapticType type) {
    switch (type) {
        case HapticType::TrickStart:
            return {0.1f, 0.8f};
        case HapticType::TrickEnd:
            return {0.05f, 0.6f};
        case HapticType::SaberReturn:
            return {0.15f, 1.0f};
        case HapticType::SpinStart:
            return {0.08f, 0.4f};
        case HapticType::SpinEnd:
            return {0.05f, 0.3f};
        default:
            return {0.1f, 0.5f};
    }
}