#pragma once

#include "PatchObject.h"

#include "ofxPython.h"
#include "PathWatcher.h"

class PythonScript : public PatchObject {

public:

    PythonScript();

    void            newObject();
    void            setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow);
    void            updateObjectContent(map<int,PatchObject*> &patchObjects);
    void            drawObjectContent(ofxFontStash *font);
    void            removeObjectContent();
    void            mouseMovedObjectContent(ofVec3f _m);
    void            dragGUIObject(ofVec3f _m);

    void            loadScript(string scriptFile);
    bool            loadProjectorSettings();
    void            onButtonEvent(ofxDatGuiButtonEvent e);

    // Filepath watcher callback
    void            pathChanged(const PathWatcher::Event &event);

    ofxPython           python;
    ofxPythonObject     klass;
    ofxPythonObject     script;
    ofxPythonObject     updatePython;
    ofxPythonObject     updateMosaicList;
    ofxPythonObject     drawPython;

    PathWatcher         watcher;
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

};
