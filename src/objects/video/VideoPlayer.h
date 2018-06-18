#pragma once

#include "PatchObject.h"

class VideoPlayer : public PatchObject {

public:

    VideoPlayer();

    void            newObject();
    void            setupObjectContent(shared_ptr<ofAppBaseWindow> &mainWindow);
    void            updateObjectContent();
    void            drawObjectContent(ofxFontStash *font);
    void            removeObjectContent();
    void            mouseMovedObjectContent(ofVec3f _m);
    void            dragGUIObject(ofVec3f _m);

    void            loadVideoFile(string videofile);
    void            onButtonEvent(ofxDatGuiButtonEvent e);

    ofVideoPlayer*      video;
    float               scaleH;
    bool                isNewObject;

    ofxDatGui*          gui;
    ofxDatGuiButton*    loadButton;
    bool                isOverGui;

    string              lastMessage;

};
