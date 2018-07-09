#pragma once

#include "PatchObject.h"

#include "ofxLua.h"
#include "PathWatcher.h"
#include "ThreadedCommand.h"

#include <atomic>

class LuaScript : public ofThread, public PatchObject, public ofxLuaListener{

public:

    LuaScript();

    void            threadedFunction();

    void            newObject();
    void            setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow);
    void            updateObjectContent(map<int,PatchObject*> &patchObjects);
    void            drawObjectContent(ofxFontStash *font);
    void            removeObjectContent();
    void            mouseMovedObjectContent(ofVec3f _m);
    void            dragGUIObject(ofVec3f _m);

    void            loadScript(string scriptFile);
    void            reloadScriptThreaded();
    bool            loadProjectorSettings();
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
    int                 output_width, output_height;

    string              mosaicTableName;
    string              tempstring;

protected:
    ThreadedCommand         tempCommand;
    std::condition_variable condition;
    bool                    needToLoadScript;
    bool                    threadLoaded;

};
