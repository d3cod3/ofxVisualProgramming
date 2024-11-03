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

#ifndef OFXVP_BUILD_WITH_MINIMAL_OBJECTS

#include "ProjectionMapping.h"

#include "GLFW/glfw3.h"

//--------------------------------------------------------------
ProjectionMapping::ProjectionMapping() : PatchObject("projection mapping"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();  // source
    _inletParams[1] = new ofTexture();  // background

    _outletParams[0] = new ofTexture();  // mapping

    this->initInletsState();

    isFullscreen                        = false;

    _mapping = new ofxMtlMapping2D();

    output_width            = STANDARD_TEXTURE_WIDTH;
    output_height           = STANDARD_TEXTURE_HEIGHT;

    window_actual_width     = STANDARD_PROJECTOR_WINDOW_WIDTH;
    window_actual_height    = STANDARD_PROJECTOR_WINDOW_HEIGHT;

    needReset           = false;

    lastWarpingConfig   = "";
    loadWarpingFlag     = false;
    saveWarpingFlag     = false;
    warpingConfigLoaded = false;

    isOverWindow        = false;
    winMouseX           = 0;
    winMouseY           = 0;

    autoRemove          = false;

    this->setIsTextureObj(true);
    this->setIsSharedContextObj(true);
    this->setIsResizable(true);
}

//--------------------------------------------------------------
void ProjectionMapping::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"source");
    this->addInlet(VP_LINK_TEXTURE,"background");

    this->addOutlet(VP_LINK_TEXTURE,"mappingOutput");
}

//--------------------------------------------------------------
void ProjectionMapping::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    fileDialog.setIsRetina(this->isRetina);

    ofGLFWWindowSettings settings;
#if defined(OFXVP_GL_VERSION_MAJOR) && defined(OFXVP_GL_VERSION_MINOR)
    settings.setGLVersion(OFXVP_GL_VERSION_MAJOR,OFXVP_GL_VERSION_MINOR);
#else
    settings.setGLVersion(3,2);
#endif
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
    window->setWindowTitle("Mapping"+ofToString(this->getId()));
    window->setVerticalSync(true);

    glfwSetWindowCloseCallback(window->getGLFWWindow(),GL_FALSE);

    ofAddListener(window->events().update,this,&ProjectionMapping::updateInWindow);
    ofAddListener(window->events().draw,this,&ProjectionMapping::drawInWindow);
    ofAddListener(window->events().keyPressed,this,&ProjectionMapping::keyPressed);
    ofAddListener(window->events().mouseDragged ,this,&ProjectionMapping::mouseDragged);
    ofAddListener(window->events().mousePressed ,this,&ProjectionMapping::mousePressed);
    ofAddListener(window->events().mouseReleased ,this,&ProjectionMapping::mouseReleased);
    ofAddListener(window->events().mouseScrolled ,this,&ProjectionMapping::mouseScrolled);
    ofAddListener(window->events().windowResized ,this,&ProjectionMapping::windowResized);

    static_cast<ofTexture *>(_inletParams[0])->allocate(output_width,output_height,GL_RGBA);

    _mapping->init(output_width,output_height,ofToDataPath("mapping/default.xml"));

    if(static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        ofLog(OF_LOG_NOTICE,"%s: PROJECTION MAPPING CREATED WITH OUTPUT RESOLUTION %ix%i",this->name.c_str(),output_width,output_height);
    }

}

//--------------------------------------------------------------
void ProjectionMapping::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(warpingConfigLoaded){
        warpingConfigLoaded = false;
        ofFile file (lastWarpingConfig);
        if (file.exists()){
            string fileExtension = ofToUpper(file.getExtension());
            if(fileExtension == "XML") {
                filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
                //filepath = file.getAbsolutePath();
                _mapping->loadMapping(filepath);
            }
        }
    }

    // reset mapping textures resolution on inlet connection
    if(this->inletsConnected[0]){
        if(static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
            if(!needReset){
                needReset = true;
                resetMappingResolution();
            }
        }
    }else{
        needReset = false;
    }

    // auto remove
    if(window->getGLFWWindow() == nullptr && !autoRemove){
        autoRemove = true;
        ofNotifyEvent(this->removeEvent, this->nId);
        this->willErase = true;
    }

    *static_cast<ofTexture *>(_outletParams[0]) = _mapping->getOutputFbo().getTexture();

}

//--------------------------------------------------------------
void ProjectionMapping::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);
}

//--------------------------------------------------------------
void ProjectionMapping::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    loadWarpingFlag = false;
    saveWarpingFlag = false;

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){

        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            drawObjectNodeConfig(); this->configMenuWidth = ImGui::GetWindowWidth();

            ImGui::EndMenu();
        }

        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        ImVec2 window_pos = ImGui::GetWindowPos()+ImVec2(IMGUI_EX_NODE_PINS_WIDTH_NORMAL, IMGUI_EX_NODE_HEADER_HEIGHT);

        if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
            calcTextureDims(*static_cast<ofTexture *>(_outletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
            _nodeCanvas.getNodeDrawList()->AddRectFilled(window_pos,window_pos+ImVec2(scaledObjW*this->scaleFactor*_nodeCanvas.GetCanvasScale(), scaledObjH*this->scaleFactor*_nodeCanvas.GetCanvasScale()),ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)));
            ImGui::SetCursorPos(ImVec2(posX+IMGUI_EX_NODE_PINS_WIDTH_SMALL+2, posY+IMGUI_EX_NODE_HEADER_HEIGHT));
            ImGui::Image((ImTextureID)(uintptr_t)static_cast<ofTexture *>(_outletParams[0])->getTextureData().textureID, ImVec2(drawW, drawH));
        }else{
            _nodeCanvas.getNodeDrawList()->AddRectFilled(window_pos,window_pos+ImVec2(scaledObjW*this->scaleFactor*_nodeCanvas.GetCanvasScale(), scaledObjH*this->scaleFactor*_nodeCanvas.GetCanvasScale()),ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)));
        }

        // get imgui node translated/scaled position/dimension for drawing textures in OF
        //objOriginX = (ImGui::GetWindowPos().x + ((IMGUI_EX_NODE_PINS_WIDTH_NORMAL - 1)*this->scaleFactor) - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale();
        //objOriginY = (ImGui::GetWindowPos().y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale();
        scaledObjW = this->width - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/_nodeCanvas.GetCanvasScale());
        scaledObjH = this->height - ((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor/_nodeCanvas.GetCanvasScale());

        _nodeCanvas.EndNodeContent();
    }

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, loadWarpingFlag, "Select a mapping config file", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".xml", "", scaleFactor)){
        ofFile file (fileDialog.selected_path);
        if (file.exists()){
            warpingConfigLoaded = true;
            lastWarpingConfig = file.getAbsolutePath();
        }
    }

    if(ImGuiEx::getFileDialog(fileDialog, saveWarpingFlag, "Save mapping settings as", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ".xml", "mappingSettings.xml", scaleFactor)){
        filepath = fileDialog.selected_path;
        // check extension
        if(fileDialog.ext != ".xml"){
            filepath += ".xml";
        }
        _mapping->saveMappingAs(filepath);

    }
}

//--------------------------------------------------------------
void ProjectionMapping::drawObjectNodeConfig(){

    loadWarpingFlag = false;
    saveWarpingFlag = false;

    ImGui::Separator();

    ImGui::Spacing();
    if(ImGui::Button("LOAD MAPPING",ImVec2(224*scaleFactor,26*scaleFactor))){
        loadWarpingFlag = true;
    }
    ImGui::Spacing();
    if(ImGui::Button("SAVE MAPPING",ImVec2(224*scaleFactor,26*scaleFactor))){
        saveWarpingFlag = true;
    }

    ImGuiEx::ObjectInfo(
                "Module for video mapping. It provides an interface with more flexible resources than the warping grid.",
                "https://mosaic.d3cod3.org/reference.php?r=projection-mapping", scaleFactor);

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, loadWarpingFlag, "Select a mapping config file", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".xml", "", scaleFactor)){
        ofFile file (fileDialog.selected_path);
        if (file.exists()){
            warpingConfigLoaded = true;
            lastWarpingConfig = file.getAbsolutePath();
        }
    }

    if(ImGuiEx::getFileDialog(fileDialog, saveWarpingFlag, "Save mapping settings as", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ".xml", "mappingSettings.xml", scaleFactor)){
        filepath = fileDialog.selected_path;
        // check extension
        if(fileDialog.ext != ".xml"){
            filepath += ".xml";
        }
        _mapping->saveMappingAs(filepath);

    }
}

//--------------------------------------------------------------
void ProjectionMapping::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);

    ofRemoveListener(window->events().draw,this,&ProjectionMapping::updateInWindow);
    ofRemoveListener(window->events().draw,this,&ProjectionMapping::drawInWindow);

    if(window->getGLFWWindow() != nullptr){
        window->finishRender();
        glfwHideWindow(window->getGLFWWindow());
        //window->setWindowShouldClose();
    }
}

//--------------------------------------------------------------
void ProjectionMapping::toggleWindowFullscreen(){
    isFullscreen = !isFullscreen;
    window->toggleFullscreen();

    _mapping->resetViewports();
}

//--------------------------------------------------------------
void ProjectionMapping::updateInWindow(ofEventArgs &e){

    _mapping->update();

    glfwGetCursorPos(window->getGLFWWindow(), &winMouseX, &winMouseY);
    if(winMouseX > 0 && winMouseY > 0 && winMouseX < window->getWindowSize().x && winMouseY < window->getWindowSize().y){
        //ofLog(OF_LOG_NOTICE,"%f:%f",winMouseX,winMouseY);
        isOverWindow = true;
    }else{
        isOverWindow = false;
    }

}

//--------------------------------------------------------------
void ProjectionMapping::drawInWindow(ofEventArgs &e){
    ofBackground(0);

    _mapping->bindBackground();
    if(this->inletsConnected[1] && static_cast<ofTexture *>(_inletParams[1])->isAllocated()){
        // mapping reference background
        static_cast<ofTexture *>(_inletParams[1])->draw(0,0,output_width,output_height);
    }else{
        ofSetColor(0);
        ofDrawRectangle(0,0,output_width,output_height);
    }
    _mapping->unbindBackground();

    _mapping->bind();
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        // base
        static_cast<ofTexture *>(_inletParams[0])->draw(0,0,output_width,output_height);
    }else{
        ofSetColor(0);
        ofDrawRectangle(0,0,output_width,output_height);
    }
    _mapping->unbind();

    _mapping->draw();

}

//--------------------------------------------------------------
void ProjectionMapping::resetMappingResolution(){

    if(output_width != static_cast<int>(static_cast<ofTexture *>(_inletParams[0])->getWidth()) || output_height != static_cast<int>(static_cast<ofTexture *>(_inletParams[0])->getHeight())){
        output_width            = static_cast<int>(static_cast<ofTexture *>(_inletParams[0])->getWidth());
        output_height           = static_cast<int>(static_cast<ofTexture *>(_inletParams[0])->getHeight());

        _mapping->reset(output_width,output_height);

        ofLog(OF_LOG_NOTICE,"%s: PROJECTION MAPPING RESOLUTION CHANGED TO %ix%i",this->name.c_str(),output_width,output_height);
    }

}

//--------------------------------------------------------------
void ProjectionMapping::keyPressed(ofKeyEventArgs &e){
    // OSX: CMD-F, WIN/LINUX: CTRL-F    (FULLSCREEN)
    if(e.hasModifier(MOD_KEY) && e.keycode == 70){
        toggleWindowFullscreen();
    }

    if(isOverWindow){
        _mapping->keyPressed(e);
    }

}

//--------------------------------------------------------------
void ProjectionMapping::mouseDragged(ofMouseEventArgs &e){
    if(isOverWindow){
        _mapping->mouseDragged(e);
    }
}

//--------------------------------------------------------------
void ProjectionMapping::mousePressed(ofMouseEventArgs &e){
    if(isOverWindow){
        _mapping->mousePressed(e);
    }
}

//--------------------------------------------------------------
void ProjectionMapping::mouseReleased(ofMouseEventArgs &e){
    if(isOverWindow){
        _mapping->mouseReleased(e);
    }
}

//--------------------------------------------------------------
void ProjectionMapping::mouseScrolled(ofMouseEventArgs &e){
    if(isOverWindow){
        _mapping->mouseScrolled(e);
    }
}

//--------------------------------------------------------------
void ProjectionMapping::windowResized(ofResizeEventArgs &e){
    _mapping->windowResized(e);
}


OBJECT_REGISTER( ProjectionMapping, "projection mapping", OFXVP_OBJECT_CAT_WINDOWING)

#endif
