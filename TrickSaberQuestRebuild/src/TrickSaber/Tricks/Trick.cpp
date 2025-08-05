#include "TrickSaber/Tricks/Trick.hpp"

DEFINE_TYPE(TrickSaber::Tricks, Trick);

using namespace TrickSaber::Tricks;

void Trick::Initialize(TrickSaber::SaberTrickManager* manager, TrickSaber::SaberTrickModel* saberTrickModel) {
    this->manager = manager;
    this->saberTrickModel = saberTrickModel;
    active = false;
}

bool Trick::StartTrick(float value) {
    active = true;
    return true;
}

void Trick::Update() {
    // Base implementation - override in derived classes
}

void Trick::EndTrick() {
    active = false;
}

void Trick::EndTrickImmediately() {
    active = false;
}

bool Trick::IsActive() {
    return active;
}

bool Trick::IsTrickInState(int state) {
    auto trickState = static_cast<TrickSaber::TrickState>(state);
    return active && trickState == TrickSaber::TrickState::Started;
}