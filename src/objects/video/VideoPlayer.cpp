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

#include "VideoPlayer.h"

//--------------------------------------------------------------
VideoPlayer::VideoPlayer() : PatchObject(){

    this->numInlets  = 4;
    this->numOutlets = 1;

    _inletParams[0] = new string();  // control
    *static_cast<string *>(_inletParams[0]) = "";
    _inletParams[1] = new float();  // playhead
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();  // speed
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();  // volume
    *(float *)&_inletParams[3] = 0.0f;

    _outletParams[0] = new ofTexture(); // output

    this->initInletsState();

    video = new ofVideoPlayer();

    isGUIObject         = true;
    this->isOverGUI     = true;

    lastMessage         = "";

    isNewObject         = false;
    isFileLoaded        = false;
    nameLabelLoaded     = false;

    posX = posY = drawW = drawH = 0.0f;

    threadLoaded    = false;
    needToLoadVideo = true;

    lastPlayhead    = 0.0f;

    loadVideoFlag   = false;

    videoWasPlaying = false;

}

//--------------------------------------------------------------
void VideoPlayer::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_STRING,"control");
    this->addInlet(VP_LINK_NUMERIC,"playhead");
    this->addInlet(VP_LINK_NUMERIC,"speed");
    this->addInlet(VP_LINK_NUMERIC,"volume");
    this->addOutlet(VP_LINK_TEXTURE,"output");
}

//--------------------------------------------------------------
void VideoPlayer::autoloadFile(string _fp){
    //this->filepath = _fp;
    this->filepath = copyFileToPatchFolder(this->patchFolderPath,_fp);
    reloadVideoThreaded();
}

//--------------------------------------------------------------
void VideoPlayer::threadedFunction(){
    while(isThreadRunning()){
        std::unique_lock<std::mutex> lock(mutex);
        if(needToLoadVideo){
            needToLoadVideo = false;
            loadVideoFile();
            threadLoaded = true;
            nameLabelLoaded = true;
        }
        condition.wait(lock);
        sleep(10);
    }
}

//--------------------------------------------------------------
void VideoPlayer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onButtonEvent(this, &VideoPlayer::onButtonEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    videoName = gui->addLabel("NONE");
    videoRes = gui->addLabel("0x0");
    gui->addBreak();
    loadButton = gui->addButton("OPEN");
    loadButton->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    if(filepath == "none"){
        isNewObject = true;
    }

    if(!isThreadRunning()){
        startThread(true);
    }
}

//--------------------------------------------------------------
void VideoPlayer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    loadButton->update();

    if(nameLabelLoaded && threadLoaded){
        nameLabelLoaded = false;
        ofFile tempFile(filepath);
        if(tempFile.getFileName().size() > 22){
            videoName->setLabel(tempFile.getFileName().substr(0,21)+"...");
        }else{
            videoName->setLabel(tempFile.getFileName());
        }
        videoRes->setLabel(ofToString(video->getWidth())+"x"+ofToString(video->getHeight()));
    }

    if(loadVideoFlag){
        loadVideoFlag = false;
        fd.openFile("load videofile"+ofToString(this->getId()),"Select a video file");
    }

}

//--------------------------------------------------------------
void VideoPlayer::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();

    if(!isFileLoaded && video->isLoaded() && video->isInitialized() && threadLoaded){
        if(video->isInitialized()){
            video->setLoopState(OF_LOOP_NONE);
            video->stop();

            ofLog(OF_LOG_NOTICE,"[verbose] video file loaded: %s",filepath.c_str());
            isFileLoaded = true;
        }else{
            if(!isNewObject){
                ofLog(OF_LOG_ERROR,"video file: %s NOT FOUND!",filepath.c_str());
            }
            filepath = "none";
        }
    }

    if(isFileLoaded && video->isLoaded() && threadLoaded){

        if(video->getWidth() != static_cast<ofTexture *>(_outletParams[0])->getWidth() || video->getHeight() != static_cast<ofTexture *>(_outletParams[0])->getHeight()){
            _outletParams[0] = new ofTexture();
            static_cast<ofTexture *>(_outletParams[0])->allocate(video->getWidth(),video->getHeight(),GL_RGB);
            static_cast<ofTexture *>(_outletParams[0])->clear();
        }else{
            static_cast<ofTexture *>(_outletParams[0])->loadData(video->getPixels());
        }

        // listen to message control (_inletParams[0])
        if(this->inletsConnected[0]){
            if(lastMessage != *static_cast<string *>(_inletParams[0])){
                lastMessage = *static_cast<string *>(_inletParams[0]);

                if(lastMessage == "play"){
                    video->firstFrame();
                    video->play();
                    videoWasPlaying = true;
                }else if(lastMessage == "pause"){
                    video->setPaused(true);
                }else if(lastMessage == "unpause"){
                    video->setPaused(false);
                    if(!videoWasPlaying){
                        video->stop();
                    }
                }else if(lastMessage == "stop"){
                    video->stop();
                    videoWasPlaying = false;
                }else if(lastMessage == "loop_normal"){
                    video->setLoopState(OF_LOOP_NORMAL);
                }else if(lastMessage == "loop_none"){
                    video->setLoopState(OF_LOOP_NONE);
                }

                //ofLog(OF_LOG_NOTICE,"%s",lastMessage.c_str());
            }
        }
        // playhead
        if(this->inletsConnected[1] && *(float *)&_inletParams[1] != -1.0f && *(float *)&_inletParams[1] != lastPlayhead){
            video->setPosition(*(float *)&_inletParams[1]);
            lastPlayhead = *(float *)&_inletParams[1];
        }
        // speed
        if(this->inletsConnected[2]){
            video->setSpeed(*(float *)&_inletParams[2]);
        }
        // volume
        if(this->inletsConnected[3]){
            video->setVolume(*(float *)&_inletParams[3]);
        }

        //scaleH = (this->width/video->getWidth())*video->getHeight();
        if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
            if(video->isPlaying()){ // play
               video->update();

               if(static_cast<ofTexture *>(_outletParams[0])->getWidth()/static_cast<ofTexture *>(_outletParams[0])->getHeight() >= this->width/this->height){
                   if(static_cast<ofTexture *>(_outletParams[0])->getWidth() > static_cast<ofTexture *>(_outletParams[0])->getHeight()){   // horizontal texture
                       drawW           = this->width;
                       drawH           = (this->width/static_cast<ofTexture *>(_outletParams[0])->getWidth())*static_cast<ofTexture *>(_outletParams[0])->getHeight();
                       posX            = 0;
                       posY            = (this->height-drawH)/2.0f;
                   }else{ // vertical texture
                       drawW           = (static_cast<ofTexture *>(_outletParams[0])->getWidth()*this->height)/static_cast<ofTexture *>(_outletParams[0])->getHeight();
                       drawH           = this->height;
                       posX            = (this->width-drawW)/2.0f;
                       posY            = 0;
                   }
               }else{ // always considered vertical texture
                   drawW           = (static_cast<ofTexture *>(_outletParams[0])->getWidth()*this->height)/static_cast<ofTexture *>(_outletParams[0])->getHeight();
                   drawH           = this->height;
                   posX            = (this->width-drawW)/2.0f;
                   posY            = 0;
               }

               static_cast<ofTexture *>(_outletParams[0])->draw(posX,posY,drawW,drawH);
            }else if(video->isPaused() && video->getCurrentFrame() > 1){ // pause
                if(static_cast<ofTexture *>(_outletParams[0])->getWidth()/static_cast<ofTexture *>(_outletParams[0])->getHeight() >= this->width/this->height){
                    if(static_cast<ofTexture *>(_outletParams[0])->getWidth() > static_cast<ofTexture *>(_outletParams[0])->getHeight()){   // horizontal texture
                        drawW           = this->width;
                        drawH           = (this->width/static_cast<ofTexture *>(_outletParams[0])->getWidth())*static_cast<ofTexture *>(_outletParams[0])->getHeight();
                        posX            = 0;
                        posY            = (this->height-drawH)/2.0f;
                    }else{ // vertical texture
                        drawW           = (static_cast<ofTexture *>(_outletParams[0])->getWidth()*this->height)/static_cast<ofTexture *>(_outletParams[0])->getHeight();
                        drawH           = this->height;
                        posX            = (this->width-drawW)/2.0f;
                        posY            = 0;
                    }
                }else{ // always considered vertical texture
                    drawW           = (static_cast<ofTexture *>(_outletParams[0])->getWidth()*this->height)/static_cast<ofTexture *>(_outletParams[0])->getHeight();
                    drawH           = this->height;
                    posX            = (this->width-drawW)/2.0f;
                    posY            = 0;
                }

                static_cast<ofTexture *>(_outletParams[0])->draw(posX,posY,drawW,drawH);
            }else{
                ofSetColor(0);
                ofDrawRectangle(0,0,this->width, this->height);
            }

            // draw player state
            ofSetColor(255,60);
            if(video->isPlaying()){ // play
                ofBeginShape();
                ofVertex(this->width - 30,this->height - 50);
                ofVertex(this->width - 30,this->height - 30);
                ofVertex(this->width - 10,this->height - 40);
                ofEndShape();
            }else if(video->isPaused() && video->getCurrentFrame() > 1){ // pause
                ofDrawRectangle(this->width - 30, this->height - 50,8,20);
                ofDrawRectangle(this->width - 18, this->height - 50,8,20);
            }else if(video->getCurrentFrame() <= 1){ // stop
                ofDrawRectangle(this->width - 30, this->height - 50,20,20);
            }

            ofSetColor(255);
            ofSetLineWidth(2);
            float phx = ofMap( video->getPosition(), 0, 1, 1, drawW-1 );
            if(phx >= 0.0f){
                ofDrawLine( phx, posY+2, phx, drawH+posY);
            }else{
                ofDrawLine( 2, posY+2, 2, drawH+posY);
            }

        }
    }else if(!isNewObject){
        ofSetColor(255,0,0);
        ofDrawRectangle(0,0,this->width,this->height);
        ofSetColor(255);
        font->draw("FILE NOT FOUND!",this->fontSize,this->width/3 + 4,this->headerHeight*2.3);
    }

    ofSetColor(255);
    font->draw(videoTimecode.timecodeForFrame(video->getCurrentFrame()),static_cast<int>(floor(this->fontSize*1.4)),this->width/3 + 8,this->headerHeight*2.3);
    font->draw(videoTimecode.timecodeForFrame(video->getTotalNumFrames()),static_cast<int>(floor(this->fontSize*1.4)),this->width/3 + 8,this->headerHeight*3.3);

    gui->draw();
    ofDisableAlphaBlending();

    ///////////////////////////////////////////
    condition.notify_one();
}

//--------------------------------------------------------------
void VideoPlayer::removeObjectContent(bool removeFileFromData){
    if(isThreadRunning()){
        video->stop();
        video->setVolume(0);
        video->close();
        stopThread();
    }
    if(removeFileFromData){
        removeFile(filepath);
    }
}

//--------------------------------------------------------------
void VideoPlayer::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    videoName->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    videoRes->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    loadButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || videoName->hitTest(_m-this->getPos()) || videoRes->hitTest(_m-this->getPos()) || loadButton->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void VideoPlayer::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        videoName->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        videoRes->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        loadButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void VideoPlayer::fileDialogResponse(ofxThreadedFileDialogResponse &response){
    if(response.id == "load videofile"+ofToString(this->getId())){
        ofFile file (response.filepath);
        if (file.exists()){
            string fileExtension = ofToUpper(file.getExtension());
            if(fileExtension == "MOV" || fileExtension == "MP4" || fileExtension == "MPEG" || fileExtension == "MPG" || fileExtension == "AVI") {
                filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
                //filepath = file.getAbsolutePath();
                reloadVideoThreaded();
            }
        }
    }
}

//--------------------------------------------------------------
void VideoPlayer::loadVideoFile(){
    if(filepath != "none"){
        filepath = forceCheckMosaicDataPath(filepath);
        //filepath = copyFileToPatchFolder(this->patchFolderPath,filepath);
        isNewObject = false;
        video->setUseTexture(false);
        video->load(filepath);
        videoTimecode.setFPS(static_cast<int>(ceil(video->getTotalNumFrames()/video->getDuration()))); // fps
        this->saveConfig(false,this->nId);
    }
}

//--------------------------------------------------------------
void VideoPlayer::reloadVideoThreaded(){
    isFileLoaded = false;
    needToLoadVideo = true;
}

//--------------------------------------------------------------
void VideoPlayer::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if (e.target == loadButton){
            loadVideoFlag = true;
        }
    }
}

OBJECT_REGISTER( VideoPlayer, "video player", OFXVP_OBJECT_CAT_VIDEO)
