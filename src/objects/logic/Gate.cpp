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

#include "Gate.h"

//--------------------------------------------------------------
Gate::Gate() : PatchObject(){

    this->numInlets  = 6;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // open
    *(float *)&_inletParams[0] = 0.0f;

    _inletParams[1] = new float();  // float1
    _inletParams[2] = new float();  // float2
    _inletParams[3] = new float();  // float3
    _inletParams[4] = new float();  // float4
    _inletParams[5] = new float();  // float5
    *(float *)&_inletParams[1] = 0.0f;
    *(float *)&_inletParams[2] = 0.0f;
    *(float *)&_inletParams[3] = 0.0f;
    *(float *)&_inletParams[4] = 0.0f;
    *(float *)&_inletParams[5] = 0.0f;

    _outletParams[0] = new float(); // output numeric
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    isOpen      = false;
    openInlet   = 0;

}

//--------------------------------------------------------------
void Gate::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"open");
    this->addInlet(VP_LINK_NUMERIC,"f1");
    this->addInlet(VP_LINK_NUMERIC,"f2");
    this->addInlet(VP_LINK_NUMERIC,"f3");
    this->addInlet(VP_LINK_NUMERIC,"f4");
    this->addInlet(VP_LINK_NUMERIC,"f5");
    this->addOutlet(VP_LINK_NUMERIC,"output");
}

//--------------------------------------------------------------
void Gate::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void Gate::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){
    
    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] < 1.0){
            isOpen = false;
        }else{
            isOpen = true;
        }
    }

    if(isOpen){
        openInlet = static_cast<int>(floor(*(float *)&_inletParams[0]));
        if(openInlet >= 1 && openInlet <= this->numInlets && this->inletsConnected[openInlet]){
            *(float *)&_outletParams[0] = *(float *)&_inletParams[openInlet];
        }
    }else{
        *(float *)&_outletParams[0] = 0.0f;
    }
    
}

//--------------------------------------------------------------
void Gate::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void Gate::removeObjectContent(bool removeFileFromData){
    
}

OBJECT_REGISTER( Gate, "gate", OFXVP_OBJECT_CAT_LOGIC);
