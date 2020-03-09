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

#include "LoadBang.h"

//--------------------------------------------------------------
LoadBang::LoadBang() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 2;

    _inletParams[0] = new float();  // time
    *(float *)&_inletParams[0] = 1000.0f;

    _outletParams[0] = new float(); // output numeric
    *(float *)&_outletParams[0] = 0.0f;

    _outletParams[1] = new string(); // output string
    *static_cast<string *>(_outletParams[1]) = "";

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    bang                = false;
    loadStart           = false;

    wait                = static_cast<size_t>(floor(*(float *)&_inletParams[0]));
    startTime           = ofGetElapsedTimeMillis();

    loaded              = false;

}

//--------------------------------------------------------------
void LoadBang::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"delay");
    this->addOutlet(VP_LINK_NUMERIC,"bang");
    this->addOutlet(VP_LINK_STRING,"bang");

    this->setCustomVar(static_cast<float>(wait),"TIME");
}

//--------------------------------------------------------------
void LoadBang::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onTextInputEvent(this, &LoadBang::onTextInputEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    timeSetting = gui->addTextInput("DELAY","1000");
    timeSetting->setUseCustomMouse(true);
    timeSetting->setText(ofToString(static_cast<int>(floor(this->getCustomVar("TIME")))));

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    wait = ofToInt(timeSetting->getText());
}

//--------------------------------------------------------------
void LoadBang::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    timeSetting->update();

    if(!loaded){
        loaded = true;
        timeSetting->setText(ofToString(static_cast<int>(floor(this->getCustomVar("TIME")))));
        wait = ofToInt(timeSetting->getText());
    }

    if(this->inletsConnected[0] && static_cast<size_t>(floor(*(float *)&_inletParams[0])) != wait){
        wait = static_cast<size_t>(floor(*(float *)&_inletParams[0]));
        timeSetting->setText(ofToString(wait));
        this->setCustomVar(static_cast<float>(wait),"TIME");
        loadStart = false;
        startTime = ofGetElapsedTimeMillis();
    }

    if(!loadStart && (ofGetElapsedTimeMillis()-startTime > wait)){
        bang = true;
        loadStart = true;
    }else{
        bang = false;
    }
    *(float *)&_outletParams[0] = static_cast<float>(bang);

    if(bang){
        *static_cast<string *>(_outletParams[1]) = "bang";
    }else{
        *static_cast<string *>(_outletParams[1]) = "";
    }

}

//--------------------------------------------------------------
void LoadBang::drawObjectContent(ofxFontStash *font){
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
void LoadBang::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    timeSetting->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || timeSetting->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }
}

//--------------------------------------------------------------
void LoadBang::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        timeSetting->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void LoadBang::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == timeSetting){
            if(isInteger(e.text)){
                this->setCustomVar(static_cast<float>(ofToInt(e.text)),"TIME");
                wait = ofToInt(e.text);
                loadStart = false;
                startTime = ofGetElapsedTimeMillis();
            }else{
                timeSetting->setText(ofToString(wait));
            }
        }
    }
}

OBJECT_REGISTER( LoadBang, "loadbang", OFXVP_OBJECT_CAT_LOGIC);
