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

#include "Range.h"

//--------------------------------------------------------------
Range::Range() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 2;

    _inletParams[0] = new float();  // input 1
    *(float *)&_inletParams[0] = 0.0f;
    _inletParams[1] = new float();  // input 2
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new float(); // output 1
    *(float *)&_outletParams[0] = 0.0f;
    _outletParams[1] = new float(); // output 2
    *(float *)&_outletParams[1] = 0.0f;

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    inputValue1 = *(float *)&_inletParams[0];
    inputValue2 = *(float *)&_inletParams[1];

    loaded              = false;
}

//--------------------------------------------------------------
void Range::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"n1");
    this->addInlet(VP_LINK_NUMERIC,"n2");
    this->addOutlet(VP_LINK_NUMERIC,"min");
    this->addOutlet(VP_LINK_NUMERIC,"max");

    this->setCustomVar(static_cast<float>(inputValue1),"NUMBER1");
    this->setCustomVar(static_cast<float>(inputValue2),"NUMBER2");
}

//--------------------------------------------------------------
void Range::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setWidth(this->width);
    gui->addBreak();
    gui->onTextInputEvent(this, &Range::onTextInputEvent);

    inputNumber1 = gui->addTextInput("","0");
    inputNumber1->setUseCustomMouse(true);
    inputNumber1->setText(ofToString(this->getCustomVar("NUMBER1")));
    inputNumber2 = gui->addTextInput("","0");
    inputNumber2->setUseCustomMouse(true);
    inputNumber2->setText(ofToString(this->getCustomVar("NUMBER2")));

    inputValue1 = this->getCustomVar("NUMBER1");
    inputValue2 = this->getCustomVar("NUMBER2");

    gui->setPosition(0,this->headerHeight);
}

//--------------------------------------------------------------
void Range::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){
    if(this->inletsConnected[0]){
      inputValue1 = *(float *)&_inletParams[0];
      inputNumber1->setText(ofToString(inputValue1));
    }
    if(this->inletsConnected[1]){
      inputValue2 = *(float *)&_inletParams[1];
      inputNumber2->setText(ofToString(inputValue2));
    }

    gui->update();
    inputNumber1->update();
    inputNumber2->update();

    *(float *)&_outletParams[0] = inputValue1;
    *(float *)&_outletParams[1] = inputValue2;

    if(!loaded){
        loaded = true;
        inputNumber1->setText(ofToString(this->getCustomVar("NUMBER1")));
        inputNumber2->setText(ofToString(this->getCustomVar("NUMBER2")));
    }

}

//--------------------------------------------------------------
void Range::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void Range::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void Range::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    inputNumber1->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    inputNumber2->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    this->isOverGUI = inputNumber1->hitTest(_m-this->getPos()) || inputNumber2->hitTest(_m-this->getPos());
}

//--------------------------------------------------------------
void Range::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        inputNumber1->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        inputNumber2->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void Range::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(e.target == inputNumber1){
        if(isInteger(e.text) || isFloat(e.text)){
            this->setCustomVar(static_cast<float>(ofToFloat(e.text)),"NUMBER1");
            inputValue1 = ofToFloat(e.text);
        }
    }else if(e.target == inputNumber2){
        if(isInteger(e.text) || isFloat(e.text)){
            this->setCustomVar(static_cast<float>(ofToFloat(e.text)),"NUMBER2");
            inputValue2 = ofToFloat(e.text);
        }
    }
}

OBJECT_REGISTER( Range, "range", OFXVP_OBJECT_CAT_MATH)

#endif