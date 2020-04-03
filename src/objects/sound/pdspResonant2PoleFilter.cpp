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

#include "pdspResonant2PoleFilter.h"

//--------------------------------------------------------------
pdspResonant2PoleFilter::pdspResonant2PoleFilter() : PatchObject(){

    this->numInlets  = 4;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();  // audio input

    _inletParams[1] = new float();          // pitch
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();          // cutoff
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();          // resonance
    *(float *)&_inletParams[3] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output

    this->initInletsState();

    isGUIObject             = true;
    this->isOverGUI         = true;

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    loaded                  = false;

}

//--------------------------------------------------------------
void pdspResonant2PoleFilter::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"pitch");
    this->addInlet(VP_LINK_NUMERIC,"cutoff");
    this->addInlet(VP_LINK_NUMERIC,"resonance");
    this->addOutlet(VP_LINK_AUDIO,"filteredSignal");

    this->setCustomVar(static_cast<float>(0.1),"PITCH");
    this->setCustomVar(static_cast<float>(0.1),"CUTOFF");
    this->setCustomVar(static_cast<float>(0),"RESONANCE");
    this->setCustomVar(static_cast<float>(0),"MODE");
}

//--------------------------------------------------------------
void pdspResonant2PoleFilter::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onSliderEvent(this, &pdspResonant2PoleFilter::onSliderEvent);
    gui->onMatrixEvent(this, &pdspResonant2PoleFilter::onMatrixEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    filterMode = gui->addLabel("LowPass");
    filterMode->setUseCustomMouse(true);

    modeSelector = gui->addMatrix("DEVICE",4,true);
    modeSelector->setUseCustomMouse(true);
    modeSelector->setRadioMode(true);
    modeSelector->getChildAt(0)->setSelected(true);
    modeSelector->onMatrixEvent(this, &pdspResonant2PoleFilter::onMatrixEvent);
    gui->addBreak();

    pitch = gui->addSlider("Pitch", 0,1,0.1);
    pitch->setUseCustomMouse(true);
    cutoff = gui->addSlider("Cutoff", 0,1,0.1);
    cutoff->setUseCustomMouse(true);
    resonance = gui->addSlider("Resonance", 0,1,0.0);
    resonance->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);
}

//--------------------------------------------------------------
void pdspResonant2PoleFilter::setupAudioOutObjectContent(pdsp::Engine &engine){

    pitch_ctrl >> filter.in_pitch();
    pitch_ctrl.set(12);
    pitch_ctrl.enableSmoothing(50.0f);

    cutoff_ctrl >> filter.in_cutoff();
    cutoff_ctrl.set(12);
    cutoff_ctrl.enableSmoothing(50.0f);

    resonance_ctrl >> filter.in_reso();
    resonance_ctrl.set(0.0f);
    resonance_ctrl.enableSmoothing(50.0f);

    mode_ctrl >> filter.in_mode();
    mode_ctrl.set(0.0f);

    this->pdspIn[0] >> filter.in_signal();

    filter.out_signal() >> this->pdspOut[0];

    filter.out_signal() >> scope >> engine.blackhole();
}

//--------------------------------------------------------------
void pdspResonant2PoleFilter::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    pitch->update();
    cutoff->update();
    resonance->update();
    modeSelector->update();

    if(this->inletsConnected[1]){
        pitch_ctrl.set(ofClamp(*(float *)&_inletParams[1],0.0f,127.0f));
        pitch->setValue(pitch_ctrl.get()/127.0f);
    }

    if(this->inletsConnected[2]){
        cutoff_ctrl.set(ofClamp(*(float *)&_inletParams[2],0.0f,127.0f));
        cutoff->setValue(cutoff_ctrl.get());
    }

    if(this->inletsConnected[3]){
        resonance_ctrl.set(ofClamp(*(float *)&_inletParams[3],0.0f,1.0f));
        resonance->setValue(resonance_ctrl.get());
    }

    if(!loaded){
        loaded = true;
        pitch->setValue(this->getCustomVar("PITCH"));
        pitch_ctrl.set(ofClamp(pitch->getValue()*127.0f,0.0f,127.0f));
        cutoff->setValue(this->getCustomVar("CUTOFF"));
        cutoff_ctrl.set(ofClamp(cutoff->getValue()*127.0f,0.0f,127.0f));
        resonance->setValue(this->getCustomVar("RESONANCE"));
        resonance_ctrl.set(ofClamp(resonance->getValue(),0.0f,1.0f));
        modeSelector->getChildAt(static_cast<int>(mode_ctrl.get()))->setSelected(true);
    }

}

//--------------------------------------------------------------
void pdspResonant2PoleFilter::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void pdspResonant2PoleFilter::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspResonant2PoleFilter::loadAudioSettings(){
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
void pdspResonant2PoleFilter::audioInObject(ofSoundBuffer &inputBuffer){

}

//--------------------------------------------------------------
void pdspResonant2PoleFilter::audioOutObject(ofSoundBuffer &outputBuffer){
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}

//--------------------------------------------------------------
void pdspResonant2PoleFilter::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    pitch->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    cutoff->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    resonance->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    filterMode->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    modeSelector->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || filterMode->hitTest(_m-this->getPos()) || modeSelector->hitTest(_m-this->getPos()) || cutoff->hitTest(_m-this->getPos()) || resonance->hitTest(_m-this->getPos()) || pitch->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void pdspResonant2PoleFilter::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        pitch->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        cutoff->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        resonance->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        filterMode->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        modeSelector->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void pdspResonant2PoleFilter::onSliderEvent(ofxDatGuiSliderEvent e){
    if(e.target == pitch){
        this->setCustomVar(static_cast<float>(e.value),"PITCH");
        pitch_ctrl.set(static_cast<float>(e.value)*127.0f);
    }else if(e.target == cutoff){
        this->setCustomVar(static_cast<float>(e.value),"CUTOFF");
        cutoff_ctrl.set(static_cast<float>(e.value)*127.0f);
    }else if(e.target == resonance){
        this->setCustomVar(static_cast<float>(e.value),"RESONANCE");
        resonance_ctrl.set(ofClamp(static_cast<float>(e.value),0.0f,1.0f));
    }

}

//--------------------------------------------------------------
void pdspResonant2PoleFilter::onMatrixEvent(ofxDatGuiMatrixEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == modeSelector){
            this->setCustomVar(static_cast<float>(e.child),"MODE");
            if(e.child == 0){
                mode_ctrl.set(0.0f);
                filterMode->setLabel("LowPass");
            }else if(e.child == 1){
                mode_ctrl.set(1.0f);
                filterMode->setLabel("BandPass");
            }else if(e.child == 2){
                mode_ctrl.set(2.0f);
                filterMode->setLabel("HighPass");
            }else if(e.child == 3){
                mode_ctrl.set(3.0f);
                filterMode->setLabel("Notch");
            }
        }
    }
}

OBJECT_REGISTER( pdspResonant2PoleFilter, "resonant 2pole filter", OFXVP_OBJECT_CAT_SOUND)

#endif