#include "UnityEngine/GameObject.hpp"
#include "TrickSaber/EnhancedSaberManager.hpp"
#include "TrickSaber/AdvancedInputSystem.hpp"
#include "TrickSaber/AdvancedTrickFeatures.hpp"
#include "TrickSaber/PhysicsHandler.hpp"

// Template instantiations for custom types
template TrickSaber::EnhancedSaberManager* UnityEngine::GameObject::AddComponent<TrickSaber::EnhancedSaberManager*>();
template TrickSaber::AdvancedInputSystem* UnityEngine::GameObject::AddComponent<TrickSaber::AdvancedInputSystem*>();
template TrickSaber::AdvancedTrickFeatures* UnityEngine::GameObject::AddComponent<TrickSaber::AdvancedTrickFeatures*>();
template TrickSaber::PhysicsHandler* UnityEngine::GameObject::AddComponent<TrickSaber::PhysicsHandler*>();