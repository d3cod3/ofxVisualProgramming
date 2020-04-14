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

#include "Metronome.h"

//--------------------------------------------------------------
Metronome::Metronome() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new float(); // time
    *(float *)&_inletParams[0] = 0.0f;

    _outletParams[0] = new float(); // output
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    wait = 1000;
    resetTime = ofGetElapsedTimeMillis();
    metroTime = ofGetElapsedTimeMillis();

    sync                = false;
    loaded              = false;

}

//--------------------------------------------------------------
void Metronome::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"time");
    this->addInlet(VP_LINK_NUMERIC,"sync");
    this->addOutlet(VP_LINK_NUMERIC,"bang");

    this->setCustomVar(static_cast<float>(wait),"TIME");
}

//--------------------------------------------------------------
void Metronome::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setWidth(this->width);
    gui->addBreak();
    gui->onTextInputEvent(this, &Metronome::onTextInputEvent);

    timeSetting = gui->addTextInput(" ","1000");
    timeSetting->setUseCustomMouse(true);
    timeSetting->setText(ofToString(static_cast<int>(floor(this->getCustomVar("TIME")))));
    gui->addBreak();
    rPlotter = gui->addValuePlotter("",0.0,1.0);
    rPlotter->setDrawMode(ofxDatGuiGraph::LINES);
    rPlotter->setSpeed(1);

    gui->setPosition(0,this->headerHeight);

    wait = ofToInt(timeSetting->getText());
}

//--------------------------------------------------------------
void Metronome::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    metroTime = ofGetElapsedTimeMillis();

    gui->update();
    timeSetting->update();

    if(this->inletsConnected[0] && static_cast<size_t>(floor(*(float *)&_inletParams[0])) != wait){
        wait = static_cast<size_t>(floor(*(float *)&_inletParams[0]));
        timeSetting->setText(ofToString(wait));
        this->setCustomVar(static_cast<float>(wait),"TIME");
    }

    if(this->inletsConnected[1]){
        sync = static_cast<bool>(floor(*(float *)&_inletParams[1]));
    }

    if(sync){
        resetTime = ofGetElapsedTimeMillis();
    }

    if(metroTime-resetTime > wait){
        resetTime = ofGetElapsedTimeMillis();
        *(float *)&_outletParams[0] = 1.0f;
    }else{
        *(float *)&_outletParams[0] = 0.0f;
    }

    rPlotter->setValue(*(float *)&_outletParams[0]);

    if(!loaded){
        loaded = true;
        timeSetting->setText(ofToString(static_cast<int>(floor(this->getCustomVar("TIME")))));
    }
}

//--------------------------------------------------------------
void Metronome::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void Metronome::removeObjectContent(bool removeFileFromData){
    
}

//--------------------------------------------------------------
void Metronome::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    timeSetting->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    this->isOverGUI = timeSetting->hitTest(_m-this->getPos());
}

//--------------------------------------------------------------
void Metronome::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        timeSetting->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }else{
        

        box->setFromCenter(_m.x, _m.y,box->getWidth(),box->getHeight());
        headerBox->set(box->getPosition().x,box->getPosition().y,box->getWidth(),headerHeight);

        x = box->getPosition().x;
        y = box->getPosition().y;

        for(int j=0;j<static_cast<int>(outPut.size());j++){
            // (outPut[j]->posFrom.x,outPut[j]->posFrom.y);
            // (outPut[j]->posFrom.x+20,outPut[j]->posFrom.y);
        }
    }
}

//--------------------------------------------------------------
void Metronome::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(e.target == timeSetting){
        if(isInteger(e.text)){
            this->setCustomVar(static_cast<float>(ofToInt(e.text)),"TIME");
            wait = ofToInt(e.text);
        }else{
            timeSetting->setText(ofToString(wait));
        }

    }
}

OBJECT_REGISTER( Metronome, "metronome", OFXVP_OBJECT_CAT_MATH)

#endif