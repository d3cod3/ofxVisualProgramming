//
//  ofxVPObjectParameter.h
//  ofxVisualProgramming
//
//  Created by Daan on 16/03/2020.
//
//

#pragma once

#include "ofxVPBaseParameter.h"
#include "ofxXmlSettings.h"
//#include "<sstream>"
#include "sstream"

#ifdef DEBUG
    #include "ofLog.h"
#endif

//#include "ofMain.h"
//#include "ofxImGui.h"
#include <type_traits> // needed for std::is_same

template <typename T>
class ofxVPObjectParameter : public ofxVPBaseParameter<T> {

public:
    //template <typename T>
    ofxVPObjectParameter(T _value, string _name) : ofxVPBaseParameter<T>(_value, _name) {

    }

    // writes the ofxVPBaseParameter data to XML.
    // The xml's cursor is already pushed into the right <param> tag.
    virtual void saveTo( ofxXmlSettings& _xmlHandle) const {
        // ensure tags are there
        _xmlHandle.addValue(OFXVP_PARAM_XML_TAG_VALUE, this->serialize() );
    }

    // load ofxVPBaseParameter from xml
    // xml's cursor is pushed to the root of the <param> tag to load
    bool loadFrom(const ofxXmlSettings& _xmlHandle){

        //effectName = _xmlHandle.getValue("paramName", getType() );

        return true;
    }

    // Global method for saving the data. Please override me.
    virtual std::string serialize() const {
        ostringstream out;
        out << &this->value;
        return out.str();
    }
    // Global method for decoding the data. Please override me.
    // Returns true if successfully parsed the value.
    virtual bool unSerialize( const std::string _serialized ){
#ifdef DEBUG
        ofLogError("ofxVPBaseParameter", "Unserialize() is unimplemented, please implement it for your specific data type before using it.");
#endif
        // set empty value ?
        //this->value = new T(_serialized);
        return false;
    }

    // Override this function with your own types.
    // Called to render the param into an ImGui view.
    virtual void drawGui() {
        // Try to guess the type, to enable some editing possibilities.
        if( false && std::is_same<T, int>::value ){
            // Line below doesn't compile.. todo
            //ImGui::DragInt( this->getDisplayName().c_str(), (T&) (this->value) );
        }
        // Fallback behaviour for any type, so the serialized version is displayed without editing capabilities.
        else {
            ImGui::Text( this->getUID().c_str(), "%s = %s", this->getUID().c_str(), serialize().c_str() );
        }
    }

    // copy constructor
    ofxVPObjectParameter<T>& operator=(T const & _value){
        this->set(_value);

        return *this;
    }

protected:

    bool bSavingEnabled;

private:

};
