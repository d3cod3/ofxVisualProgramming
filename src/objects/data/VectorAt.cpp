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

#include "VectorAt.h"

//--------------------------------------------------------------
VectorAt::VectorAt() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new vector<float>();  // input vector
    _inletParams[1] = new float();          // at
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new float();         // output
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    vectorAt            = 0;
    loaded              = false;

}

//--------------------------------------------------------------
void VectorAt::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_ARRAY,"vector");
    this->addInlet(VP_LINK_NUMERIC,"at");
    this->addOutlet(VP_LINK_NUMERIC,"value");

    this->setCustomVar(0,"AT");
}

//--------------------------------------------------------------
void VectorAt::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onTextInputEvent(this, &VectorAt::onTextInputEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    atPosition = gui->addTextInput("AT","0");
    atPosition->setUseCustomMouse(true);
    atPosition->setText(ofToString(static_cast<int>(floor(this->getCustomVar("AT")))));

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);
}

//--------------------------------------------------------------
void VectorAt::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    gui->update();
    header->update();
    atPosition->update();

    if(!loaded){
        loaded = true;
        atPosition->setText(ofToString(static_cast<int>(floor(this->getCustomVar("AT")))));
        vectorAt = ofToInt(atPosition->getText());
    }

    if(this->inletsConnected[1]){
        atPosition->setText(ofToString(static_cast<int>(floor(*(float *)&_inletParams[1]))));
        vectorAt = static_cast<int>(floor(*(float *)&_inletParams[1]));
    }

    if(this->inletsConnected[0] && _inletParams[0]){
        if(static_cast<vector<float> *>(_inletParams[0])->size() > 0){
            if(vectorAt < static_cast<vector<float> *>(_inletParams[0])->size()){
                *(float *)&_outletParams[0] = static_cast<vector<float> *>(_inletParams[0])->at(vectorAt);
            }else{
                *(float *)&_outletParams[0] = 0.0f;
            }
        }else{
            *(float *)&_outletParams[0] = 0.0f;
        }
    }else{
        *(float *)&_outletParams[0] = 0.0f;
    }


}

//--------------------------------------------------------------
void VectorAt::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    font->draw(ofToString(*(float *)&_outletParams[0]),this->fontSize,this->width/2,this->headerHeight*2.3);
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void VectorAt::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void VectorAt::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    atPosition->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || atPosition->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }
}

//--------------------------------------------------------------
void VectorAt::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        atPosition->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void VectorAt::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == atPosition){
            if(isInteger(e.text)){
                this->setCustomVar(static_cast<float>(ofToInt(e.text)),"AT");
                vectorAt = ofToInt(e.text);
            }else{
                atPosition->setText(e.text);
            }
        }
    }
}

OBJECT_REGISTER( VectorAt, "vector at", OFXVP_OBJECT_CAT_DATA)

#endif