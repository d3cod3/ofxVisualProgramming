#pragma once

#include "PatchObject.h"

#include "ofxLua.h"
#include "PathWatcher.h"

class LuaScript : public PatchObject, ofxLuaListener{

public:

    LuaScript();

    void            newObject();
    void            setupObjectContent(shared_ptr<ofAppBaseWindow> &mainWindow);
    void            updateObjectContent();
    void            drawObjectContent(ofxFontStash *font);
    void            removeObjectContent();
    void            mouseMovedObjectContent(ofVec3f _m);
    void            dragGUIObject(ofVec3f _m);

    void            loadScript(string scriptFile);
    void            onButtonEvent(ofxDatGuiButtonEvent e);

    // Filepath watcher callback
    void            pathChanged(const PathWatcher::Event &event);

    // ofxLua error callback
    void            errorReceived(std::string& msg);

    ofxLua              lua;
    PathWatcher         watcher;
    bool                scriptLoaded;
    bool                isNewObject;

    ofxDatGui*          gui;
    ofxDatGuiButton*    loadButton;
    ofxDatGuiButton*    editButton;
    bool                isOverGui;

    ofFbo               *fbo;
    ofImage             *kuro;
    float               scaleH;

    string              mosaicTableName;
    string              tempstring;

};
