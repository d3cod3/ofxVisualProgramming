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

#include "SchemeScript.h"

//--------------------------------------------------------------
SchemeScript::SchemeScript() : PatchObject("scheme live coding"){

    // SET YOUR INLETS/OUTLETS
    this->numInlets  = 1;
    this->numOutlets = 0;

    _inletParams[0] = new vector<float>();  // data IN

    this->initInletsState();

    this->setIsTextureObj(true);
    this->setIsSharedContextObj(true);

    isFullscreen                        = false;
    thposX = thposY = thdrawW = thdrawH = 0.0f;
    isNewScriptConnected                = false;

    this->output_width      = STANDARD_TEXTURE_WIDTH;
    this->output_height     = STANDARD_TEXTURE_HEIGHT;

    temp_width              = this->output_width;
    temp_height             = this->output_height;

    window_actual_width     = STANDARD_PROJECTOR_WINDOW_WIDTH;
    window_actual_height    = STANDARD_PROJECTOR_WINDOW_HEIGHT;

    needReset           = false;
    hideMouse           = false;
    loadRandExample     = false;

    posX = posY = drawW = drawH = 0.0f;

    prevW                   = this->width;
    prevH                   = this->height;

    schemeIcon              = new ofImage();
    loadSchemeScriptFlag    = false;
    schemeScriptLoaded      = false;

    emptyData.assign(1,0.0f);

    loaded              = false;
    autoRemove          = false;

}

//--------------------------------------------------------------
void SchemeScript::newObject(){
    // SET OBJECT NAME AND INLETS/OUTLETS TYPES/NAMES
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"_mosaic_data_inlet");

    this->setCustomVar(static_cast<float>(isFullscreen),"FULLSCREEN");
    this->setCustomVar(static_cast<float>(this->output_width),"OUTPUT_WIDTH");
    this->setCustomVar(static_cast<float>(this->output_height),"OUTPUT_HEIGHT");
    this->setCustomVar(0.0f,"OUTPUT_POSX");
    this->setCustomVar(0.0f,"OUTPUT_POSY");
    this->setCustomVar(static_cast<float>(hideMouse),"HIDE_MOUSE");
    this->setCustomVar(static_cast<float>(prevW),"WIDTH");
    this->setCustomVar(static_cast<float>(prevH),"HEIGHT");

}

//--------------------------------------------------------------
void SchemeScript::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    loadWindowSettings();

    ofGLFWWindowSettings settings;
    settings.setGLVersion(2,1); // NEEDED BY ofxGLEditor
    settings.shareContextWith = mainWindow;
    settings.decorated = true;
    settings.resizable = true;
    settings.stencilBits = 0;
    // RETINA FIX
    if(mainWindow->getPixelScreenCoordScale() > 1){
        if(ofGetScreenWidth() > 3360 && ofGetScreenHeight() > 2100){
            window_actual_width     = STANDARD_PROJECTOR_WINDOW_WIDTH*2;
            window_actual_height    = STANDARD_PROJECTOR_WINDOW_HEIGHT*2;
        }
        settings.setPosition(ofDefaultVec2(mainWindow->getScreenSize().x-(window_actual_width+100),400));
    }else{
        settings.setPosition(ofDefaultVec2(mainWindow->getScreenSize().x-(STANDARD_PROJECTOR_WINDOW_WIDTH+50),200));
    }
    settings.setSize(window_actual_width, window_actual_height);

    window = dynamic_pointer_cast<ofAppGLFWWindow>(ofCreateWindow(settings));
    window->setVerticalSync(false);
    window->setWindowPosition(this->getCustomVar("OUTPUT_POSX"),this->getCustomVar("OUTPUT_POSY"));

    glfwSetWindowCloseCallback(window->getGLFWWindow(),GL_FALSE);

    ofAddListener(window->events().draw,this,&SchemeScript::drawInWindow);
    ofAddListener(window->events().keyPressed,this,&SchemeScript::keyPressed);
    ofAddListener(window->events().keyReleased,this,&SchemeScript::keyReleased);
    ofAddListener(window->events().mouseMoved ,this,&SchemeScript::mouseMoved);
    ofAddListener(window->events().mouseDragged ,this,&SchemeScript::mouseDragged);
    ofAddListener(window->events().mousePressed ,this,&SchemeScript::mousePressed);
    ofAddListener(window->events().mouseReleased ,this,&SchemeScript::mouseReleased);
    ofAddListener(window->events().mouseScrolled ,this,&SchemeScript::mouseScrolled);
    ofAddListener(window->events().windowResized ,this,&SchemeScript::windowResized);

    // INIT Scheme and register API
    scheme.setup();

    // init drawing FBO
    ofDisableArbTex();
    fbo = new ofFbo();
    fbo->allocate(ofGetScreenWidth(),ofGetScreenHeight() ,GL_RGBA32F_ARB,4);
    fbo->begin();
    ofClear(0,0,0,255);
    fbo->end();
    ofEnableArbTex();

    isFullscreen = false;
    thposX = thposY = thdrawW = thdrawH = 0.0f;
    scaleTextureToWindow(fbo->getWidth(), fbo->getHeight(), 1280, 720);
    scheme.setWindowDim(fbo->getWidth(), fbo->getHeight());

    // load editor font
    ofxEditor::loadFont("livecoding/scheme/fonts/PrintChar21.ttf", 24);
    editor.setup(this);

    hideEditor = false;
    // load scheme syntax
    syntax = new ofxEditorSyntax();
    syntax->loadFile("livecoding/scheme/schemeSyntax.xml");
    editor.getSettings().addSyntax("Scheme", syntax);
    for(size_t i =0;i<=9;i++){
        editor.setLangSyntax("Scheme",i);
    }
    // syntax highlighter colors
    colorScheme.loadFile("livecoding/scheme/colorScheme.xml");
    editor.setColorScheme(&colorScheme);

    cursorColor = ofColor(255, 255, 0, 200);

    ofDisableArbTex();
    schemeIcon->load("images/scheme.png");
    ofEnableArbTex();

    fileDialog.setIsRetina(this->isRetina);

    needToLoadScript = true;
    scriptLoaded = false;
    eval = true;
    scriptError = false;

}

//--------------------------------------------------------------
void SchemeScript::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    // receive external data
    if(this->inletsConnected[0] && !static_cast<vector<float> *>(_inletParams[0])->empty()){
        scheme.setExternalData(*static_cast<vector<float> *>(_inletParams[0]));
    }else{
        scheme.setExternalData(emptyData);
    }

    // auto remove
    if(window->getGLFWWindow() == nullptr && !autoRemove){
        autoRemove = true;
        ofNotifyEvent(this->removeEvent, this->nId);
        this->willErase = true;
    }

    if(loadRandExample){
        loadRandExample = false;
        scheme.clearScript();
        ofDirectory temp;
        temp.listDir(ofToDataPath("livecoding/scheme/examples/"));
        int rf = static_cast<int>(floor(ofRandom(temp.getFiles().size())));

        if(temp.getFiles().size() <= rf){
            rf = 0;
        }

        ofFile rand(temp.getFile(rf).getAbsolutePath());

        sketchContent = ofBufferFromFile(rand.getAbsolutePath());
        editor.setText(sketchContent);
        eval = true;
        scriptError = false;
        editor.getSettings().setCursorColor(cursorColor);

    }

    if(schemeScriptLoaded){
        schemeScriptLoaded = false;
        ofFile nf(lastScriptLoaded);

        sketchContent = ofBufferFromFile(nf.getAbsolutePath());
        editor.setText(sketchContent);
        eval = true;
        scriptError = false;
        editor.getSettings().setCursorColor(cursorColor);
    }

    if(!loaded){
        loaded = true;
        temp_width = static_cast<int>(floor(this->getCustomVar("OUTPUT_WIDTH")));
        temp_height = static_cast<int>(floor(this->getCustomVar("OUTPUT_HEIGHT")));
        hideMouse = static_cast<int>(floor(this->getCustomVar("HIDE_MOUSE")));
        prevW = this->getCustomVar("WIDTH");
        prevH = this->getCustomVar("HEIGHT");
        this->width             = prevW;
        this->height            = prevH;

        // setup drawing  dimensions
        scaleTextureToWindow(STANDARD_TEXTURE_WIDTH, STANDARD_TEXTURE_HEIGHT, window->getWidth(),window->getHeight());

        window->setWindowTitle("scheme live coding | id: "+ofToString(this->getId()));

        if(static_cast<bool>(floor(this->getCustomVar("FULLSCREEN"))) != isFullscreen){
            window->setWindowPosition(this->getCustomVar("OUTPUT_POSX"),this->getCustomVar("OUTPUT_POSY"));
            toggleWindowFullscreen();
        }

        needReset = true;
    }

}

//--------------------------------------------------------------
void SchemeScript::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

}

//--------------------------------------------------------------
void SchemeScript::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    ImGui::SetCurrentContext(_nodeCanvas.getContext());

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){

        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            drawObjectNodeConfig();

            ImGui::EndMenu();
        }

        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        ImVec2 window_pos = ImGui::GetWindowPos()+ImVec2(IMGUI_EX_NODE_PINS_WIDTH_NORMAL, IMGUI_EX_NODE_HEADER_HEIGHT);
        _nodeCanvas.getNodeDrawList()->AddRectFilled(window_pos,window_pos+ImVec2(scaledObjW*this->scaleFactor*_nodeCanvas.GetCanvasScale(), scaledObjH*this->scaleFactor*_nodeCanvas.GetCanvasScale()),ImGui::GetColorU32(ImVec4(34.0/255.0, 34.0/255.0, 34.0/255.0, 1.0f)));

        if(schemeIcon->getTexture().isAllocated()){
            calcTextureDims(schemeIcon->getTexture(), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
            ImGui::SetCursorPos(ImVec2(posX+(IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor), posY+(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor)));
            ImGui::Image((ImTextureID)(uintptr_t)schemeIcon->getTexture().getTextureData().textureID, ImVec2(drawW, drawH));
        }

        // get imgui node translated/scaled position/dimension for drawing textures in OF
        //objOriginX = (ImGui::GetWindowPos().x + ((IMGUI_EX_NODE_PINS_WIDTH_NORMAL - 1)*this->scaleFactor) - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale();
        //objOriginY = (ImGui::GetWindowPos().y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale();
        scaledObjW = this->width - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL+IMGUI_EX_NODE_PINS_WIDTH_SMALL)*this->scaleFactor/_nodeCanvas.GetCanvasScale();
        scaledObjH = this->height - (IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor/_nodeCanvas.GetCanvasScale();


        _nodeCanvas.EndNodeContent();
    }

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();

}

//--------------------------------------------------------------
void SchemeScript::drawObjectNodeConfig(){

    loadSchemeScriptFlag = false;

    ImGui::Spacing();
    ImGui::Text("Loaded Buffer:");
    if(filepath == "none"){
        ImGui::Text("%s",filepath.c_str());
    }else{
        ImGui::Text("%s",editor.getEditorFilename(editor.getCurrentEditor()).c_str());
    }
    ImGui::Spacing();
    ImGui::Text("Rendering at: %.0fx%.0f",fbo->getWidth(),fbo->getHeight());
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    if(ImGui::Button(ICON_FA_RANDOM "  Load Random Script",ImVec2(224*scaleFactor,26*scaleFactor))){
        loadRandExample = true;
    }
    ImGui::Spacing();
    ImGui::Spacing();
    if(ImGui::Button(ICON_FA_FILE_CODE "  Load Script",ImVec2(224*scaleFactor,26*scaleFactor))){
        loadSchemeScriptFlag = true;
    }

    ImGuiEx::ObjectInfo(
            "Scheme Live Coding - A scheme language based graphical live coding environment with OF batteries.",
            "https://github.com/d3cod3/ofxScheme", scaleFactor);

    // file dialog, load a scheme script in current buffer
    if(ImGuiEx::getFileDialog(fileDialog, loadSchemeScriptFlag, "Select a scheme script", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".scm", "", scaleFactor)){
        schemeScriptLoaded = true;
        lastScriptLoaded = fileDialog.selected_path;
    }
}

//--------------------------------------------------------------
void SchemeScript::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);

    ofRemoveListener(window->events().draw,this,&SchemeScript::drawInWindow);

    if(window->getGLFWWindow() != nullptr){
        window->finishRender();
#ifdef TARGET_LINUX
        glfwHideWindow(window->getGLFWWindow());
#else
        window->setWindowShouldClose();
#endif
    }
}

//--------------------------------------------------------------
void SchemeScript::scaleTextureToWindow(float texW, float texH, float winW, float winH){
    // wider texture than window
    if(texW/texH >= winW/winH){
        thdrawW           = winW;
        thdrawH           = (texH*winW) / texW;
        thposX            = 0;
        thposY            = (winH-thdrawH)/2.0f;
        //ofLog(OF_LOG_NOTICE," |wider texture than window|  Window: %fx%f, Texture[%fx%f] drawing %fx%f at %f,%f",winW,winH,texW,texH,thdrawW,thdrawH,thposX,thposY);
    // wider window than texture
    }else{
        thdrawW           = (texW*winH) / texH;
        thdrawH           = winH;
        thposX            = (winW-thdrawW)/2.0f;
        thposY            = 0;
        //ofLog(OF_LOG_NOTICE," |wider window than texture|  Window: %fx%f, Texture[%fx%f] drawing %fx%f at %f,%f",winW,winH,texW,texH,thdrawW,thdrawH,thposX,thposY);
    }

}

//--------------------------------------------------------------
void SchemeScript::toggleWindowFullscreen(){
    isFullscreen = !isFullscreen;
    ofToggleFullscreen();

    this->setCustomVar(static_cast<float>(isFullscreen),"FULLSCREEN");

    if(!isFullscreen){
        window->setWindowShape(window_actual_width, window_actual_height);
        scaleTextureToWindow(fbo->getWidth(), fbo->getHeight(), window_actual_width, window_actual_height);
    }else{
        scaleTextureToWindow(fbo->getWidth(), fbo->getHeight(), window->getScreenSize().x,window->getScreenSize().y);
    }

    this->setCustomVar(window->getWindowPosition().x,"OUTPUT_POSX");
    this->setCustomVar(window->getWindowPosition().y,"OUTPUT_POSY");
}

//--------------------------------------------------------------
void SchemeScript::drawInWindow(ofEventArgs &e){
    unusedArgs(e);

    scheme.setMouse((window->events().getMouseX() - thposX)/thdrawW * fbo->getWidth(),(window->events().getMouseY() - thposY)/thdrawH * fbo->getHeight());
    scheme.update();
    scheme.setScreenTexture(fbo->getTexture());

    if(needToLoadScript){
        needToLoadScript = false;
        loadBuffers();
    }

    if(eval){
        eval = false;
        scriptBuffer = editor.getText();
    }

    ofBackground(0);

    if(hideMouse){
        window->hideCursor();
    }else{
        window->showCursor();
    }

    if(scriptLoaded){
        fbo->begin();
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
        ofPushView();
        ofPushStyle();
        ofPushMatrix();

        ofEnableAlphaBlending();

        if(!scriptError){
            const char* res = scheme.evalScript(scriptBuffer);
            if(res != nullptr){
                scriptError = true;
                ofLog(OF_LOG_ERROR,"%s",res);
                editor.getSettings().setCursorColor(ofColor::red);
            }
        }

        ofDisableAlphaBlending();

        ofPopMatrix();
        ofPopStyle();
        ofPopView();
        glPopAttrib();
        fbo->end();

        ofSetColor(255);
        fbo->draw(thposX,thposY,thdrawW,thdrawH);

        if(window->getGLFWWindow() != nullptr && !hideEditor) {
            editor.draw();
        }
    }

}

//--------------------------------------------------------------
void SchemeScript::loadWindowSettings(){
    this->output_width = static_cast<int>(floor(this->getCustomVar("OUTPUT_WIDTH")));
    this->output_height = static_cast<int>(floor(this->getCustomVar("OUTPUT_HEIGHT")));

    temp_width      = this->output_width;
    temp_height     = this->output_height;
}

//--------------------------------------------------------------
void SchemeScript::keyPressed(ofKeyEventArgs &e){

    bool modifierPressed = ofxEditor::getSuperAsModifier() ? ofGetKeyPressed(OF_KEY_SUPER) : ofGetKeyPressed(OF_KEY_CONTROL);
    if(modifierPressed) {
        switch(e.key) {
        case 's':
            filepath = ofToDataPath(ofToString(editor.getCurrentEditor())+".scm",true);
            editor.saveFile(filepath,editor.getCurrentEditor());
            return;
        case 'e':
            scheme.clearScript();
            eval = true;
            scriptError = false;
            editor.getSettings().setCursorColor(cursorColor);
            return;
        case 'f':
            toggleWindowFullscreen();
            return;
        case 'l':
            editor.setLineWrapping(!editor.getLineWrapping());
            return;
        case 'n':
            editor.setLineNumbers(!editor.getLineNumbers());
            return;
        case 't':
            hideEditor = !hideEditor;
            return;
        case 'k':
            editor.setAutoFocus(!editor.getAutoFocus());
            return;

        }
    }

    // send regular key pressed to script if the editor is hidden
    if(window->getGLFWWindow() != nullptr && !hideEditor) {
        editor.keyPressed(e.key);
    }

}

//--------------------------------------------------------------
void SchemeScript::keyReleased(ofKeyEventArgs &e){

}

//--------------------------------------------------------------
void SchemeScript::mouseMoved(ofMouseEventArgs &e){

}

//--------------------------------------------------------------
void SchemeScript::mouseDragged(ofMouseEventArgs &e){

}

//--------------------------------------------------------------
void SchemeScript::mousePressed(ofMouseEventArgs &e){

}

//--------------------------------------------------------------
void SchemeScript::mouseReleased(ofMouseEventArgs &e){

    this->setCustomVar(window->getWindowPosition().x,"OUTPUT_POSX");
    this->setCustomVar(window->getWindowPosition().y,"OUTPUT_POSY");
}

//--------------------------------------------------------------
void SchemeScript::mouseScrolled(ofMouseEventArgs &e){

}

//--------------------------------------------------------------
void SchemeScript::windowResized(ofResizeEventArgs &e){
    scaleTextureToWindow(fbo->getWidth(), fbo->getHeight(), e.width,e.height);
    editor.resize(e.width,e.height);

    this->setCustomVar(window->getWindowPosition().x,"OUTPUT_POSX");
    this->setCustomVar(window->getWindowPosition().y,"OUTPUT_POSY");
}

//--------------------------------------------------------------
void SchemeScript::loadBuffers(){

    for(size_t i=1;i<=9;i++){
        filepath = "livecoding/scheme/"+ofToString(i)+".scm";
        ofLog(OF_LOG_NOTICE,"%s",filepath.c_str());
        if(i == 1){
            ofFile temp(filepath);
            string filesFolder = temp.getEnclosingDirectory()+"/files";
            scriptFolder.listDir(filesFolder);
            scheme.clearScript();
            scheme.setScriptPath(scriptFolder.getAbsolutePath());
            currentScriptFile.open(filepath);
            sketchContent = ofBufferFromFile(currentScriptFile.getAbsolutePath());

        }
        // open script files into editor buffers
        editor.openFile(filepath,i);
    }

    scriptLoaded = true;

}

// REGISTER THE OBJECT
OBJECT_REGISTER( SchemeScript, "scheme live coding", OFXVP_OBJECT_CAT_SCRIPTING)

#endif
