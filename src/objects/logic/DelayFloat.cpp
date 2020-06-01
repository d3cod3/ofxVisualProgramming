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

#include "DelayFloat.h"

//--------------------------------------------------------------
DelayFloat::DelayFloat() : PatchObject(){

    this->numInlets  = 3;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;

    _inletParams[1] = new float();  // number
    *(float *)&_inletParams[1] = 0.0f;

    _inletParams[2] = new float();  // ms
    *(float *)&_inletParams[2] = 1000.0f;

    _outletParams[0] = new float(); // output numeric
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    bang                = false;
    delayBang           = false;
    number              = 0.0f;

    loadStart           = false;
    loaded              = false;

    wait                = static_cast<size_t>(floor(*(float *)&_inletParams[2]));
    startTime           = ofGetElapsedTimeMillis();

}

//--------------------------------------------------------------
void DelayFloat::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"number");
    this->addInlet(VP_LINK_NUMERIC,"ms");
    this->addOutlet(VP_LINK_NUMERIC,"number");

    this->setCustomVar(0,"NUMBER");
    this->setCustomVar(static_cast<float>(wait),"MS");
}

//--------------------------------------------------------------
void DelayFloat::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setWidth(this->width);
    gui->onTextInputEvent(this, &DelayFloat::onTextInputEvent);

    numberBox = gui->addTextInput("","0");
    numberBox->setUseCustomMouse(true);
    numberBox->setText(ofToString(this->getCustomVar("NUMBER")));

    number = this->getCustomVar("NUMBER");

    inputNumber = gui->addTextInput("","1000");
    inputNumber->setUseCustomMouse(true);
    inputNumber->setText(ofToString(this->getCustomVar("MS")));

    wait = this->getCustomVar("MS");

    gui->setPosition(0,this->headerHeight + inputNumber->getHeight() + 6);
}

//--------------------------------------------------------------
void DelayFloat::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    gui->update();
    numberBox->update();
    inputNumber->update();

    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] == 1.0 && !bang){
            bang        = true;
            loadStart   = false;
            startTime   = ofGetElapsedTimeMillis();
        }
    }

    if(this->inletsConnected[1]){
      number = *(float *)&_inletParams[1];
      numberBox->setText(ofToString(number));
    }

    if(this->inletsConnected[2]){
      wait                = static_cast<size_t>(floor(*(float *)&_inletParams[2]));
      inputNumber->setText(ofToString(wait));
    }

    if(!loadStart && (ofGetElapsedTimeMillis()-startTime > wait)){
        bang        = false;
        loadStart   = true;
        delayBang   = true;
    }else{
        delayBang   = false;
    }

    if(delayBang && this->inletsConnected[1]){
        *(float *)&_outletParams[0] = *(float *)&_inletParams[1];
    }else if(delayBang && !this->inletsConnected[1]){
        *(float *)&_outletParams[0] = static_cast<float>(ofToFloat(numberBox->getText()));
    }

    if(!loaded){
        loaded = true;
        numberBox->setText(ofToString(this->getCustomVar("NUMBER")));
        inputNumber->setText(ofToString(this->getCustomVar("MS")));
    }

}

//--------------------------------------------------------------
void DelayFloat::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(bang){
        ofSetColor(250,250,5);
        ofDrawRectangle(0,0,this->width,this->height);
    }
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void DelayFloat::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void DelayFloat::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    inputNumber->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    numberBox->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    this->isOverGUI = inputNumber->hitTest(_m-this->getPos()) || numberBox->hitTest(_m-this->getPos());
}

//--------------------------------------------------------------
void DelayFloat::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        inputNumber->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        numberBox->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void DelayFloat::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(e.target == inputNumber){
        if(isInteger(e.text) || isFloat(e.text)){
            this->setCustomVar(static_cast<float>(ofToInt(e.text)),"MS");
            wait        = ofToInt(e.text);
        }else{
            inputNumber->setText(ofToString(wait));
        }

    }else if(e.target == numberBox){
        if(isInteger(e.text) || isFloat(e.text)){
            this->setCustomVar(static_cast<float>(ofToFloat(e.text)),"NUMBER");
            number = ofToFloat(e.text);
        }
    }
}

OBJECT_REGISTER( DelayFloat, "delay float", OFXVP_OBJECT_CAT_LOGIC)

#endif