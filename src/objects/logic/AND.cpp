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

#include "AND.h"

//--------------------------------------------------------------
AND::AND() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // b1
    *(float *)&_inletParams[0] = 0.0f;
    _inletParams[1] = new float();  // b2
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new float(); // output
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    this->height        /= 2;

    bang                = false;

}

//--------------------------------------------------------------
void AND::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"b1");
    this->addInlet(VP_LINK_NUMERIC,"b2");
    this->addOutlet(VP_LINK_NUMERIC,"result");
}

//--------------------------------------------------------------
void AND::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void AND::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){
    if(this->inletsConnected[0] && this->inletsConnected[1]){
        if(*(float *)&_inletParams[0] >= 1.0 && *(float *)&_inletParams[1] >= 1.0){
            *(float *)&_outletParams[0] = 1;
            bang                = true;
        }else{
            *(float *)&_outletParams[0] = 0;
            bang                = false;
        }
    }else{
        *(float *)&_outletParams[0] = 0;
        bang                = false;
    }
}

//--------------------------------------------------------------
void AND::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(bang){
      ofSetColor(250,250,5);
      ofDrawRectangle(0,0,this->width,this->height);
    }
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void AND::removeObjectContent(bool removeFileFromData){

}

OBJECT_REGISTER( AND, "&&", OFXVP_OBJECT_CAT_LOGIC)
