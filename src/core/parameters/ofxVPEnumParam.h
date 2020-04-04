#pragma once

#include "ofxVPIntParam.h"

class ofxVPEnumParam : public ofxVPIntParam {

public:

    //template <typename T>
    ofxVPEnumParam(int _value, string _name) : ofxVPIntParam(_value, _name) {

    }

    virtual ~ofxVPEnumParam(){

    }

    // convert int to float
    //operator float() const { return (float)get(); }

    virtual bool drawGui() {

        ImGui::LabelText( this->getUID().c_str(), "%s = %s", this->getUID().c_str(), serialize().c_str() );
        ImGui::DragInt( getUID().c_str(), &value);
        return true;
    }
};
