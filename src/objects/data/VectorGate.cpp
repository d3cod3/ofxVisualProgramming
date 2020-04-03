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

#include "VectorGate.h"

//--------------------------------------------------------------
VectorGate::VectorGate() : PatchObject(){

    this->numInlets  = 6;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // open
    *(float *)&_inletParams[0] = 0.0f;

    _inletParams[1] = new vector<float>();  // vector1
    _inletParams[2] = new vector<float>();  // vector2
    _inletParams[3] = new vector<float>();  // vector3
    _inletParams[4] = new vector<float>();  // vector4
    _inletParams[5] = new vector<float>();  // vector5

    _outletParams[0] = new vector<float>(); // output numeric

    this->initInletsState();

    isOpen      = false;
    openInlet   = 0;

}

//--------------------------------------------------------------
void VectorGate::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"open");
    this->addInlet(VP_LINK_ARRAY,"v1");
    this->addInlet(VP_LINK_ARRAY,"v2");
    this->addInlet(VP_LINK_ARRAY,"v3");
    this->addInlet(VP_LINK_ARRAY,"v4");
    this->addInlet(VP_LINK_ARRAY,"v5");
    this->addOutlet(VP_LINK_ARRAY,"output");
}

//--------------------------------------------------------------
void VectorGate::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void VectorGate::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){
    static_cast<vector<float> *>(_outletParams[0])->clear();

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
            for(size_t s=0;s<static_cast<size_t>(static_cast<vector<float> *>(_inletParams[openInlet])->size());s++){
                static_cast<vector<float> *>(_outletParams[0])->push_back(static_cast<vector<float> *>(_inletParams[openInlet])->at(s));
            }
        }
    }

}

//--------------------------------------------------------------
void VectorGate::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void VectorGate::removeObjectContent(bool removeFileFromData){

}

OBJECT_REGISTER( VectorGate, "vector gate", OFXVP_OBJECT_CAT_DATA)

#endif