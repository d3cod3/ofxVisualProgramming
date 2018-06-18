#pragma once

#include "PatchObject.h"

class SimpleNoise : public PatchObject {

public:

    SimpleNoise();

    void            newObject();
    void            setupObjectContent(shared_ptr<ofAppBaseWindow> &mainWindow);
    void            updateObjectContent();
    void            drawObjectContent(ofxFontStash *font);
    void            removeObjectContent();

    float           timePosition;

};
