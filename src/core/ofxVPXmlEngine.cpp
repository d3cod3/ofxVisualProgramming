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

#include "ofxVPXmlEngine.h"

//--------------------------------------------------------------
void ofxVPXmlEngine::loadMosaicPatch(std::string path){
    // load the file and check if it was loaded ok
    filepath = path;
    isLoaded = xml.load_file(filepath.c_str()) != pugi::status_ok;

    // check if is a valid Mosaic patch
    if(isLoaded){
        isMosaicPatch = checkIsMosaicPatch();
        if(!isMosaicPatch){
            #ifdef OFXVP_DEBUG
            std::cout << "The XML file loaded is not a valid Mosaic patch" << std::endl;
            #endif
        }else{
            release = getMosaicRelease();
            if(nodeExists(xml,"settings")){
                settingNode = getPatchChildNode(xml,"settings");
            }
        }
    }else{
        #ifdef OFXVP_DEBUG
        std::cout << "ERROR, XML file not loaded!" << std::endl;
        #endif
    }
}

//--------------------------------------------------------------
void ofxVPXmlEngine::saveMosaicPatch(std::string path){
    if(isMosaicPatch){
        xml.save_file(path.c_str());
    }
}

//--------------------------------------------------------------
bool ofxVPXmlEngine::checkIsMosaicPatch(){
    if(nodeExists(xml,"www")){
        if(ofxVP_XML_ENGINE_VALUE_CAST<std::string>(getPatchValue(xml,"www")) == MOSAIC_CHECK_STRING){
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------
void ofxVPXmlEngine::printTests(){
    if(isMosaicPatch){

        // ----------------------------------------- read tests

        printMosaicRelease();

        printPatchSettings();

        printPatchObjects();

        printPatchConnections();

        // ----------------------------------------- write tests

        // Add object
        addNewObject("testing","none","root",ofVec2f(ofRandom(400),ofRandom(400)));
        int lid = getLastObjectID();
        std::cout << "Last object ID is: " << lid << std::endl;
        printObject(lid);

        // Edit object filepath/subpatch/position
        setObjectSubpatch(lid,"audioInput");
        setObjectPos(lid,ofVec2f(300,333));

        // Add/edit/remove inlet
        addObjectInlet(lid,0,"number");
        setObjectInletType(lid,0,1);

        // Add/edit/remove outlet
        addObjectOutlet(lid,0,"steps"); // 0
        addObjectOutlet(lid,0,"data");  // 1
        setObjectOutletType(lid,1,2);

        // Add/edit/remove link
        addObjectLink(lid,0,9,0);
        addObjectLink(lid,0,4,0);
        addObjectLink(lid,1,5,1);
        printObject(lid);
        setObjectLinkToObjID(lid,0,0,7);
        setObjectLinkToInletID(lid,1,0,2);

        printObject(lid);
        removeObjectInlet(lid,0);
        removeObjectLink(lid,0,1);
        printObject(lid);
        removeAllObjectOutletLinks(lid,0);
        printObject(lid);
        removeAllObjectLinks(lid);
        printObject(lid);
        removeObjectOutlet(lid,1);
        removeObjectOutlet(lid,0);
        printObject(lid);

        // Add/edit/remove var
        printObjectVars(lid);
        addObjectVar(lid,"TESTING",10.0f);
        printObjectVars(lid);
        setObjectVar(lid,"TESTING",ofRandom(100));
        printObjectVars(lid);
        removeObjectVar(lid,"TESTING");
        printObjectVars(lid);

        // Remove object
        removeObject(lid);
        std::cout << "Last object ID is: " << getLastObjectID() << std::endl;

        // Edit settings
        setMosaicConfig("bpm",180);
        printPatchSettings();
        setMosaicConfig("bpm",120);

    }
}

//--------------------------------------------------------------
void ofxVPXmlEngine::printMosaicRelease(){
    std::cout << "Opening a valid Mosaic " << release << " patch" << std::endl;
}

//--------------------------------------------------------------
void ofxVPXmlEngine::printPatchSettings(){
    if(settingNode){
        std::cout << "Mosaic Patch: " << filepath << " Settings:" << std::endl;
        for(pugi::xml_node_iterator it = settingNode.begin(); it != settingNode.end(); it++){
            std::cout << it->name() << " : " << getPatchChildInt(settingNode,it->name()) << std::endl;
        }
    }
}

//--------------------------------------------------------------
void ofxVPXmlEngine::printPatchObjects(){
    auto objectsXml = getPatchObjects();
    for(auto & obj: objectsXml){
        auto n = obj.node();
        int objID = getPatchChildInt(n,"id");
        std::cout << "Loading " << getPatchChildString(n,"name") << " object with id " << objID << " at canvas position (" << getObjectPosition(objID).x << "," << getObjectPosition(objID).y << ")" << std::endl;
    }
}

//--------------------------------------------------------------
void ofxVPXmlEngine::printObjectVars(int id){

    std::cout << "Printing object with ID " << id << " vars --------------------"  << std::endl;

    pugi::xpath_node_set vars = getObjectVars(id);

    for(auto & var: vars){
        auto v = var.node();

        std::cout << getPatchChildString(v,"name") << " : " << getPatchChildFloat(v,"value") << std::endl;
    }
}

//--------------------------------------------------------------
void ofxVPXmlEngine::printObjectInlets(int id){

    std::cout << "Printing object with ID " << id << " inlets --------------------"  << std::endl;

    pugi::xpath_node_set inlets = getObjectInlets(id);

    for(auto & inlet: inlets){
        auto i = inlet.node();

        std::cout << getPatchChildString(i,"name") << " : type  " << getPatchChildFloat(i,"type") << std::endl;
    }
}

//--------------------------------------------------------------
void ofxVPXmlEngine::printObjectOutlets(int id){

    std::cout << "Printing object with ID " << id << " outlets --------------------"  << std::endl;

    pugi::xpath_node_set outlets = getObjectOutlets(id);

    for(auto & outlet: outlets){
        auto o = outlet.node();

        std::cout << getPatchChildString(o,"name") << " : type  " << getPatchChildFloat(o,"type") << std::endl;
    }
}

//--------------------------------------------------------------
void ofxVPXmlEngine::printObjectLinks(int objid, int outlet_order){
    std::cout << "Printing active links from outlet ID " << outlet_order << " of object with ID " << objid << " --------------------"  << std::endl;

    pugi::xpath_node_set links = getObjectLinks(objid,outlet_order);

    for(auto & link: links){
        auto l = link.node();

        std::cout << "Link to object ID " << getPatchChildInt(l,"id") << " : to inlet  " << getPatchChildInt(l,"inlet") << std::endl;
    }
}


//--------------------------------------------------------------
void ofxVPXmlEngine::printObject(int id){
    std::cout << "Printing object with ID " << id << std::endl;

    auto objectsXml = getPatchObjects();
    for(auto & obj: objectsXml){
        auto n = obj.node();
        if(getPatchChildInt(n,"id") == id){
            std::cout << "Object: " << getObjectName(id) << " of subpatch: " << getObjectSubpatch(id) << std::endl;
            std::cout << "with filepath: " << getObjectFilepath(id) << " at position (" << getObjectPosition(id).x << "," << getObjectPosition(id).y << ")" << std::endl;

            // inlets
            pugi::xpath_node_set inlets = getObjectInlets(id);
            if(!inlets.empty()){
                std::cout << "has the following inlets:" << std::endl;
                for(auto & inlet: inlets){
                    auto i = inlet.node();
                    std::cout << "    " << getPatchChildString(i,"name") << " of type  " << getPatchChildFloat(i,"type") << std::endl;
                }
            }else{
                std::cout << "has NO inlets" << std::endl;
            }

            // outlets with links
            pugi::xpath_node_set outlets = getObjectOutlets(id);
            if(!outlets.empty()){
                std::cout << "has the following outlets:" << std::endl;
                int oi = 0;
                for(auto & outlet: outlets){
                    auto i = outlet.node();
                    std::cout << "    " << getPatchChildString(i,"name") << " of type  " << getPatchChildFloat(i,"type") << std::endl;
                    pugi::xpath_node_set links = getObjectLinks(id,oi);
                    if(!links.empty()){
                        std::cout << "        with links to:" << std::endl;
                        for(auto & link: links){
                            auto l = link.node();
                            std::cout << "        Link to object " << getObjectName(getPatchChildInt(l,"id")) << " : to inlet  " << getPatchChildInt(l,"inlet") << std::endl;
                        }
                    }else{
                        std::cout << "        outlet " << oi << " has NO links" << std::endl;
                    }
                    oi++;
                }
            }else{
                std::cout << "has NO outlets" << std::endl;
            }

            // vars
            pugi::xpath_node_set vars = getObjectVars(id);
            if(!vars.empty()){
                std::cout << "has the following vars:" << std::endl;
                for(auto & var: vars){
                    auto v = var.node();
                    std::cout << "    " << getPatchChildString(v,"name") << " : " << getPatchChildFloat(v,"value") << std::endl;
                }
            }else{
                std::cout << "has NO vars" << std::endl;
            }

        }
    }
}

//--------------------------------------------------------------
void ofxVPXmlEngine::printPatchConnections(){
    std::cout << "Printing loaded Mosaic patch connection MAP --------------------"  << std::endl;

    auto objectsXml = getPatchObjects();
    for(auto & obj: objectsXml){
        auto n = obj.node();
        int objID = getPatchChildInt(n,"id");
        std::string name = getObjectName(objID);
        std::string subpatch = getObjectSubpatch(objID);
        pugi::xpath_node_set outlets = getObjectOutlets(objID);
        if(!outlets.empty()){
            int index = 0;
            for(auto & outlet: outlets){
                auto o = outlet.node();
                pugi::xpath_node_set links = getObjectLinks(objID,index);
                if(!links.empty()){
                    for(auto & link: links){
                        auto l = link.node();
                        int toObjectID = getPatchChildInt(l,"id");
                        int toInletID = getPatchChildInt(l,"inlet");
                        std::cout << "Connecting object " << name << " in subpatch " << subpatch << " from outlet " << index << " ( " << getPatchChildString(o,"name") << " )" << "to object " << getObjectName(toObjectID) << " inlet " << toInletID << std::endl;
                    }
                }
                index++;
            }
        }
    }

}

//--------------------------------------------------------------
void ofxVPXmlEngine::scrambleObjectVars(int id){
    pugi::xpath_node_set vars = getObjectVars(id);

    for(auto & var: vars){
        auto v = var.node();

        setPatchValue(v, "value", ofRandomuf());
    }

    saveMosaicPatch(filepath);
}

//--------------------------------------------------------------
bool ofxVPXmlEngine::checkIsObjectNode(pugi::xml_node node){
    if(node != empty){
        return true;
    }
    return false;
}

//--------------------------------------------------------------
int ofxVPXmlEngine::getLastObjectID(){
    int res = 0;
    auto objectsXml = getPatchObjects();
    for(auto & obj: objectsXml){
        auto n = obj.node();
        int nid = getPatchChildInt(n,"id") ;
        if(nid > res){
            res = nid;
        }
    }

    return res;
}

//--------------------------------------------------------------
void ofxVPXmlEngine::addNewObject(std::string name, std::string _filepath, std::string subpatch, ofVec2f pos){
    pugi::xml_node newObject = xml.append_child("object");
    int nextID = getLastObjectID() + 1;
    newObject.append_child("id");
    newObject.append_child("name");
    newObject.append_child("filepath");
    newObject.append_child("subpatch");
    pugi::xml_node objPos = newObject.append_child("position");
    objPos.append_child("x");
    objPos.append_child("y");
    newObject.append_child("inlets");
    newObject.append_child("outlets");
    newObject.append_child("vars");

    setPatchValue(newObject,"id", nextID);
    setPatchValue(newObject,"name", name);
    setPatchValue(newObject,"filepath", _filepath);
    setPatchValue(newObject,"subpatch", subpatch);
    setPatchValue(objPos,"x", pos.x);
    setPatchValue(objPos,"y", pos.y);

    saveMosaicPatch(filepath);

}

//--------------------------------------------------------------
void ofxVPXmlEngine::setObjectFilepath(int objid, std::string _filepath){
    pugi::xml_node n = getObjectNode(objid);

    setPatchValue(n,"filepath", _filepath);

    saveMosaicPatch(filepath);
}

//--------------------------------------------------------------
void ofxVPXmlEngine::setObjectSubpatch(int objid, std::string subpatch){
    pugi::xml_node n = getObjectNode(objid);

    setPatchValue(n,"subpatch", subpatch);

    saveMosaicPatch(filepath);
}

//--------------------------------------------------------------
void ofxVPXmlEngine::setObjectPos(int objid, ofVec2f pos){
    pugi::xml_node op = getObjectNode(objid).child("position");

    setPatchValue(op,"x", pos.x);
    setPatchValue(op,"y", pos.y);

    saveMosaicPatch(filepath);
}

//--------------------------------------------------------------
void ofxVPXmlEngine::removeObject(int id){
    pugi::xml_node n = getObjectNode(id);

    xml.remove_child(n);

    saveMosaicPatch(filepath);
}

//--------------------------------------------------------------
void ofxVPXmlEngine::addObjectInlet(int objid, int type, std::string name){
    pugi::xml_node oi = getObjectNode(objid).child("inlets");

    pugi::xml_node ni = oi.append_child("link");

    ni.append_child("type");
    ni.append_child("name");

    setPatchValue(ni,"type", type);
    setPatchValue(ni,"name", name);

    saveMosaicPatch(filepath);
}

//--------------------------------------------------------------
void ofxVPXmlEngine::setObjectInletType(int objid, int inlet_order, int type){
    pugi::xpath_node_set inlets = getObjectInlets(objid);
    if(!inlets.empty()){
        int index = 0;
        for(auto & inlet: inlets){
            if(index == inlet_order){
                auto i = inlet.node();
                setPatchValue(i,"type", type);
                break;
            }
            index++;
        }
    }

    saveMosaicPatch(filepath);
}

//--------------------------------------------------------------
void ofxVPXmlEngine::setObjectInletName(int objid, int inlet_order, std::string name){
    pugi::xpath_node_set inlets = getObjectInlets(objid);
    if(!inlets.empty()){
        int index = 0;
        for(auto & inlet: inlets){
            if(index == inlet_order){
                auto i = inlet.node();
                setPatchValue(i,"name", name);
                break;
            }
            index++;
        }
    }

    saveMosaicPatch(filepath);
}

//--------------------------------------------------------------
void ofxVPXmlEngine::removeObjectInlet(int objid, int inlet_order){
    pugi::xml_node oi = getObjectNode(objid).child("inlets");
    pugi::xpath_node_set inlets = getObjectInlets(objid);
    if(!inlets.empty()){
        int index = 0;
        for(auto & inlet: inlets){
            if(index == inlet_order){
                auto i = inlet.node();
                oi.remove_child(i);
                break;
            }
            index++;
        }
    }

    saveMosaicPatch(filepath);
}

//--------------------------------------------------------------
void ofxVPXmlEngine::addObjectOutlet(int objid, int type, std::string name){
    pugi::xml_node oo = getObjectNode(objid).child("outlets");

    pugi::xml_node no = oo.append_child("link");

    no.append_child("type");
    no.append_child("name");

    setPatchValue(no,"type", type);
    setPatchValue(no,"name", name);

    saveMosaicPatch(filepath);
}

//--------------------------------------------------------------
void ofxVPXmlEngine::setObjectOutletType(int objid, int outlet_order, int type){
    pugi::xpath_node_set outlets = getObjectOutlets(objid);
    if(!outlets.empty()){
        int index = 0;
        for(auto & outlet: outlets){
            if(index == outlet_order){
                auto o = outlet.node();
                setPatchValue(o,"type", type);
                break;
            }
            index++;
        }
    }

    saveMosaicPatch(filepath);
}

//--------------------------------------------------------------
void ofxVPXmlEngine::setObjectOutletName(int objid, int outlet_order, std::string name){
    pugi::xpath_node_set outlets = getObjectOutlets(objid);
    if(!outlets.empty()){
        int index = 0;
        for(auto & outlet: outlets){
            if(index == outlet_order){
                auto o = outlet.node();
                setPatchValue(o,"name", name);
                break;
            }
            index++;
        }
    }

    saveMosaicPatch(filepath);
}

//--------------------------------------------------------------
void ofxVPXmlEngine::removeObjectOutlet(int objid, int outlet_order){
    pugi::xml_node oo = getObjectNode(objid).child("outlets");
    pugi::xpath_node_set outlets = getObjectOutlets(objid);
    if(!outlets.empty()){
        int index = 0;
        for(auto & outlet: outlets){
            if(index == outlet_order){
                auto o = outlet.node();
                oo.remove_child(o);
                break;
            }
            index++;
        }
    }

    saveMosaicPatch(filepath);
}

//--------------------------------------------------------------
void ofxVPXmlEngine::addObjectLink(int objid, int outlet_order, int toObjId, int toInletId){
    pugi::xpath_node_set outlets = getObjectOutlets(objid);
    if(!outlets.empty()){
        int index = 0;
        for(auto & outlet: outlets){
            if(index == outlet_order){
                auto o = outlet.node();
                pugi::xml_node nl = o.append_child("to");
                nl.append_child("id");
                nl.append_child("inlet");
                setPatchValue(nl,"id",toObjId);
                setPatchValue(nl,"inlet",toInletId);
                break;
            }
            index++;
        }
    }

    saveMosaicPatch(filepath);
}

//--------------------------------------------------------------
void ofxVPXmlEngine::setObjectLinkToObjID(int objid, int outlet_order, int link_order, int toObjId){
    pugi::xpath_node_set links = getObjectLinks(objid,outlet_order);
    if(!links.empty()){
        int index = 0;
        for(auto & link: links){
            if(index == link_order){
                auto l = link.node();
                setPatchValue(l,"id",toObjId);
                break;
            }
            index++;
        }
    }

    saveMosaicPatch(filepath);
}

//--------------------------------------------------------------
void ofxVPXmlEngine::setObjectLinkToInletID(int objid, int outlet_order, int link_order, int toInletId){
    pugi::xpath_node_set links = getObjectLinks(objid,outlet_order);
    if(!links.empty()){
        int index = 0;
        for(auto & link: links){
            if(index == link_order){
                auto l = link.node();
                setPatchValue(l,"inlet",toInletId);
                break;
            }
            index++;
        }
    }

    saveMosaicPatch(filepath);
}

//--------------------------------------------------------------
void ofxVPXmlEngine::removeObjectLink(int objid, int outlet_order, int link_order){
    pugi::xpath_node_set outlets = getObjectOutlets(objid);
    pugi::xml_node outletParent;

    if(!outlets.empty()){
        int index = 0;
        for(auto & outlet: outlets){
            if(index == outlet_order){
                outletParent = outlet.node();
                break;
            }
            index++;
        }

        pugi::xpath_node_set links = getObjectLinks(objid,outlet_order);
        if(!links.empty()){
            int index2 = 0;
            for(auto & link: links){
                if(index2 == link_order){
                    auto l = link.node();
                    outletParent.remove_child(l);
                    break;
                }
                index2++;
            }
        }
    }

    saveMosaicPatch(filepath);
}

//--------------------------------------------------------------
void ofxVPXmlEngine::removeAllObjectOutletLinks(int objid, int outlet_order){
    pugi::xpath_node_set outlets = getObjectOutlets(objid);
    pugi::xml_node outletParent;

    if(!outlets.empty()){
        int index = 0;
        for(auto & outlet: outlets){
            if(index == outlet_order){
                outletParent = outlet.node();
                break;
            }
            index++;
        }

        pugi::xpath_node_set links = getObjectLinks(objid,outlet_order);
        if(!links.empty()){
            for(auto & link: links){
                auto l = link.node();
                outletParent.remove_child(l);
            }
        }
    }

    saveMosaicPatch(filepath);
}

//--------------------------------------------------------------
void ofxVPXmlEngine::removeAllObjectLinks(int objid){
    pugi::xpath_node_set outlets = getObjectOutlets(objid);

    if(!outlets.empty()){
        int index = 0;
        for(auto & outlet: outlets){
            pugi::xml_node outletParent = outlet.node();
            pugi::xpath_node_set links = getObjectLinks(objid,index);
            if(!links.empty()){
                for(auto & link: links){
                    auto l = link.node();
                    outletParent.remove_child(l);
                }
            }
            index++;
        }
    }

    saveMosaicPatch(filepath);
}


//--------------------------------------------------------------
void ofxVPXmlEngine::addObjectVar(int objid, std::string varName, float varValue){
    pugi::xml_node n = getObjectNode(objid);
    if(checkIsObjectNode(n)){
        pugi::xml_node vars = getPatchChildNode(n,"vars");
        pugi::xml_node newVar = vars.append_child("var");

        newVar.append_child("name");
        newVar.append_child("value");

        setPatchValue(newVar,"name", varName);
        setPatchValue(newVar,"value", varValue);

        saveMosaicPatch(filepath);
    }

}

//--------------------------------------------------------------
void ofxVPXmlEngine::setObjectVar(int objid, std::string varName, float varValue){
    pugi::xpath_node_set vars = getObjectVars(objid);

    if(!vars.empty()){
        for(auto & var: vars){
            auto v = var.node();
            if(getPatchChildString(v,"name") == varName){
                setPatchValue(v,"value",varValue);
                break;
            }
        }

        saveMosaicPatch(filepath);
    }
}

//--------------------------------------------------------------
void ofxVPXmlEngine::removeObjectVar(int objid, std::string varName){
    pugi::xml_node n = getObjectNode(objid);
    pugi::xml_node varsParent = getPatchChildNode(n,"vars");
    pugi::xpath_node_set vars = getObjectVars(objid);

    if(!vars.empty() && checkIsObjectNode(n)){
        for(auto & var: vars){
            auto v = var.node();
            if(getPatchChildString(v,"name") == varName){
                varsParent.remove_child(v);
                break;
            }
        }

        saveMosaicPatch(filepath);
    }
}

//--------------------------------------------------------------
void ofxVPXmlEngine::setMosaicConfig(std::string configVar, int value){
    setPatchValue(settingNode,configVar,value);
    saveMosaicPatch(filepath);
}

//--------------------------------------------------------------
int ofxVPXmlEngine::getMosaicConfig(std::string configVar){
    return getPatchChildInt(settingNode,configVar);
}

//--------------------------------------------------------------
pugi::xml_node ofxVPXmlEngine::getObjectNode(int id){
    if(isMosaicPatch){
        auto objectsXml = getPatchObjects();
        for(auto & obj: objectsXml){
            auto n = obj.node();
            if(getPatchChildInt(n,"id") == id){
                return n;
            }
        }
    }
    return empty;
}

//--------------------------------------------------------------
std::string ofxVPXmlEngine::getObjectName(int id){
    pugi::xml_node n = getObjectNode(id);
    if(checkIsObjectNode(n)){
        return getPatchChildString(n,"name");
    }
    return "";
}

//--------------------------------------------------------------
std::string ofxVPXmlEngine::getObjectFilepath(int id){
    pugi::xml_node n = getObjectNode(id);
    if(checkIsObjectNode(n)){
        return getPatchChildString(n,"filepath");
    }
    return "";
}

//--------------------------------------------------------------
std::string ofxVPXmlEngine::getObjectSubpatch(int id){
    pugi::xml_node n = getObjectNode(id);
    if(checkIsObjectNode(n)){
        return getPatchChildString(n,"subpatch");
    }
    return "";
}

//--------------------------------------------------------------
ofVec2f ofxVPXmlEngine::getObjectPosition(int id){
    pugi::xml_node n = getObjectNode(id);
    if(checkIsObjectNode(n)){
        pugi::xml_node pos = getPatchChildNode(n,"position");
        return ofVec2f(getPatchChildFloat(pos,"x"),getPatchChildFloat(pos,"y"));
    }

    return ofVec2f(0,0);
}

//--------------------------------------------------------------
pugi::xpath_node_set ofxVPXmlEngine::getObjectVars(int id){
    pugi::xml_node n = getObjectNode(id);
    if(checkIsObjectNode(n)){
        pugi::xml_node vars = getPatchChildNode(n,"vars");
        auto objVars = vars.select_nodes("var");
        if(objVars.empty()){
            return empty_set;
        }else{
            return objVars;
        }
    }
    return empty_set;
}

//--------------------------------------------------------------
pugi::xpath_node_set ofxVPXmlEngine::getObjectInlets(int id){
    pugi::xml_node n = getObjectNode(id);
    if(checkIsObjectNode(n)){
        pugi::xml_node inlets = getPatchChildNode(n,"inlets");
        auto objInlets = inlets.select_nodes("link");
        if(objInlets.empty()){
            return empty_set;
        }else{
            return objInlets;
        }
    }
    return empty_set;
}

//--------------------------------------------------------------
pugi::xpath_node_set ofxVPXmlEngine::getObjectOutlets(int id){
    pugi::xml_node n = getObjectNode(id);
    if(checkIsObjectNode(n)){
        pugi::xml_node outlets = getPatchChildNode(n,"outlets");
        auto objOutlets = outlets.select_nodes("link");
        if(objOutlets.empty()){
            return empty_set;
        }else{
            return objOutlets;
        }
    }
    return empty_set;
}

//--------------------------------------------------------------
pugi::xpath_node_set ofxVPXmlEngine::getObjectLinks(int objid, int outlet_order){
    pugi::xml_node n = getObjectNode(objid);
    if(checkIsObjectNode(n)){
        pugi::xml_node outlets = getPatchChildNode(n,"outlets");
        auto objOutlets = outlets.select_nodes("link");
        if(objOutlets.empty()){
            return empty_set;
        }else{
            int index = 0;
            for(auto & link: objOutlets){
                if(index == outlet_order){
                    pugi::xml_node selectedOutlet = link.node();

                    auto outletLinks = selectedOutlet.select_nodes("to");
                    if(outletLinks.empty()){
                        return empty_set;
                    }else{
                        return outletLinks;
                    }
                }
                index++;
            }
        }
    }
    return empty_set;
}

//--------------------------------------------------------------
float ofxVPXmlEngine::getObjectVar(int objid, std::string varname){

    pugi::xpath_node_set vars = getObjectVars(objid);

    if(vars.empty()){
        return 0.0f;
    }else{
        for(auto & var: vars){
            auto v = var.node();
            if(getPatchChildString(v,"name") == varname){
                return getPatchChildFloat(v,"value");
            }
        }
    }

    return 0.0f;
}

//--------------------------------------------------------------
int ofxVPXmlEngine::getInletType(int objid, int inlet_order){
    pugi::xpath_node_set inlets = getObjectInlets(objid);

    if(inlets.empty()){
        return -1;
    }else{
        int index = 0;
        for(auto & inlet: inlets){
            if(index == inlet_order){
                auto i = inlet.node();
                return getPatchChildInt(i,"type");
            }
            index++;
        }
    }

    return -1;

}

//--------------------------------------------------------------
std::string ofxVPXmlEngine::getInletName(int objid, int inlet_order){
    pugi::xpath_node_set inlets = getObjectInlets(objid);

    if(inlets.empty()){
        return "";
    }else{
        int index = 0;
        for(auto & inlet: inlets){
            if(index == inlet_order){
                auto i = inlet.node();
                return getPatchChildString(i,"name");
            }
            index++;
        }
    }

    return "";

}

//--------------------------------------------------------------
int ofxVPXmlEngine::getOutletType(int objid, int outlet_order){
    pugi::xpath_node_set outlets = getObjectOutlets(objid);

    if(outlets.empty()){
        return -1;
    }else{
        int index = 0;
        for(auto & outlet: outlets){
            if(index == outlet_order){
                auto o = outlet.node();
                return getPatchChildInt(o,"type");
            }
            index++;
        }
    }

    return -1;

}

//--------------------------------------------------------------
std::string ofxVPXmlEngine::getOutletName(int objid, int outlet_order){
    pugi::xpath_node_set outlets = getObjectOutlets(objid);

    if(outlets.empty()){
        return "";
    }else{
        int index = 0;
        for(auto & outlet: outlets){
            if(index == outlet_order){
                auto o = outlet.node();
                return getPatchChildString(o,"name");
            }
            index++;
        }
    }

    return "";

}

//--------------------------------------------------------------
int ofxVPXmlEngine::getLinkID(int objid, int outlet_order, int link_order){
    pugi::xpath_node_set links = getObjectLinks(objid,outlet_order);

    if(links.empty()){
        return -1;
    }else{
        int index = 0;
        for(auto & link: links){
            if(index == link_order){
                auto l = link.node();
                return getPatchChildInt(l,"id");
            }
            index++;
        }
    }

    return -1;
}

//--------------------------------------------------------------
int ofxVPXmlEngine::getLinkToInlet(int objid, int outlet_order, int link_order){
    pugi::xpath_node_set links = getObjectLinks(objid,outlet_order);

    if(links.empty()){
        return -1;
    }else{
        int index = 0;
        for(auto & link: links){
            if(index == link_order){
                auto l = link.node();
                return getPatchChildInt(l,"inlet");
            }
            index++;
        }
    }

    return -1;
}
