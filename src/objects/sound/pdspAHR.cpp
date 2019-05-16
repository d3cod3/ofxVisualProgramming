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

#include "pdspAHR.h"

//--------------------------------------------------------------
pdspAHR::pdspAHR() : PatchObject(){

    this->numInlets  = 0;
    this->numOutlets = 1;

    _outletParams[0] = new ofSoundBuffer(); // audio output

    this->initInletsState();

    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

}

//--------------------------------------------------------------
void pdspAHR::newObject(){
    this->setName("AHR");
    this->addOutlet(VP_LINK_AUDIO);
}

//--------------------------------------------------------------
void pdspAHR::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();
}

//--------------------------------------------------------------
void pdspAHR::setupAudioOutObjectContent(pdsp::Engine &engine){

    noise >> this->pdspOut[0];
}

//--------------------------------------------------------------
void pdspAHR::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){

}

//--------------------------------------------------------------
void pdspAHR::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void pdspAHR::removeObjectContent(){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspAHR::loadAudioSettings(){
    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){
        if (XML.pushTag("settings")){
            sampleRate = XML.getValue("sample_rate_in",0);
            bufferSize = XML.getValue("buffer_size",0);

            for(int i=0;i<bufferSize;i++){
                static_cast<vector<float> *>(_outletParams[1])->push_back(0.0f);
            }

            XML.popTag();
        }
    }
}

//--------------------------------------------------------------
void pdspAHR::audioOutObject(ofSoundBuffer &outputBuffer){
    // SIGNAL BUFFER
    
}
