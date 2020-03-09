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

#include "pdspLFO.h"

//--------------------------------------------------------------
pdspLFO::pdspLFO() : PatchObject(){

    this->numInlets  = 3;
    this->numOutlets = 5;

    _inletParams[0] = new float();  // retrig (bang)
    *(float *)&_inletParams[0] = 0.0f;
    _inletParams[1] = new float();  // frequency
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();  // phase
    *(float *)&_inletParams[2] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // triangle LFO
    _outletParams[1] = new ofSoundBuffer(); // sine     LFO
    _outletParams[2] = new ofSoundBuffer(); // saw      LFO
    _outletParams[3] = new ofSoundBuffer(); // square   LFO
    _outletParams[4] = new ofSoundBuffer(); // random   LFO

    this->initInletsState();

    isGUIObject             = true;
    this->isOverGUI         = true;

    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    loaded                  = false;

}

//--------------------------------------------------------------
void pdspLFO::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"frequency");
    this->addInlet(VP_LINK_NUMERIC,"phase");
    this->addOutlet(VP_LINK_AUDIO,"triangle");
    this->addOutlet(VP_LINK_AUDIO,"sine");
    this->addOutlet(VP_LINK_AUDIO,"saw");
    this->addOutlet(VP_LINK_AUDIO,"square");
    this->addOutlet(VP_LINK_AUDIO,"random");

    this->setCustomVar(0.5f,"FREQUENCY");
    this->setCustomVar(0.0f,"PHASE");
}

//--------------------------------------------------------------
void pdspLFO::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onSliderEvent(this, &pdspLFO::onSliderEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    oscInfo = gui->addLabel("0 Hz");
    gui->addBreak();
    slider = gui->addSlider("Freq",0,5,this->getCustomVar("FREQUENCY"));
    slider->setUseCustomMouse(true);
    sliderPhase = gui->addSlider("Phase",-1.0f,1.0f,this->getCustomVar("PHASE"));
    sliderPhase->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);
}

//--------------------------------------------------------------
void pdspLFO::setupAudioOutObjectContent(pdsp::Engine &engine){

    retrig_ctrl.out_trig() >> lfo.in_retrig();

    pitch_ctrl >> lfo.in_freq();
    pitch_ctrl.set(0.5f);
    pitch_ctrl.enableSmoothing(50.0f);

    phase_ctrl >> lfo.in_phase_start();
    phase_ctrl.set(0.0f);
    phase_ctrl.enableSmoothing(50.0f);

    lfo.out_triangle() >> this->pdspOut[0];
    lfo.out_triangle() >> scope_tri >> engine.blackhole();

    lfo.out_sine() >> this->pdspOut[1];
    lfo.out_sine() >> scope_sine >> engine.blackhole();

    lfo.out_saw() >> this->pdspOut[2];
    lfo.out_saw() >> scope_saw >> engine.blackhole();

    lfo.out_square() >> this->pdspOut[3];
    lfo.out_square() >> scope_square >> engine.blackhole();

    lfo.out_random() >> this->pdspOut[4];
    lfo.out_random() >> scope_random >> engine.blackhole();
}

//--------------------------------------------------------------
void pdspLFO::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    slider->update();
    sliderPhase->update();

    // retrig
    if(this->inletsConnected[0]){
        retrig_ctrl.trigger(ofClamp(*(float *)&_inletParams[0],0.0f,1.0f));
    }else{
        retrig_ctrl.off();
    }

    // frequency
    if(this->inletsConnected[1]){
        pitch_ctrl.set(ofClamp(*(float *)&_inletParams[1],0.0f,5.0f));
        slider->setValue(pitch_ctrl.get());
        oscInfo->setLabel(ofToString(ofClamp(*(float *)&_inletParams[1],0.0f,5.0f)) + " Hz");
    }

    // phase
    if(this->inletsConnected[2]){
        phase_ctrl.set(ofClamp(*(float *)&_inletParams[2],-1.0f,1.0f));
        sliderPhase->setValue(phase_ctrl.get());
    }

    if(!loaded){
        loaded = true;
        slider->setValue(this->getCustomVar("FREQUENCY"));
        oscInfo->setLabel(ofToString(ofClamp(this->getCustomVar("FREQUENCY"),0.0f,5.0f)) + " Hz");
        sliderPhase->setValue(this->getCustomVar("PHASE"));
        pitch_ctrl.set(ofClamp(slider->getValue(),0.0f,5.0f));
        phase_ctrl.set(ofClamp(sliderPhase->getValue(),-1.0,1.0));
    }

}

//--------------------------------------------------------------
void pdspLFO::drawObjectContent(ofxFontStash *font){
    ofSetColor(0);
    ofDrawRectangle(0,0,this->width,this->height);
    ofEnableAlphaBlending();
    ofSetColor(255);
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void pdspLFO::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspLFO::loadAudioSettings(){
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
void pdspLFO::audioOutObject(ofSoundBuffer &outputBuffer){
    // SIGNAL BUFFER
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope_tri.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[1])->copyFrom(scope_sine.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[2])->copyFrom(scope_saw.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[3])->copyFrom(scope_square.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[4])->copyFrom(scope_random.getBuffer().data(), bufferSize, 1, sampleRate);
}

//--------------------------------------------------------------
void pdspLFO::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    oscInfo->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    slider->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    sliderPhase->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || oscInfo->hitTest(_m-this->getPos()) || slider->hitTest(_m-this->getPos()) || sliderPhase->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void pdspLFO::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        oscInfo->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        slider->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        sliderPhase->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void pdspLFO::onSliderEvent(ofxDatGuiSliderEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == slider){
            this->setCustomVar(static_cast<float>(e.value),"FREQUENCY");
            pitch_ctrl.set(ofClamp(static_cast<float>(e.value),0.0f,5.0f));
            oscInfo->setLabel(ofToString(ofClamp(static_cast<float>(e.value),0.0f,5.0f)) + " Hz");
        }else if(e.target == sliderPhase){
            this->setCustomVar(static_cast<float>(e.value),"PHASE");
            phase_ctrl.set(ofClamp(static_cast<float>(e.value),-1.0f,1.0f));
        }
    }

}

OBJECT_REGISTER( pdspLFO, "lfo", OFXVP_OBJECT_CAT_SOUND);
