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

#include "OscPulse.h"

//--------------------------------------------------------------
OscPulse::OscPulse() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 2;

    _inletParams[0] = new float();  // pitch
    *(float *)&_inletParams[0] = 0.0f;
    _inletParams[1] = new float();  // pulse width
    *(float *)&_inletParams[1] = 0.5f;

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
void OscPulse::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"pitch");
    this->addInlet(VP_LINK_NUMERIC,"pulseWidth");
    this->addOutlet(VP_LINK_AUDIO,"signal");
    this->addOutlet(VP_LINK_ARRAY,"dataBuffer");

    this->setCustomVar(static_cast<float>(30),"PITCH");
    this->setCustomVar(0.5f,"PW");
}

//--------------------------------------------------------------
void OscPulse::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onSliderEvent(this, &OscPulse::onSliderEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    oscInfo = gui->addLabel("0 Hz");
    gui->addBreak();
    slider = gui->addSlider("Pitch",0,127,30);
    slider->setUseCustomMouse(true);
    sliderPW = gui->addSlider("PW",0.0f,1.0f,this->getCustomVar("PW"));
    sliderPW->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);
}

//--------------------------------------------------------------
void OscPulse::setupAudioOutObjectContent(pdsp::Engine &engine){
    pitch_ctrl >> osc.in_pitch();
    pitch_ctrl.set(72.0f);
    pitch_ctrl.enableSmoothing(50.0f);

    pw_ctrl >> osc.in_pw();
    pw_ctrl.set(0.5f);
    pw_ctrl.enableSmoothing(50.0f);

    osc.out_pulse() >> this->pdspOut[0];
    osc.out_pulse() >> scope >> engine.blackhole();
}

//--------------------------------------------------------------
void OscPulse::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    slider->update();
    sliderPW->update();

    if(this->inletsConnected[0]){
        pitch_ctrl.set(ofClamp(*(float *)&_inletParams[0],0,127));
        slider->setValue(pitch_ctrl.get());
        oscInfo->setLabel(ofToString(pdsp::PitchToFreq::eval(ofClamp(*(float *)&_inletParams[0],0,127))) + " Hz");
    }

    if(this->inletsConnected[1]){
        pw_ctrl.set(ofClamp(*(float *)&_inletParams[1],0.0,1.0));
        sliderPW->setValue(pw_ctrl.get());
    }

    if(!loaded){
        loaded = true;
        slider->setValue(this->getCustomVar("PITCH"));
        oscInfo->setLabel(ofToString(pdsp::PitchToFreq::eval(ofClamp(this->getCustomVar("PITCH"),0,127))) + " Hz");
        sliderPW->setValue(this->getCustomVar("PW"));
        pitch_ctrl.set(ofClamp(slider->getValue(),0,127));
        pw_ctrl.set(ofClamp(sliderPW->getValue(),0.0,1.0));
    }

}

//--------------------------------------------------------------
void OscPulse::drawObjectContent(ofxFontStash *font){
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
void OscPulse::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void OscPulse::loadAudioSettings(){
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
void OscPulse::audioOutObject(ofSoundBuffer &outputBuffer){
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
void OscPulse::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    oscInfo->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    slider->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    sliderPW->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || oscInfo->hitTest(_m-this->getPos()) || slider->hitTest(_m-this->getPos()) || sliderPW->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void OscPulse::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        oscInfo->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        slider->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        sliderPW->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void OscPulse::onSliderEvent(ofxDatGuiSliderEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == slider){
            this->setCustomVar(static_cast<float>(e.value),"PITCH");
            pitch_ctrl.set(ofClamp(static_cast<float>(e.value),0,127));
            oscInfo->setLabel(ofToString(pdsp::PitchToFreq::eval(ofClamp(static_cast<float>(e.value),0,127))) + " Hz");
        }else if(e.target == sliderPW){
            this->setCustomVar(static_cast<float>(e.value),"PW");
            pw_ctrl.set(ofClamp(static_cast<float>(e.value),0.0,1.0));
        }
    }

}

OBJECT_REGISTER( OscPulse, "pulse", OFXVP_OBJECT_CAT_SOUND)
