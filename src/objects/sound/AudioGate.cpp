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

#ifndef OFXVP_BUILD_WITH_MINIMAL_OBJECTS

#include "AudioGate.h"

//--------------------------------------------------------------
AudioGate::AudioGate() : PatchObject(){

    this->numInlets  = 6;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // open
    *(float *)&_inletParams[0] = 0.0f;

    _inletParams[1] = new ofSoundBuffer();  // sig1
    _inletParams[2] = new ofSoundBuffer();  // sig2
    _inletParams[3] = new ofSoundBuffer();  // sig3
    _inletParams[4] = new ofSoundBuffer();  // sig4
    _inletParams[5] = new ofSoundBuffer();  // sig5

    _outletParams[0] = new ofSoundBuffer(); // audio output

    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    this->initInletsState();

    isOpen              = false;
    openInlet           = 0;
    changedOpenInlet    = false;

}

//--------------------------------------------------------------
void AudioGate::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"open");
    this->addInlet(VP_LINK_AUDIO,"s1");
    this->addInlet(VP_LINK_AUDIO,"s2");
    this->addInlet(VP_LINK_AUDIO,"s3");
    this->addInlet(VP_LINK_AUDIO,"s4");
    this->addInlet(VP_LINK_AUDIO,"s5");
    this->addOutlet(VP_LINK_AUDIO,"output");
}

//--------------------------------------------------------------
void AudioGate::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    for(int i=1;i<6;i++){
        static_cast<ofSoundBuffer *>(_inletParams[i])->set(0.0f);
    }

    static_cast<ofSoundBuffer *>(_outletParams[0])->set(0.0f);
}

//--------------------------------------------------------------
void AudioGate::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){
    
    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] < 1.0){
            isOpen = false;
        }else{
            isOpen = true;
        }
    }

    if(isOpen){
        if(static_cast<int>(floor(*(float *)&_inletParams[0])) != openInlet){
            changedOpenInlet = true;
        }
        openInlet = static_cast<int>(floor(*(float *)&_inletParams[0]));
        waveform.clear();
        for(size_t i = 0; i < static_cast<ofSoundBuffer *>(_outletParams[0])->getNumFrames(); i++) {
            float sample = static_cast<ofSoundBuffer *>(_outletParams[0])->getSample(i,0);
            float x = ofMap(i, 0, static_cast<ofSoundBuffer *>(_outletParams[0])->getNumFrames(), 0, this->width);
            float y = ofMap(hardClip(sample), -1, 1, 0, this->height);
            waveform.addVertex(x, y);
        }
        if(changedOpenInlet){
            changedOpenInlet = false;
            this->pdspOut[0].disconnectIn();
            this->pdspIn[openInlet] >> this->pdspOut[0];
        }
    }else{
        if(static_cast<int>(floor(*(float *)&_inletParams[0])) != openInlet){
            changedOpenInlet = true;
        }
        openInlet = static_cast<int>(floor(*(float *)&_inletParams[0]));
        if(changedOpenInlet){
            changedOpenInlet = false;
            this->pdspOut[0].disconnectIn();
        }
    }
    
}

//--------------------------------------------------------------
void AudioGate::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    waveform.draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void AudioGate::removeObjectContent(bool removeFileFromData){
    
}

//--------------------------------------------------------------
void AudioGate::audioOutObject(ofSoundBuffer &outputBuffer){
    if(isOpen){
        if(openInlet >= 1 && openInlet <= this->numInlets){
            *static_cast<ofSoundBuffer *>(_outletParams[0]) = *static_cast<ofSoundBuffer *>(_inletParams[openInlet]);
        }
    }else{
        *static_cast<ofSoundBuffer *>(_outletParams[0]) *= 0.0f;
    }
}

OBJECT_REGISTER( AudioGate, "audio gate", OFXVP_OBJECT_CAT_SOUND)

#endif