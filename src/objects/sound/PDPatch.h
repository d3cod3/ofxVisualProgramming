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

#if defined(TARGET_WIN32)
    // Unavailable on windows.
<<<<<<< HEAD
#elif !defined(OFXVP_BUILD_WITH_MINIMAL_OBJECTS)
=======
#else
>>>>>>> master

#pragma once

#include "PatchObject.h"
#include "PathWatcher.h"
#include "ImGuiFileBrowser.h"
#include "IconsFontAwesome5.h"

#include "ofxPd.h"
#include "ofxPdExternals.h"


using namespace pd;

class PDPatch : public PatchObject, public PdReceiver, public PdMidiReceiver{

public:

    PDPatch();

    void            autoloadFile(string _fp) override;
    void            newObject() override;
    void            setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow) override;
    void            setupAudioOutObjectContent(pdsp::Engine &engine) override;
    void            updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects) override;

    void            drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer) override;
    void            drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ) override;
    void            drawObjectNodeConfig() override;

    void            removeObjectContent(bool removeFileFromData=false) override;

    void            audioInObject(ofSoundBuffer &inputBuffer) override;
    void            audioOutObject(ofSoundBuffer &outputBuffer) override;

    void            loadAudioSettings();
    void            loadPatch(string scriptFile);

    // Filepath watcher callback
    void            pathChanged(const PathWatcher::Event &event);

    // pd message receiver callbacks
    void            print(const std::string& message) override;

    void            receiveBang(const std::string& dest) override;
    void            receiveFloat(const std::string& dest, float value) override;
    void            receiveSymbol(const std::string& dest, const std::string& symbol) override;
    void            receiveList(const std::string& dest, const List& list) override;
    void            receiveMessage(const std::string& dest, const std::string& msg, const List& list) override;

    // pd midi receiver callbacks
    void            receiveNoteOn(const int channel, const int pitch, const int velocity) override;
    void            receiveControlChange(const int channel, const int controller, const int value) override;
    void            receiveProgramChange(const int channel, const int value) override;
    void            receivePitchBend(const int channel, const int value) override;
    void            receiveAftertouch(const int channel, const int value) override;
    void            receivePolyAftertouch(const int channel, const int pitch, const int value) override;
    void            receiveMidiByte(const int port, const int byte) override;


    ofxPd               pd;
    Patch               currentPatch;
    ofFile              currentPatchFile;
    PathWatcher         watcher;
    bool                isNewObject;

    ofSoundBuffer       lastInputBuffer;
    ofSoundBuffer       lastInputBuffer1;
    ofSoundBuffer       lastInputBuffer2;
    ofSoundBuffer       lastInputBuffer3;
    ofSoundBuffer       lastInputBuffer4;
    ofSoundBuffer       lastOutputBuffer;
    ofSoundBuffer       lastOutputBuffer1;
    ofSoundBuffer       lastOutputBuffer2;
    ofSoundBuffer       lastOutputBuffer3;
    ofSoundBuffer       lastOutputBuffer4;

    ofImage*            pdIcon;
    float               posX, posY, drawW, drawH;
    float               scaledObjW, scaledObjH;
    float               objOriginX, objOriginY;
    float               canvasZoom;

    pdsp::ExternalInput ch1IN, ch2IN, ch3IN, ch4IN;
    pdsp::ExternalInput ch1OUT, ch2OUT, ch3OUT, ch4OUT;
    pdsp::PatchNode     mixIN, mixOUT;

    int                 bufferSize;
    int                 sampleRate;

    imgui_addons::ImGuiFileBrowser          fileDialog;

    string              lastLoadedPatch;
    string              prevExternalsFolder;
    string              lastExternalsFolder;
    bool                loadPatchFlag;
    bool                savePatchFlag;
    bool                setExternalFlag;
    bool                patchLoaded;
    bool                patchSaved;
    bool                externalPathSaved;


    bool                loading;
    bool                loaded;

protected:

    OBJECT_FACTORY_PROPS

};

#endif
