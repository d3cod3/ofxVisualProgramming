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
#include "ofxVPXmlEngine.h"

#include "ofxPDSP.h"
#include "ofxImGui.h"
#include "imgui_node_canvas.h"
#include "imgui_profiler.h"
#include "FileBrowser/ImGuiFileBrowser.h"
#include "IconsFontAwesome5.h"

#include "Kernel.h"
#include "PatchObject.h"


struct SubpatchConnection{
    int             objID;
    std::string     name;
    uint8_t         type;   // LINK_TYPE
    uint8_t         inOut;  // 0 receive, 1 send
};

class ofxVisualProgramming : public pdsp::Wrapper {
    
public:

    ofxVisualProgramming();
    ~ofxVisualProgramming();

    void            setRetina(bool retina, float retinaScale=1);
    void            setup(ofxImGui::Gui* guiRef = nullptr, std::string release="");
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

    std::shared_ptr<PatchObject>    selectObject(std::string objname);
    void            addObject(std::string name, ofVec2f pos,std::string fp="none");
    std::shared_ptr<PatchObject>    getLastAddedObject();

    void            resetObject(int &id);
    void            resetObject(int id);
    void            reconnectObjectOutlets(int &id);
    void            removeObject(int &id);
    void            duplicateObject(int &id);
    void            disconnectObject(int id);

    bool            connect(int fromID, int fromOutlet, int toID,int toInlet, int linkType);
    void            checkSpecialConnection(int fromID, int toID, int linkType);
    void            resetSystemObjects();
    void            resetSpecificSystemObjects(std::string name);
    bool            weAlreadyHaveObject(std::string name);
    void            clearObjectsMap();
    bool            isObjectInLibrary(std::string name);
    bool            isObjectIDInPatchMap(int id);
    std::string          getObjectNameFromID(int id);
    int             getSubpatchIndex(string name);

    void            newPatch(std::string release);
    void            newTempPatchFromFile(std::string patchFile);
    void            preloadPatch(std::string patchFile);
    void            openPatch(std::string patchFile);
    void            loadPatch(std::string patchFile);
    void            loadPatchSharedContextObjects();
    void            reloadPatch();
    void            savePatchAs(std::string patchFile);
    void            setPatchVariable(std::string var, int value);

    void            setAudioInDevice(int ind);
    void            setAudioOutDevice(int ind);
    void            setAudioDevices(int ind, int outd);
    void            setAudioSampleRate(int sr);
    void            setAudioBufferSize(int bs);
    void            activateDSP();
    void            deactivateDSP();

    void            resetCanvas();

    // PATCH FILE
    ofxVPXmlEngine                      ofxVPXml;
    bool                                isPrePugiXmlRelease;

    // PATCH CANVAS
    ofRectangle                         canvasViewport;
    ofxImGui::Gui*                      ofxVPGui;
    ImGuiEx::NodeCanvas                 nodeCanvas;
    ImGuiEx::ProfilersWindow            profiler;
    ImGuiEx::ProfilerTask               *pt;
    bool                                isCanvasVisible;
    bool                                isCanvasActive;


    // PATCH DRAWING RESOURCES
    ofTrueTypeFont                      *font;
    int                                 fontSize;
    bool                                isRetina;
    float                               scaleFactor;

    // PUGG external plugins objects
    pugg::Kernel                        plugins_kernel;

    // PATCH OBJECTS
    std::map<int,std::shared_ptr<PatchObject>>      patchObjects;
    std::map<std::string,std::string>               scriptsObjectsFilesPaths;
    std::vector<std::pair<int,int>>                 leftToRightIndexOrder;
    std::vector<int>                                eraseIndexes;
    ofPoint                                         nextObjectPosition;

    std::map<std::string,std::vector<SubpatchConnection>>   subpatchesMap;
    std::string                                             currentSubpatch;
    std::string                                             newSubpatchName;

    int                                 selectedObjectID;
    int                                 actualObjectID;
    int                                 lastAddedObjectID;
    bool                                bLoadingNewObject;
    bool                                bLoadingNewPatch;
    bool                                bPopulatingObjectsMap;
    bool                                clearingObjectsMap;

    // LOAD/SAVE
    std::string                         currentPatchFile;
    std::string                         currentPatchFolderPath;
    int                                 output_width;
    int                                 output_height;
    std::string                         alphabet;
    int                                 newFileCounter;

    // SYSTEM
    std::shared_ptr<ofAppGLFWWindow>    mainWindow;
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

    std::vector<ofSoundDevice>               audioDevices;
    std::vector<std::string>                 audioDevicesStringIN;
    std::vector<std::string>                 audioDevicesStringOUT;
    std::vector<int>                         audioDevicesID_IN;
    std::vector<int>                         audioDevicesID_OUT;
    std::vector<std::string>                 audioDevicesSR;
    std::vector<std::string>                 audioDevicesBS;
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
    bool                                pauseDSP;

    // MEMORY
    uint64_t                            loadPatchTime;
    uint64_t                            resetTime;
    uint64_t                            deferredLoadTime;
    uint64_t                            wait;
    bool                                deferredLoad;

private:
    void audioProcess(float *input, int bufferSize, int nChannels);

    mutable ofMutex                     vp_mutex;

    std::shared_ptr<ofAppGLFWWindow>    failsafeWindow;

};
