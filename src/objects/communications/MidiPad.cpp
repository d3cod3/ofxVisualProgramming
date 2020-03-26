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

#include "MidiPad.h"

//--------------------------------------------------------------
MidiPad::MidiPad() : PatchObject(){

    this->numInlets  = 3;
    this->numOutlets = 2;

    _inletParams[0] = new float();  // pitch (index)
    *(float *)&_inletParams[0] = 0.0f;
    _inletParams[1] = new float();  // value
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();  // velocity
    *(float *)&_inletParams[2] = 0.0f;

    _outletParams[0] = new float(); // value
    *(float *)&_outletParams[0] = 0.0f;
    _outletParams[1] = new float(); // velocity
    *(float *)&_outletParams[1] = 0.0f;

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

}

//--------------------------------------------------------------
void MidiPad::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"pitch");
    this->addInlet(VP_LINK_NUMERIC,"value");
    this->addInlet(VP_LINK_NUMERIC,"velocity");
    this->addOutlet(VP_LINK_NUMERIC,"value");
    this->addOutlet(VP_LINK_NUMERIC,"velocity");

    this->setCustomVar(0.0f,"INDEX");
}

//--------------------------------------------------------------
void MidiPad::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setTheme(new ofxDatGuiThemeCharcoal());
    gui->setAutoDraw(false);
    gui->setWidth(this->width);
    gui->addBreak();
    gui->onTextInputEvent(this, &MidiPad::onTextInputEvent);

    inputNumber = gui->addTextInput("","0");
    inputNumber->setUseCustomMouse(true);
    inputNumber->setText(ofToString(this->getCustomVar("INDEX")));

    gui->setPosition(0,this->headerHeight);

}

//--------------------------------------------------------------
void MidiPad::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    inputNumber->update();

    if(this->inletsConnected[0]){
        if(static_cast<int>(floor(*(float *)&_inletParams[0])) != 0){
            lastPitch = static_cast<int>(floor(*(float *)&_inletParams[0]));
        }
        if(this->inletsConnected[1]){
            if(lastPitch == static_cast<int>(floor(this->getCustomVar("INDEX")))){
                *(float *)&_outletParams[0] = *(float *)&_inletParams[1]/127.0f;
            }
        }
        if(this->inletsConnected[2]){
            if(lastPitch == static_cast<int>(floor(this->getCustomVar("INDEX")))){
                *(float *)&_outletParams[1] = *(float *)&_inletParams[2]/127.0f;
            }
        }

    }else{
        *(float *)&_outletParams[0] = 0.0f;
    }

}

//--------------------------------------------------------------
void MidiPad::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(30,31,36);
    ofDrawRectangle(0,0,this->width,this->height);
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(this->inletsConnected[0] && this->inletsConnected[1]){
        ofSetColor(250,250,5,*(float *)&_outletParams[0]*255);
        ofDrawRectangle(0,0,this->width,this->height);
    }
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void MidiPad::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void MidiPad::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    inputNumber->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    this->isOverGUI = inputNumber->hitTest(_m-this->getPos());
}

//--------------------------------------------------------------
void MidiPad::dragGUIObject(ofVec3f _m){
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
            outPut[j]->linkVertices[0].move(outPut[j]->posFrom.x,outPut[j]->posFrom.y);
            outPut[j]->linkVertices[1].move(outPut[j]->posFrom.x+20,outPut[j]->posFrom.y);
        }
    }
}

//--------------------------------------------------------------
void MidiPad::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(e.target == inputNumber){
        if(isInteger(e.text) || isFloat(e.text)){
            this->setCustomVar(static_cast<float>(ofToFloat(e.text)),"INDEX");
        }

    }
}

OBJECT_REGISTER( MidiPad, "midi pad", OFXVP_OBJECT_CAT_COMMUNICATIONS)
