#pragma once

#include "PatchObject.h"

class moVideoViewer : public PatchObject {

public:

    moVideoViewer();

    void            newObject();
    void            setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow);
    void            updateObjectContent(map<int,PatchObject*> &patchObjects);
    void            drawObjectContent(ofxFontStash *font);
    void            removeObjectContent();

    float           scaleH;

};
