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

#include "OutputWindow.h"
#include "LuaScript.h"

#include "GLFW/glfw3.h"

//--------------------------------------------------------------
OutputWindow::OutputWindow() : PatchObject("output window"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();  // projector
    _inletParams[1] = new LiveCoding(); // script reference

    _outletParams[0] = new vector<float>(); // mouse

    this->specialLinkTypeName = "LiveCoding";

    this->initInletsState();

    isFullscreen                        = false;
    thposX = thposY = thdrawW = thdrawH = 0.0f;
    isNewScriptConnected                = false;

    this->output_width      = STANDARD_TEXTURE_WIDTH;
    this->output_height     = STANDARD_TEXTURE_HEIGHT;

    temp_width              = this->output_width;
    temp_height             = this->output_height;

    window_actual_width     = STANDARD_PROJECTOR_WINDOW_WIDTH;
    window_actual_height    = STANDARD_PROJECTOR_WINDOW_HEIGHT;

    useMapping              = false;
    edgesLuminance          = 0.5f;
    edgesGamma              = 1.0f;
    edgesExponent           = 1.0f;
    edgeL = edgeR = edgeT = edgeB = 0.5f;

    needReset           = false;
    hideMouse           = false;

    loadWarpingFlag     = false;
    saveWarpingFlag     = false;

    loaded              = false;

    autoRemove          = false;

    this->setIsTextureObj(true);
    this->setIsSharedContextObj(true);
    this->setIsResizable(true);

    prevW                   = this->width;
    prevH                   = this->height;
}

//--------------------------------------------------------------
void OutputWindow::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"projector");
    this->addInlet(VP_LINK_SPECIAL,"script");

    this->addOutlet(VP_LINK_ARRAY,"mouse");

    this->setCustomVar(static_cast<float>(isFullscreen),"FULLSCREEN");
    this->setCustomVar(static_cast<float>(this->output_width),"OUTPUT_WIDTH");
    this->setCustomVar(static_cast<float>(this->output_height),"OUTPUT_HEIGHT");
    this->setCustomVar(0.0f,"OUTPUT_POSX");
    this->setCustomVar(0.0f,"OUTPUT_POSY");
    this->setCustomVar(static_cast<float>(useMapping),"USE_MAPPING");
    this->setCustomVar(static_cast<float>(hideMouse),"HIDE_MOUSE");
    this->setCustomVar(edgesLuminance,"EDGES_LUMINANCE");
    this->setCustomVar(edgesGamma,"EDGES_GAMMA");
    this->setCustomVar(edgesExponent,"EDGES_EXPONENT");
    this->setCustomVar(edgeL,"EDGE_LEFT");
    this->setCustomVar(edgeR,"EDGE_RIGHT");
    this->setCustomVar(edgeT,"EDGE_TOP");
    this->setCustomVar(edgeB,"EDGE_BOTTOM");

    this->setCustomVar(static_cast<float>(prevW),"WIDTH");
    this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
}

//--------------------------------------------------------------
void OutputWindow::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    fileDialog.setIsRetina(this->isRetina);

    loadWindowSettings();

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
    window->setVerticalSync(false);
    window->setWindowPosition(this->getCustomVar("OUTPUT_POSX"),this->getCustomVar("OUTPUT_POSY"));

    glfwSetWindowCloseCallback(window->getGLFWWindow(),GL_FALSE);

    ofAddListener(window->events().draw,this,&OutputWindow::drawInWindow);
    ofAddListener(window->events().keyPressed,this,&OutputWindow::keyPressed);
    ofAddListener(window->events().keyReleased,this,&OutputWindow::keyReleased);
    ofAddListener(window->events().mouseMoved ,this,&OutputWindow::mouseMoved);
    ofAddListener(window->events().mouseDragged ,this,&OutputWindow::mouseDragged);
    ofAddListener(window->events().mousePressed ,this,&OutputWindow::mousePressed);
    ofAddListener(window->events().mouseReleased ,this,&OutputWindow::mouseReleased);
    ofAddListener(window->events().mouseScrolled ,this,&OutputWindow::mouseScrolled);
    ofAddListener(window->events().windowResized ,this,&OutputWindow::windowResized);

    if(ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated()){
        ofLog(OF_LOG_NOTICE,"%s: NEW PROJECTOR WINDOW CREATED WITH RESOLUTION %ix%i",this->name.c_str(),this->output_width,this->output_height);
    }

    // init outlet mouse data
    ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->assign(3,0.0f);

}

//--------------------------------------------------------------
void OutputWindow::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(needReset){
        needReset = false;
        int fromObjID = -1;
        int fromOutletID = -1;
        bool isSpecialLink = false;
        if(this->inletsConnected[0]){
            for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
                if(it->second != nullptr){
                    if(patchObjects[it->first] != nullptr && it->first != this->getId() && !patchObjects[it->first]->getWillErase()){
                        for(int o=0;o<static_cast<int>(it->second->outPut.size());o++){
                            if(!it->second->outPut[o]->isDisabled && it->second->outPut[o]->toObjectID == this->getId()){
                                fromObjID = it->first;
                                fromOutletID = it->second->outPut[o]->fromOutletID;
                                if(it->second->getName() == "lua script" || it->second->getName() == "glsl shader"){
                                    it->second->resetResolution(this->getId(),this->output_width,this->output_height);
                                    isSpecialLink = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        // reset
        resetOutputResolution();
        if(this->inletsConnected[0] && fromObjID != -1 && fromOutletID != -1){
            this->disconnectFrom(patchObjects,0);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            this->connectTo(patchObjects,fromObjID,fromOutletID,0,VP_LINK_TEXTURE);

            if(isSpecialLink){
                if(!isFullscreen){
                    scaleTextureToWindow(this->output_width,this->output_height, window_actual_width, window_actual_height);
                }else{
                    scaleTextureToWindow(this->output_width,this->output_height, window->getScreenSize().x,window->getScreenSize().y);
                }
            }
        }
    }

    // Manage the different scripts reference available (ofxLua)
    if(!isNewScriptConnected && this->inletsConnected[1]){
        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if(it->second != nullptr){
                if(patchObjects[it->first] != nullptr && it->first != this->getId() && !patchObjects[it->first]->getWillErase()){
                    for(int o=0;o<static_cast<int>(it->second->outPut.size());o++){
                        if(!it->second->outPut[o]->isDisabled && it->second->outPut[o]->toObjectID == this->getId()){
                            if(it->second->getName() == "lua script"){
                                _inletParams[1] = new LiveCoding();
                            }else{
                                _inletParams[1] = nullptr;
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    isNewScriptConnected = this->inletsConnected[1];

    if(!loaded){
        loaded = true;
        temp_width = static_cast<int>(floor(this->getCustomVar("OUTPUT_WIDTH")));
        temp_height = static_cast<int>(floor(this->getCustomVar("OUTPUT_HEIGHT")));
        useMapping = static_cast<int>(floor(this->getCustomVar("USE_MAPPING")));
        hideMouse = static_cast<int>(floor(this->getCustomVar("HIDE_MOUSE")));
        edgesLuminance = this->getCustomVar("EDGES_LUMINANCE");
        edgesGamma = this->getCustomVar("EDGES_GAMMA");
        edgesExponent = this->getCustomVar("EDGES_EXPONENT");
        edgeL = this->getCustomVar("EDGE_LEFT");
        edgeR = this->getCustomVar("EDGE_RIGHT");
        edgeT = this->getCustomVar("EDGE_TOP");
        edgeB = this->getCustomVar("EDGE_BOTTOM");
        prevW = this->getCustomVar("WIDTH");
        prevH = this->getCustomVar("HEIGHT");
        this->width             = prevW;
        this->height            = prevH;

        // setup drawing  dimensions
        if(inletsConnected[0]){
            scaleTextureToWindow(ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->getWidth(), ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->getHeight(), window->getWidth(),window->getHeight());
        }else{
            scaleTextureToWindow(STANDARD_TEXTURE_WIDTH, STANDARD_TEXTURE_HEIGHT, window->getWidth(),window->getHeight());
        }


        // setup warping
        warpController = new ofxWarpController();
        if(filepath != "none"){
            warpController->loadSettings(filepath);
        }else{
            warpController->buildWarp<ofxWarpPerspectiveBilinear>();
        }

        warpController->getWarp(0)->setSize(window->getScreenSize().x,window->getScreenSize().y);
        warpController->getWarp(0)->setEdges(glm::vec4(edgeL, edgeT, edgeR, edgeB));
        warpController->getWarp(0)->setLuminance(edgesLuminance);
        warpController->getWarp(0)->setGamma(edgesGamma);
        warpController->getWarp(0)->setExponent(edgesExponent);

        window->setWindowTitle("output window | id: "+ofToString(this->getId()));

        if(static_cast<bool>(floor(this->getCustomVar("FULLSCREEN"))) != isFullscreen){
            window->setWindowPosition(this->getCustomVar("OUTPUT_POSX"),this->getCustomVar("OUTPUT_POSY"));
            toggleWindowFullscreen();
        }

        needReset = true;
    }

    // auto remove
    if(window->getGLFWWindow() == nullptr && !autoRemove){
        autoRemove = true;
        ofNotifyEvent(this->removeEvent, this->nId);
        this->willErase = true;
    }

}

//--------------------------------------------------------------
void OutputWindow::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){

    unusedArgs(font,glRenderer);

}

//--------------------------------------------------------------
void OutputWindow::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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
        _nodeCanvas.getNodeDrawList()->AddRectFilled(window_pos,window_pos+ImVec2(scaledObjW*this->scaleFactor*_nodeCanvas.GetCanvasScale(), scaledObjH*this->scaleFactor*_nodeCanvas.GetCanvasScale()),ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)));
        if(this->inletsConnected[0] && ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated()){
            calcTextureDims(*ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
            ImGui::SetCursorPos(ImVec2(posX+(IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor), posY+(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor)));
            ImGui::Image((ImTextureID)(uintptr_t)ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->getTextureData().textureID, ImVec2(drawW, drawH));
        }

        // get imgui node translated/scaled position/dimension for drawing textures in OF
        //objOriginX = (ImGui::GetWindowPos().x + ((IMGUI_EX_NODE_PINS_WIDTH_NORMAL - 1)*this->scaleFactor) - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale();
        //objOriginY = (ImGui::GetWindowPos().y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale();
        scaledObjW = this->width - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL+IMGUI_EX_NODE_PINS_WIDTH_SMALL)*this->scaleFactor/_nodeCanvas.GetCanvasScale();
        scaledObjH = this->height - (IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor/_nodeCanvas.GetCanvasScale();

        if(this->width != prevW){
            prevW = this->width;
            this->setCustomVar(static_cast<float>(prevW),"WIDTH");
        }
        if(this->height != prevH){
            prevH = this->height;
            this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
        }

        _nodeCanvas.EndNodeContent();
    }

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, loadWarpingFlag, "Select a warping config file", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".json", "", scaleFactor)){
        ofFile file (fileDialog.selected_path);
        if (file.exists()){
            filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
            warpController->loadSettings(filepath);
            edgesLuminance = this->getCustomVar("EDGES_LUMINANCE");
            edgesGamma = this->getCustomVar("EDGES_GAMMA");
            edgesExponent = this->getCustomVar("EDGES_EXPONENT");
            edgeL = this->getCustomVar("EDGE_LEFT");
            edgeR = this->getCustomVar("EDGE_RIGHT");
            edgeT = this->getCustomVar("EDGE_TOP");
            edgeB = this->getCustomVar("EDGE_BOTTOM");
        }
    }

    if(ImGuiEx::getFileDialog(fileDialog, saveWarpingFlag, "Save warping settings as", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ".json", "warpSettings.json", scaleFactor)){
        filepath = fileDialog.selected_path;
        // check extension
        if(fileDialog.ext != ".json"){
            filepath += ".json";
        }
        warpController->saveSettings(filepath);
    }
}

//--------------------------------------------------------------
void OutputWindow::drawObjectNodeConfig(){
    loadWarpingFlag = false;
    saveWarpingFlag = false;

    ImGui::Spacing();
    if(ImGui::InputInt("Width",&temp_width)){
        if(temp_width > OUTPUT_TEX_MAX_WIDTH){
            temp_width = this->output_width;
        }
    }
    ImGui::SameLine(); ImGuiEx::HelpMarker("You can set the output resolution WxH (limited for now at max. 4800x4800)");

    if(ImGui::InputInt("Height",&temp_height)){
        if(temp_height > OUTPUT_TEX_MAX_HEIGHT){
            temp_height = this->output_height;
        }
    }
    ImGui::Spacing();
    if(ImGui::Button("APPLY",ImVec2(224*scaleFactor,26*scaleFactor))){
        needReset = true;
    }
    ImGui::Spacing();
    ImGui::Spacing();
    if(ImGui::Checkbox("HIDE MOUSE",&hideMouse)){
        this->setCustomVar(hideMouse,"HIDE_MOUSE");
    }

    ImGui::Separator();

    ImGui::Spacing();
    if(ImGui::Checkbox("WARPING",&useMapping)){
        this->setCustomVar(useMapping,"USE_MAPPING");
    }
    ImGui::SameLine(); ImGuiEx::HelpMarker("Warping can be visualized/edited only in fullscreen mode!");

    if(useMapping){
        ImGui::Spacing();
        if(ImGui::SliderFloat("Luminance",&edgesLuminance,0.0f,1.0f)){
            this->setCustomVar(edgesLuminance,"EDGES_LUMINANCE");
            if(warpController->getNumWarps() > 0){
                warpController->getWarp(0)->setLuminance(edgesLuminance);
            }
        }
        ImGui::Spacing();
        if(ImGui::SliderFloat("Gamma",&edgesGamma,0.5f,1.0f)){
            this->setCustomVar(edgesGamma,"EDGES_GAMMA");
            if(warpController->getNumWarps() > 0){
                warpController->getWarp(0)->setGamma(edgesGamma);
            }
        }
        ImGui::Spacing();
        if(ImGui::SliderFloat("Exponent",&edgesExponent,0.0f,2.0f)){
            this->setCustomVar(edgesExponent,"EDGES_EXPONENT");
            if(warpController->getNumWarps() > 0){
                warpController->getWarp(0)->setExponent(edgesExponent);
            }
        }
        ImGui::Spacing();
        if(ImGui::SliderFloat("Edge Left",&edgeL,0.0f,1.0f)){
            this->setCustomVar(edgeL,"EDGE_LEFT");
            if(warpController->getNumWarps() > 0){
                warpController->getWarp(0)->setEdges(glm::vec4(edgeL, edgeT, edgeR, edgeB));
            }
        }
        if(ImGui::SliderFloat("Edge Right",&edgeR,0.0f,1.0f)){
            this->setCustomVar(edgeR,"EDGE_RIGHT");
            if(warpController->getNumWarps() > 0){
                warpController->getWarp(0)->setEdges(glm::vec4(edgeL, edgeT, edgeR, edgeB));
            }
        }
        if(ImGui::SliderFloat("Edge Top",&edgeT,0.0f,1.0f)){
            this->setCustomVar(edgeT,"EDGE_TOP");
            if(warpController->getNumWarps() > 0){
                warpController->getWarp(0)->setEdges(glm::vec4(edgeL, edgeT, edgeR, edgeB));
            }
        }
        if(ImGui::SliderFloat("Edge Bottom",&edgeB,0.0f,1.0f)){
            this->setCustomVar(edgeB,"EDGE_BOTTOM");
            if(warpController->getNumWarps() > 0){
                warpController->getWarp(0)->setEdges(glm::vec4(edgeL, edgeT, edgeR, edgeB));
            }
        }
    }

    ImGui::Separator();

    ImGui::Spacing();
    if(ImGui::Button("LOAD WARPING",ImVec2(224*scaleFactor,26*scaleFactor))){
        loadWarpingFlag = true;
    }
    ImGui::Spacing();
    if(ImGui::Button("SAVE WARPING",ImVec2(224*scaleFactor,26*scaleFactor))){
        saveWarpingFlag = true;
    }

    ImGuiEx::ObjectInfo(
                "This object creates a output window projector. (cmd/ctrl) + F focusing the window = activate/deactivate fullscreen. With warping option active and fullscreen you can adjust the projection surface.",
                "https://mosaic.d3cod3.org/reference.php?r=output-window", scaleFactor);

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, loadWarpingFlag, "Select a warping config file", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".json", "", scaleFactor)){
        ofFile file (fileDialog.selected_path);
        if (file.exists()){
            filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
            warpController->loadSettings(filepath);
            edgesLuminance = this->getCustomVar("EDGES_LUMINANCE");
            edgesGamma = this->getCustomVar("EDGES_GAMMA");
            edgesExponent = this->getCustomVar("EDGES_EXPONENT");
            edgeL = this->getCustomVar("EDGE_LEFT");
            edgeR = this->getCustomVar("EDGE_RIGHT");
            edgeT = this->getCustomVar("EDGE_TOP");
            edgeB = this->getCustomVar("EDGE_BOTTOM");
        }
    }

    if(ImGuiEx::getFileDialog(fileDialog, saveWarpingFlag, "Save warping settings as", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ".json", "warpSettings.json", scaleFactor)){
        filepath = fileDialog.selected_path;
        // check extension
        if(fileDialog.ext != ".json"){
            filepath += ".json";
        }
        warpController->saveSettings(filepath);
    }
}

//--------------------------------------------------------------
void OutputWindow::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);

    ofRemoveListener(window->events().draw,this,&OutputWindow::drawInWindow);

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
void OutputWindow::scaleTextureToWindow(float texW, float texH, float winW, float winH){
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
void OutputWindow::toggleWindowFullscreen(){
    isFullscreen = !isFullscreen;
    window->toggleFullscreen();

    this->setCustomVar(static_cast<float>(isFullscreen),"FULLSCREEN");

    if(!isFullscreen){
        window->setWindowShape(window_actual_width, window_actual_height);
        scaleTextureToWindow(ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->getWidth(), ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->getHeight(), window_actual_width, window_actual_height);
    }else{
        scaleTextureToWindow(ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->getWidth(), ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->getHeight(), window->getScreenSize().x,window->getScreenSize().y);
    }

    this->setCustomVar(window->getWindowPosition().x,"OUTPUT_POSX");
    this->setCustomVar(window->getWindowPosition().y,"OUTPUT_POSY");
}

//--------------------------------------------------------------
void OutputWindow::drawInWindow(ofEventArgs &e){
    unusedArgs(e);

    ofBackground(0);

    if(hideMouse){
        window->hideCursor();
    }else{
        window->showCursor();
    }

    ofPushStyle();
    ofPushView();
    ofPushMatrix();
    if(this->inletsConnected[0] && ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated()){

        ofSetColor(255);
        if(useMapping && isFullscreen){
            warpController->getWarp(0)->draw(*ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0]));
        }else{
            ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->draw(thposX, thposY, thdrawW, thdrawH);
        }
    }
    ofPopMatrix();
    ofPopView();
    ofPopStyle();

}

//--------------------------------------------------------------
void OutputWindow::loadWindowSettings(){
    this->output_width = static_cast<int>(floor(this->getCustomVar("OUTPUT_WIDTH")));
    this->output_height = static_cast<int>(floor(this->getCustomVar("OUTPUT_HEIGHT")));

    temp_width      = this->output_width;
    temp_height     = this->output_height;
}

//--------------------------------------------------------------
void OutputWindow::resetOutputResolution(){

    if(this->output_width != temp_width || this->output_height != temp_height){
        this->output_width = temp_width;
        this->output_height = temp_height;

        this->setCustomVar(static_cast<float>(this->output_width),"OUTPUT_WIDTH");
        this->setCustomVar(static_cast<float>(this->output_height),"OUTPUT_HEIGHT");
        this->saveConfig(false);

        if(!isFullscreen){
            scaleTextureToWindow(ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->getWidth(), ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->getHeight(), window->getWidth(),window->getHeight());
        }else{
            scaleTextureToWindow(ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->getWidth(), ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->getHeight(), window->getScreenSize().x,window->getScreenSize().y);
        }

        warpController->getWarp(0)->setSize(this->output_width,this->output_height);

        ofLog(OF_LOG_NOTICE,"%s: RESOLUTION CHANGED TO %ix%i",this->name.c_str(),this->output_width,this->output_height);
    }

}

//--------------------------------------------------------------
void OutputWindow::keyPressed(ofKeyEventArgs &e){

    if(this->inletsConnected[0] && ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated() && isFullscreen){
        warpController->onKeyPressed(e.key);
    }

    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated()){
        ofxVP_CAST_PIN_PTR<LiveCoding>(_inletParams[1])->lua.scriptKeyPressed(e.key);
    }
}

//--------------------------------------------------------------
void OutputWindow::keyReleased(ofKeyEventArgs &e){
    // OSX: CMD-F, WIN/LINUX: CTRL-F    (FULLSCREEN)
    if(e.hasModifier(MOD_KEY) && e.keycode == 70){
        toggleWindowFullscreen();
    // OSX: CMD-T, WIN/LINUX: CTRL-T    (HIDE LIVECODING EDITOR)
    }else if(e.hasModifier(MOD_KEY) && e.keycode == 84){
        ofxVP_CAST_PIN_PTR<LiveCoding>(_inletParams[1])->hide = !ofxVP_CAST_PIN_PTR<LiveCoding>(_inletParams[1])->hide;
    }

    if(this->inletsConnected[0] && ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated() && isFullscreen){
        warpController->onKeyReleased(e.key);
    }

    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated()){
        if(ofxVP_CAST_PIN_PTR<LiveCoding>(_inletParams[1])->lua.isValid()){
            ofxVP_CAST_PIN_PTR<LiveCoding>(_inletParams[1])->lua.scriptKeyReleased(e.key);
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::mouseMoved(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated() && isFullscreen){
        warpController->onMouseMoved(window->events().getMouseX(),window->events().getMouseY());
    }

    ofVec2f tm = ofVec2f(((window->events().getMouseX()-thposX)/thdrawW * this->output_width),((window->events().getMouseY()-thposY)/thdrawH * this->output_height));
    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated()){
        if(ofxVP_CAST_PIN_PTR<LiveCoding>(_inletParams[1])->lua.isValid()){
            ofxVP_CAST_PIN_PTR<LiveCoding>(_inletParams[1])->lua.scriptMouseMoved(static_cast<int>(tm.x),static_cast<int>(tm.y));
        }
    }

    ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(0) = tm.x;
    ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(1) = tm.y;
    ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(2) = e.button;

}

//--------------------------------------------------------------
void OutputWindow::mouseDragged(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated() && isFullscreen){
        warpController->onMouseDragged(window->events().getMouseX(),window->events().getMouseY());
    }

    ofVec2f tm = ofVec2f(((window->events().getMouseX()-thposX)/thdrawW * this->output_width),((window->events().getMouseY()-thposY)/thdrawH * this->output_height));
    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated()){
        if(ofxVP_CAST_PIN_PTR<LiveCoding>(_inletParams[1])->lua.isValid()){
            ofxVP_CAST_PIN_PTR<LiveCoding>(_inletParams[1])->lua.scriptMouseDragged(static_cast<int>(tm.x),static_cast<int>(tm.y), e.button);
        }
    }

    ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(0) = tm.x;
    ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(1) = tm.y;
    ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(2) = e.button;
}

//--------------------------------------------------------------
void OutputWindow::mousePressed(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated() && isFullscreen){
        warpController->onMousePressed(window->events().getMouseX(),window->events().getMouseY());
    }

    ofVec2f tm = ofVec2f(((window->events().getMouseX()-thposX)/thdrawW * this->output_width),((window->events().getMouseY()-thposY)/thdrawH * this->output_height));
    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated()){
        if(ofxVP_CAST_PIN_PTR<LiveCoding>(_inletParams[1])->lua.isValid()){
            ofxVP_CAST_PIN_PTR<LiveCoding>(_inletParams[1])->lua.scriptMousePressed(static_cast<int>(tm.x),static_cast<int>(tm.y), e.button);
        }
    }

    ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(0) = tm.x;
    ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(1) = tm.y;
    ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(2) = e.button;
}

//--------------------------------------------------------------
void OutputWindow::mouseReleased(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated() && isFullscreen){
        warpController->onMouseReleased(window->events().getMouseX(),window->events().getMouseY());
    }
    ofVec2f tm = ofVec2f(((window->events().getMouseX()-thposX)/thdrawW * this->output_width),((window->events().getMouseY()-thposY)/thdrawH * this->output_height));
    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated()){
        if(ofxVP_CAST_PIN_PTR<LiveCoding>(_inletParams[1])->lua.isValid()){
            ofxVP_CAST_PIN_PTR<LiveCoding>(_inletParams[1])->lua.scriptMouseReleased(static_cast<int>(tm.x),static_cast<int>(tm.y), e.button);
        }
    }

    ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(0) = tm.x;
    ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(1) = tm.y;
    ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(2) = e.button;

    this->setCustomVar(window->getWindowPosition().x,"OUTPUT_POSX");
    this->setCustomVar(window->getWindowPosition().y,"OUTPUT_POSY");
}

//--------------------------------------------------------------
void OutputWindow::mouseScrolled(ofMouseEventArgs &e){
    ofVec2f tm = ofVec2f(((window->events().getMouseX()-thposX)/thdrawW * this->output_width),((window->events().getMouseY()-thposY)/thdrawH * this->output_height));
    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated()){
        if(ofxVP_CAST_PIN_PTR<LiveCoding>(_inletParams[1])->lua.isValid()){
            ofxVP_CAST_PIN_PTR<LiveCoding>(_inletParams[1])->lua.scriptMouseScrolled(static_cast<int>(tm.x),static_cast<int>(tm.y), e.scrollX,e.scrollY);
        }
    }

    ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(0) = tm.x;
    ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(1) = tm.y;
    ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(2) = e.button;
}

//--------------------------------------------------------------
void OutputWindow::windowResized(ofResizeEventArgs &e){
    if(ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated()){
        scaleTextureToWindow(ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->getWidth(), ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->getHeight(), e.width,e.height);
    }

    this->setCustomVar(window->getWindowPosition().x,"OUTPUT_POSX");
    this->setCustomVar(window->getWindowPosition().y,"OUTPUT_POSY");
}


OBJECT_REGISTER( OutputWindow, "output window", OFXVP_OBJECT_CAT_WINDOWING)

#endif
