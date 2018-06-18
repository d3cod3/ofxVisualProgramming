#pragma once

#include "PatchObject.h"

class moSlider : public PatchObject {

public:

    moSlider();

    void            newObject();
    void            setupObjectContent(shared_ptr<ofAppBaseWindow> &mainWindow);
    void            updateObjectContent();
    void            drawObjectContent(ofxFontStash *font);
    void            removeObjectContent();
    void            mouseMovedObjectContent(ofVec3f _m);
    void            dragGUIObject(ofVec3f _m);

    void            onSliderEvent(ofxDatGuiSliderEvent e);

    ofxDatGui*          gui;
    ofxDatGuiSlider*    slider;
    bool                isOverGui;

};
