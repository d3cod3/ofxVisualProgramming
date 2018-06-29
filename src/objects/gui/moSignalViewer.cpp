#include "moSignalViewer.h"

//--------------------------------------------------------------
moSignalViewer::moSignalViewer() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 0;

    _inletParams[0] = new ofSoundBuffer();  // signal

    for(int i=0;i<this->numInlets;i++){
        this->inletsConnected.push_back(false);
    }

    this->isBigGuiViewer    = true;
    this->width             *= 2;
}

//--------------------------------------------------------------
void moSignalViewer::newObject(){
    this->setName("signal viewer");
    this->addInlet(VP_LINK_AUDIO,"signal");
}

//--------------------------------------------------------------
void moSignalViewer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    
}

//--------------------------------------------------------------
void moSignalViewer::updateObjectContent(){
    if(this->inletsConnected[0]){
        waveform.clear();
        for(size_t i = 0; i < static_cast<ofSoundBuffer *>(_inletParams[0])->getNumFrames(); i++) {
            float sample = static_cast<ofSoundBuffer *>(_inletParams[0])->getSample(i,0);
            float x = ofMap(i, 0, static_cast<ofSoundBuffer *>(_inletParams[0])->getNumFrames(), 0, this->width);
            float y = ofMap(hardClip(sample), -1, 1, 0, this->height);
            waveform.addVertex(x, y);
        }
    }

}

//--------------------------------------------------------------
void moSignalViewer::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    waveform.draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void moSignalViewer::removeObjectContent(){
    
}
