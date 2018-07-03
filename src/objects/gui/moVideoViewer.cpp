#include "moVideoViewer.h"

//--------------------------------------------------------------
moVideoViewer::moVideoViewer() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 0;

    _inletParams[0] = new ofTexture();  // texture

    for(int i=0;i<this->numInlets;i++){
        this->inletsConnected.push_back(false);
    }

    scaleH                  = 0.0f;

    this->isBigGuiViewer    = true;
    this->width             *= 2;
    this->height            *= 2;
}

//--------------------------------------------------------------
void moVideoViewer::newObject(){
    this->setName("video viewer");
    this->addInlet(VP_LINK_TEXTURE,"texture");
}

//--------------------------------------------------------------
void moVideoViewer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    
}

//--------------------------------------------------------------
void moVideoViewer::updateObjectContent(map<int,PatchObject*> &patchObjects){


}

//--------------------------------------------------------------
void moVideoViewer::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(this->inletsConnected[0]){
        scaleH = (this->width/static_cast<ofTexture *>(_inletParams[0])->getWidth())*static_cast<ofTexture *>(_inletParams[0])->getHeight();
        static_cast<ofTexture *>(_inletParams[0])->draw(0,this->height/2 - scaleH/2,this->width,scaleH);
    }
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void moVideoViewer::removeObjectContent(){
    
}
