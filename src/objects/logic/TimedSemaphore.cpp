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

#include "TimedSemaphore.h"

//--------------------------------------------------------------
TimedSemaphore::TimedSemaphore() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;

    _inletParams[1] = new float();  // ms
    *(float *)&_inletParams[1] = 1000.0f;

    _outletParams[0] = new float(); // output numeric
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    bang                = false;

    loadStart           = true;

    wait                = 1000;
    startTime           = ofGetElapsedTimeMillis();

}

//--------------------------------------------------------------
void TimedSemaphore::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"ms");
    this->addOutlet(VP_LINK_NUMERIC,"bang");

    this->setCustomVar(static_cast<float>(wait),"MS");
}

//--------------------------------------------------------------
void TimedSemaphore::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setWidth(this->width);
    gui->addBreak();
    gui->onTextInputEvent(this, &TimedSemaphore::onTextInputEvent);

    inputNumber = gui->addTextInput("","1000");
    inputNumber->setUseCustomMouse(true);
    inputNumber->setText(ofToString(this->getCustomVar("MS")));

    wait = this->getCustomVar("MS");

    gui->setPosition(0,this->headerHeight);
}

//--------------------------------------------------------------
void TimedSemaphore::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){
    if(this->inletsConnected[1]){
      wait                = static_cast<size_t>(floor(*(float *)&_inletParams[1]));
      inputNumber->setText(ofToString(wait));
    }

    gui->update();
    inputNumber->update();

    if(this->inletsConnected[0] && loadStart){
        if(*(float *)&_inletParams[0] == 1.0 && !bang){
            bang        = true;
            loadStart   = false;
            startTime   = ofGetElapsedTimeMillis();
        }
    }else{
      bang        = false;
    }

    if(!loadStart && (ofGetElapsedTimeMillis()-startTime > wait)){
        loadStart   = true;
    }
    
    *(float *)&_outletParams[0] = static_cast<float>(bang);

}

//--------------------------------------------------------------
void TimedSemaphore::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofSetCircleResolution(50);
    ofEnableAlphaBlending();
    if(bang){
        ofSetColor(250,250,5);
        ofDrawRectangle(0,0,this->width,this->height);
    }
    if(loadStart){
        ofSetColor(5,250,5);
        ofDrawCircle(this->width-20,this->height-20,10);
    }else{
        ofSetColor(250,5,5);
        ofDrawCircle(this->width-20,this->height-20,10);
    }
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void TimedSemaphore::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void TimedSemaphore::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    inputNumber->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    this->isOverGUI = inputNumber->hitTest(_m-this->getPos());
}

//--------------------------------------------------------------
void TimedSemaphore::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        inputNumber->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }else{
        ofNotifyEvent(dragEvent, nId);

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
void TimedSemaphore::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(e.target == inputNumber){
        if(isInteger(e.text) || isFloat(e.text)){
            this->setCustomVar(static_cast<float>(ofToInt(e.text)),"MS");
            wait        = ofToInt(e.text);
        }else{
            inputNumber->setText(ofToString(wait));
        }

    }
}

OBJECT_REGISTER( TimedSemaphore, "timed semaphore", OFXVP_OBJECT_CAT_LOGIC)

#endif