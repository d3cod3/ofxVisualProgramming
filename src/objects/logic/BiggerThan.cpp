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

#include "BiggerThan.h"

//--------------------------------------------------------------
BiggerThan::BiggerThan() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // input number
    *(float *)&_inletParams[0] = 0.0f;

    _outletParams[0] = new float(); // output
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    this->height        /= 2;

    isGUIObject         = true;
    this->isOverGUI     = true;

    inputValue = *(float *)&_inletParams[0];
}

//--------------------------------------------------------------
void BiggerThan::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"number");
    this->addOutlet(VP_LINK_NUMERIC,"trigger");

    this->setCustomVar(static_cast<float>(inputValue),"NUMBER");
}

//--------------------------------------------------------------
void BiggerThan::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setWidth(this->width);
    gui->addBreak();
    gui->onTextInputEvent(this, &BiggerThan::onTextInputEvent);

    inputNumber = gui->addTextInput("","0");
    inputNumber->setUseCustomMouse(true);
    inputNumber->setText(ofToString(this->getCustomVar("NUMBER")));
    
    equalsTo = this->getCustomVar("NUMBER");

    gui->setPosition(0,this->headerHeight);
}

//--------------------------------------------------------------
void BiggerThan::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    if(this->inletsConnected[0]){
        inputValue = *(float *)&_inletParams[0];
        if(inputValue > equalsTo){
            *(float *)&_outletParams[0] = 1;
        }else{
            *(float *)&_outletParams[0] = 0;
        }
    }else{
        *(float *)&_outletParams[0] = 0;
    }

    gui->update();
    inputNumber->update();

}

//--------------------------------------------------------------
void BiggerThan::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void BiggerThan::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void BiggerThan::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    inputNumber->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    this->isOverGUI = inputNumber->hitTest(_m-this->getPos());
}

//--------------------------------------------------------------
void BiggerThan::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        inputNumber->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void BiggerThan::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(e.target == inputNumber){
        if(isInteger(e.text) || isFloat(e.text)){
            this->setCustomVar(static_cast<float>(ofToFloat(e.text)),"NUMBER");
            equalsTo = ofToFloat(e.text);
        }

    }
}

OBJECT_REGISTER( BiggerThan, ">", OFXVP_OBJECT_CAT_LOGIC)

#endif