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

#include "Select.h"

//--------------------------------------------------------------
Select::Select() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 17;

    _inletParams[0] = new float();  // state
    *(float *)&_inletParams[0] = 0.0f;

    for(int i=0;i<this->numOutlets;i++){
        _outletParams[i] = new float(); // output numeric
        *(float *)&_outletParams[i] = 0.0f;
    }

    this->height          *= 3;

    this->initInletsState();

    selector    = 0;
    lastValue   = 0;
}

//--------------------------------------------------------------
void Select::newObject(){
    this->setName("select");
    this->addInlet(VP_LINK_NUMERIC,"sel");
    for(int i=0;i<this->numOutlets;i++){
        this->addOutlet(VP_LINK_NUMERIC,"bang"+ofToString(i));
    }
}

//--------------------------------------------------------------
void Select::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    for(int i=0;i<this->numOutlets;i++){
        bangs.push_back(false);
    }
}

//--------------------------------------------------------------
void Select::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){
    
    if(this->inletsConnected[0]){
        if(static_cast<int>(floor(*(float *)&_inletParams[0])) != lastValue && static_cast<int>(floor(*(float *)&_inletParams[0])) < bangs.size()){
            lastValue = static_cast<int>(floor(*(float *)&_inletParams[0]));
            bangs.at(lastValue) = true;
        }else if(static_cast<int>(floor(*(float *)&_inletParams[0])) != lastValue && static_cast<int>(floor(*(float *)&_inletParams[0])) >= bangs.size()){
            lastValue = static_cast<int>(floor(*(float *)&_inletParams[0]));
            bangs.at(bangs.size()-1) = true;
        }else{
            for(int i=0;i<bangs.size();i++){
                bangs.at(i) = false;
            }
        }

        for(int i=0;i<bangs.size();i++){
            *(float *)&_outletParams[i] = static_cast<float>(bangs.at(i));
        }
    }
    
}

//--------------------------------------------------------------
void Select::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void Select::removeObjectContent(bool removeFileFromData){
    
}
