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

#include "ofxVPConfig.h"

#include "ofxPDSP.h"
#include "ofxImGui.h"
#include "imgui_node_canvas.h"
#include "imgui_profiler.h"
#include "FileBrowser/ImGuiFileBrowser.h"
#include "IconsFontAwesome5.h"

#include "Kernel.h"
#include "PatchObject.h"


struct SubpatchConnection{
    int     objID;
    string  name;
    uint8_t type;   // LINK_TYPE
    uint8_t inOut;  // 0 receive, 1 send
};

class ofxVisualProgramming : public pdsp::Wrapper {
    
public:

    ofxVisualProgramming();
    ~ofxVisualProgramming();

    void            setRetina(bool retina, float retinaScale=1);
    void            setup(ofxImGui::Gui* guiRef = nullptr, string release="");
    void            setupFailsafeWindow();
    void            update();
    void            updateRetina(float scale);
    void            updateCanvasViewport();
    void            updateSubpatchNavigation();
    void            draw();
    void            closeDrawMainMenu();
    void            drawInspector();
    void            drawLivePatchingSession();
    void            drawSubpatchNavigation();
    void            resetTempFolder();
    void            cleanPatchDataFolder();
    void            reloadFont();
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
    void            addObject(string name, ofVec2f pos,std::string fp="none");
    shared_ptr<PatchObject>    getLastAddedObject();

    void            resetObject(int &id);
    void            resetObject(int id);
    void            reconnectObjectOutlets(int &id);
    void            removeObject(int &id);
    void            duplicateObject(int &id);
    void            disconnectObject(int id);

    bool            connect(int fromID, int fromOutlet, int toID,int toInlet, int linkType);
    void            checkSpecialConnection(int fromID, int toID, int linkType);
    void            resetSystemObjects();
    void            resetSpecificSystemObjects(string name);
    bool            weAlreadyHaveObject(string name);
    void            deleteObject(int id);
    void            clearObjectsMap();
    bool            isObjectInLibrary(string name);
    bool            isObjectIDInPatchMap(int id);
    string          getObjectNameFromID(int id);
    int             getSubpatchIndex(string name);

    void            newPatch(string release);
    void            newTempPatchFromFile(string patchFile);
    void            preloadPatch(string patchFile);
    void            openPatch(string patchFile);
    void            loadPatch(string patchFile);
    void            loadPatchSharedContextObjects();
    void            reloadPatch();
    void            savePatchAs(string patchFile);
    void            setPatchVariable(string var, int value);

    void            setAudioInDevice(int ind);
    void            setAudioOutDevice(int ind);
    void            setAudioDevices(int ind, int outd);
    void            setAudioSampleRate(int sr);
    void            setAudioBufferSize(int bs);
    void            activateDSP();
    void            deactivateDSP();

    void            resetCanvas();

    // PATCH CANVAS
    ofRectangle                     canvasViewport;
    ofxImGui::Gui*                  ofxVPGui;
    ImGuiEx::NodeCanvas             nodeCanvas;
    ImGuiEx::ProfilersWindow        profiler;
    ImGuiEx::ProfilerTask           *pt;
    bool                            isCanvasVisible;
    bool                            isCanvasActive;


    // PATCH DRAWING RESOURCES
    ofTrueTypeFont                  *font;
    int                             fontSize;
    bool                            isRetina;
    float                           scaleFactor;

    // PUGG external plugins objects
    pugg::Kernel                    plugins_kernel;

    // PATCH OBJECTS
    map<int,shared_ptr<PatchObject>>    patchObjects;
    map<string,string>                  scriptsObjectsFilesPaths;
    vector<pair<int,int>>               leftToRightIndexOrder;
    vector<int>                         eraseIndexes;
    ofPoint                             nextObjectPosition;

    map<string,vector<SubpatchConnection>>  subpatchesMap;
    string                                  currentSubpatch;
    string                                  newSubpatchName;

    int                                 selectedObjectID;
    int                                 actualObjectID;
    int                                 lastAddedObjectID;
    bool                                bLoadingNewObject;
    bool                                bLoadingNewPatch;
    bool                                bPopulatingObjectsMap;
    bool                                clearingObjectsMap;

    // LOAD/SAVE
    string                              currentPatchFile;
    string                              currentPatchFolderPath;
    string                              tempPatchFile;
    int                                 output_width;
    int                                 output_height;
    string                              alphabet;
    int                                 newFileCounter;

    // SYSTEM
    shared_ptr<ofAppGLFWWindow>         mainWindow;
    bool                                profilerActive;
    bool                                inspectorActive;
    bool                                navigationActive;
    bool                                isOverProfiler;
    bool                                isOverInspector;
    bool                                isOverSubpatchNavigator;
    bool                                inited;

    // LIVE PATCHING
    int                                 livePatchingObiID;

    // Sound Stream
    pdsp::Engine                        *engine;
    ofSoundBuffer                       inputBuffer;
    ofSoundBuffer                       emptyBuffer;

    vector<ofSoundDevice>               audioDevices;
    vector<string>                      audioDevicesStringIN;
    vector<string>                      audioDevicesStringOUT;
    vector<int>                         audioDevicesID_IN;
    vector<int>                         audioDevicesID_OUT;
    vector<string>                      audioDevicesSR;
    vector<string>                      audioDevicesBS;
    ofSoundStream                       soundStreamIN;
    ofSoundBuffer                       lastInputBuffer;
    ofPolyline                          inputBufferWaveform;
    int                                 audioINDev;
    int                                 audioOUTDev;
    int                                 audioGUIINIndex;
    int                                 audioGUIOUTIndex;
    int                                 audioGUIINChannels;
    int                                 audioGUIOUTChannels;
    int                                 audioGUISRIndex;
    int                                 audioGUIBSIndex;
    int                                 audioSampleRate;
    int                                 audioBufferSize;
    int                                 audioNumBuffers;
    int                                 bpm;
    bool                                isInputDeviceAvailable;
    bool                                isOutputDeviceAvailable;
    bool                                dspON;

    // MEMORY
    uint64_t                loadPatchTime;
    uint64_t                resetTime;
    uint64_t                deferredLoadTime;
    uint64_t                wait;
    bool                    deferredLoad;

private:
    void audioProcess(float *input, int bufferSize, int nChannels);

    mutable ofMutex                 vp_mutex;

    shared_ptr<ofAppGLFWWindow>     failsafeWindow;

};
