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

#ifndef OFXVP_BUILD_WITH_MINIMAL_OBJECTS

#pragma once

#include "PatchObject.h"

#include "ofxLua.h"
#include "ofxEditor.h"
#include "PathWatcher.h"

#include "ImGuiFileBrowser.h"
#include "IconsFontAwesome5.h"

struct LiveCoding{
    ofxLua          lua;
    ofxEditor       liveEditor;
    string          filepath;
    bool            hide;
};

class LuaScript : public PatchObject, public ofxLuaListener{

public:

    LuaScript();

    void            autoloadFile(string _fp) override;
    void            autosaveNewFile(string fromFile) override;

    void            newObject() override;
    void            setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow) override;
    void            updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects) override;
    void            drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer) override;
    void            drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ) override;
    void            drawObjectNodeConfig() override;

    void            removeObjectContent(bool removeFileFromData=false) override;

    void            resetResolution(int fromID, int newWidth, int newHeight) override;

    void            initResolution();

    void            openScript(string scriptFile);
    void            saveScript(string scriptFile);
    void            loadScript(string scriptFile);
    void            unloadScript();
    void            clearScript();
    void            reloadScriptThreaded();

    // Filepath watcher callback
    void            pathChanged(const PathWatcher::Event &event);

    // ofxLua error callback
    void            errorReceived(std::string& msg) override;

    ofxEditorSyntax         liveEditorSyntax;
    ofxEditorColorScheme    liveEditorColors;

    PathWatcher         watcher;
    ofFile              currentScriptFile;
    bool                scriptLoaded;
    bool                isNewObject;
    bool                isError;
    bool                setupTrigger;

    ofFbo               *fbo;
    ofImage             *kuro;
    float               posX, posY, drawW, drawH;

    float               prevW, prevH;

    string              mosaicTableName;
    string              luaTablename;
    string              mosaicStringName;
    string              tempstring;

    imgui_addons::ImGuiFileBrowser          fileDialog;
    string                                  lastLuaScript;
    string                                  newFileFromFilepath;
    bool                                    loadLuaScriptFlag;
    bool                                    saveLuaScriptFlag;
    bool                                    luaScriptLoaded;
    bool                                    luaScriptSaved;
    bool                                    loaded;
    size_t                                  loadTime;

    float               scaledObjW, scaledObjH;
    float               objOriginX, objOriginY;
    float               canvasZoom;

protected:

    bool                    needToLoadScript;

private:

    OBJECT_FACTORY_PROPS

};

#endif
