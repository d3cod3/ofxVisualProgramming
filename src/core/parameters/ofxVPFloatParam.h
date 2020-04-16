
#pragma once

#include "ofxVPObjectParameter.h"

#include <type_traits>

// Float param alias
typedef ofxVPObjectParameter<float> ofxVPFloatParam;

// Template specialisation
template <>
void ofxVPObjectParameter<float>::drawGui() {
    ImGui::DragFloat( this->getDisplayName().c_str(), &(this->value));
}

template <>
std::string ofxVPObjectParameter<float>::serialize() const {
    return std::to_string( this->value );
}

template <>
bool ofxVPObjectParameter<float>::unSerialize( const std::string _serialized ) {
    this->value = std::stof(_serialized);
    return true;
}
