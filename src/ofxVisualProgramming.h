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

#include "ofMain.h"

#include "include.h"
#include "config.h"

#include "ofxGLError.h"
#include "ofxInfiniteCanvas.h"
#include "ofxTimeMeasurements.h"

#include "PatchObject.h"


class ofxVisualProgramming {
    
public:

    ofxVisualProgramming();
    ~ofxVisualProgramming();

    void            setup();
    void            initObjectMatrix();
    void            setupGUI();
    void            update();
    void            draw();
    void            exit();

    void            mouseMoved(ofMouseEventArgs &e);
    void            mouseDragged(ofMouseEventArgs &e);
    void            mousePressed(ofMouseEventArgs &e);
    void            mouseReleased(ofMouseEventArgs &e);
    void            mouseScrolled(ofMouseEventArgs &e);

    void            audioIn(ofSoundBuffer &inputBuffer);
    void            audioOut(ofSoundBuffer &outBuffer);

    // GUI
    void            onButtonEvent(ofxDatGuiButtonEvent e);
    void            onScrollViewEvent(ofxDatGuiScrollViewEvent e);

    void            activeObject(int oid);

    PatchObject*    selectObject(string objname);
    void            addObject(string name, ofVec2f pos);
    void            dragObject(int &id);
    void            resetObject(int &id);
    void            removeObject(int &id);
    void            iconifyObject(int &id);
    bool            connect(int fromID, int fromOutlet, int toID,int toInlet, int linkType);

    void            newPatch();
    void            openPatch(string patchFile);
    void            loadPatch(string patchFile);
    void            savePatchAs(string patchFile);
    void            setPatchVariable(string var, int value);

    void            setAudioInDevice(int index);
    void            setAudioOutDevice(int index);

    // PATCH CANVAS
    ofxInfiniteCanvas       canvas;
    ofEasyCam               easyCam;

    // PATCH DRAWING RESOURCES
    ofxFontStash            *font;
    int                     fontSize;
    bool                    isRetina;
    int                     scaleFactor;

    // PATCH OBJECTS
    map<int,PatchObject*>   patchObjects;
    vector<int>             eraseIndexes;
    int                     selectedObjectLinkType;
    int                     selectedObjectLink;
    int                     selectedObjectID;
    bool                    draggingObject;
    int                     actualObjectID;

    // LOAD/SAVE
    string                  currentPatchFile;
    string                  tempPatchFile;
    int                     output_width;
    int                     output_height;
    string                  alphabet;
    int                     newFileCounter;

    // SYSTEM
    shared_ptr<ofAppGLFWWindow>     mainWindow;
    ofxGLError                      glError;
    string                          glVersion;
    string                          glShadingVersion;

    // GUI
    ofxDatGui                       *gui;
    ofxDatGuiHeader                 *guiHeader;
    map<string,vector<string>>      objectsMatrix;
    vector<ofxDatGuiScrollView*>    objectNavigators;
    vector<ofxDatGuiFolder*>        objectFolders;
    bool                            isOverGui;

    // Sound Stream
    vector<ofSoundDevice>   audioDevices;
    vector<string>          audioDevicesString;
    ofSoundStream           soundStreamIN;
    ofSoundStreamSettings   soundStreamINSettings;
    ofSoundStream           soundStreamOUT;
    ofSoundStreamSettings   soundStreamOUTSettings;
    std::mutex              inputAudioMutex;
    std::mutex              outputAudioMutex;
    ofSoundBuffer           lastInputBuffer;
    ofSoundBuffer           lastOutputBuffer;
    ofPolyline              inputBufferWaveform;
    ofPolyline              outputBufferWaveform;
    int                     audioINDev;
    int                     audioOUTDev;
    int                     audioBufferSize;

    // MEMORY
    uint64_t                resetTime;
    uint64_t                wait;
    
private:
    
    
};
