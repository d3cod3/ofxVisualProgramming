//                                      __
//     ____ ___  ___  ____ ___  ____   / /__   __
//    / __ `__ \/ _ \/ __ `__ \/ __ \ / __/ | / /
//   / / / / / /  __/ / / / / / /_/ // /_ | |/ /
//  /_/ /_/ /_/\___/_/ /_/ /_/\____(_)__/ |___/
//
//
//  Created by Memo Akten, www.memo.tv
//
//  acts like a std::map, but preserves order of insertion
//
//  Original code by Memo Akten, updated for ofxVP
//
//  Use it with std::shared_ptr:
//
//  Ex. ofxVP::OrderedMap<std::string, std::shared_ptr<PatchObject> > patchObjects;

#pragma once

#include "ofMain.h"

namespace ofxVP {

template<typename keyType, typename T>
class OrderedMap {

public:

    // get size
    int size() const;

    // add new item
    // item will be cloned, stored internally and added to an stl::map and stl::vector as a pointer
    // if T is shared_ptr, ownership is taken care of automatically by shared_ptr
    // returns a reference to the new object added
    T& push_back(const keyType& key, const T& t);

    // return reference to the stored object
    // throws an exception of the index or key doesn't exist
    T& at(int index);                       // get by index
    const T& at(int index) const;           // get by index

    T& at(const keyType& key);              // get by key
    const T& at(const keyType& key) const;  // get by key

    // [] operator overloads for above
    // these also throw an exception if the index or key doesn't exist
    T& operator[](int index);               // get by index
    const T& operator[](int index) const;

    T& operator[](const keyType& key);      // get by key
    const T& operator[](const keyType& key) const;

    // get the key for item at index. returns blank if doesn't exist
    keyType keyFor(int index) const;

    // get the index for item with key. returns -ve if doesn't exist
    int indexFor(const keyType& key) const;

    // see if key exists
    bool exists(const keyType& key) const;

    // check if the stored object is null
    bool checkNullptr(const T& t) const;

    // change key
    void changeKey(int index, const keyType& newKey);
    void changeKey(const keyType& oldKey, const keyType& newKey);

    // erase by key or index
    void erase(int index);
    void erase(const keyType& key);

    // clear
    void clear();


    // ADVANCED
    // if you know the index and the key
    // fast erase without any validity checks
    void fastErase(int index, const keyType& key);

private:
    std::map<keyType, std::pair<T, int> > _map;     // the actual data is stored here, along with index in vector
    std::vector< keyType > _vector;                 // vector of keys (to store the order)

    void validateIndex(int index, std::string errorMessage) const;
    void validateKey(const keyType& key, std::string errorMessage) const;

    // if something is erased, the indices in the map need to be updated
    void updateMapIndices();

};

//--------------------------------------------------------------
template<typename keyType, typename T>
void OrderedMap<keyType, T>::clear() {
    _vector.clear();
    _map.clear();
}

//--------------------------------------------------------------
template<typename keyType, typename T>
int OrderedMap<keyType, T>::size() const {
    // if these aren't equal, something went wrong somewhere.
    if(_map.size() != _vector.size()) throw runtime_error("ofxVP::OrderedMap::size() - map size doesn't equal vector size");
    return _vector.size();
}

//--------------------------------------------------------------
template<typename keyType, typename T>
T& OrderedMap<keyType, T>::push_back(const keyType& key, const T& t) {
    if(exists(key)) {
        throw invalid_argument("ofxVP::OrderedMap::push_back(keyType, T&) - key already exists");
        return at(key);
    } else {
        _map[key] = make_pair(t, _vector.size());
        _vector.push_back(key);
        size();	// to validate if correctly added to both containers, should be ok
        return at(key);
    }
}

//--------------------------------------------------------------
template<typename keyType, typename T>
T& OrderedMap<keyType, T>::at(int index) {
    validateIndex(index, "ofxVP::OrderedMap::at(int)");
    return at(_vector[index]);
}

//--------------------------------------------------------------
template<typename keyType, typename T>
const T& OrderedMap<keyType, T>::at(int index) const {
    validateIndex(index, "ofxVP::OrderedMap::at(int)");
    return at(_vector[index]);
}

//--------------------------------------------------------------
template<typename keyType, typename T>
T& OrderedMap<keyType, T>::at(const keyType& key) {
    validateKey(key, "ofxVP::OrderedMap::at(keyType)");
    return _map.at(key).first;
}

//--------------------------------------------------------------
template<typename keyType, typename T>
const T& OrderedMap<keyType, T>::at(const keyType& key) const {
    validateKey(key, "ofxVP::OrderedMap::at(keyType)");
    return _map.at(key).first;
}

//--------------------------------------------------------------
template<typename keyType, typename T>
T& OrderedMap<keyType, T>::operator[](int index) {
    return at(index);
}

//--------------------------------------------------------------
template<typename keyType, typename T>
const T& OrderedMap<keyType, T>::operator[](int index) const {
    return at(index);
}

//--------------------------------------------------------------
template<typename keyType, typename T>
T& OrderedMap<keyType, T>::operator[](const keyType& key) {
    return at(key);
}

//--------------------------------------------------------------
template<typename keyType, typename T>
const T& OrderedMap<keyType, T>::operator[](const keyType& key) const {
    return at(key);
}

//--------------------------------------------------------------
template<typename keyType, typename T>
keyType OrderedMap<keyType, T>::keyFor(int index) const {
    validateIndex(index, "ofxVP::OrderedMap::keyFor(int)");
    return _vector[index];
}

//--------------------------------------------------------------
template<typename keyType, typename T>
int OrderedMap<keyType, T>::indexFor(const keyType& key) const {
    validateKey(key, "ofxVP::OrderedMap::indexFor(keyType)");
    return _map.at(key).second;
}

//--------------------------------------------------------------
template<typename keyType, typename T>
void OrderedMap<keyType, T>::changeKey(int index, const keyType& newKey) {
    validateIndex(index, "ofxVP::OrderedMap::changeKey(int)");
    changeKey(keyFor(index), newKey);
}

//--------------------------------------------------------------
template<typename keyType, typename T>
void OrderedMap<keyType, T>::changeKey(const keyType& oldKey, const keyType& newKey) {
    validateKey(oldKey, "ofxVP::OrderedMap::changeKey(keyType)");

    // save temp copy
    auto t = _map[oldKey];

    // erase from map, and reinsert
    _map.erase(oldKey);
    _map[newKey] = t;

    // change key from the vector
    _vector.at(t.second) = newKey;
}

//--------------------------------------------------------------
template<typename keyType, typename T>
void OrderedMap<keyType, T>::erase(int index) {
    validateIndex(index, "ofxVP::OrderedMap::erase(int)");
    fastErase(index, keyFor(index));
    size(); // validate map and vector have same sizes to make sure everything worked alright
}

//--------------------------------------------------------------
template<typename keyType, typename T>
void OrderedMap<keyType, T>::erase(const keyType& key) {
    validateKey(key, "ofxVP::OrderedMap::erase(keyType)");
    fastErase(indexFor(key), key);
    size(); // validate map and vector have same sizes to make sure everything worked alright
}


//--------------------------------------------------------------
template<typename keyType, typename T>
void OrderedMap<keyType, T>::fastErase(int index, const keyType& key) {
    _map.erase(key);
    _vector.erase(_vector.begin() + index);
    updateMapIndices();
}


//--------------------------------------------------------------
template<typename keyType, typename T>
bool OrderedMap<keyType, T>::exists(const keyType& key) const {
    return _map.find(key) != _map.end();
}

//--------------------------------------------------------------
template<typename keyType, typename T>
bool OrderedMap<keyType, T>::checkNullptr(const T& t) const {
    return t == nullptr;
}

//--------------------------------------------------------------
template<typename keyType, typename T>
void OrderedMap<keyType, T>::validateIndex(int index, std::string errorMessage) const {
    #ifdef OFXVP_DEBUG
    if(index<0 || index >= _vector.size()) throw invalid_argument(errorMessage + " - index doesn't exist");
    #endif
}

//--------------------------------------------------------------
template<typename keyType, typename T>
void OrderedMap<keyType, T>::validateKey(const keyType& key, std::string errorMessage) const {
    #ifdef OFXVP_DEBUG
    if(!exists(key)) throw invalid_argument(errorMessage + " - key doesn't exist");
    #endif
}

//--------------------------------------------------------------
template<typename keyType, typename T>
void OrderedMap<keyType, T>::updateMapIndices() {
    for(int i=0; i<_vector.size(); i++) {
        const keyType& key = _vector[i];
        _map[key].second = i;
    }
}


} // End namespace ofxVP
