/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2018 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

    Mosaic is distributed under the MIT License. This gives everyone the
    freedoms to use Mosaic in any context: commercial or non-commercial,
    public or private, open or closed source.

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

#include "config.h"
#include "utils.h"

#include "ofxFontStash.h"
#include "ofxDatGui.h"
#include "ofxXmlSettings.h"

#include "DraggableVertex.h"

enum LINK_TYPE {
    VP_LINK_NUMERIC,
    VP_LINK_STRING,
    VP_LINK_ARRAY,
    VP_LINK_TEXTURE,
    VP_LINK_AUDIO,
    VP_LINK_SCRIPT
};

struct PatchLink{
    vector<DraggableVertex> linkVertices;
    ofVec2f                 posFrom;
    ofVec2f                 posTo;
    int                     type;
    int                     fromOutletID;
    int                     toObjectID;
    int                     toInletID;
    bool                    isDisabled;
};

struct PushButton{
    char letter;
    bool *state;
    int  offset;
};

class PatchObject {

public:

    PatchObject();
    virtual ~PatchObject();

    void                    setup(shared_ptr<ofAppGLFWWindow> &mainWindow);
    void                    update(map<int,PatchObject*> &patchObjects);
    void                    draw(ofxFontStash *font);

    // Virtual Methods
    virtual void            newObject() {}
    virtual void            setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow) {}
    virtual void            updateObjectContent(map<int,PatchObject*> &patchObjects) {}
    virtual void            drawObjectContent(ofxFontStash *font) {}
    virtual void            removeObjectContent() {}
    virtual void            mouseMovedObjectContent(ofVec3f _m) {}
    virtual void            dragGUIObject(ofVec3f _m) {}
    virtual void            audioInObject(ofSoundBuffer &inputBuffer) {}
    virtual void            audioOutObject(ofSoundBuffer &outBuffer) {}

    // Mouse Events
    void                    mouseMoved(float mx, float my);
    void                    mouseDragged(float mx, float my);
    void                    mousePressed(float mx, float my);
    void                    mouseReleased(float mx, float my);

    // Sound
    void                    audioIn(ofSoundBuffer &inputBuffer);
    void                    audioOut(ofSoundBuffer &outBuffer);

    void                    move(int _x, int _y);
    bool                    isOver(ofPoint pos);
    void                    iconify();
    ofVec2f                 getInletPosition(int iid);
    ofVec2f                 getOutletPosition(int oid);

    // LOAD/SAVE
    bool                    loadConfig(shared_ptr<ofAppGLFWWindow> &mainWindow,int oTag, string &configFile);
    bool                    saveConfig(bool newConnection,int objID);
    bool                    removeLinkFromConfig(int outlet);

    void                    addButton(char letter, bool *variableToControl, int offset);
    void                    addInlet(int type,string name) { inlets.push_back(type);inletsNames.push_back(name); }
    void                    addOutlet(int type) { outlets.push_back(type); }
    void                    setCustomVar(float value, string name){ customVars[name] = value; }
    float                   getCustomVar(string name) { return customVars[name]; }

    // GETTERS
    int                     getId() const { return nId; }
    ofPoint                 getPos() const { return ofPoint(x,y); }
    string                  getName() const { return name; }
    bool                    getIsActive() const { return bActive; }
    int                     getInletType(int iid) const { return inlets[iid]; }
    int                     getOutletType(int oid) const { return outlets[oid]; }
    int                     getNumInlets() { return inlets.size(); }
    int                     getNumOutlets() { return outlets.size(); }
    bool                    getWillErase() { return willErase; }

    // SETTERS
    void                    setName(string _name) { name = _name; }
    void                    setFilepath(string fp) { filepath = fp; }
    void                    setPatchfile(string pf) { patchFile = pf; }
    void                    setIsRetina(bool ir) { isRetina = ir; }
    void                    setIsActive(bool ia) { bActive = ia; }

    // UTILS
    void                    bezierLink(DraggableVertex from, DraggableVertex to, float _width);

    // patch object connections
    vector<PatchLink*>      outPut;
    vector<bool>            inletsConnected;

    void                    *_inletParams[MAX_INLETS];
    void                    *_outletParams[MAX_OUTLETS];

    // heaader buttons
    vector<PushButton*>     headerButtons;
    ofRectangle             *headerBox;

    ofEvent<int>            removeEvent;
    ofEvent<int>            dragEvent;
    ofEvent<int>            iconifyEvent;

protected:

    // Drawing vars
    ofRectangle             *box;
    ofColor                 *color;
    float                   x, y, width, height, headerHeight;
    int                     letterWidth, letterHeight, offSetWidth;
    int                     buttonOffset;
    int                     fontSize;

    // Core vars
    string                  name;
    string                  filepath;
    string                  patchFile;
    vector<string>          inletsNames;
    vector<int>             inlets;
    vector<int>             outlets;
    map<string,float>       customVars;
    int                     numInlets;
    int                     numOutlets;
    int                     nId;
    bool                    bActive;
    bool                    iconified;
    bool                    isMouseOver;
    bool                    isRetina;
    bool                    isGUIObject;
    bool                    isBigGuiViewer;
    bool                    isAudioINObject;
    bool                    isAudioOUTObject;
    bool                    willErase;
    float                   retinaScale;

};
