/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2018 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

    ofxVisualProgramming is distributed under the MIT License.
    This gives everyone the freedoms to use ofxVisualProgramming in any context:
    commercial or non-commercial, public or private, open or closed source.

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.

    See https://github.com/d3cod3/ofxVisualProgramming for documentation

==============================================================================*/

#pragma once

#include "PatchObject.h"

#if defined(TARGET_LINUX) || defined(TARGET_OSX)
#include "ofxPython.h"
#endif

#include "ofxWarp.h"

class OutputWindow : public PatchObject {

public:

    OutputWindow();

    void            newObject();
    void            setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow);
    void            updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd);
    void            drawObjectContent(ofxFontStash *font);
    void            removeObjectContent(bool removeFileFromData=false);
    void            mouseMovedObjectContent(ofVec3f _m);
    void            dragGUIObject(ofVec3f _m);
    void            fileDialogResponse(ofxThreadedFileDialogResponse &response);

    glm::vec2       reduceToAspectRatio(int _w, int _h);
    void            scaleTextureToWindow(int theScreenW, int theScreenH);
    void            toggleWindowFullscreen();
    void            drawInWindow(ofEventArgs &e);

    void            loadWindowSettings();
    void            resetResolution();

    void            keyPressed(ofKeyEventArgs &e);
    void            keyReleased(ofKeyEventArgs &e);
    void            mouseMoved(ofMouseEventArgs &e);
    void            mouseDragged(ofMouseEventArgs &e);
    void            mousePressed(ofMouseEventArgs &e);
    void            mouseReleased(ofMouseEventArgs &e);
    void            mouseScrolled(ofMouseEventArgs &e);
    void            windowResized(ofResizeEventArgs &e);

    void            onToggleEvent(ofxDatGuiToggleEvent e);
    void            onButtonEvent(ofxDatGuiButtonEvent e);
    void            onTextInputEvent(ofxDatGuiTextInputEvent e);
    void            onSliderEvent(ofxDatGuiSliderEvent e);


    shared_ptr<ofAppGLFWWindow>             window;
    bool                                    isFullscreen;
    bool                                    isNewScriptConnected;
    int                                     inletScriptType;

    int                                     temp_width, temp_height;
    int                                     window_actual_width, window_actual_height;
    float                                   posX, posY, drawW, drawH;
    glm::vec2                               asRatio;
    float                                   thposX, thposY, thdrawW, thdrawH;
    bool                                    needReset;

    ofxWarpController                       *warpController;
    ofFbo                                   *finalTexture;
    bool                                    isWarpingLoaded;

    ofxDatGui*                              gui;
    ofxDatGuiHeader*                        header;
    ofxDatGuiTextInput*                     guiTexWidth;
    ofxDatGuiTextInput*                     guiTexHeight;
    ofxDatGuiButton*                        applyButton;
    ofxDatGuiToggle*                        useMapping;
    ofxDatGuiSlider*                        edgesExponent;
    ofxDatGuiSlider*                        edgeL;
    ofxDatGuiSlider*                        edgeR;
    ofxDatGuiSlider*                        edgeT;
    ofxDatGuiSlider*                        edgeB;
    ofxDatGuiButton*                        loadWarping;
    ofxDatGuiButton*                        saveWarping;

    string                                  lastWarpingConfig;
    bool                                    loadWarpingFlag;
    bool                                    saveWarpingFlag;
    bool                                    warpingConfigLoaded;

    bool                                    loaded;
    bool                                    autoRemove;

    OBJECT_FACTORY_PROPS;
};
