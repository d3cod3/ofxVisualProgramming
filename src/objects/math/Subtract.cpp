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

#include "Subtract.h"

//--------------------------------------------------------------
Subtract::Subtract() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // input number 1
    *(float *)&_inletParams[0] = 0.0f;
    _inletParams[1] = new float();  // input number 2
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new float(); // output
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    this->height        /= 2;

    isGUIObject         = true;
    this->isOverGUI     = true;

    number              = 0.0f;
    loaded              = false;

}

//--------------------------------------------------------------
void Subtract::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"n1");
    this->addInlet(VP_LINK_NUMERIC,"n2");
    this->addOutlet(VP_LINK_NUMERIC,"result");

    this->setCustomVar(0,"NUMBER");

}

//--------------------------------------------------------------
void Subtract::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onTextInputEvent(this, &Subtract::onTextInputEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    numberBox = gui->addTextInput("N2","0");
    numberBox->setUseCustomMouse(true);
    numberBox->setText(ofToString(static_cast<int>(floor(this->getCustomVar("NUMBER")))));

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);
}

//--------------------------------------------------------------
void Subtract::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    gui->update();
    header->update();
    numberBox->update();

    if(!loaded){
        loaded = true;
        numberBox->setText(ofToString(this->getCustomVar("NUMBER")));
        number = this->getCustomVar("NUMBER");
    }

    if(this->inletsConnected[1]){
        numberBox->setText(ofToString(*(float *)&_inletParams[1]));
        number = *(float *)&_inletParams[1];
    }

    if(this->inletsConnected[0]){
      *(float *)&_outletParams[0] = *(float *)&_inletParams[0] - number;
    }else{
      *(float *)&_outletParams[0] = 0.0f;
    }

}

//--------------------------------------------------------------
void Subtract::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    font->draw(ofToString(*(float *)&_outletParams[0]),this->fontSize,this->width/2,this->headerHeight*2);
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void Subtract::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void Subtract::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    numberBox->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || numberBox->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }
}

//--------------------------------------------------------------
void Subtract::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void Subtract::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == numberBox){
            if(isInteger(e.text)){
                this->setCustomVar(static_cast<float>(ofToInt(e.text)),"NUMBER");
                number = ofToFloat(e.text);
            }else if(isFloat(e.text)){
                this->setCustomVar(ofToFloat(e.text),"NUMBER");
                number = ofToFloat(e.text);
            }else{
                numberBox->setText(e.text);
            }
        }
    }
}

OBJECT_REGISTER( Subtract, "subtract", OFXVP_OBJECT_CAT_MATH)

#endif