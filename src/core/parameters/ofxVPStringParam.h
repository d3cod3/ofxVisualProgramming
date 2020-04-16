#pragma once

#include "ofxVPObjectParameter.h"

// This is not yet official / production ready, let's see what it's worth
#include "imgui_stdlib.h" // ImGui::InPutText with strings.

#include <type_traits>

// Template specialisation
template <>
void ofxVPObjectParameter<std::string>::drawGui() {
    // Note: trying experimental std:string support
    // https://github.com/ocornut/imgui/tree/master/misc/cpp
    // Otherwise there would have to be a limited char buffer.
    ImGui::InputText(this->getDisplayName().c_str(), &(this->value) );
}

template <>
std::string ofxVPObjectParameter< std::string >::serialize() const {
    return this->value;
}

template <>
bool ofxVPObjectParameter< std::string >::unSerialize( const std::string _serialized ) {
    this->value = _serialized;
    return true;
}
