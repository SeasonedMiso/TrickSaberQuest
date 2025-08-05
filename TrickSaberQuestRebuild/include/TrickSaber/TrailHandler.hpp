#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "GlobalNamespace/Saber.hpp"

DECLARE_CLASS_CODEGEN(TrickSaber, TrailHandler, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_METHOD(void, Initialize, GlobalNamespace::Saber* saber);
    
public:
    GlobalNamespace::Saber* saber = nullptr;
);