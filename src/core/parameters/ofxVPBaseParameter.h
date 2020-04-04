
#pragma once

#include "ofxVPHasUid.h"
#include "ofxImGui.h"

#define OFXVP_PARAM_XML_TAG_WRAPPER "param"
#define OFXVP_PARAM_XML_TAG_VALUE "paramValue"

// inspired from OF's ofParam<type>
template <typename T>
class ofxVPBaseParameter : public ofxVPHasUID {

public:

    ofxVPBaseParameter(T _value, std::string _paramName) : ofxVPHasUID(_paramName), value(_value) {

        registerUniqueIdentifier(_paramName, this);
    }

    virtual ~ofxVPBaseParameter(){

    }

    virtual bool set(const T& _value){
        value = _value;

        // todo: notify

        return true;
    }

    // copy
//    ofxVPBaseParameter<T>& operator=(T _value){
//        this->set(_value);
//        return *this;
//    }

    // assign
    ofxVPBaseParameter<T>& operator=(const T& _value){
        this->set(_value);
        return *this;
    }

//	ofxVPParameter<T>& operator=(T&& _value){
//		set(_value);
//		return *this;
//	}

    // cast ofxVPParameter<type> to type.
    T& operator=(const ofxVPBaseParameter<T>& _param) const{
        return _param.get();
    }
    T& operator=(const ofxVPBaseParameter<T>&& _param){
        return _param.get();
    }

    // implicit conversion operator
    //operator T() const { return get(); }
    operator T&() { return get(); }

    virtual T& get(){
        return value;
    }

    // copy value to respect const correctness (but slower)
    virtual T get() const{
        return value;
    }

    // Experimental: alternative method to get() the value dereferencing the pointer
    T* operator->(){
        return &get();
    }

protected:
    T value;

    // todo:
    bool bReadOnly;
};
