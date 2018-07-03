#pragma once

#include "PatchObject.h"

class moSignalViewer : public PatchObject {

public:

    moSignalViewer();

    void            newObject();
    void            setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow);
    void            updateObjectContent(map<int,PatchObject*> &patchObjects);
    void            drawObjectContent(ofxFontStash *font);
    void            removeObjectContent();

    ofPolyline      waveform;

};
