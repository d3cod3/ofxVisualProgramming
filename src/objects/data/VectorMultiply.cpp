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

#include "VectorMultiply.h"

//--------------------------------------------------------------
VectorMultiply::VectorMultiply() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new vector<float>();  // input data
    _inletParams[1] = new float();  // multiplier
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new vector<float>(); // output

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    number              = 0.0f;
    loaded              = false;
}

//--------------------------------------------------------------
void VectorMultiply::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_ARRAY,"data");
    this->addInlet(VP_LINK_NUMERIC,"mult");
    this->addOutlet(VP_LINK_ARRAY,"result");

    this->setCustomVar(0,"NUMBER");

}

//--------------------------------------------------------------
void VectorMultiply::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onTextInputEvent(this, &VectorMultiply::onTextInputEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    numberBox = gui->addTextInput("MULT","0");
    numberBox->setUseCustomMouse(true);
    numberBox->setText(ofToString(static_cast<int>(floor(this->getCustomVar("NUMBER")))));

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);
}

//--------------------------------------------------------------
void VectorMultiply::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

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

    static_cast<vector<float> *>(_outletParams[0])->clear();
    if(this->inletsConnected[0]){
        for(size_t s=0;s<static_cast<size_t>(static_cast<vector<float> *>(_inletParams[0])->size());s++){
            static_cast<vector<float> *>(_outletParams[0])->push_back(static_cast<vector<float> *>(_inletParams[0])->at(s)*number);
        }
    }
}

//--------------------------------------------------------------
void VectorMultiply::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    font->draw("Size: "+ofToString(static_cast<size_t>(static_cast<vector<float> *>(_outletParams[0])->size())),this->fontSize,this->width/2,this->headerHeight*2.3);
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void VectorMultiply::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void VectorMultiply::mouseMovedObjectContent(ofVec3f _m){
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
void VectorMultiply::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        numberBox->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void VectorMultiply::onTextInputEvent(ofxDatGuiTextInputEvent e){
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

OBJECT_REGISTER( VectorMultiply, "vector multiply", OFXVP_OBJECT_CAT_DATA);
