#pragma once

#include "PatchObject.h"
#include "ofxLua.h"

class OutputWindow : public PatchObject {

public:

    OutputWindow();

    void            newObject();
    void            setupObjectContent(shared_ptr<ofAppBaseWindow> &mainWindow);
    void            updateObjectContent();
    void            drawObjectContent(ofxFontStash *font);
    void            removeObjectContent();

    glm::vec2       reduceToAspectRatio(int _w, int _h);
    void            toggleWindowFullscreen();
    void            drawInWindow(ofEventArgs &e);

    void            keyPressed(ofKeyEventArgs &e);
    void            keyReleased(ofKeyEventArgs &e);
    void            mouseMoved(ofMouseEventArgs &e);
    void            mouseDragged(ofMouseEventArgs &e);
    void            mousePressed(ofMouseEventArgs &e);
    void            mouseReleased(ofMouseEventArgs &e);
    void            mouseScrolled(ofMouseEventArgs &e);

    std::shared_ptr<ofAppBaseWindow>        window;
    bool                                    isFullscreen;

    int                                     output_width, output_height;
    int                                     window_actual_width, window_actual_height;
    float                                   posX, posY, drawW, drawH;
    glm::vec2                               asRatio;
    float                                   scaleH;

};
