#pragma once

#include "GlobalNamespace/VRController.hpp"
#include "GlobalNamespace/OVRInput.hpp"
#include "UnityEngine/XR/XRNode.hpp"
#include "TrickSaber/Enums.hpp"

namespace TrickSaber::Input {
    class InputHandler {
    public:
        virtual ~InputHandler() = default;
        virtual bool IsActivated(float& value) = 0;
        virtual bool IsDeactivated() = 0;
    };
    
    class TriggerHandler : public InputHandler {
    private:
        UnityEngine::XR::XRNode node;
        float threshold;
        bool wasPressed = false;
        
    public:
        TriggerHandler(UnityEngine::XR::XRNode node, float threshold);
        bool IsActivated(float& value) override;
        bool IsDeactivated() override;
    };
    
    class ThumbstickHandler : public InputHandler {
    private:
        UnityEngine::XR::XRNode node;
        float threshold;
        ThumbstickDir direction;
        bool wasPressed = false;
        
    public:
        ThumbstickHandler(UnityEngine::XR::XRNode node, float threshold, ThumbstickDir direction);
        bool IsActivated(float& value) override;
        bool IsDeactivated() override;
    };
    
    class GripHandler : public InputHandler {
    private:
        GlobalNamespace::OVRInput::Controller controller;
        float threshold;
        bool wasPressed = false;
        
    public:
        GripHandler(GlobalNamespace::OVRInput::Controller controller, float threshold);
        bool IsActivated(float& value) override;
        bool IsDeactivated() override;
    };
}