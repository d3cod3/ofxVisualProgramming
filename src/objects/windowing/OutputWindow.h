#pragma once

#include "PatchObject.h"
#include "ofxLua.h"
#include "ofxPython.h"

class OutputWindow : public PatchObject {

public:

    OutputWindow();

    void            newObject();
    void            setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow);
    void            updateObjectContent(map<int,PatchObject*> &patchObjects);
    void            drawObjectContent(ofxFontStash *font);
    void            removeObjectContent();

    glm::vec2       reduceToAspectRatio(int _w, int _h);
    void            scaleTextureToWindow(int theScreenW, int theScreenH);
    void            toggleWindowFullscreen();
    void            drawInWindow(ofEventArgs &e);

    bool            loadWindowSettings();

    void            keyPressed(ofKeyEventArgs &e);
    void            keyReleased(ofKeyEventArgs &e);
    void            mouseMoved(ofMouseEventArgs &e);
    void            mouseDragged(ofMouseEventArgs &e);
    void            mousePressed(ofMouseEventArgs &e);
    void            mouseReleased(ofMouseEventArgs &e);
    void            mouseScrolled(ofMouseEventArgs &e);
    void            windowResized(ofResizeEventArgs &e);

    std::shared_ptr<ofAppGLFWWindow>        window;
    bool                                    isFullscreen;
    bool                                    isNewScriptConnected;
    int                                     inletScriptType;

    int                                     output_width, output_height;
    int                                     window_actual_width, window_actual_height;
    float                                   posX, posY, drawW, drawH;
    glm::vec2                               asRatio;
    float                                   scaleH;

};
