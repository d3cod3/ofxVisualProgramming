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

#include "Spigot.h"

//--------------------------------------------------------------
Spigot::Spigot() : PatchObject(){

    this->numInlets  = 6;
    this->numOutlets = 5;

    _inletParams[0] = new float();  // state
    *(float *)&_inletParams[0] = 0.0f;

    _inletParams[1] = new float();  // float
    *(float *)&_inletParams[1] = 0.0f;

    _inletParams[2] = new string();  // string
    *static_cast<string *>(_inletParams[2]) = "";

    _inletParams[3] = new vector<float>(); // vector

    _inletParams[4] = new ofTexture(); // texture

    _inletParams[5] = new ofSoundBuffer();  // signal

    _outletParams[0] = new float(); // output numeric
    *(float *)&_outletParams[0] = 0.0f;

    _outletParams[1] = new string();  // string
    *static_cast<string *>(_outletParams[1]) = "";

    _outletParams[2] = new vector<float>(); // vector

    _outletParams[3] = new ofTexture(); // texture

    _outletParams[4] = new ofSoundBuffer();  // signal

    this->initInletsState();

    isOpen  = false;

    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    empty   = new vector<float>();
    kuro    = new ofImage();
}

//--------------------------------------------------------------
void Spigot::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"state");
    this->addInlet(VP_LINK_NUMERIC,"float");
    this->addInlet(VP_LINK_STRING,"string");
    this->addInlet(VP_LINK_ARRAY,"vector");
    this->addInlet(VP_LINK_TEXTURE,"texture");
    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addOutlet(VP_LINK_NUMERIC);
    this->addOutlet(VP_LINK_STRING);
    this->addOutlet(VP_LINK_ARRAY);
    this->addOutlet(VP_LINK_TEXTURE);
    this->addOutlet(VP_LINK_AUDIO);
}

//--------------------------------------------------------------
void Spigot::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    empty->assign(1,0);

    // load kuro
    kuro->load("images/kuro.jpg");

    static_cast<ofSoundBuffer *>(_inletParams[5])->set(0.0f);
    static_cast<ofSoundBuffer *>(_outletParams[4])->set(0.0f);
}

//--------------------------------------------------------------
void Spigot::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){
    
    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] < 1.0){
            isOpen = false;
        }else{
            isOpen = true;
        }
    }

    if(isOpen){
        *(float *)&_outletParams[0] = *(float *)&_inletParams[1];
        *static_cast<string *>(_outletParams[1]) = *static_cast<string *>(_inletParams[2]);
        *static_cast<vector<float> *>(_outletParams[2]) = *static_cast<vector<float> *>(_inletParams[3]);
        *static_cast<ofTexture *>(_outletParams[3]) = *static_cast<ofTexture *>(_inletParams[4]);
        *static_cast<ofSoundBuffer *>(_outletParams[4]) = *static_cast<ofSoundBuffer *>(_inletParams[5]);
    }else{
        *(float *)&_outletParams[0] = -1.0f;
        *static_cast<string *>(_outletParams[1]) = "empty";
        *static_cast<vector<float> *>(_outletParams[2]) = *empty;
        *static_cast<ofTexture *>(_outletParams[3]) = kuro->getTexture();
        *static_cast<ofSoundBuffer *>(_outletParams[4]) *= 0.0f;
    }
    
}

//--------------------------------------------------------------
void Spigot::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    ofSetLineWidth(3);
    if(!isOpen){
        ofSetColor(240,10,10);
        ofDrawLine(this->headerHeight*4,this->headerHeight,this->width-this->headerHeight,this->height);
    }else{
        ofSetColor(250,250,250);
        ofDrawLine(this->headerHeight*4,this->height/2 + this->headerHeight,this->width-this->headerHeight,this->height/2 + this->headerHeight);
    }
    ofSetLineWidth(1);
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void Spigot::removeObjectContent(bool removeFileFromData){
    
}

//--------------------------------------------------------------
void Spigot::mouseReleasedObjectContent(ofVec3f _m){
    if(!this->inletsConnected[0]){
        isOpen = !isOpen;
    }
}

//--------------------------------------------------------------
void Spigot::audioOutObject(ofSoundBuffer &outputBuffer){
    if(isOpen){
        if(this->inletsConnected[5]){
            *static_cast<ofSoundBuffer *>(_outletParams[4]) = *static_cast<ofSoundBuffer *>(_inletParams[5]);
        }
    }else{
        *static_cast<ofSoundBuffer *>(_outletParams[4]) *= 0.0f;
    }
}

OBJECT_REGISTER( Spigot, "spigot", OFXVP_OBJECT_CAT_LOGIC)
