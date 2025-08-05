#include "TrickSaber/Input/InputHandler.hpp"
#include "GlobalNamespace/OVRInput.hpp"
#include "UnityEngine/XR/InputTracking.hpp"
#include <cmath>

using namespace TrickSaber::Input;
using namespace GlobalNamespace;

TriggerHandler::TriggerHandler(UnityEngine::XR::XRNode node, float threshold) 
    : node(node), threshold(threshold), wasPressed(false) {}

bool TriggerHandler::IsActivated(float& value) {
    auto controller = (node == UnityEngine::XR::XRNode::LeftHand) ? 
        OVRInput::Controller::LTouch : OVRInput::Controller::RTouch;
    
    value = OVRInput::Get(OVRInput::Axis1D::PrimaryIndexTrigger, controller);
    bool isPressed = value >= threshold;
    
    if (isPressed && !wasPressed) {
        wasPressed = true;
        return true;
    }
    wasPressed = isPressed;
    return false;
}

bool TriggerHandler::IsDeactivated() {
    float value;
    auto controller = (node == UnityEngine::XR::XRNode::LeftHand) ? 
        OVRInput::Controller::LTouch : OVRInput::Controller::RTouch;
    
    value = OVRInput::Get(OVRInput::Axis1D::PrimaryIndexTrigger, controller);
    bool isPressed = value >= threshold;
    
    if (!isPressed && wasPressed) {
        wasPressed = false;
        return true;
    }
    wasPressed = isPressed;
    return false;
}

ThumbstickHandler::ThumbstickHandler(UnityEngine::XR::XRNode node, float threshold, ThumbstickDir direction)
    : node(node), threshold(threshold), direction(direction), wasPressed(false) {}

bool ThumbstickHandler::IsActivated(float& value) {
    auto controller = (node == UnityEngine::XR::XRNode::LeftHand) ? 
        OVRInput::Controller::LTouch : OVRInput::Controller::RTouch;
    
    auto stick = OVRInput::Get(OVRInput::Axis2D::PrimaryThumbstick, controller);
    
    if (direction == ThumbstickDir::Horizontal) {
        value = stick.x;
    } else {
        value = stick.y;
    }
    
    bool isPressed = std::abs(value) >= threshold;
    
    if (isPressed && !wasPressed) {
        wasPressed = true;
        return true;
    }
    wasPressed = isPressed;
    return false;
}

bool ThumbstickHandler::IsDeactivated() {
    auto controller = (node == UnityEngine::XR::XRNode::LeftHand) ? 
        OVRInput::Controller::LTouch : OVRInput::Controller::RTouch;
    
    auto stick = OVRInput::Get(OVRInput::Axis2D::PrimaryThumbstick, controller);
    
    float value = (direction == ThumbstickDir::Horizontal) ? stick.x : stick.y;
    bool isPressed = std::abs(value) >= threshold;
    
    if (!isPressed && wasPressed) {
        wasPressed = false;
        return true;
    }
    wasPressed = isPressed;
    return false;
}

GripHandler::GripHandler(GlobalNamespace::OVRInput::Controller controller, float threshold)
    : controller(controller), threshold(threshold), wasPressed(false) {}

bool GripHandler::IsActivated(float& value) {
    value = OVRInput::Get(OVRInput::Axis1D::PrimaryHandTrigger, controller);
    bool isPressed = value >= threshold;
    
    if (isPressed && !wasPressed) {
        wasPressed = true;
        return true;
    }
    wasPressed = isPressed;
    return false;
}

bool GripHandler::IsDeactivated() {
    float value = OVRInput::Get(OVRInput::Axis1D::PrimaryHandTrigger, controller);
    bool isPressed = value >= threshold;
    
    if (!isPressed && wasPressed) {
        wasPressed = false;
        return true;
    }
    wasPressed = isPressed;
    return false;
}