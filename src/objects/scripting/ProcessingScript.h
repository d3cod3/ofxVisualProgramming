/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2019 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "ofxJava.h"

#include "PathWatcher.h"
#include "ThreadedCommand.h"


class ProcessingScript : public PatchObject {

public:

    ProcessingScript();

    void            autoloadFile(string _fp);
    void            newObject();
    void            setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow);
    void            updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd);
    void            drawObjectContent(ofxFontStash *font);
    void            removeObjectContent(bool removeFileFromData=false);
    void            mouseMovedObjectContent(ofVec3f _m);
    void            dragGUIObject(ofVec3f _m);
    void            fileDialogResponse(ofxThreadedFileDialogResponse &response);

    void            loadScript(string scriptFile);
    void            clearScript();
    void            reloadScriptThreaded();

    void            onButtonEvent(ofxDatGuiButtonEvent e);

    // Filepath watcher callback
    void            pathChanged(const PathWatcher::Event &event);

    ofxJava             *jvm;

    PathWatcher         watcher;
    ofFile              currentScriptFile;
    ofBuffer            dataIn;
    ofBuffer            dataOut;
    bool                isNewObject;
    bool                needToGrab;

    float               posX, posY, drawW, drawH;

    ofxDatGui*          gui;
    ofxDatGuiHeader*    header;
    ofxDatGuiLabel*     scriptName;
    ofxDatGuiButton*    newButton;
    ofxDatGuiButton*    loadButton;
    ofxDatGuiButton*    editButton;
    ofxDatGuiButton*    clearButton;
    ofxDatGuiButton*    reloadButton;

    string              lastProcessingScript;
    bool                loadProcessingScriptFlag;
    bool                saveProcessingScriptFlag;
    bool                processingScriptLoaded;
    bool                processingScriptSaved;

    bool                modalInfo;


protected:
    ThreadedCommand         tempCommand;

};
