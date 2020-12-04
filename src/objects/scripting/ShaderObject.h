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

#include "PathWatcher.h"

#include "ImGuiFileBrowser.h"
#include "IconsFontAwesome5.h"

class ofxPingPong {
public:
    void allocate( int _width, int _height, int _internalformat = GL_RGBA){
        // Allocate
        for(int i = 0; i < 2; i++)
            FBOs[i].allocate(_width,_height, _internalformat );

        // Clean
        clear();

        // Set everything to 0
        flag = 0;
        swap();
        flag = 0;
    }

    void swap(){
        src = &(FBOs[(flag)%2]);
        dst = &(FBOs[++(flag)%2]);
    }

    void clear(){
        for(int i = 0; i < 2; i++){
            FBOs[i].begin();
            ofClear(0,0);
            FBOs[i].end();
        }
    }

    ofFbo& operator[]( int n ){ return FBOs[n];}

    ofFbo   *src;       // Source       ->  Ping
    ofFbo   *dst;       // Destination  ->  Pong

private:
    ofFbo   FBOs[2];    // Real addresses of ping/pong FBO´s
    int     flag;       // Integer for making a quick swap
};

enum ShaderSliderType { ShaderSliderType_INT, ShaderSliderType_FLOAT, ShaderSliderType_COUNT };

class ShaderObject : public PatchObject{

public:

    ShaderObject();

    void            autoloadFile(string _fp) override;

    void            newObject() override;
    void            setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow) override;
    void            updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects) override;
    void            drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer) override;
    void            drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ) override;
    void            drawObjectNodeConfig() override;

    void            removeObjectContent(bool removeFileFromData=false) override;
    
    void            resetResolution(int fromID, int newWidth, int newHeight) override;

    void            initResolution();
    void            doFragmentShader();

    void            loadScript(string scriptFile);

    // Filepath watcher callback
    void            pathChanged(const PathWatcher::Event &event);

    ofxPingPong         *pingPong;
    vector<ofFbo*>      textures;
    ofShader            *shader;
    ofVboMesh           quad;
    ofFile              currentScriptFile;
    string              fragmentShader;
    string              vertexShader;
    int                 nTextures, internalFormat;
    bool                needReset;

    vector<float>       shaderSliders;
    vector<string>      shaderSlidersLabel;
    vector<int>         shaderSlidersIndex;
    vector<int>         shaderSlidersType;
    
    PathWatcher         watcher;
    bool                scriptLoaded;
    bool                isNewObject;
    bool                reloading;
    bool                oneBang;

    imgui_addons::ImGuiFileBrowser          fileDialog;
    string                                  newScriptName;

    string              lastShaderScript;
    string              lastVertexShaderPath;
    bool                loadShaderScriptFlag;
    bool                saveShaderScriptFlag;
    bool                shaderScriptLoaded;
    bool                shaderScriptSaved;

    ofFbo               *fbo;
    ofImage             *kuro;
    float               posX, posY, drawW, drawH;
    float               scaledObjW, scaledObjH;
    float               objOriginX, objOriginY;
    float               canvasZoom;

    float               prevW, prevH;

    bool                loaded;

protected:
    OBJECT_FACTORY_PROPS

};

#endif
