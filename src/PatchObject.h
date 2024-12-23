/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2018 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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
#include "utils.h"

#include "ofxXmlSettings.h"
#include "ofxPDSP.h"
#include "ofxPingPong.h"

#include "objectFactory.h"
#include "ofxVPHasUid.h"
#include "ofxVPObjectParameter.h"

#include "ofxImGui.h"
#include "imgui_node_canvas.h"
#include "imgui_helpers.h"

#include "Driver.h"

enum LINK_TYPE {
    VP_LINK_NUMERIC,    // 0
    VP_LINK_STRING,     // 1
    VP_LINK_ARRAY,      // 2
    VP_LINK_TEXTURE,    // 3
    VP_LINK_AUDIO,      // 4
    VP_LINK_SPECIAL,    // 5
    VP_LINK_PIXELS,     // 6
    VP_LINK_FBO         // 7
};

struct PatchLink{
    ImVec2                  posFrom;
    ImVec2                  posTo;
    int                     type;
    int                     fromOutletID;
    int                     toObjectID;
    int                     toInletID;
    int                     id;
    bool                    isDisabled;
    bool                    isDeactivated;
};


class PatchObject : public ofxVPHasUID {

public:

    PatchObject(const std::string& _customUID = "patchObject");
    virtual ~PatchObject();

    void                    setup(shared_ptr<ofAppGLFWWindow> &mainWindow);
    void                    setupDSP(pdsp::Engine &engine);
    void                    update(map<int,shared_ptr<PatchObject>> &patchObjects, pdsp::Engine &engine);
    void                    updateWirelessLinks(map<int,shared_ptr<PatchObject>> &patchObjects);
    void                    draw(ofTrueTypeFont *font);
    void                    drawImGuiNode(ImGuiEx::NodeCanvas& _nodeCanvas, map<int,shared_ptr<PatchObject>> &patchObjects);
    void                    drawImGuiNodeConfig();

    // Virtual Methods
    virtual void            newObject() {}

    virtual void            autoloadFile(string _fp) { unusedArgs(_fp); }
    virtual void            autosaveNewFile(string fromFile) { unusedArgs(fromFile); }

    virtual void            setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow) { unusedArgs(mainWindow); }
    virtual void            setupAudioOutObjectContent(pdsp::Engine &engine) { unusedArgs(engine); }
    virtual void            updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects) { unusedArgs(patchObjects); }
    virtual void            updateAudioObjectContent(pdsp::Engine &engine) { unusedArgs(engine); }
    virtual void            drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer) { unusedArgs(font,glRenderer); }
    virtual void            drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ) { unusedArgs(_nodeCanvas); }
    virtual void            drawObjectNodeConfig() {}
    virtual void            removeObjectContent(bool removeFileFromData=false) { unusedArgs(removeFileFromData); }

    virtual void            audioInObject(ofSoundBuffer &inputBuffer) { unusedArgs(inputBuffer); }
    virtual void            audioOutObject(ofSoundBuffer &outputBuffer) { unusedArgs(outputBuffer); }

    virtual void            customReset() {}
    virtual void            resetSystemObject() {}
    virtual void            resetResolution(int fromID=-1, int newWidth=-1, int newHeight=-1) { unusedArgs(fromID,newWidth,newHeight); }

    // Keyboard Events
    void                    keyPressed(ofKeyEventArgs &e,map<int,shared_ptr<PatchObject>> &patchObjects);
    void                    keyReleased(ofKeyEventArgs &e,map<int,shared_ptr<PatchObject>> &patchObjects);

    // Sound
    void                    audioIn(ofSoundBuffer &inputBuffer);
    void                    audioOut(ofSoundBuffer &outputBuffer);

    void                    move(int _x, int _y);

    // PatchLinks utils
    bool                    connectTo(map<int,shared_ptr<PatchObject>> &patchObjects, int fromObjectID, int fromOutlet, int toInlet, int linkType);
    void                    disconnectFrom(map<int,shared_ptr<PatchObject>> &patchObjects, int objectInlet);
    void                    disconnectLink(map<int,shared_ptr<PatchObject>> &patchObjects, int linkID);
    void                    openWirelessLink(int objectOutlet) { if(getNumOutlets()>objectOutlet){ resetWirelessPin=objectOutlet;initWirelessLink = true; } };
    void                    closeWirelessLink(int objectOutlet) { if(getNumOutlets()>objectOutlet){ resetWirelessPin=objectOutlet;resetWirelessLink = true; } };

    // LOAD/SAVE
    bool                    loadConfig(shared_ptr<ofAppGLFWWindow> &mainWindow, pdsp::Engine &engine,int oTag, string &configFile);
    bool                    saveConfig(bool newConnection);
    bool                    removeLinkFromConfig(int outlet, int toObjectID, int toInletID);

    void                    addInlet(int type,string name) { inletsType.push_back(type);inletsNames.push_back(name); inletsIDs.push_back(""); inletsWirelessReceive.push_back(false); inletsPositions.push_back( ImVec2(this->x, this->y + this->height*.5f) ); }
    void                    addOutlet(int type,string name = "") { outletsType.push_back(type);outletsNames.push_back(name); outletsIDs.push_back(""); outletsWirelessSend.push_back(false); outletsPositions.push_back( ImVec2( this->x + this->width, this->y + this->height*.5f) ); }
    void                    initInletsState() { for(int i=0;i<numInlets;i++){ inletsConnected.push_back(false); } }
    void                    setCustomVar(float value, string name){ customVars[name] = value; saveConfig(false); }
    float                   getCustomVar(string name) { if ( customVars.find(name) != customVars.end() ) { return customVars[name]; }else{ return 0; } }
    float                   existsCustomVar(string name) { if ( customVars.find(name) != customVars.end() ) { return true; }else{ return false; } }
    void                    substituteCustomVar(string oldName, string newName) { if ( customVars.find(oldName) != customVars.end() ) { customVars[newName] = customVars[oldName]; customVars.erase(oldName); } }
    bool                    clearCustomVars();
    map<string,float>       loadCustomVars();

    // GETTERS
    int                     getId() const { return nId; }
    ofPoint                 getPos() const { return ofPoint(x,y); }
    string                  getName() const { return name; }
    string                  getSpecialName() const { return specialName; }
    bool                    getIsResizable() const { return isResizable; }
    bool                    getIsRetina() const { return isRetina; }
    bool                    getIsSystemObject() const { return isSystemObject; }
    bool                    getIsActive() const { return bActive; }
    bool                    getIsAudioINObject() const { return isAudioINObject; }
    bool                    getIsAudioOUTObject() const { return isAudioOUTObject; }
    bool                    getIsPDSPPatchableObject() const { return isPDSPPatchableObject; }
    bool                    getIsTextureObject() const { return isTextureObject; }
    bool                    getIsSharedContextObject() const { return isSharedContextObject; }
    bool                    getIsHardwareObject() const { return isHardwareObject; }
    int                     getInletType(int iid) const { return inletsType[iid]; }
    string                  getInletID(int iid) const { return inletsIDs[iid]; }
    bool                    getInletWirelessReceive(int iid) const { return inletsWirelessReceive[iid]; }
    string                  getInletTypeName(const int& iid) const;
    ofColor                 getInletColor(const int& iid) const;
    ofColor                 getOutletColor(const int& oid) const;
    int                     getOutletType(int oid) const { return outletsType[oid]; }
    string                  getOutletName(int oid) const { return outletsNames[oid]; }
    string                  getOutletID(int oid) const { return outletsIDs[oid]; }
    bool                    getOutletWirelessSend(int oid) const { return outletsWirelessSend[oid]; }
    string                  getOutletTypeName(const int& oid) const;
    ImVec2                  getInletPosition(int iid);
    ImVec2                  getOutletPosition(int oid);
    int                     getNumInlets() { return static_cast<int>(inletsType.size()); }
    int                     getNumOutlets() { return static_cast<int>(outletsType.size()); }
    bool                    getIsOutletConnected(int oid);
    bool                    getWillErase() { return willErase; }

    float                   getObjectWidth() { return width; }
    float                   getObjectHeight() { return height; }
    int                     getOutputWidth() { return output_width; }
    int                     getOutputHeight() { return output_height; }
    float                   getConfigmenuWidth() { return configMenuWidth; }

    string                  getFilepath() { return filepath; }

    // SETTERS
    void                    setName(string _name) { name = _name; }
    void                    setSpecialName(string _name) { specialName = _name; }
    void                    setFilepath(string fp) { filepath = fp; }

    void                    setPatchfile(string pf);

    void                    setIsTextureObj(bool it) { isTextureObject = it; }
    void                    setIsSharedContextObj(bool isc) { isSharedContextObject = isc; }
    void                    setIsHardwareObj(bool ih) { isHardwareObject = ih; }
    void                    setIsResizable(bool ir) { isResizable = ir; }
    void                    setIsRetina(bool ir, float sf);
    void                    setIsActive(bool ia) { bActive = ia; }
    void                    setWillErase(bool e) { willErase = e; }
    void                    setIsObjectSelected(bool s) { isObjectSelected = s; }
    void                    setConfigmenuWidth(float cmw) { configMenuWidth = cmw; }
    void                    setDimensions(float w, float h) { width = w; height = h;}
    void                    setSubpatch(string sp) { subpatchName = sp; }
    void                    setInletID(int inlet, string ID) { inletsIDs[inlet] = ID; }
    void                    setOutletID(int outlet, string ID) { outletsIDs[outlet] = ID; }
    void                    setInletWirelessReceive(int inlet, bool wireless) { inletsWirelessReceive.at(inlet) = wireless; }
    void                    setOutletWirelessSend(int outlet, bool wireless) { outletsWirelessSend.at(outlet) = wireless; }

    // PUGG Plugin System
    static const int version = 1;
    static const std::string server_name() {return "PatchObjectServer";}

    // patch object connections
    vector<shared_ptr<PatchLink>>       outPut;
    vector<int>                         linksToDisconnect;
    vector<int>                         linksDeactivated;
    vector<int>                         objectsSelected;
    vector<bool>                        inletsConnected;

    // subpatch vars
    string                              subpatchName;

    // inlets/outlets
    void                                *_inletParams[MAX_INLETS];
    void                                *_outletParams[MAX_OUTLETS];

    // PDSP nodes
    map<int,pdsp::PatchNode>            pdspIn;
    map<int,pdsp::PatchNode>            pdspOut;

    // events
    ofEvent<int>                        resetEvent;
    ofEvent<int>                        removeEvent;
    ofEvent<int>                        reconnectOutletsEvent;
    ofEvent<int>                        duplicateEvent;

    string                              specialLinkTypeName;

    // Wireless object vars
    string                              wirelessName;
    int                                 wirelessType;

protected:

    // Texture drawing object vars
    int                     output_width, output_height;

    // Drawing vars
    float                   x, y, width, height, headerHeight;
    int                     fontSize;
    ImVec2                  canvasTranslation;
    float                   canvasScale;
    float                   configMenuWidth;
    float                   scaleFactor;

    // Core vars
    string                  name;
    string                  specialName;
    string                  filepath;
    string                  patchFile;
    string                  patchFolderPath;
    vector<string>          inletsNames;
    vector<string>          outletsNames;
    vector<string>          inletsIDs;
    vector<string>          outletsIDs;
    vector<bool>            inletsWirelessReceive;
    vector<bool>            outletsWirelessSend;
    vector<ImVec2>          inletsPositions; // ImVec2 to prevent too much type casting
    vector<ImVec2>          outletsPositions; // Will hold screenpositions of pins, updated by ImGui
    vector<int>             inletsType;
    vector<int>             outletsType;
    map<string,float>       customVars;


    int                     numInlets;
    int                     numOutlets;
    int                     nId;
    bool                    isSystemObject;
    bool                    bActive;
    bool                    isObjectSelected;
    bool                    isOverGUI;
    bool                    isRetina;
    bool                    isGUIObject;
    bool                    isAudioINObject;
    bool                    isAudioOUTObject;
    bool                    isPDSPPatchableObject;
    bool                    isTextureObject;
    bool                    isSharedContextObject;
    bool                    isHardwareObject;
    bool                    isResizable;
    bool                    willErase;
    bool                    initWirelessLink;
    bool                    resetWirelessLink;
    int                     resetWirelessPin;

};

// PUGG driver class
class PatchObjectDriver : public pugg::Driver
{
public:
    PatchObjectDriver(string name, int version) : pugg::Driver(PatchObject::server_name(),name,version) {}
    virtual PatchObject* create() = 0;
};

// This macro allows easy object registration
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define OBJECT_REGISTER(TYPE, NAME, CATEGORY)                           \
namespace ofxVPObjects {                                                \
namespace factory {                                                     \
namespace {                                                             \
template<class T>                                                       \
class objectRegistration;                                               \
                                                                        \
template<>                                                              \
class objectRegistration<TYPE>{                                         \
    static const ::ofxVPObjects::factory::RegistryEntry<TYPE>& reg;     \
};                                                                      \
                                                                        \
const ::ofxVPObjects::factory::RegistryEntry<TYPE>&                     \
objectRegistration<TYPE>::reg =                                         \
::ofxVPObjects::factory::RegistryEntry<TYPE>::Instance(NAME, CATEGORY); \
                                                                        \
}}}                                                                     \
const std::string TYPE::objectName(NAME);                               \
const std::string TYPE::objectCategory(CATEGORY);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define OBJECT_FACTORY_PROPS                    \
private:                                        \
    static const std::string objectName;        \
    static const std::string objectCategory;    \

// ---------------------------------------------
