#pragma once

#include "ofMain.h"
#include "ofxVPBaseParameter.h"

// creates and keeps in sync a copy of the variable, to access from a thread
template <typename T>
class ofxVPThreadedParameter : public ofxVPBaseParameter<T> {

public:
//	ofxVPThreadedParam(T _value) : ofxVPParameter<T>(_value), threadedValue(_value) {
//
//	}

    ofxVPThreadedParameter(T _value, string _name) : ofxVPParameter<T>(_value, _name), threadedValue(_value) {

    }

    virtual ~ofxVPThreadedParameter(){
        paramToThread.close();
        paramFromThread.close();
    }

    ofxVPThreadedParameter<T>& operator=(T const & _value){
        this->set(_value);
        return *this;
    }
//	// cast ofxVPThreadedParam<type> to type.
//	T& operator=(const ofxVPThreadedParam<T>& _param){
//		return _param.get();
//	}

    // implicit conversion operator
    operator ofxVPParameter<T>&() { return *this; }

    virtual bool set(const T& _value){
        //if(ofThread::isMainThread()){
        if( main_thread_id == std::this_thread::get_id() ){
            // set from main thread
            ofxVPParameter<T>::set(_value);
            paramToThread.send(_value);
        }
        else {
            setFromThread(_value);
        }

        return true; // tmp
    }

    // use to change the value from thread
    virtual bool setFromThread(const T& _value){
        threadedValue = _value;
        paramFromThread.send(_value);

        // todo: notify

        return true; // tmp
    }

    virtual T& get(){
        //if(ofThread::isMainThread()){
        if( main_thread_id == std::this_thread::get_id() ){
            // get from main thread
            return ofxVPParameter<T>::get();
        }
        else {
            return getFromThread();
        }
    }

    virtual T get() const {
        return ofxVPParameter<T>::value;
    }

    virtual T& getFromThread(){
        return threadedValue;
    }

    virtual T getFromThread() const{
        return threadedValue;
    }

    // to sync the variable on the thread's update()
    virtual bool syncFromThread(){
        // empty queue and keep last param
        bool changed = false;
        while(paramToThread.tryReceive(threadedValue)){
            changed = true;
        }
        if(changed){
            // todo: notfiy change
        }
        return changed;
    }

    virtual bool syncFromMainThread(){
        // empty queue and keep last param
        bool changed = false;
        while(paramToThread.tryReceive(ofxVPParameter<T>::value)){
            changed = true;
        }
        if(changed){
            // todo: notifiy threaded change
        }
        return true;
    }

    virtual void triggerUpdate(){
        if( main_thread_id == std::this_thread::get_id() ){
            paramToThread.send(ofxVPParameter<T>::value);
        }
        else {
            paramFromThread.send(threadedValue);
        }
    }

    static std::thread::id main_thread_id;

protected:
//	string getUniqueName(){
//		return "";
//	}

    T threadedValue; // to access from thread only

    ofThreadChannel<T> paramToThread;
    ofThreadChannel<T> paramFromThread;
};
template <typename T>
std::thread::id ofxVPThreadedParam<T>::main_thread_id = std::this_thread::get_id();
