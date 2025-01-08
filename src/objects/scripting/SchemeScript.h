/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2025 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "ImGuiFileBrowser.h"
#include "imgui_node_canvas.h"
#include "IconsFontAwesome5.h"

#include "ofxScheme.h"
#include "ofxGLEditor.h"

// app key commands:
//    MOD -> CTRL or Super (Windows Key, Mac CMD)
//
// MOD + e: execute script or selected text
// MOD + l: toggle line wrapping
// MOD + n: toggle line numbers
// MOD + f: toggle fullscreen
// MOD + k: toggle auto focus zooming
// MOD + t: show/hide editor
// MOD + 1 to MOD + 9: switch to editor 1 - 9
// MOD + b: blow up the cursor
// MOD + a: select all text in the current editor
// MOD + a + SHIFT: clear all text in the current editor
// MOD + c: copy from system clipboard
// MOD + v: paste to system clipboard
// MOD + z: undo last key input action
// MOD + y: redo last key input action
//
// see ofxGLEditor.h for editor key commands
//

class SchemeScript : public PatchObject, public ofxGLEditorListener {

public:

    // CONSTRUCTOR
  SchemeScript();

  // BASIC METHODS

  // inlets/oulets instatiation
  void              newObject() override;
  // object setup
  void              setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow) override;
  // object update
  void              updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects) override;
  // object draw
  void              drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer) override;
  void              drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ) override;
  void              drawObjectNodeConfig() override;
  // call on remove object
  void              removeObjectContent(bool removeFileFromData=false) override;
  
  void              drawInWindow(ofEventArgs &e);

  void              loadWindowSettings();

  void              keyPressed(ofKeyEventArgs &e);
  void              keyReleased(ofKeyEventArgs &e);
  void              mouseMoved(ofMouseEventArgs &e);
  void              mouseDragged(ofMouseEventArgs &e);
  void              mousePressed(ofMouseEventArgs &e);
  void              mouseReleased(ofMouseEventArgs &e);
  void              mouseScrolled(ofMouseEventArgs &e);
  void              windowResized(ofResizeEventArgs &e);

  void              loadBuffers();


  void              scaleTextureToWindow(float texW, float texH, float winW, float winH);
  void              toggleWindowFullscreen();


  shared_ptr<ofAppGLFWWindow>               window;
  bool                                      isFullscreen;
  bool                                      isNewScriptConnected;
  bool                                      hideMouse;

  int                                       temp_width, temp_height;
  int                                       window_actual_width, window_actual_height;
  float                                     posX, posY, drawW, drawH;
  float                                     thposX, thposY, thdrawW, thdrawH;
  bool                                      needReset;

  ofxScheme                                 scheme;
  std::string                               scriptBuffer;
  ofFbo                                     *fbo;
  std::vector<float>                        emptyData;
  std::string                               lastScriptLoaded;
  bool                                      eval;
  bool                                      scriptError;
  bool                                      loadRandExample;
  bool                                      loadSchemeScriptFlag;
  bool                                      schemeScriptLoaded;

  ofxGLEditor                               editor;
  ofxEditorSyntax*                          syntax;
  ofxEditorColorScheme                      colorScheme;
  ofColor                                   cursorColor;
  bool                                      hideEditor;

  imgui_addons::ImGuiFileBrowser            fileDialog;
  ofFile                                    currentScriptFile;
  ofBuffer                                  sketchContent;
  string                                    filepath;
  bool                                      needToLoadScript;
  bool                                      scriptLoaded;

  ofDirectory                               scriptFolder;
  ofImage                                   *schemeIcon;

  float                                     scaledObjW, scaledObjH;
  float                                     objOriginX, objOriginY;
  float                                     canvasZoom;

  bool                                      loaded;
  float                                     prevW, prevH;
  bool                                      autoRemove;

private:
    OBJECT_FACTORY_PROPS

protected:

};

#endif
