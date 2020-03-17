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

#include "ofxLua.h"
#include "ofxEditor.h"
#include "PathWatcher.h"
#include "ThreadedCommand.h"

#include <atomic>

class LuaScript : public PatchObject, public ofxLuaListener{

public:

    LuaScript();

    void            autoloadFile(string _fp);
    void            autosaveNewFile(string fromFile);

    void            newObject();
    void            setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow);
    void            updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd);
    void            drawObjectContent(ofxFontStash *font);
    void            removeObjectContent(bool removeFileFromData=false);
    void            mouseMovedObjectContent(ofVec3f _m);
    void            dragGUIObject(ofVec3f _m);
    void            resetResolution(int fromID, int newWidth, int newHeight);
    void            fileDialogResponse(ofxThreadedFileDialogResponse &response);

    void            initResolution();

    void            unloadScript();
    void            loadScript(string scriptFile);
    void            clearScript();
    void            reloadScriptThreaded();

    void            onButtonEvent(ofxDatGuiButtonEvent e);

    // Filepath watcher callback
    void            pathChanged(const PathWatcher::Event &event);

    // ofxLua error callback
    void            errorReceived(std::string& msg);

    ofxEditorSyntax         liveEditorSyntax;
    ofxEditorColorScheme    liveEditorColors;

    PathWatcher         watcher;
    ofFile              currentScriptFile;
    bool                scriptLoaded;
    bool                nameLabelLoaded;
    bool                isNewObject;
    bool                isError;
    bool                setupTrigger;

    ofxDatGui*          gui;
    ofxDatGuiHeader*    header;
    ofxDatGuiLabel*     scriptName;
    ofxDatGuiButton*    newButton;
    ofxDatGuiButton*    loadButton;
    ofxDatGuiButton*    editButton;
    ofxDatGuiButton*    clearButton;
    ofxDatGuiButton*    reloadButton;

    ofFbo               *fbo;
    ofImage             *kuro;
    float               posX, posY, drawW, drawH;

    string              mosaicTableName;
    string              luaTablename;
    string              tempstring;

    string              lastLuaScript;
    string              newFileFromFilepath;
    bool                loadLuaScriptFlag;
    bool                saveLuaScriptFlag;
    bool                luaScriptLoaded;
    bool                luaScriptSaved;
    bool                loaded;
    size_t              loadTime;

    bool                modalInfo;

protected:
    ThreadedCommand         tempCommand;
    bool                    needToLoadScript;
    bool                    threadLoaded;

    OBJECT_FACTORY_PROPS
};
