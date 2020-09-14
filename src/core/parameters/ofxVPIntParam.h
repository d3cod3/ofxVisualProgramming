
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

template <>
std::string ofxVPObjectParameter<int>::serialize() const {
    return std::to_string( this->value );
}

template <>
bool ofxVPObjectParameter<int>::unSerialize( const std::string _serialized ) {
    this->value = std::stoi(_serialized);
    return true;
}
