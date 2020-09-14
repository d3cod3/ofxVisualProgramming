#pragma once

#include "ofxVPObjectParameter.h"

#include <type_traits>

// Int param alias
typedef ofxVPObjectParameter<bool> ofxVPBoolParam;

// Template specialisation
template <>
void ofxVPObjectParameter<bool>::drawGui() {
    ImGui::Checkbox( getDisplayName().c_str(), &(this->value) );
}

template <>
std::string ofxVPObjectParameter<bool>::serialize() const {
    return this->value?"1":"0";
}

template <>
bool ofxVPObjectParameter<bool>::unSerialize( const std::string _serialized ) {
    this->value = (_serialized=="1") ? true : false;
    return true;
}
