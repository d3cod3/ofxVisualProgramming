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

#include "pdspBitNoise.h"

//--------------------------------------------------------------
pdspBitNoise::pdspBitNoise() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 2;

    _inletParams[0] = new float();          // pitch
    *(float *)&_inletParams[0] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output
    _outletParams[1] = new vector<float>(); // audio buffer

    this->initInletsState();

    isGUIObject             = true;
    this->isOverGUI         = true;

    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    loaded                  = false;

}

//--------------------------------------------------------------
void pdspBitNoise::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"pitch");
    this->addOutlet(VP_LINK_AUDIO,"signal");
    this->addOutlet(VP_LINK_ARRAY,"dataBuffer");

    this->setCustomVar(static_cast<float>(-100),"PITCH");
}

//--------------------------------------------------------------
void pdspBitNoise::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onSliderEvent(this, &pdspBitNoise::onSliderEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    pitch = gui->addSlider("Pitch", -100,150,this->getCustomVar("PITCH"));
    pitch->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);
}

//--------------------------------------------------------------
void pdspBitNoise::setupAudioOutObjectContent(pdsp::Engine &engine){

    pitch_ctrl >> noise.in_pitch();
    pitch_ctrl.set(-100.0f);
    pitch_ctrl.enableSmoothing(50.0f);

    noise.ch_noise(0) >> this->pdspOut[0];
    noise.ch_noise(0) >> scope >> engine.blackhole();
}

//--------------------------------------------------------------
void pdspBitNoise::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    pitch->update();

    if(this->inletsConnected[0]){
        pitch_ctrl.set(ofClamp(*(float *)&_inletParams[0],-100,150));
        pitch->setValue(pitch_ctrl.get());
    }

    if(!loaded){
        loaded = true;
        pitch->setValue(this->getCustomVar("PITCH"));
        pitch_ctrl.set(ofClamp(pitch->getValue(),-100,150));
    }

}

//--------------------------------------------------------------
void pdspBitNoise::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(0);
    ofDrawRectangle(0,0,this->width,this->height);
    ofEnableAlphaBlending();
    ofSetColor(255,255,120);
    waveform.draw();
    ofSetColor(255);
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void pdspBitNoise::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspBitNoise::loadAudioSettings(){
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
void pdspBitNoise::audioOutObject(ofSoundBuffer &outputBuffer){
    waveform.clear();
    for(size_t i = 0; i < scope.getBuffer().size(); i++) {
        float sample = scope.getBuffer().at(i);
        float x = ofMap(i, 0, scope.getBuffer().size(), 0, this->width);
        float y = ofMap(hardClip(sample), -1, 1, headerHeight, this->height);
        waveform.addVertex(x, y);

        // SIGNAL BUFFER DATA
        static_cast<vector<float> *>(_outletParams[1])->at(i) = sample;
    }
    // SIGNAL BUFFER
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}

//--------------------------------------------------------------
void pdspBitNoise::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    pitch->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || pitch->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void pdspBitNoise::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        pitch->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }else{
        ofNotifyEvent(dragEvent, nId);

        box->setFromCenter(_m.x, _m.y,box->getWidth(),box->getHeight());
        headerBox->set(box->getPosition().x,box->getPosition().y,box->getWidth(),headerHeight);

        x = box->getPosition().x;
        y = box->getPosition().y;

        for(int j=0;j<static_cast<int>(outPut.size());j++){
            outPut[j]->linkVertices[0].move(outPut[j]->posFrom.x,outPut[j]->posFrom.y);
            outPut[j]->linkVertices[1].move(outPut[j]->posFrom.x+20,outPut[j]->posFrom.y);
        }
    }
}

//--------------------------------------------------------------
void pdspBitNoise::onSliderEvent(ofxDatGuiSliderEvent e){
    if(e.target == pitch){
        this->setCustomVar(static_cast<float>(e.value),"PITCH");
        pitch_ctrl.set(static_cast<float>(e.value));
    }
}

OBJECT_REGISTER( pdspBitNoise, "bit noise", OFXVP_OBJECT_CAT_SOUND)

#endif