#pragma once

#include "PatchObject.h"

class SimpleRandom : public PatchObject {

public:

    SimpleRandom();

    void            newObject();
    void            setupObjectContent(shared_ptr<ofAppBaseWindow> &mainWindow);
    void            updateObjectContent();
    void            drawObjectContent(ofxFontStash *font);
    void            removeObjectContent();

    ofxDatGui*              gui;
    ofxDatGuiValuePlotter*  rPlotter;

    bool                    changeRange;
    float                   lastMinRange, lastMaxRange;

};
