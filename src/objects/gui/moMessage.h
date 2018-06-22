#pragma once

#include "PatchObject.h"

class moMessage : public PatchObject {

public:

    moMessage();

    void            newObject();
    void            setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow);
    void            updateObjectContent();
    void            drawObjectContent(ofxFontStash *font);
    void            removeObjectContent();
    void            mouseMovedObjectContent(ofVec3f _m);
    void            dragGUIObject(ofVec3f _m);

    void            onButtonEvent(ofxDatGuiButtonEvent e);
    void            onTextInputEvent(ofxDatGuiTextInputEvent e);

    ofxDatGui*          gui;
    ofxDatGuiButton*    sendButton;
    ofxDatGuiTextInput* message;

    bool                isOverGui;
    string              actualMessage;

};
