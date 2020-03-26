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

#include "pdspDataOscillator.h"

//--------------------------------------------------------------
pdspDataOscillator::pdspDataOscillator() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 2;

    _inletParams[0] = new float();  // pitch
    *(float *)&_inletParams[0] = 0.0f;

    _inletParams[1] = new vector<float>(); // data

    _outletParams[0] = new ofSoundBuffer(); // audio output
    _outletParams[1] = new vector<float>(); // audio buffer

    this->initInletsState();

    isGUIObject             = true;
    this->isOverGUI         = true;

    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    loaded                  = false;
    reinitDataTable         = false;

}

//--------------------------------------------------------------
void pdspDataOscillator::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"pitch");
    this->addInlet(VP_LINK_ARRAY,"data");
    this->addOutlet(VP_LINK_AUDIO,"signal");
    this->addOutlet(VP_LINK_ARRAY,"dataBuffer");

    this->setCustomVar(static_cast<float>(30),"PITCH");
}

//--------------------------------------------------------------
void pdspDataOscillator::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onSliderEvent(this, &pdspDataOscillator::onSliderEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    oscInfo = gui->addLabel("0 Hz");
    gui->addBreak();
    slider = gui->addSlider("Pitch",0,127,30);
    slider->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);
}

//--------------------------------------------------------------
void pdspDataOscillator::setupAudioOutObjectContent(pdsp::Engine &engine){
    // we filter the frequency below 20 hz (not audible) just to remove DC offsets
    20.0f >> leakDC.in_freq();

    pitch_ctrl >> osc.in_pitch();
    pitch_ctrl.set(72.0f);
    pitch_ctrl.enableSmoothing(50.0f);

    datatable.setup(256,256);
    datatable.smoothing(0.5f);
    osc.setTable( datatable );

    osc >> leakDC >> this->pdspOut[0];
    osc >> leakDC >> scope >> engine.blackhole();
}

//--------------------------------------------------------------
void pdspDataOscillator::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    slider->update();

    // PITCH
    if(this->inletsConnected[0]){
        pitch_ctrl.set(ofClamp(*(float *)&_inletParams[0],0,127));
        slider->setValue(pitch_ctrl.get());
        oscInfo->setLabel(ofToString(pdsp::PitchToFreq::eval(ofClamp(*(float *)&_inletParams[0],0,127))) + " Hz");
    }

    // DATA
    if(datatable.ready()){
        if(this->inletsConnected[1]){
            if(datatable.ready()){
                datatable.begin();
                if(static_cast<vector<float> *>(_inletParams[1])->size() <= datatable.getTableLength()){
                    for(int n=0; n<static_cast<int>(static_cast<vector<float> *>(_inletParams[1])->size()); n++){
                        int pos = static_cast<int>(floor(ofMap(n,0,static_cast<int>(static_cast<vector<float> *>(_inletParams[1])->size()),0,datatable.getTableLength())));
                        float sample = ofMap(static_cast<vector<float> *>(_inletParams[1])->at(n), 0.0f, 1.0f, -0.5f, 0.5f);
                        datatable.data(pos, sample);
                    }
                }else{
                    for(int n=0; n<datatable.getTableLength(); ++n){
                        int pos = static_cast<int>(floor(ofMap(n,0,datatable.getTableLength(),0,static_cast<int>(static_cast<vector<float> *>(_inletParams[1])->size()))));
                        float sample = ofMap(static_cast<vector<float> *>(_inletParams[1])->at(pos), 0.0f, 1.0f, -0.5f, 0.5f);
                        datatable.data(n, sample);
                    }
                }

                datatable.end(false);
            }
        }else{
            if(datatable.ready()){
                datatable.begin();
                for(int n=0; n<datatable.getTableLength(); ++n){
                    float sample = 0.0f;
                    datatable.data(n, sample);
                }
                datatable.end(false);
            }
        }

    }

    if(!loaded){
        loaded = true;
        slider->setValue(this->getCustomVar("PITCH"));
        oscInfo->setLabel(ofToString(pdsp::PitchToFreq::eval(ofClamp(this->getCustomVar("PITCH"),0,127))) + " Hz");
        pitch_ctrl.set(ofClamp(slider->getValue(),0,127));
    }

}

//--------------------------------------------------------------
void pdspDataOscillator::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    waveform.draw();
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void pdspDataOscillator::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspDataOscillator::loadAudioSettings(){
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
void pdspDataOscillator::audioOutObject(ofSoundBuffer &outputBuffer){
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
void pdspDataOscillator::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    oscInfo->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    slider->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || oscInfo->hitTest(_m-this->getPos()) || slider->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void pdspDataOscillator::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        oscInfo->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        slider->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void pdspDataOscillator::onSliderEvent(ofxDatGuiSliderEvent e){
    this->setCustomVar(static_cast<float>(e.value),"PITCH");
    pitch_ctrl.set(ofClamp(static_cast<float>(e.value),0,127));
    oscInfo->setLabel(ofToString(pdsp::PitchToFreq::eval(ofClamp(static_cast<float>(e.value),0,127))) + " Hz");
}

OBJECT_REGISTER( pdspDataOscillator, "data oscillator", OFXVP_OBJECT_CAT_SOUND)
