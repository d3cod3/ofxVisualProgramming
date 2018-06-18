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

    for(int i=0;i<this->numInlets;i++){
        this->inletsConnected.push_back(false);
    }

    video = new ofVideoPlayer();

    isGUIObject         = true;
    isOverGui           = true;

    lastMessage         = "";

    isNewObject         = false;

    scaleH              = 0.0f;
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
void VideoPlayer::setupObjectContent(shared_ptr<ofAppBaseWindow> &mainWindow){
    if(filepath != "none"){
        loadVideoFile(filepath);
    }else{
        isNewObject = true;
    }

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width/3 * 2);
    gui->onButtonEvent(this, &VideoPlayer::onButtonEvent);

    loadButton = gui->addButton("OPEN");
    loadButton->setUseCustomMouse(true);

    gui->setPosition(this->width/3 + 1,this->height - loadButton->getHeight());

}

//--------------------------------------------------------------
void VideoPlayer::updateObjectContent(){
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
    loadButton->update();
}

//--------------------------------------------------------------
void VideoPlayer::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(video->isLoaded()){
        scaleH = (OBJECT_WIDTH/video->getWidth())*video->getHeight();
        static_cast<ofTexture *>(_outletParams[0])->draw(0,OBJECT_HEIGHT/2 - scaleH/2,OBJECT_WIDTH,scaleH);
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
    loadButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    isOverGui = loadButton->hitTest(_m-this->getPos());
}

//--------------------------------------------------------------
void VideoPlayer::dragGUIObject(ofVec3f _m){
    if(isOverGui){
        //gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        //loadButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }else{
        ofNotifyEvent(dragEvent, nId);

        box->setFromCenter(_m.x, _m.y,box->getWidth(),box->getHeight());
        headerBox->set(box->getPosition().x,box->getPosition().y,box->getWidth(),headerHeight);

        x = box->getPosition().x;
        y = box->getPosition().y;

        for(int j=0;j<outPut.size();j++){
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
    }else{
        filepath = "none";
    }

}

//--------------------------------------------------------------
void VideoPlayer::onButtonEvent(ofxDatGuiButtonEvent e){
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
