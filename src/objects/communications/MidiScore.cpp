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

#include "MidiScore.h"

//--------------------------------------------------------------
MidiScore::MidiScore() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 2;

    _inletParams[0] = new vector<float>();  // midi notes vector

    _outletParams[0] = new float(); // trigger
    *(float *)&_outletParams[0] = 0.0f;
    _outletParams[1] = new float(); // note
    *(float *)&_outletParams[1] = 0.0f;

    this->initInletsState();

}

//--------------------------------------------------------------
void MidiScore::newObject(){
    this->setName("midi score");
    this->addInlet(VP_LINK_ARRAY,"score");
    this->addOutlet(VP_LINK_NUMERIC,"trigger");
    this->addOutlet(VP_LINK_NUMERIC,"midiNote");
}

//--------------------------------------------------------------
void MidiScore::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void MidiScore::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){

    if(this->inletsConnected[0]){
        isTrigger = false;
        for(size_t s=0;s<static_cast<size_t>(static_cast<vector<float> *>(_inletParams[0])->size());s++){
            if(static_cast<size_t>(static_cast<vector<float> *>(_inletParams[0])->at(s)) != 0){
                isTrigger = true;
                break;
            }
        }
        if(isTrigger){
            *(float *)&_outletParams[0] = 1.0f;
            for(size_t s=0;s<static_cast<size_t>(static_cast<vector<float> *>(_inletParams[0])->size());s++){
                if(static_cast<size_t>(static_cast<vector<float> *>(_inletParams[0])->at(s)) != 0){
                    *(float *)&_outletParams[1] = static_cast<float>(s);
                }
            }
        }else{
            *(float *)&_outletParams[0] = 0.0f;
        }
    }else{
        *(float *)&_outletParams[0] = 0.0f;
        *(float *)&_outletParams[1] = 0.0f;
    }

}

//--------------------------------------------------------------
void MidiScore::drawObjectContent(ofxFontStash *font){
    ofSetColor(30,31,36);
    ofDrawRectangle(0,0,this->width,this->height);
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(this->inletsConnected[0] && isTrigger){
        ofSetColor(250,250,5,*(float *)&_outletParams[0]*255);
        ofDrawRectangle(0,0,this->width,this->height);
    }
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void MidiScore::removeObjectContent(){

}
