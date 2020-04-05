
#pragma once

#include "ofxVPObjectParameter.h"

#include <type_traits>

// Int param alias
typedef ofxVPObjectParameter<int> ofxVPIntParam;

// Template specialisation
template <>
void ofxVPObjectParameter<int>::drawGui() {
    ImGui::DragInt( getDisplayName().c_str(), &(this->value));
}
