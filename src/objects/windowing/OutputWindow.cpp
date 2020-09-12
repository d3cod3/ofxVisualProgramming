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
    this->numOutlets = 0;

    _inletParams[0] = new ofTexture();  // projector
    _inletParams[1] = new LiveCoding(); // script reference

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

    warpedTexture           = new ofFbo();

    useMapping              = false;
    edgesLuminance          = 0.5f;
    edgesGamma              = 1.0f;
    edgesExponent           = 1.0f;
    edgeL = edgeR = edgeT = edgeB = 0.5f;

    needReset           = false;
    isWarpingLoaded     = false;

    loadWarpingFlag     = false;
    saveWarpingFlag     = false;

    loaded              = false;

    autoRemove          = false;

    this->setIsTextureObj(true);
}

//--------------------------------------------------------------
void OutputWindow::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"projector");
    this->addInlet(VP_LINK_SPECIAL,"script");

    this->setCustomVar(static_cast<float>(isFullscreen),"FULLSCREEN");
    this->setCustomVar(static_cast<float>(this->output_width),"OUTPUT_WIDTH");
    this->setCustomVar(static_cast<float>(this->output_height),"OUTPUT_HEIGHT");
    this->setCustomVar(0.0f,"OUTPUT_POSX");
    this->setCustomVar(0.0f,"OUTPUT_POSY");
    this->setCustomVar(static_cast<float>(useMapping),"USE_MAPPING");
    this->setCustomVar(edgesLuminance,"EDGES_LUMINANCE");
    this->setCustomVar(edgesGamma,"EDGES_GAMMA");
    this->setCustomVar(edgesExponent,"EDGES_EXPONENT");
    this->setCustomVar(edgeL,"EDGE_LEFT");
    this->setCustomVar(edgeR,"EDGE_RIGHT");
    this->setCustomVar(edgeT,"EDGE_TOP");
    this->setCustomVar(edgeB,"EDGE_BOTTOM");
}

//--------------------------------------------------------------
void OutputWindow::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    fileDialog.setIsRetina(this->isRetina);

    loadWindowSettings();

    ofGLFWWindowSettings settings;
    settings.setGLVersion(2,1);
    settings.shareContextWith = mainWindow;
    settings.decorated = true;
    settings.resizable = true;
    settings.stencilBits = 0;
    // RETINA FIX
    if(ofGetScreenWidth() >= RETINA_MIN_WIDTH && ofGetScreenHeight() >= RETINA_MIN_HEIGHT){
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
    window->setWindowTitle("Projector"+ofToString(this->getId()));
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

    static_cast<ofTexture *>(_inletParams[0])->allocate(this->output_width,this->output_height,GL_RGBA32F_ARB);
    warpedTexture->allocate(this->output_width,this->output_height,GL_RGBA32F_ARB,4);

    if(static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        ofLog(OF_LOG_NOTICE,"%s: NEW PROJECTOR WINDOW CREATED WITH RESOLUTION %ix%i",this->name.c_str(),this->output_width,this->output_height);
    }

    // setup drawing  dimensions
    asRatio = reduceToAspectRatio(this->output_width,this->output_height);
    window_asRatio = reduceToAspectRatio(window->getWidth(),window->getHeight());
    scaleTextureToWindow(window->getWidth(),window->getHeight());

}

//--------------------------------------------------------------
void OutputWindow::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    if(needReset){
        needReset = false;
        resetOutputResolution();
        if(this->inletsConnected[0]){
            for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
                if(patchObjects[it->first] != nullptr && it->first != this->getId() && !patchObjects[it->first]->getWillErase()){
                    for(int o=0;o<static_cast<int>(it->second->outPut.size());o++){
                        if(!it->second->outPut[o]->isDisabled && it->second->outPut[o]->toObjectID == this->getId()){
                            if(it->second->getName() == "lua script" || it->second->getName() == "glsl shader"){
                                it->second->resetResolution(this->getId(),this->output_width,this->output_height);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    // Manage the different scripts reference available (ofxLua)
    if(!isNewScriptConnected && this->inletsConnected[1]){
        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
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

    isNewScriptConnected = this->inletsConnected[1];

    if(!loaded){
        loaded = true;
        temp_width = static_cast<int>(floor(this->getCustomVar("OUTPUT_WIDTH")));
        temp_height = static_cast<int>(floor(this->getCustomVar("OUTPUT_HEIGHT")));
        useMapping = static_cast<int>(floor(this->getCustomVar("USE_MAPPING")));
        edgesLuminance = this->getCustomVar("EDGES_LUMINANCE");
        edgesGamma = this->getCustomVar("EDGES_GAMMA");
        edgesExponent = this->getCustomVar("EDGES_EXPONENT");
        edgeL = this->getCustomVar("EDGE_LEFT");
        edgeR = this->getCustomVar("EDGE_RIGHT");
        edgeT = this->getCustomVar("EDGE_TOP");
        edgeB = this->getCustomVar("EDGE_BOTTOM");
        if(static_cast<bool>(floor(this->getCustomVar("FULLSCREEN"))) != isFullscreen){
            window->setWindowPosition(this->getCustomVar("OUTPUT_POSX"),this->getCustomVar("OUTPUT_POSY"));
            toggleWindowFullscreen();
        }
    }

    // auto remove
    if(window->getGLFWWindow() == nullptr && !autoRemove){
        autoRemove = true;
        ofNotifyEvent(this->removeEvent, this->nId);
        this->willErase = true;
    }

}

//--------------------------------------------------------------
void OutputWindow::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    // draw node texture preview with OF
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(scaledObjW*canvasZoom > 90.0f){
            drawNodeOFTexture(*static_cast<ofTexture *>(_inletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
        }
    }else{
        if(scaledObjW*canvasZoom > 90.0f){
            ofSetColor(34,34,34);
            ofDrawRectangle(objOriginX - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/canvasZoom), objOriginY-(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor/canvasZoom),scaledObjW + (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/canvasZoom),scaledObjH + (((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor)/canvasZoom) );
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){



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

        // get imgui node translated/scaled position/dimension for drawing textures in OF
        objOriginX = (ImGui::GetWindowPos().x + ((IMGUI_EX_NODE_PINS_WIDTH_NORMAL - 1)*this->scaleFactor) - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale();
        objOriginY = (ImGui::GetWindowPos().y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale();
        scaledObjW = this->width - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/_nodeCanvas.GetCanvasScale());
        scaledObjH = this->height - ((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor/_nodeCanvas.GetCanvasScale());

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
        if(fileDialog.ext != "json"){
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

    ImGui::Separator();

    ImGui::Spacing();
    if(ImGui::Checkbox("WARPING",&useMapping)){
        this->setCustomVar(useMapping,"USE_MAPPING");
        if(useMapping){
            if(!isWarpingLoaded){
                isWarpingLoaded = true;
                // setup warping (warping first load)
                warpController = new ofxWarpController();
                if(filepath != "none"){
                    warpController->loadSettings(filepath);
                }else{
                    shared_ptr<ofxWarpPerspectiveBilinear>  warp = warpController->buildWarp<ofxWarpPerspectiveBilinear>();
                    warp->setSize(window->getScreenSize().x,window->getScreenSize().y);
                    warp->setEdges(glm::vec4(this->getCustomVar("EDGE_LEFT"), this->getCustomVar("EDGE_TOP"), this->getCustomVar("EDGE_RIGHT"), this->getCustomVar("EDGE_BOTTOM")));
                    warp->setLuminance(this->getCustomVar("EDGES_LUMINANCE"));
                    warp->setGamma(this->getCustomVar("EDGES_GAMMA"));
                    warp->setExponent(this->getCustomVar("EDGES_EXPONENT"));
                }
            }
        }
    }
    ImGui::SameLine(); ImGuiEx::HelpMarker("Warping can be visualized/edited only in fullscreen mode!");

    if(useMapping && isWarpingLoaded){
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
}

//--------------------------------------------------------------
void OutputWindow::removeObjectContent(bool removeFileFromData){
    if(window->getGLFWWindow() != nullptr){
        window->setWindowShouldClose();
    }
}

//--------------------------------------------------------------
glm::vec2 OutputWindow::reduceToAspectRatio(int _w, int _h){
    glm::vec2 _res;
    int temp = _w*_h;
    if(temp>0){
        for(int tt = temp; tt>1; tt--){
            if((_w%tt==0) && (_h%tt==0)){
                _w/=tt;
                _h/=tt;
            }
        }
    }else if (temp<0){
        for (int tt = temp; tt<-1; tt++){
            if ((_w%tt==0) && (_h%tt==0)){
                _w/=tt;
                _h/=tt;
            }
        }
    }
    _res = glm::vec2(_w,_h);
    return _res;
}

//--------------------------------------------------------------
void OutputWindow::scaleTextureToWindow(int theScreenW, int theScreenH){
    // wider texture than screen
    if(asRatio.x/asRatio.y >= window_asRatio.x/window_asRatio.y){
        thdrawW           = theScreenW;
        thdrawH           = (this->output_height*theScreenW) / this->output_width;
        thposX            = 0;
        thposY            = (theScreenH-thdrawH)/2.0f;
    // wider screen than texture
    }else{
        thdrawW           = (this->output_width*theScreenH) / this->output_height;
        thdrawH           = theScreenH;
        thposX            = (theScreenW-thdrawW)/2.0f;
        thposY            = 0;
    }
    //ofLog(OF_LOG_NOTICE,"Window: %ix%i, Texture; %fx%f at %f,%f",theScreenW,theScreenH,thdrawW,thdrawH,thposX,thposY);
}

//--------------------------------------------------------------
void OutputWindow::toggleWindowFullscreen(){
    isFullscreen = !isFullscreen;
    window->toggleFullscreen();

    this->setCustomVar(static_cast<float>(isFullscreen),"FULLSCREEN");

    if(!isFullscreen){
        window->setWindowShape(window_actual_width, window_actual_height);
        scaleTextureToWindow(window_actual_width, window_actual_height);
    }else{
        scaleTextureToWindow(window->getScreenSize().x,window->getScreenSize().y);
        if(!isWarpingLoaded){
            isWarpingLoaded = true;
            // setup warping (warping first load)
            warpController = new ofxWarpController();
            if(filepath != "none"){
                warpController->loadSettings(filepath);
            }else{
                shared_ptr<ofxWarpPerspectiveBilinear>  warp = warpController->buildWarp<ofxWarpPerspectiveBilinear>();
                warp->setSize(window->getScreenSize().x,window->getScreenSize().y);
                warp->setEdges(glm::vec4(this->getCustomVar("EDGE_LEFT"), this->getCustomVar("EDGE_TOP"), this->getCustomVar("EDGE_RIGHT"), this->getCustomVar("EDGE_BOTTOM")));
                warp->setLuminance(this->getCustomVar("EDGES_LUMINANCE"));
                warp->setGamma(this->getCustomVar("EDGES_GAMMA"));
                warp->setExponent(this->getCustomVar("EDGES_EXPONENT"));
            }
        }
    }

    this->setCustomVar(window->getWindowPosition().x,"OUTPUT_POSX");
    this->setCustomVar(window->getWindowPosition().y,"OUTPUT_POSY");
}

//--------------------------------------------------------------
void OutputWindow::drawInWindow(ofEventArgs &e){
    ofBackground(0);

    ofPushStyle();
    ofSetColor(255);
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){

        warpedTexture->begin();
        ofClear(0,0,0,255);
        static_cast<ofTexture *>(_inletParams[0])->draw(0,0,this->output_width,this->output_height);
        warpedTexture->end();

        if(useMapping && isFullscreen){
            warpController->getWarp(0)->draw(warpedTexture->getTexture());
        }else{
            warpedTexture->draw(thposX, thposY, thdrawW, thdrawH);
        }
    }else{
        ofSetColor(0);
        ofDrawRectangle(posX, posY, drawW, drawH);
    }
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

        _inletParams[0] = new ofTexture();
        static_cast<ofTexture *>(_inletParams[0])->allocate(this->output_width,this->output_height,GL_RGBA32F_ARB);

        warpedTexture           = new ofFbo();
        warpedTexture->allocate(this->output_width,this->output_height,GL_RGBA32F_ARB,4);
        warpedTexture->begin();
        ofClear(0,0,0,255);
        warpedTexture->end();

        if(static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
            this->setCustomVar(static_cast<float>(this->output_width),"OUTPUT_WIDTH");
            this->setCustomVar(static_cast<float>(this->output_height),"OUTPUT_HEIGHT");
            this->saveConfig(false);

            asRatio = reduceToAspectRatio(this->output_width,this->output_height);

            if(!isFullscreen){
                window_asRatio = reduceToAspectRatio(window->getWidth(),window->getHeight());
                scaleTextureToWindow(window->getWidth(),window->getHeight());
            }else{
                window_asRatio = reduceToAspectRatio(window->getScreenSize().x,window->getScreenSize().y);
                scaleTextureToWindow(window->getScreenSize().x,window->getScreenSize().y);
            }

            warpController = new ofxWarpController();
            shared_ptr<ofxWarpPerspectiveBilinear>  warp = warpController->buildWarp<ofxWarpPerspectiveBilinear>();
            warp->setSize(this->output_width,this->output_height);
            warp->setEdges(glm::vec4(this->getCustomVar("EDGE_LEFT"), this->getCustomVar("EDGE_TOP"), this->getCustomVar("EDGE_RIGHT"), this->getCustomVar("EDGE_BOTTOM")));
            warp->setLuminance(this->getCustomVar("EDGES_LUMINANCE"));
            warp->setGamma(this->getCustomVar("EDGES_GAMMA"));
            warp->setExponent(this->getCustomVar("EDGES_EXPONENT"));

            ofLog(OF_LOG_NOTICE,"%s: RESOLUTION CHANGED TO %ix%i",this->name.c_str(),this->output_width,this->output_height);
        }
    }

}

//--------------------------------------------------------------
void OutputWindow::keyPressed(ofKeyEventArgs &e){

    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated() && isFullscreen){
        warpController->onKeyPressed(e.key);
    }

    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(static_cast<LiveCoding *>(_inletParams[1])->hide){
            static_cast<LiveCoding *>(_inletParams[1])->lua.scriptKeyPressed(e.key);
        }else{
            static_cast<LiveCoding *>(_inletParams[1])->liveEditor.keyPressed(e.key);
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::keyReleased(ofKeyEventArgs &e){
    // OSX: CMD-F, WIN/LINUX: CTRL-F    (FULLSCREEN)
    if(e.hasModifier(MOD_KEY) && e.keycode == 70){
        toggleWindowFullscreen();
    // OSX: CMD-E, WIN/LINUX: CTRL-E    (EXECUTE SCRIPT)
    }else if(e.hasModifier(MOD_KEY) && e.keycode == 69){
        static_cast<LiveCoding *>(_inletParams[1])->liveEditor.saveFile(static_cast<LiveCoding *>(_inletParams[1])->filepath);
    // OSX: CMD-T, WIN/LINUX: CTRL-T    (HIDE LIVECODING EDITOR)
    }else if(e.hasModifier(MOD_KEY) && e.keycode == 84){
        static_cast<LiveCoding *>(_inletParams[1])->hide = !static_cast<LiveCoding *>(_inletParams[1])->hide;
    // OSX: CMD-K, WIN/LINUX: CTRL-K    (TOGGLE AUTO FOCUS)
    }else if(e.hasModifier(MOD_KEY) && e.keycode == 75){
        static_cast<LiveCoding *>(_inletParams[1])->liveEditor.setAutoFocus(!static_cast<LiveCoding *>(_inletParams[1])->liveEditor.getAutoFocus());
    // OSX: CMD-L, WIN/LINUX: CTRL-L    (TOGGLE LINE WRAPPING)
    }else if(e.hasModifier(MOD_KEY) && e.keycode == 76){
        static_cast<LiveCoding *>(_inletParams[1])->liveEditor.setLineWrapping(!static_cast<LiveCoding *>(_inletParams[1])->liveEditor.getLineWrapping());
    // OSX: CMD-N, WIN/LINUX: CTRL-N    (TOGGLE LINE NUMBERS)
    }else if(e.hasModifier(MOD_KEY) && e.keycode == 78){
        static_cast<LiveCoding *>(_inletParams[1])->liveEditor.setLineNumbers(!static_cast<LiveCoding *>(_inletParams[1])->liveEditor.getLineNumbers());
    }

    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated() && isFullscreen){
        warpController->onKeyReleased(e.key);
    }

    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(static_cast<LiveCoding *>(_inletParams[1])->lua.isValid()){
            static_cast<LiveCoding *>(_inletParams[1])->lua.scriptKeyReleased(e.key);
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::mouseMoved(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated() && isFullscreen){
        warpController->onMouseMoved(window->events().getMouseX(),window->events().getMouseY());
    }

    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        ofVec2f tm = ofVec2f(((window->events().getMouseX()-thposX)/thdrawW * this->output_width),((window->events().getMouseY()-thposY)/thdrawH * this->output_height));
        if(static_cast<LiveCoding *>(_inletParams[1])->lua.isValid()){
            static_cast<LiveCoding *>(_inletParams[1])->lua.scriptMouseMoved(static_cast<int>(tm.x),static_cast<int>(tm.y));
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::mouseDragged(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated() && isFullscreen){
        warpController->onMouseDragged(window->events().getMouseX(),window->events().getMouseY());
    }
    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        ofVec2f tm = ofVec2f(((window->events().getMouseX()-thposX)/thdrawW * this->output_width),((window->events().getMouseY()-thposY)/thdrawH * this->output_height));
        if(static_cast<LiveCoding *>(_inletParams[1])->lua.isValid()){
            static_cast<LiveCoding *>(_inletParams[1])->lua.scriptMouseDragged(static_cast<int>(tm.x),static_cast<int>(tm.y), e.button);
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::mousePressed(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated() && isFullscreen){
        warpController->onMousePressed(window->events().getMouseX(),window->events().getMouseY());
    }
    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        ofVec2f tm = ofVec2f(((window->events().getMouseX()-thposX)/thdrawW * this->output_width),((window->events().getMouseY()-thposY)/thdrawH * this->output_height));
        if(static_cast<LiveCoding *>(_inletParams[1])->lua.isValid()){
            static_cast<LiveCoding *>(_inletParams[1])->lua.scriptMousePressed(static_cast<int>(tm.x),static_cast<int>(tm.y), e.button);
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::mouseReleased(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated() && isFullscreen){
        warpController->onMouseReleased(window->events().getMouseX(),window->events().getMouseY());
    }
    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        ofVec2f tm = ofVec2f(((window->events().getMouseX()-thposX)/thdrawW * this->output_width),((window->events().getMouseY()-thposY)/thdrawH * this->output_height));
        if(static_cast<LiveCoding *>(_inletParams[1])->lua.isValid()){
            static_cast<LiveCoding *>(_inletParams[1])->lua.scriptMouseReleased(static_cast<int>(tm.x),static_cast<int>(tm.y), e.button);
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::mouseScrolled(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        ofVec2f tm = ofVec2f(((window->events().getMouseX()-thposX)/thdrawW * this->output_width),((window->events().getMouseY()-thposY)/thdrawH * this->output_height));
        if(static_cast<LiveCoding *>(_inletParams[1])->lua.isValid()){
            static_cast<LiveCoding *>(_inletParams[1])->lua.scriptMouseScrolled(static_cast<int>(tm.x),static_cast<int>(tm.y), e.scrollX,e.scrollY);
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::windowResized(ofResizeEventArgs &e){
    window_asRatio = reduceToAspectRatio(window->getWidth(),window->getHeight());
    scaleTextureToWindow(window->getWidth(),window->getHeight());

    this->setCustomVar(window->getWindowPosition().x,"OUTPUT_POSX");
    this->setCustomVar(window->getWindowPosition().y,"OUTPUT_POSY");

    if(this->inletsConnected[2]){
        static_cast<ofxEditor *>(_inletParams[2])->resize(window->getWidth(),window->getHeight());
    }
}


OBJECT_REGISTER( OutputWindow, "output window", OFXVP_OBJECT_CAT_WINDOWING)

#endif
