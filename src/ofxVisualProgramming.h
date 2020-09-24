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

#include "config.h"

#include "ofxInfiniteCanvas.h"
#include "ofxPDSP.h"
#include "ofxImGui.h"
#include "imgui_node_canvas.h"
#include "imgui_profiler.h"
#include "FileBrowser/ImGuiFileBrowser.h"
#include "IconsFontAwesome5.h"

#include "Kernel.h"
#include "PatchObject.h"


#define OFXVP_DEBUG 0


class ofxVisualProgramming : public pdsp::Wrapper {
    
public:

    ofxVisualProgramming();
    ~ofxVisualProgramming();

    void            setup(ofxImGui::Gui* guiRef = nullptr);
    void            update();
    void            updateCanvasViewport();
    void            draw();
    void            drawInspector();
    void            drawLivePatchingSession();
    void            resetTempFolder();
    void            cleanPatchDataFolder();
    void            exit();

    void            mouseMoved(ofMouseEventArgs &e);
    void            mouseDragged(ofMouseEventArgs &e);
    void            mousePressed(ofMouseEventArgs &e);
    void            mouseReleased(ofMouseEventArgs &e);
    void            mouseScrolled(ofMouseEventArgs &e);

    void            keyPressed(ofKeyEventArgs &e);
    void            keyReleased(ofKeyEventArgs &e);

    void            activeObject(int oid);

    shared_ptr<PatchObject>    selectObject(string objname);
    void            addObject(string name, ofVec2f pos);
    shared_ptr<PatchObject>    getLastAddedObject();

    void            resetObject(int &id);
    void            resetObject(int id);
    void            reconnectObjectOutlets(int &id);
    void            removeObject(int &id);
    void            duplicateObject(int &id);

    bool            connect(int fromID, int fromOutlet, int toID,int toInlet, int linkType);
    void            checkSpecialConnection(int fromID, int toID, int linkType);
    void            resetSystemObjects();
    void            resetSpecificSystemObjects(string name);
    bool            weAlreadyHaveObject(string name);
    void            deleteObject(int id);
    void            clearObjectsMap();

    void            newPatch();
    void            newTempPatchFromFile(string patchFile);
    void            preloadPatch(string patchFile);
    void            openPatch(string patchFile);
    void            loadPatch(string patchFile);
    void            reloadPatch();
    void            savePatchAs(string patchFile);
    void            setPatchVariable(string var, int value);

    void            setAudioInDevice(int ind);
    void            setAudioOutDevice(int ind);
    void            activateDSP();
    void            deactivateDSP();

    void            setIsHoverMenu(bool ish){ isHoverMenu = ish; }
    void            setIsHoverLogger(bool isl){ isHoverLogger = isl; }
    void            setIsHoverCodeEditor(bool isl){ isHoverCodeEditor = isl; }

    // PATCH CANVAS
    ofxInfiniteCanvas               canvas;
    ofEasyCam                       easyCam;
    ofRectangle                     canvasViewport;
    ofxImGui::Gui*                  ofxVPGui;
    ImGuiEx::NodeCanvas             nodeCanvas;
    ImGuiEx::ProfilersWindow        profiler;


    // PATCH DRAWING RESOURCES
    ofxFontStash            *font;
    int                     fontSize;
    bool                    isRetina;
    int                     scaleFactor;

    // PUGG external plugins objects
    pugg::Kernel            plugins_kernel;

    // PATCH OBJECTS
    map<int,shared_ptr<PatchObject>>    patchObjects;
    map<string,string>                  scriptsObjectsFilesPaths;
    vector<pair<int,int>>               leftToRightIndexOrder;
    vector<int>                         eraseIndexes;

    map<string,vector<string>>          subpatchesTree;
    string                              currentSubpatch;

    int                                 selectedObjectID;
    int                                 actualObjectID;
    int                                 lastAddedObjectID;
    bool                                bLoadingNewObject;
    bool                                bLoadingNewPatch;
    bool                                clearingObjectsMap;

    // LOAD/SAVE
    string                  currentPatchFile;
    string                  currentPatchFolderPath;
    string                  tempPatchFile;
    int                     output_width;
    int                     output_height;
    string                  alphabet;
    int                     newFileCounter;

    // SYSTEM
    shared_ptr<ofAppGLFWWindow>     mainWindow;
    bool                            profilerActive;
    bool                            inspectorActive;
    bool                            inited;

    // GUI
    string                          inspectorTitle;
    float                           lastInspectorWidth;
    bool                            isHoverMenu;
    bool                            isHoverLogger;
    bool                            isHoverCodeEditor;

    // LIVE PATCHING
    int                             livePatchingObiID;

    // Sound Stream
    pdsp::Engine            *engine;
    ofSoundBuffer           inputBuffer;
    ofSoundBuffer           emptyBuffer;

    vector<ofSoundDevice>   audioDevices;
    vector<string>          audioDevicesStringIN;
    vector<string>          audioDevicesStringOUT;
    vector<int>             audioDevicesID_IN;
    vector<int>             audioDevicesID_OUT;
    ofSoundStream           soundStreamIN;
    std::mutex              inputAudioMutex;
    ofSoundBuffer           lastInputBuffer;
    ofPolyline              inputBufferWaveform;
    int                     audioINDev;
    int                     audioOUTDev;
    int                     audioGUIINIndex;
    int                     audioGUIOUTIndex;
    int                     audioSampleRate;
    int                     audioBufferSize;
    int                     bpm;
    bool                    dspON;

    // MEMORY
    uint64_t                resetTime;
    uint64_t                wait;

private:
    void audioProcess(float *input, int bufferSize, int nChannels);
};
