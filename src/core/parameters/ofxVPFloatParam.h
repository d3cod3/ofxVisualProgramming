
#pragma once

#include "ofxVPObjectParameter.h"

#include <type_traits>

// Int param alias
typedef ofxVPObjectParameter<float> ofxVPFloatParam;

// Template specialisation
template <>
void ofxVPObjectParameter<float>::drawGui() {
    ImGui::DragFloat( this->getDisplayName().c_str(), &(this->value));
}
