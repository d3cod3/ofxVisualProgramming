#pragma once

#include "ofxVPBaseParameter.h"

#include <type_traits>

// Int param alias
//typedef ofxVPParam<int> ofxVPIntParam;
//ofxVPParam::ofxVPParam

class ofxVPIntParameter : public ofxVPBaseParameter<int> {

public:

    //template <typename T>
    ofxVPIntParameter(int _value, std::string _name) : ofxVPBaseParameter<int>(_value, _name) {

    }

    virtual ~ofxVPIntParameter(){

    }

    virtual bool drawGui() {

        //ImGui::LabelText( this->getUID().c_str(), "%s = %s", this->getUID().c_str(), serialize().c_str() );
        ImGui::DragInt( getUID().c_str(), &value);
        return true;
    }
};
