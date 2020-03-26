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

#include "OutputWindow.h"
#include "LuaScript.h"

#include "GLFW/glfw3.h"

//--------------------------------------------------------------
OutputWindow::OutputWindow() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 0;

    _inletParams[0] = new ofTexture();  // projector
    _inletParams[1] = new LiveCoding(); // script reference

    this->initInletsState();

    isFullscreen                        = false;
    thposX = thposY = thdrawW = thdrawH = 0.0f;
    isNewScriptConnected                = false;
    inletScriptType                     = 0;        // 0 -> LUA, 1 -> PYTHON, .....

    this->output_width      = STANDARD_TEXTURE_WIDTH;
    this->output_height     = STANDARD_TEXTURE_HEIGHT;

    temp_width              = this->output_width;
    temp_height             = this->output_height;

    window_actual_width     = STANDARD_PROJECTOR_WINDOW_WIDTH;
    window_actual_height    = STANDARD_PROJECTOR_WINDOW_HEIGHT;

    finalTexture        = new ofFbo();

    isGUIObject         = true;
    this->isOverGUI     = true;

    needReset           = false;
    isWarpingLoaded     = false;

    lastWarpingConfig   = "";
    loadWarpingFlag     = false;
    saveWarpingFlag     = false;
    warpingConfigLoaded = false;

    loaded              = false;

    autoRemove          = false;
}

//--------------------------------------------------------------
void OutputWindow::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_TEXTURE,"projector");
    this->addInlet(VP_LINK_SPECIAL,"script");

    this->setCustomVar(static_cast<float>(this->output_width),"OUTPUT_WIDTH");
    this->setCustomVar(static_cast<float>(this->output_height),"OUTPUT_HEIGHT");
    this->setCustomVar(static_cast<float>(0.0),"USE_MAPPING");
    this->setCustomVar(1.0f,"EDGES_EXPONENT");
    this->setCustomVar(0.5f,"EDGE_LEFT");
    this->setCustomVar(0.5f,"EDGE_RIGHT");
    this->setCustomVar(0.5f,"EDGE_TOP");
    this->setCustomVar(0.5f,"EDGE_BOTTOM");
}

//--------------------------------------------------------------
void OutputWindow::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
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
    window->setVerticalSync(true);

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
    finalTexture->allocate(this->output_width,this->output_height,GL_RGBA32F_ARB,4);

    if(static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        ofLog(OF_LOG_NOTICE,"%s: NEW PROJECTOR WINDOW CREATED WITH RESOLUTION %ix%i",this->name.c_str(),this->output_width,this->output_height);
    }

    // setup drawing  dimensions
    asRatio = reduceToAspectRatio(this->output_width,this->output_height);
    scaleTextureToWindow(window->getWidth(),window->getHeight());

    // GUI
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onToggleEvent(this, &OutputWindow::onToggleEvent);
    gui->onButtonEvent(this, &OutputWindow::onButtonEvent);
    gui->onTextInputEvent(this, &OutputWindow::onTextInputEvent);
    gui->onSliderEvent(this, &OutputWindow::onSliderEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    guiTexWidth = gui->addTextInput("WIDTH",ofToString(this->output_width));
    guiTexWidth->setUseCustomMouse(true);
    guiTexHeight = gui->addTextInput("HEIGHT",ofToString(this->output_height));
    guiTexHeight->setUseCustomMouse(true);
    applyButton = gui->addButton("APPLY");
    applyButton->setUseCustomMouse(true);
    applyButton->setLabelAlignment(ofxDatGuiAlignment::CENTER);
    gui->addBreak();
    useMapping = gui->addToggle("WARPING",static_cast<int>(floor(this->getCustomVar("USE_MAPPING"))));
    useMapping->setUseCustomMouse(true);
    gui->addBreak();
    edgesExponent = gui->addSlider("Exponent",0.8f,1.2f,this->getCustomVar("EDGES_EXPONENT"));
    edgesExponent->setUseCustomMouse(true);
    edgeL = gui->addSlider("Edge Left",0.0f,1.0f,this->getCustomVar("EDGE_LEFT"));
    edgeL->setUseCustomMouse(true);
    edgeR = gui->addSlider("Edge Right",0.0f,1.0f,this->getCustomVar("EDGE_RIGHT"));
    edgeR->setUseCustomMouse(true);
    edgeT = gui->addSlider("Edge Top",0.0f,1.0f,this->getCustomVar("EDGE_TOP"));
    edgeT->setUseCustomMouse(true);
    edgeB = gui->addSlider("Edge Bottom",0.0f,1.0f,this->getCustomVar("EDGE_BOTTOM"));
    edgeB->setUseCustomMouse(true);
    gui->addBreak();
    loadWarping = gui->addButton("LOAD WARPING");
    loadWarping->setUseCustomMouse(true);
    loadWarping->setLabelAlignment(ofxDatGuiAlignment::CENTER);
    saveWarping = gui->addButton("SAVE WARPING");
    saveWarping->setUseCustomMouse(true);
    saveWarping->setLabelAlignment(ofxDatGuiAlignment::CENTER);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);
}

//--------------------------------------------------------------
void OutputWindow::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    guiTexWidth->update();
    guiTexHeight->update();
    applyButton->update();
    useMapping->update();
    edgesExponent->update();
    edgeL->update();
    edgeR->update();
    edgeT->update();
    edgeB->update();
    loadWarping->update();
    saveWarping->update();

    if(loadWarpingFlag){
        loadWarpingFlag = false;
        fd.openFile("open warp config"+ofToString(this->getId()),"Select a warping config file");
    }

    if(warpingConfigLoaded){
        warpingConfigLoaded = false;
        ofFile file (lastWarpingConfig);
        if (file.exists()){
            string fileExtension = ofToUpper(file.getExtension());
            if(fileExtension == "JSON") {
                filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
                //filepath = file.getAbsolutePath();
                warpController->loadSettings(filepath);
                edgesExponent->setValue(this->getCustomVar("EDGES_EXPONENT"));
                edgeL->setValue(this->getCustomVar("EDGE_LEFT"));
                edgeR->setValue(this->getCustomVar("EDGE_RIGHT"));
                edgeT->setValue(this->getCustomVar("EDGE_TOP"));
                edgeB->setValue(this->getCustomVar("EDGE_BOTTOM"));
            }
        }
    }

    if(saveWarpingFlag){
        saveWarpingFlag = false;
        fd.saveFile("save warp config"+ofToString(this->getId()),"Save warping settings as","warpSettings.json");
    }

    if(needReset){
        needReset = false;
        resetResolution();
        if(this->inletsConnected[0]){
            for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
                if(patchObjects[it->first] != nullptr && it->first != this->getId() && !patchObjects[it->first]->getWillErase()){
                    for(int o=0;o<static_cast<int>(it->second->outPut.size());o++){
                        if(!it->second->outPut[o]->isDisabled && it->second->outPut[o]->toObjectID == this->getId()){
                            if(it->second->getName() == "lua script" || it->second->getName() == "shader object"){
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
                            inletScriptType         = 0;
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
        guiTexWidth->setText(ofToString(this->getCustomVar("OUTPUT_WIDTH")));
        guiTexHeight->setText(ofToString(this->getCustomVar("OUTPUT_HEIGHT")));
        useMapping->setChecked(static_cast<int>(floor(this->getCustomVar("USE_MAPPING"))));
        edgesExponent->setValue(this->getCustomVar("EDGES_EXPONENT"));
        edgeL->setValue(this->getCustomVar("EDGE_LEFT"));
        edgeR->setValue(this->getCustomVar("EDGE_RIGHT"));
        edgeT->setValue(this->getCustomVar("EDGE_TOP"));
        edgeB->setValue(this->getCustomVar("EDGE_BOTTOM"));
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
    ofEnableAlphaBlending();
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(static_cast<ofTexture *>(_inletParams[0])->getWidth() >= static_cast<ofTexture *>(_inletParams[0])->getHeight()){   // horizontal texture
            thdrawW           = this->width;
            thdrawH           = (this->width/static_cast<ofTexture *>(_inletParams[0])->getWidth())*static_cast<ofTexture *>(_inletParams[0])->getHeight();
            thposX            = 0;
            thposY            = (this->height-thdrawH)/2.0f;
        }else{ // vertical texture
            thdrawW           = (static_cast<ofTexture *>(_inletParams[0])->getWidth()*this->height)/static_cast<ofTexture *>(_inletParams[0])->getHeight();
            thdrawH           = this->height;
            thposX            = (this->width-thdrawW)/2.0f;
            thposY            = 0;
        }
        static_cast<ofTexture *>(_inletParams[0])->draw(thposX,thposY,thdrawW,thdrawH);
    }else{
        ofSetColor(0);
        ofDrawRectangle(0,0,this->width,this->height);
    }
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void OutputWindow::removeObjectContent(bool removeFileFromData){
    if(window->getGLFWWindow() != nullptr){
        window->setWindowShouldClose();
    }
}

//--------------------------------------------------------------
void OutputWindow::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    guiTexWidth->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    guiTexHeight->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    applyButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    useMapping->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    edgesExponent->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    edgeL->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    edgeR->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    edgeT->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    edgeB->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    loadWarping->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    saveWarping->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || guiTexWidth->hitTest(_m-this->getPos()) || guiTexHeight->hitTest(_m-this->getPos())
                        || edgesExponent->hitTest(_m-this->getPos()) || edgeL->hitTest(_m-this->getPos()) || edgeR->hitTest(_m-this->getPos()) || edgeT->hitTest(_m-this->getPos()) || edgeB->hitTest(_m-this->getPos())
                        || applyButton->hitTest(_m-this->getPos()) || useMapping->hitTest(_m-this->getPos()) || loadWarping->hitTest(_m-this->getPos()) || saveWarping->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void OutputWindow::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        guiTexWidth->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        guiTexHeight->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        applyButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        useMapping->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        edgesExponent->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        edgeL->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        edgeR->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        edgeT->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        edgeB->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        loadWarping->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        saveWarping->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }else{
        ofNotifyEvent(dragEvent, nId);

        box->setFromCenter(_m.x, _m.y,box->getWidth(),box->getHeight());
        headerBox->set(box->getPosition().x,box->getPosition().y,box->getWidth(),headerHeight);

        x = box->getPosition().x;
        y = box->getPosition().y;

        for(int j=0;j<static_cast<int>(outPut.size());j++){
            outPut[j]->linkVertices[0].move(outPut[j]->posFrom.x,outPut[j]->posFrom.y);
            outPut[j]->linkVertices[1].move(outPut[j]->posFrom.x+20,outPut[j]->posFrom.y);
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::fileDialogResponse(ofxThreadedFileDialogResponse &response){
    if(response.id == "open warp config"+ofToString(this->getId())){
        warpingConfigLoaded = true;
        lastWarpingConfig = response.filepath;
    }else if(response.id == "save warp config"+ofToString(this->getId())){
        ofFile newFile (response.filepath);
        filepath = newFile.getAbsolutePath();
        warpController->saveSettings(filepath);
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
    if(this->output_width >= this->output_height){   // horizontal texture
        if(isFullscreen){
            if(drawW >= theScreenW){
                drawW           = theScreenW;
                drawH           = drawW*asRatio.y / asRatio.x;
                posX            = 0;
                posY            = (theScreenH-drawH)/2.0f;
            }else{
                drawW           = (this->output_width*theScreenH)/this->output_height;
                drawH           = theScreenH;
                posX            = (theScreenW-drawW)/2.0f;
                posY            = 0;
            }
        }else{
            if(this->output_width >= theScreenW){
                drawW           = theScreenW;
                drawH           = drawW*asRatio.y / asRatio.x;
                posX            = 0;
                posY            = (theScreenH-drawH)/2.0f;
            }else{
                drawW           = (this->output_width*theScreenH)/this->output_height;
                drawH           = theScreenH;
                posX            = (theScreenW-drawW)/2.0f;
                posY            = 0;
            }
        }


    }else{                              // vertical texture
        if(isFullscreen){
            if(drawH >= theScreenH){
                drawW           = (this->output_width*theScreenH)/this->output_height;
                drawH           = theScreenH;
                posX            = (theScreenW-drawW)/2.0f;
                posY            = 0;
            }else{
                drawW           = theScreenW;
                drawH           = drawW*asRatio.y / asRatio.x;
                posX            = 0;
                posY            = (theScreenH-drawH)/2.0f;
            }
        }else{
            if(output_height >= theScreenH){
                drawW           = (this->output_width*theScreenH)/this->output_height;
                drawH           = theScreenH;
                posX            = (theScreenW-drawW)/2.0f;
                posY            = 0;
            }else{
                drawW           = theScreenW;
                drawH           = drawW*asRatio.y / asRatio.x;
                posX            = 0;
                posY            = (theScreenH-drawH)/2.0f;
            }
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::toggleWindowFullscreen(){
    isFullscreen = !isFullscreen;
    window->toggleFullscreen();

    if(!isFullscreen){
        window->setWindowShape(window_actual_width, window_actual_height);
        scaleTextureToWindow(window->getWidth(),window->getHeight());
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
                warp->setExponent(this->getCustomVar("EDGES_EXPONENT"));
            }
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::drawInWindow(ofEventArgs &e){
    finalTexture->begin();
    ofClear(0,0,0,255);
    static_cast<ofTexture *>(_inletParams[0])->draw(0,0,this->output_width,this->output_height);
    finalTexture->end();

    ofBackground(0);

    ofPushStyle();
    ofSetColor(255);
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
#ifdef TARGET_LINUX
        if(isFullscreen){
            if(useMapping->getChecked()){
                warpController->getWarp(0)->draw(*static_cast<ofTexture *>(_inletParams[0]));
            }else{
                static_cast<ofTexture *>(_inletParams[0])->draw(posX, posY, drawW, drawH);
            }
        }else{
            static_cast<ofTexture *>(_inletParams[0])->draw(posX, posY, drawW, drawH);
        }
#else
        if(isFullscreen){
            if(useMapping->getChecked()){
                warpController->getWarp(0)->draw(finalTexture->getTexture());
            }else{
                finalTexture->draw(posX, posY, drawW, drawH);
            }
        }else{
            finalTexture->draw(posX, posY, drawW, drawH);
        }
#endif
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
void OutputWindow::resetResolution(){

    if(this->output_width != temp_width || this->output_height != temp_height){
        this->output_width = temp_width;
        this->output_height = temp_height;

        _inletParams[0] = new ofTexture();
        static_cast<ofTexture *>(_inletParams[0])->allocate(this->output_width,this->output_height,GL_RGBA32F_ARB);

        finalTexture = new ofFbo();
        finalTexture->allocate(this->output_width,this->output_height,GL_RGBA32F_ARB,4);

        finalTexture->begin();
        ofClear(0,0,0,255);
        finalTexture->end();

        if(static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
            this->setCustomVar(static_cast<float>(this->output_width),"OUTPUT_WIDTH");
            this->setCustomVar(static_cast<float>(this->output_height),"OUTPUT_HEIGHT");
            this->saveConfig(false,this->nId);

            asRatio = reduceToAspectRatio(this->output_width,this->output_height);
            if(!isFullscreen){
                scaleTextureToWindow(window->getWidth(),window->getHeight());
            }else{
                scaleTextureToWindow(window->getScreenSize().x,window->getScreenSize().y);
            }

            warpController = new ofxWarpController();
            shared_ptr<ofxWarpPerspectiveBilinear>  warp = warpController->buildWarp<ofxWarpPerspectiveBilinear>();
            warp->setSize(this->output_width,this->output_height);
            warp->setEdges(glm::vec4(this->getCustomVar("EDGE_LEFT"), this->getCustomVar("EDGE_TOP"), this->getCustomVar("EDGE_RIGHT"), this->getCustomVar("EDGE_BOTTOM")));
            warp->setExponent(this->getCustomVar("EDGES_EXPONENT"));

            ofLog(OF_LOG_NOTICE,"%s: RESOLUTION CHANGED TO %ix%i",this->name.c_str(),this->output_width,this->output_height);
        }
    }

}

//--------------------------------------------------------------
void OutputWindow::keyPressed(ofKeyEventArgs &e){
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
        warpController->onKeyPressed(e.key);
    }

    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(inletScriptType == 0){
            if(static_cast<LiveCoding *>(_inletParams[1])->hide){
                static_cast<LiveCoding *>(_inletParams[1])->lua.scriptKeyPressed(e.key);
            }else{
                static_cast<LiveCoding *>(_inletParams[1])->liveEditor.keyPressed(e.key);
            }
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::keyReleased(ofKeyEventArgs &e){
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated() && isFullscreen){
        warpController->onKeyReleased(e.key);
    }

    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(inletScriptType == 0){
            if(static_cast<LiveCoding *>(_inletParams[1])->lua.isValid()){
                static_cast<LiveCoding *>(_inletParams[1])->lua.scriptKeyReleased(e.key);
            }
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::mouseMoved(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated() && isFullscreen){
        warpController->onMouseMoved(window->events().getMouseX(),window->events().getMouseY());
    }

    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        ofVec2f tm = ofVec2f(((window->events().getMouseX()-posX)/drawW * this->output_width),((window->events().getMouseY()-posY)/drawH * this->output_height));
        if(inletScriptType == 0){
            if(static_cast<LiveCoding *>(_inletParams[1])->lua.isValid()){
                static_cast<LiveCoding *>(_inletParams[1])->lua.scriptMouseMoved(static_cast<int>(tm.x),static_cast<int>(tm.y));
            }
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::mouseDragged(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated() && isFullscreen){
        warpController->onMouseDragged(window->events().getMouseX(),window->events().getMouseY());
    }
    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        ofVec2f tm = ofVec2f(((window->events().getMouseX()-posX)/drawW * this->output_width),((window->events().getMouseY()-posY)/drawH * this->output_height));
        if(inletScriptType == 0){
            if(static_cast<LiveCoding *>(_inletParams[1])->lua.isValid()){
                static_cast<LiveCoding *>(_inletParams[1])->lua.scriptMouseDragged(static_cast<int>(tm.x),static_cast<int>(tm.y), e.button);
            }
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::mousePressed(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated() && isFullscreen){
        warpController->onMousePressed(window->events().getMouseX(),window->events().getMouseY());
    }
    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        ofVec2f tm = ofVec2f(((window->events().getMouseX()-posX)/drawW * this->output_width),((window->events().getMouseY()-posY)/drawH * this->output_height));
        if(inletScriptType == 0){
            if(static_cast<LiveCoding *>(_inletParams[1])->lua.isValid()){
                static_cast<LiveCoding *>(_inletParams[1])->lua.scriptMousePressed(static_cast<int>(tm.x),static_cast<int>(tm.y), e.button);
            }
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::mouseReleased(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated() && isFullscreen){
        warpController->onMouseReleased(window->events().getMouseX(),window->events().getMouseY());
    }
    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        ofVec2f tm = ofVec2f(((window->events().getMouseX()-posX)/drawW * this->output_width),((window->events().getMouseY()-posY)/drawH * this->output_height));
        if(inletScriptType == 0){
            if(static_cast<LiveCoding *>(_inletParams[1])->lua.isValid()){
                static_cast<LiveCoding *>(_inletParams[1])->lua.scriptMouseReleased(static_cast<int>(tm.x),static_cast<int>(tm.y), e.button);
            }
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::mouseScrolled(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        ofVec2f tm = ofVec2f(((window->events().getMouseX()-posX)/drawW * this->output_width),((window->events().getMouseY()-posY)/drawH * this->output_height));
        if(inletScriptType == 0){
            if(static_cast<LiveCoding *>(_inletParams[1])->lua.isValid()){
                static_cast<LiveCoding *>(_inletParams[1])->lua.scriptMouseScrolled(static_cast<int>(tm.x),static_cast<int>(tm.y), e.scrollX,e.scrollY);
            }
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::windowResized(ofResizeEventArgs &e){
    scaleTextureToWindow(window->getWidth(),window->getHeight());
    if(this->inletsConnected[2]){
        static_cast<ofxEditor *>(_inletParams[2])->resize(window->getWidth(),window->getHeight());
    }
}

//--------------------------------------------------------------
void OutputWindow::onToggleEvent(ofxDatGuiToggleEvent e){
    if(!header->getIsCollapsed()){
        if (e.target == useMapping){
            this->setCustomVar(static_cast<float>(e.checked),"USE_MAPPING");
        }
    }

}

//--------------------------------------------------------------
void OutputWindow::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if (e.target == applyButton){
            needReset = true;
        }else if(e.target == loadWarping){
            loadWarpingFlag = true;
        }else if(e.target == saveWarping){
            saveWarpingFlag = true;
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(!header->getIsCollapsed()){
        int tempInValue = ofToInt(e.text);
        if(e.target == guiTexWidth){
            if(tempInValue <= OUTPUT_TEX_MAX_WIDTH){
                temp_width = tempInValue;
            }else{
                temp_width = this->output_width;
            }
        }else if(e.target == guiTexHeight){
            if(tempInValue <= OUTPUT_TEX_MAX_HEIGHT){
                temp_height = tempInValue;
            }else{
                temp_height = this->output_height;
            }
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::onSliderEvent(ofxDatGuiSliderEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == edgesExponent){
            this->setCustomVar(static_cast<float>(e.value),"EDGES_EXPONENT");
        }else if(e.target == edgeL){
            this->setCustomVar(static_cast<float>(e.value),"EDGE_LEFT");
        }else if(e.target == edgeR){
            this->setCustomVar(static_cast<float>(e.value),"EDGE_RIGHT");
        }else if(e.target == edgeT){
            this->setCustomVar(static_cast<float>(e.value),"EDGE_TOP");
        }else if(e.target == edgeB){
            this->setCustomVar(static_cast<float>(e.value),"EDGE_BOTTOM");
        }

        if(isFullscreen && warpController->getNumWarps() > 0){
            warpController->getWarp(0)->setEdges(glm::vec4(this->getCustomVar("EDGE_LEFT"), this->getCustomVar("EDGE_TOP"), this->getCustomVar("EDGE_RIGHT"), this->getCustomVar("EDGE_BOTTOM")));
            warpController->getWarp(0)->setExponent(this->getCustomVar("EDGES_EXPONENT"));
        }

    }
}

OBJECT_REGISTER( OutputWindow, "output window", OFXVP_OBJECT_CAT_WINDOWING)
