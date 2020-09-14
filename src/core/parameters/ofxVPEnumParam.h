#pragma once

#include "ofxVPIntParam.h"

// ofxVPEnumParam
// A list of strings mapped to integer values.
// Returns -1 if none selected.

//template <>
class ofxVPEnumParam : public ofxVPObjectParameter<int> {

public:

    ofxVPEnumParam(int _value, std::string _name)
        : ofxVPObjectParameter<int>(_value, _name)
    {

    };

    ofxVPEnumParam(int _value, std::string _name, const std::vector<std::string>& _keyNames )
        : ofxVPObjectParameter<int>(_value, _name ),
        stringValues( _keyNames )
    {

    };

    // explicitly map to child assignment operators, otherwise the compiler will generate wrong ones.
    using ofxVPBaseParameter<int>::operator =;

    virtual std::string getString(const std::string& _outOfRangeValue="", const int& _valueForKey = -1) {
        // set default key to current value
        int key = ( _valueForKey == -1 )? this->value :  _valueForKey;

        // Check key
        if(key==-1 ||  !isValidKey(key) ){ // First condition is almost always true, leave for performance
            if( _outOfRangeValue.length() > 0 ){
                return "Undefined";
            }
            else {
                return _outOfRangeValue;
            }
        }
        // Return value
        return stringValues.at(key);
    }

//    std::string getStringChr(const char* _outOfRangeValue="", const int& _valueForKey = -1) {
//        return getString((std::string)std::string(_outOfRangeValue), _valueForKey);
//    }

    bool isValidKey(const std::vector<std::string>::size_type& _key) const {
        // Note: (_key >= 0ul) is not needed as _key is already casted
        return ( _key < stringValues.size() );
    }

    virtual bool set( const int& _value ) override {
        // verify values
        if(_value >= stringValues.size()){
            if(this->value == -1) // Stay unselected
                this->value = cachedValue = -1;
            else // Select max value
                this->value = cachedValue = stringValues.size()-1;

            return false; // Value remains valid, but didn't set the provided value
        }
        else if(_value < 0){
            this->value = cachedValue = -1;//0;
            return false;  // Value remains valid, but didn't set the provided value
        }

        // Accept values
        this->value = cachedValue = _value;
        return true;
    }

    virtual void drawGui() override {

        if(ImGui::BeginCombo(this->getDisplayName().c_str(), getString("Select...").c_str() )){
            if(stringValues.size() > 0){
                for(int i=0; i < stringValues.size(); ++i){
                    bool is_selected = (this->value == i );
                    if (ImGui::Selectable(stringValues.at(i).c_str(), is_selected))
                        this->value = i;
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
            }
            else {
                ImGui::Selectable(" - Empty -", false);
            }

            ImGui::EndCombo();
        }
        //return true;
    }

private:
    const std::vector<std::string> stringValues;
    int cachedValue = -1;
};
