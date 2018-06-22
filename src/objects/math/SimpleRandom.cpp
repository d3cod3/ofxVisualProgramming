#include "SimpleRandom.h"

//--------------------------------------------------------------
SimpleRandom::SimpleRandom() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // min
    _inletParams[1] = new float();  // max
    *(float *)&_inletParams[0] = 0.0f;
    *(float *)&_inletParams[1] = 1.0f;

    _outletParams[0] = new float(); // output
    *(float *)&_outletParams[0] = 0.0f;

    for(int i=0;i<this->numInlets;i++){
        this->inletsConnected.push_back(false);
    }

    changeRange = false;
    lastMinRange = *(float *)&_inletParams[0];
    lastMaxRange = *(float *)&_inletParams[1];
}

//--------------------------------------------------------------
void SimpleRandom::newObject(){
    this->setName("simple random");
    this->addInlet(VP_LINK_NUMERIC,"min");
    this->addInlet(VP_LINK_NUMERIC,"max");
    this->addOutlet(VP_LINK_NUMERIC);
}

//--------------------------------------------------------------
void SimpleRandom::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    ofSeedRandom(ofGetElapsedTimeMillis());

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setWidth(this->width);

    rPlotter = gui->addValuePlotter("",*(float *)&_inletParams[0],*(float *)&_inletParams[1]);
    rPlotter->setDrawMode(ofxDatGuiGraph::LINES);
    rPlotter->setSpeed(1);

    gui->setPosition(0,this->height-rPlotter->getHeight());
}

//--------------------------------------------------------------
void SimpleRandom::updateObjectContent(){
    *(float *)&_outletParams[0] = ofRandom(*(float *)&_inletParams[0],*(float *)&_inletParams[1]);

    gui->update();
    rPlotter->setValue(*(float *)&_outletParams[0]);
    if(this->inletsConnected[0] || this->inletsConnected[1]){
        if(lastMinRange != *(float *)&_inletParams[0] || lastMaxRange != *(float *)&_inletParams[1]){
            changeRange = true;
        }
        if(changeRange){
            changeRange = false;
            rPlotter->setRange(*(float *)&_inletParams[0],*(float *)&_inletParams[1]);
        }
    }
}

//--------------------------------------------------------------
void SimpleRandom::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    font->draw(ofToString(*(float *)&_outletParams[0]),this->fontSize,this->width/2,this->headerHeight*2.3);
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void SimpleRandom::removeObjectContent(){
    
}
