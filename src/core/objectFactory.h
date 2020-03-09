//
//  objectFactory.h
//  ofxVisualProgramming
//
//  Created by Daan de Lange on 03/03/2020.
//
//  Allows objects to register and be instantiated from a string.
//
//	Based on code from https://github.com/Karma-Kusala/karmaMapper

#pragma once

// Standard libraries
#include <map>
#include <string>
#include <utility>
#include <vector>

#ifdef DEBUG
#include <iostream>
#endif

class PatchObject; // forward declare

namespace ofxVPObjects {
    namespace factory {

        typedef PatchObject* (*CreateObjectFunc)();
        typedef std::map<std::string, CreateObjectFunc> objectRegistry;
        typedef std::map<std::string, std::vector<std::string> > objectCategories;

        // getter singletons
        inline objectRegistry& getObjectRegistry(){
            static objectRegistry singletonReg;
            return singletonReg;
		}
        inline objectCategories& getCategories(){
            static objectCategories singletonCats;
            return singletonCats;
        }
		
		template<class T>
        PatchObject* createObject() {
			return new T;
		}
		
		template<class T>
        struct RegistryEntry{
            public:
                static RegistryEntry<T>& Instance(const std::string& name, const std::string& category="undefined") {
                    static RegistryEntry<T> inst(name, category);
                    return inst;
                }
			
            private:
                RegistryEntry(const std::string& name, const std::string& category="undefined") {
                    // map name to constructor
                    objectRegistry& reg = getObjectRegistry();
                    CreateObjectFunc func = createObject<T>;
                    std::pair<objectRegistry::iterator, bool> ret = reg.insert(objectRegistry::value_type(name, func));

                    if (ret.second == false) {
                        // Object with same name is already registered.
#ifdef DEBUG
                        std::cout << "Object Factory :: Object «"<< name <<"» already registered, is this behaviour normal ?" << std::endl;
#endif
                        // go no further
                        return;
                    }

                    // set category
                    objectCategories& cats = getCategories();
                    std::vector<std::string> vec;
                    cats.insert(objectCategories::value_type(category, vec));
                    cats[category].push_back(name);
                }

                 // C++11 feature : prevent creation of clones
                RegistryEntry(const RegistryEntry<T>&) = delete;
                RegistryEntry& operator=(const RegistryEntry<T>&) = delete;
		};
		
	} // namespace factory
	
} // namespace ofxVPObjects

