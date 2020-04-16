#pragma once

#include "ofxVPObjectParameter.h"

#include <type_traits>

// Int param alias
typedef ofxVPObjectParameter<ofFloatColor> ofxVPColorParam;

// Template specialisation
template <>
void ofxVPObjectParameter<ofFloatColor>::drawGui() {
    ImGui::ColorEdit4( getDisplayName().c_str(), this->value.v );
}

template <>
std::string ofxVPObjectParameter<ofFloatColor>::serialize() const {
    // ofColor_ has native serialisation
    return ofxVPObjectParameter::serialize();
}

template <>
bool ofxVPObjectParameter<ofFloatColor>::unSerialize( const std::string _serialized ) {
    // ofColor_ has native serialisation
    std::istringstream in(_serialized);
    in >> this->value;
    return true;
}
