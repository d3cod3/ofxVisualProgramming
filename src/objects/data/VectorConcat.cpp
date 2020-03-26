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

#include "VectorConcat.h"

//--------------------------------------------------------------
VectorConcat::VectorConcat() : PatchObject(){

    this->numInlets  = 6;
    this->numOutlets = 1;

    _inletParams[0] = new vector<float>(); // vector1
    _inletParams[1] = new vector<float>(); // vector2
    _inletParams[2] = new vector<float>(); // vector3
    _inletParams[3] = new vector<float>(); // vector4
    _inletParams[4] = new vector<float>(); // vector5
    _inletParams[5] = new vector<float>(); // vector6

    _outletParams[0] = new vector<float>();  // final vector

    this->initInletsState();

}

//--------------------------------------------------------------
void VectorConcat::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_ARRAY,"v1");
    this->addInlet(VP_LINK_ARRAY,"v2");
    this->addInlet(VP_LINK_ARRAY,"v3");
    this->addInlet(VP_LINK_ARRAY,"v4");
    this->addInlet(VP_LINK_ARRAY,"v5");
    this->addInlet(VP_LINK_ARRAY,"v6");
    this->addOutlet(VP_LINK_ARRAY,"output");
}

//--------------------------------------------------------------
void VectorConcat::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void VectorConcat::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){
    static_cast<vector<float> *>(_outletParams[0])->clear();
    for(int i=0;i<this->numInlets;i++){
        if(this->inletsConnected[i]){
            for(size_t s=0;s<static_cast<size_t>(static_cast<vector<float> *>(_inletParams[i])->size());s++){
                static_cast<vector<float> *>(_outletParams[0])->push_back(static_cast<vector<float> *>(_inletParams[i])->at(s));
            }
        }
    }
}

//--------------------------------------------------------------
void VectorConcat::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    font->draw("Size: "+ofToString(static_cast<size_t>(static_cast<vector<float> *>(_outletParams[0])->size())),this->fontSize,this->width/2,this->headerHeight*2.3);
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void VectorConcat::removeObjectContent(bool removeFileFromData){

}

OBJECT_REGISTER( VectorConcat, "vector concat", OFXVP_OBJECT_CAT_DATA)
