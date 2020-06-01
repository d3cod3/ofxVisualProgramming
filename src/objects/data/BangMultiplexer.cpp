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

#ifndef OFXVP_BUILD_WITH_MINIMAL_OBJECTS

#include "BangMultiplexer.h"

//--------------------------------------------------------------
BangMultiplexer::BangMultiplexer() : PatchObject(){

    this->numInlets  = 6;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // float1
    _inletParams[1] = new float();  // float2
    _inletParams[2] = new float();  // float3
    _inletParams[3] = new float();  // float4
    _inletParams[4] = new float();  // float5
    _inletParams[5] = new float();  // float6
    *(float *)&_inletParams[0] = 0.0f;
    *(float *)&_inletParams[1] = 0.0f;
    *(float *)&_inletParams[2] = 0.0f;
    *(float *)&_inletParams[3] = 0.0f;
    *(float *)&_inletParams[4] = 0.0f;
    *(float *)&_inletParams[5] = 0.0f;

    _outletParams[0] = new float();  // output
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

}

//--------------------------------------------------------------
void BangMultiplexer::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"f1");
    this->addInlet(VP_LINK_NUMERIC,"f2");
    this->addInlet(VP_LINK_NUMERIC,"f3");
    this->addInlet(VP_LINK_NUMERIC,"f4");
    this->addInlet(VP_LINK_NUMERIC,"f5");
    this->addInlet(VP_LINK_NUMERIC,"f6");
    this->addOutlet(VP_LINK_NUMERIC,"bang");
}

//--------------------------------------------------------------
void BangMultiplexer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    
}

//--------------------------------------------------------------
void BangMultiplexer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    for(int i=0;i<this->numInlets;i++){
        if(this->inletsConnected[i]){
            *(float *)&_outletParams[0] = *(float *)&_inletParams[i];
            if(*(float *)&_inletParams[i] != 0){
                break;
            }
        }
    }
}

//--------------------------------------------------------------
void BangMultiplexer::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    for(int i=0;i<this->numInlets;i++){
        font->draw(ofToString(*(float *)&_inletParams[i]),this->fontSize,this->width/2,this->headerHeight*2.3 + (i*this->fontSize*1.15));
    }
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void BangMultiplexer::removeObjectContent(bool removeFileFromData){

}

OBJECT_REGISTER( BangMultiplexer, "bang multiplexer", OFXVP_OBJECT_CAT_DATA)

#endif