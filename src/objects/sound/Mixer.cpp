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

#include "Mixer.h"

//--------------------------------------------------------------
Mixer::Mixer() : PatchObject(){

    this->numInlets  = 6;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();  // audio input 1
    _inletParams[1] = new ofSoundBuffer();  // audio input 2
    _inletParams[2] = new ofSoundBuffer();  // audio input 3
    _inletParams[3] = new ofSoundBuffer();  // audio input 4
    _inletParams[4] = new ofSoundBuffer();  // audio input 5
    _inletParams[5] = new ofSoundBuffer();  // audio input 6

    _outletParams[0] = new ofSoundBuffer(); // audio output

    this->initInletsState();

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

}

//--------------------------------------------------------------
void Mixer::newObject(){
    this->setName("mixer");
    this->addInlet(VP_LINK_AUDIO,"s1");
    this->addInlet(VP_LINK_AUDIO,"s2");
    this->addInlet(VP_LINK_AUDIO,"s3");
    this->addInlet(VP_LINK_AUDIO,"s4");
    this->addInlet(VP_LINK_AUDIO,"s5");
    this->addInlet(VP_LINK_AUDIO,"s6");
    this->addOutlet(VP_LINK_AUDIO,"mainOutput");
}

//--------------------------------------------------------------
void Mixer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();
}

//--------------------------------------------------------------
void Mixer::setupAudioOutObjectContent(pdsp::Engine &engine){

    this->pdspIn[0] >> mix;
    this->pdspIn[1] >> mix;
    this->pdspIn[2] >> mix;
    this->pdspIn[3] >> mix;
    this->pdspIn[4] >> mix;
    this->pdspIn[5] >> mix;

    mix >> this->pdspOut[0];
    mix >> scope >> engine.blackhole();
}

//--------------------------------------------------------------
void Mixer::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){

    waveform.clear();
    for(size_t i = 0; i < scope.getBuffer().size(); i++) {
        float sample = scope.getBuffer().at(i);
        float x = ofMap(i, 0, scope.getBuffer().size(), 0, this->width);
        float y = ofMap(hardClip(sample), -1, 1, headerHeight, this->height);
        waveform.addVertex(x, y);
    }

}

//--------------------------------------------------------------
void Mixer::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    waveform.draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void Mixer::removeObjectContent(){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void Mixer::loadAudioSettings(){
    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){
        if (XML.pushTag("settings")){
            sampleRate = XML.getValue("sample_rate_in",0);
            bufferSize = XML.getValue("buffer_size",0);
            XML.popTag();
        }
    }
}

//--------------------------------------------------------------
void Mixer::audioInObject(ofSoundBuffer &inputBuffer){

}

//--------------------------------------------------------------
void Mixer::audioOutObject(ofSoundBuffer &outputBuffer){
    // SIGNAL BUFFER
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}
