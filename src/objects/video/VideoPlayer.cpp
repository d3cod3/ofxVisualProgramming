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
    *(string *)&_inletParams[0] = "";
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

    posX = posY = drawW = drawH = 0.0f;
}

//--------------------------------------------------------------
void VideoPlayer::newObject(){
    this->setName("video player");
    this->addInlet(VP_LINK_STRING,"control");
    this->addInlet(VP_LINK_NUMERIC,"playhead");
    this->addInlet(VP_LINK_NUMERIC,"speed");
    this->addInlet(VP_LINK_NUMERIC,"volume");
    this->addOutlet(VP_LINK_TEXTURE);
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
    gui->addBreak();
    loadButton = gui->addButton("OPEN");
    loadButton->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    if(filepath != "none"){
        loadVideoFile(filepath);
    }else{
        isNewObject = true;
    }
}

//--------------------------------------------------------------
void VideoPlayer::updateObjectContent(map<int,PatchObject*> &patchObjects){
    if(video->isLoaded()){
        video->update();
        *static_cast<ofTexture *>(_outletParams[0]) = video->getTexture();

        // listen to message control (_inletParams[0])
        if(this->inletsConnected[0]){
            if(lastMessage != *(string *)&_inletParams[0]){
                lastMessage = *(string *)&_inletParams[0];

                if(lastMessage == "play"){
                    video->play();
                }else if(lastMessage == "pause"){
                    video->setPaused(true);
                }else if(lastMessage == "unpause"){
                    video->setPaused(false);
                }else if(lastMessage == "stop"){
                    video->stop();
                }
            }
        }
        // playhead
        if(this->inletsConnected[1]){
            video->setPosition(*(float *)&_inletParams[1]);
        }
        // speed
        if(this->inletsConnected[2]){
            video->setSpeed(*(float *)&_inletParams[2]);
        }
        // volume
        if(this->inletsConnected[3]){
            video->setVolume(*(float *)&_inletParams[3]);
        }
    }

    gui->update();
    header->update();
    loadButton->update();
}

//--------------------------------------------------------------
void VideoPlayer::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(video->isLoaded()){
        //scaleH = (this->width/video->getWidth())*video->getHeight();
        if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
            if(static_cast<ofTexture *>(_outletParams[0])->getWidth() >= static_cast<ofTexture *>(_outletParams[0])->getHeight()){   // horizontal texture
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
            static_cast<ofTexture *>(_outletParams[0])->draw(posX,posY,drawW,drawH);
        }
    }else if(!isNewObject){
        ofSetColor(255,0,0);
        ofDrawRectangle(0,0,this->width,this->height);
        ofSetColor(255);
        font->draw("FILE NOT FOUND!",this->fontSize,this->width/3 + 4,this->headerHeight*2.3);
    }
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void VideoPlayer::removeObjectContent(){

}

//--------------------------------------------------------------
void VideoPlayer::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    loadButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || loadButton->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void VideoPlayer::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void VideoPlayer::loadVideoFile(string videofile){
    filepath = videofile;

    video->load(filepath);

    if(video->isLoaded()){
        static_cast<ofTexture *>(_outletParams[0])->allocate(video->getPixels());

        // TESTING
        video->setLoopState(OF_LOOP_NORMAL);
        video->play();

        ofFile tempFile(filepath);
        if(tempFile.getFileName().size() > 22){
            videoName->setLabel(tempFile.getFileName().substr(0,21)+"...");
        }else{
            videoName->setLabel(tempFile.getFileName());
        }
    }else{
        filepath = "none";
    }

}

//--------------------------------------------------------------
void VideoPlayer::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if (e.target == loadButton){
            ofFileDialogResult openFileResult= ofSystemLoadDialog("Select a video file");
            if (openFileResult.bSuccess){
                ofFile file (openFileResult.getPath());
                if (file.exists()){
                    string fileExtension = ofToUpper(file.getExtension());
                    if(fileExtension == "MOV" || fileExtension == "MP4" || fileExtension == "MPEG" || fileExtension == "MPG" || fileExtension == "AVI") {
                        loadVideoFile(file.getAbsolutePath());
                    }
                }
            }
        }
    }
}
