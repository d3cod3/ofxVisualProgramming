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
ProjectionMapping::ProjectionMapping() : PatchObject(){

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

    isGUIObject         = true;
    this->isOverGUI     = true;

    needReset           = false;

    lastWarpingConfig   = "";
    loadWarpingFlag     = false;
    saveWarpingFlag     = false;
    warpingConfigLoaded = false;

    isOverWindow        = false;
    winMouseX           = 0;
    winMouseY           = 0;

    autoRemove          = false;
}

//--------------------------------------------------------------
void ProjectionMapping::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_TEXTURE,"source");
    this->addInlet(VP_LINK_TEXTURE,"background");
    this->addOutlet(VP_LINK_TEXTURE,"mappingOutput");
}

//--------------------------------------------------------------
void ProjectionMapping::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

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

    // GUI
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onButtonEvent(this, &ProjectionMapping::onButtonEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
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
void ProjectionMapping::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    loadWarping->update();
    saveWarping->update();

    if(loadWarpingFlag){
        loadWarpingFlag = false;
        fd.openFile("open mapping config"+ofToString(this->getId()),"Select a mapping config file");
    }

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

    if(saveWarpingFlag){
        saveWarpingFlag = false;
        fd.saveFile("save mapping config"+ofToString(this->getId()),"Save mapping settings as","mappingSettings.xml");
    }

    // reset mapping textures resolution on inlet connection
    if(this->inletsConnected[0]){
        if(static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
            if(!needReset){
                needReset = true;
                resetResolution();
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
void ProjectionMapping::drawObjectContent(ofxFontStash *font){
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
void ProjectionMapping::removeObjectContent(bool removeFileFromData){
    if(window->getGLFWWindow() != nullptr){
        window->setWindowShouldClose();
    }
}

//--------------------------------------------------------------
void ProjectionMapping::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    loadWarping->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    saveWarping->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || loadWarping->hitTest(_m-this->getPos()) || saveWarping->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void ProjectionMapping::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void ProjectionMapping::fileDialogResponse(ofxThreadedFileDialogResponse &response){
    if(response.id == "open mapping config"+ofToString(this->getId())){
        warpingConfigLoaded = true;
        lastWarpingConfig = response.filepath;
    }else if(response.id == "save mapping config"+ofToString(this->getId())){
        ofFile newFile (response.filepath);
        filepath = newFile.getAbsolutePath();
        _mapping->saveMappingAs(filepath);
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
void ProjectionMapping::resetResolution(){

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
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){

    }

    if(isOverWindow){
        _mapping->mouseDragged(e);
    }

}

//--------------------------------------------------------------
void ProjectionMapping::mousePressed(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        //warpController->onMousePressed(window->events().getMouseX(),window->events().getMouseY());

    }

    if(isOverWindow){
        _mapping->mousePressed(e);
    }

}

//--------------------------------------------------------------
void ProjectionMapping::mouseReleased(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){

    }

    if(isOverWindow){
        _mapping->mouseReleased(e);
    }

}

//--------------------------------------------------------------
void ProjectionMapping::mouseScrolled(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){

    }

    if(isOverWindow){
        _mapping->mouseScrolled(e);
    }
}

//--------------------------------------------------------------
void ProjectionMapping::windowResized(ofResizeEventArgs &e){
    _mapping->windowResized(e);
}

//--------------------------------------------------------------
void ProjectionMapping::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == loadWarping){
            loadWarpingFlag = true;
        }else if(e.target == saveWarping){
            saveWarpingFlag = true;
        }
    }
}

OBJECT_REGISTER( ProjectionMapping, "projection mapping", OFXVP_OBJECT_CAT_WINDOWING);

#endif
