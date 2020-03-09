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

#include "Inverter.h"

//--------------------------------------------------------------
Inverter::Inverter() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;

    _outletParams[0] = new float(); // output numeric
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    trigger = true;

}

//--------------------------------------------------------------
void Inverter::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"trigger");
    this->addOutlet(VP_LINK_NUMERIC,"invertedTrigger");
}

//--------------------------------------------------------------
void Inverter::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    
}

//--------------------------------------------------------------
void Inverter::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){
    
    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] < 1.0f){
            trigger = true;
        }else{
            trigger = false;
        }
    }
    *(float *)&_outletParams[0] = static_cast<float>(trigger);
}

//--------------------------------------------------------------
void Inverter::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(trigger){
        ofSetLineWidth(6);
        ofSetColor(250,250,5);
        if(this->isRetina){
            ofDrawLine(0,this->headerHeight,this->width,this->height-12);
            ofDrawLine(this->width,this->headerHeight,0,this->height-12);
        }else{
            ofDrawLine(0,this->headerHeight,this->width,this->height-6);
            ofDrawLine(this->width,this->headerHeight,0,this->height-6);
        }

        ofSetLineWidth(1);
    }
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void Inverter::removeObjectContent(bool removeFileFromData){
    
}

OBJECT_REGISTER( Inverter, "inverter", OFXVP_OBJECT_CAT_LOGIC);
