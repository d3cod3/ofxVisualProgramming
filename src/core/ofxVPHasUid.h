
#pragma once

#include <string>
#include <map>
#include <iostream>
#include "ofLog.h"

class ofxVPHasUID {

public:
    ofxVPHasUID(const std::string& _name = "UID") : displayName(_name) {
        myUID = _name;
        if( !registerUniqueIdentifier(myUID, this) ){
            //myUID = _name;
            ofLogError("There was a problem registering a unique ID. Watchout for problems !");
        }
    }
    ofxVPHasUID(const char* _chars ) : displayName( std::string( _chars ) ) {
        std::string _name = displayName;
        if(_name.length()<1){
            _name="UID";
        }
        if( registerUniqueIdentifier(_name, this) ){
            myUID = _name;
        }
    }

    virtual ~ofxVPHasUID(){
        // rm name from available names
        if(allUIDs.find(this)!=allUIDs.end()){
            allUIDs.erase(this);
        }
    }

    std::string getUID() const{
        return myUID;
    }

    // "DisplayName" is the initially requested name, not guaranteed to be unique.
    std::string getDisplayName() const{
        return myUID;
    }

    static bool registerUniqueIdentifier(std::string& _name, ofxVPHasUID* _instance) {

        if(_instance == nullptr){
            return false;
        }

        // is the instance already registered ?
        {
            auto it = allUIDs.find( _instance );
            if( it != allUIDs.end() ){
                std::cout << "Instance is already registered with name = " << _name << std::endl;
                // return correct name
                _name = it->second;
                return true;
            }
        }

        // Instance is not registered yet. Add new one.
        makeIdentifierUnique(_name);
        allUIDs.emplace(std::make_pair(_instance, _name));

        return true;
    }

    static void makeIdentifierUnique(std::string& _name, unsigned int _inc=1) {
        std::string newName = _name;
        if(_inc > 1){ // dont put 1 in the name
            newName += std::to_string(_inc);
        }

        //std::cout << "Searching for : " << newName << std::endl;

        // recurse until name is unique
        if( !isIdentifierUnique(newName) ){
            //std::cout << "Recursing !" << std::endl;
            makeIdentifierUnique(_name, _inc+1 );
            return;
        }

        //std::cout << "Got UNIQUE ! : " << newName << std::endl;
        _name = newName;

//        for(auto it=allUIDs.begin(); it!=allUIDs.end(); ++it){
//            if(it->second.compare(newName) == 0){
//                makeIdentifierUnique(_name, _inc+1);
//                return;
//            }
//        }

    }

    static bool isIdentifierUnique(const std::string& _name) {
        //std::string newName = _name;
        //newName += std::to_string(_inc);
        //auto it = std::find_if(allUIDs.cbegin(), allUIDs.cend(), [_name](decltype(*beg_)
        auto it = std::find_if(allUIDs.begin(), allUIDs.end(), [_name](const std::map<ofxVPHasUID*, std::string>::value_type& pair){ return pair.second.compare(_name)==0;});
        return (it == allUIDs.end());
    }

protected:
    std::string myUID;
    const std::string displayName;
    static std::map<ofxVPHasUID*, std::string> allUIDs;
};
