/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2025 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

    ofxVisualProgramming is distributed under the MIT License.
    This gives everyone the freedoms to use ofxVisualProgramming in any context:
    commercial or non-commercial, public or private, open or closed source.

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.

    See https://github.com/d3cod3/ofxVisualProgramming for documentation

==============================================================================*/

#pragma once

#include "ofMain.h"

#include "ofxVPConfig.h"

template<typename TYPE>
inline TYPE ofxVP_XML_ENGINE_VALUE_CAST(const pugi::char_t* _value){
    return static_cast<TYPE>(_value);
}


// A pugiXml interface engine for ofxVisualProgramming
class ofxVPXmlEngine{
    
public:

    ofxVPXmlEngine() { empty.set_name("emptyNode"); }
    ~ofxVPXmlEngine() {}

    // LOAD/SAVE
    std::string getMosaicPatch() { return filepath; }
    void loadMosaicPatch(std::string path);
    void saveMosaicPatch(std::string path);
    bool isPatchOK() { return isMosaicPatch; }
    pugi::xpath_node_set getScriptLanguageMethods(std::string path);

    // TESTING
    void printTests();
    void printMosaicRelease();
    void printPatchSettings();
    void printPatchObjects();
    void printObjectVars(int id);
    void printObjectInlets(int id);
    void printObjectOutlets(int id);
    void printObjectLinks(int objid, int outlet_order);
    void printObject(int id);
    void printPatchConnections();
    void scrambleObjectVars(int id);

    // CHECKS
    bool checkIsMosaicPatch();
    bool objIDExists(int id);
    bool nodeExists(pugi::xml_node &node,std::string name) { return bool(node.child(name.c_str())); }
    bool checkIsObjectNode(pugi::xml_node node);
    bool isEmptyNode(pugi::xml_node n) {return  n == empty ? true : false;  }

    // GETTERS
    pugi::xml_node getObjectAtPos(int pos);
    pugi::xpath_node_set getPatchObjects() { return xml.select_nodes("object");}
    pugi::xpath_node_set getObjectVars(int id);
    pugi::xpath_node_set getObjectInlets(int id);
    pugi::xpath_node_set getObjectOutlets(int id);
    pugi::xpath_node_set getObjectLinks(int objid, int outlet_order);

    pugi::xml_node getObjectNode(int id);
    std::string getObjectName(int id);
    std::string getObjectFilepath(int id);
    std::string getObjectSubpatch(int id);
    ofVec2f getObjectPosition(int id);

    bool getPatchChildBool(pugi::xml_node &node, std::string childname) { return node.child(childname.c_str()).text().as_bool(); }
    int getPatchChildInt(pugi::xml_node &node, std::string childname) { return node.child(childname.c_str()).text().as_int(); }
    float getPatchChildFloat(pugi::xml_node &node, std::string childname) { return node.child(childname.c_str()).text().as_float(); }
    std::string getPatchChildString(pugi::xml_node &node, std::string childname) { return node.child(childname.c_str()).text().as_string(); }

    // READ/WRITE/REMOVE
    int getLastObjectID();
    int addNewObject(std::string name, std::string _filepath, std::string subpatch, ofVec2f pos);
    void setObjectFilepath(int objid, std::string _filepath);
    void setObjectSubpatch(int objid, std::string subpatch);
    void setObjectPos(int objid, ofVec2f pos);
    void appendObjectInletsBlock(int objid) { getObjectNode(objid).append_child("inlets"); }
    void appendObjectOutletsBlock(int objid) { getObjectNode(objid).append_child("outlets"); }
    void appendObjectVarsBlock(int objid) { getObjectNode(objid).append_child("vars"); }
    void removeObject(int id);

    void addObjectInlet(int objid, int type, std::string name);
    void setObjectInletType(int objid, int inlet_order, int type);
    void setObjectInletName(int objid, int inlet_order, std::string name);
    void removeObjectInlet(int objid, int inlet_order);
    void removeObjectInlets(int objid);

    void addObjectOutlet(int objid, int type, std::string name);
    void setObjectOutletType(int objid, int outlet_order, int type);
    void setObjectOutletName(int objid, int outlet_order, std::string name);
    void removeObjectOutlet(int objid, int outlet_order);
    void removeObjectOutlets(int objid);

    void addObjectLink(int objid, int outlet_order, int toObjId, int toInletId);
    void setObjectLinkToObjID(int objid, int outlet_order, int link_order, int toObjId);
    void setObjectLinkToInletID(int objid, int outlet_order, int link_order, int toInletId);
    void removeObjectLink(int objid, int outlet_order, int link_order);
    void removeAllObjectOutletLinks(int objid, int outlet_order);
    void removeAllObjectLinks(int objid);
    void removeAllLinksToObject(int objid, int objNumInlets);

    void addObjectVar(int objid, std::string varName, float varValue);
    void setObjectVar(int objid, std::string varName, float varValue);
    void removeObjectVar(int objid, std::string varName);
    void removeObjectVars(int objid);

    void setMosaicConfig(std::string configVar, int value);
    int getMosaicConfigInt(std::string configVar);
    float getMosaicConfigFloat(std::string configVar);
    bool getMosaicConfigBool(std::string configVar);

    void setMosaicRelease(std::string release) { setPatchValue(xml,"release",release); saveMosaicPatch(filepath); }
    std::string getMosaicRelease(){ return ofxVP_XML_ENGINE_VALUE_CAST<std::string>(getPatchValue(xml,"release")); }
    size_t getMosaicReleaseMAJOR(){ return size_t(ofToInt(string(1,getMosaicRelease().at(0)))); }
    size_t getMosaicReleaseMINOR(){ return size_t(ofToInt(string(1,getMosaicRelease().at(1)))); }
    size_t getMosaicReleasePATCH(){ return size_t(ofToInt(string(1,getMosaicRelease().at(2)))); }

    bool checkReleaseIsPrePugiXml(std::string currentRelease);

    void setPatchValue(pugi::xml_node &node, std::string childname, int value) {
        assert(isLoaded);
        if(node.child(childname.c_str()).empty()){
            node.append_child(childname.c_str());
        }
        node.child(childname.c_str()).text().set(std::to_string(value).c_str());
    }
    void setPatchValue(pugi::xml_node &node, std::string childname, float value) {
        assert(isLoaded);
        if(node.child(childname.c_str()).empty()){
            node.append_child(childname.c_str());
        }
        node.child(childname.c_str()).text().set(std::to_string(value).c_str());
    }
    void setPatchValue(pugi::xml_node &node, std::string childname, std::string value) {
        assert(isLoaded);
        if(node.child(childname.c_str()).empty()){
            node.append_child(childname.c_str());
        }
        node.child(childname.c_str()).text().set(value.c_str());
    }
    

private:
    
    pugi::xml_node getPatchChildNode(pugi::xml_node parent, std::string name) { return parent.select_node(name.c_str()).node(); }

    float getObjectVar(int objid, std::string varname);
    int getInletType(int objid, int inlet_order);
    std::string getInletName(int objid, int inlet_order);
    int getOutletType(int objid, int outlet_order);
    std::string getOutletName(int objid, int outlet_order);
    int getLinkID(int objid, int outlet_order, int link_order);
    int getLinkToInlet(int objid, int outlet_order, int link_order);

    const pugi::char_t* getPatchValue(pugi::xml_node &node, std::string name){ return node.child_value(name.c_str()); }

    pugi::xml_document      xml;
    pugi::xml_node          settingNode;
    pugi::xml_node          empty;
    pugi::xpath_node_set    empty_set;

    string                  filepath;
    string                  release;

    bool                    isLoaded=false;
    bool                    isMosaicPatch=false;

};
