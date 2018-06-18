#include "SimpleNoise.h"

//--------------------------------------------------------------
SimpleNoise::SimpleNoise() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // step
    *(float *)&_inletParams[0] = 0.001f;

    _outletParams[0] = new float(); // output
    *(float *)&_outletParams[0] = 0.0f;

    for(int i=0;i<this->numInlets;i++){
        this->inletsConnected.push_back(false);
    }

    timePosition = ofRandom(10000);
}

//--------------------------------------------------------------
void SimpleNoise::newObject(){
    this->setName("simple noise");
    this->addInlet(VP_LINK_NUMERIC,"step");
    this->addOutlet(VP_LINK_NUMERIC);
}

//--------------------------------------------------------------
void SimpleNoise::setupObjectContent(shared_ptr<ofAppBaseWindow> &mainWindow){

}

//--------------------------------------------------------------
void SimpleNoise::updateObjectContent(){
    *(float *)&_outletParams[0] = ofNoise(timePosition);

    timePosition += *(float *)&_inletParams[0];
}

//--------------------------------------------------------------
void SimpleNoise::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    font->draw(ofToString(*(float *)&_outletParams[0]),this->fontSize,this->width/2,this->headerHeight*2.3);
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void SimpleNoise::removeObjectContent(){
    
}
