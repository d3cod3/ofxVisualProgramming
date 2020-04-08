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

#include "pdspDelay.h"

#define DELAY_MAX_TIME 10000

//--------------------------------------------------------------
pdspDelay::pdspDelay() : PatchObject(){

    this->numInlets  = 4;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();  // audio input

    _inletParams[1] = new float();          // time
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();          // damping
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();          // feedback
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
void pdspDelay::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"time");
    this->addInlet(VP_LINK_NUMERIC,"damping");
    this->addInlet(VP_LINK_NUMERIC,"feedback");
    this->addOutlet(VP_LINK_AUDIO,"delayedSignal");

    this->setCustomVar(static_cast<float>(0.1),"TIME");
    this->setCustomVar(static_cast<float>(0),"DAMPING");
    this->setCustomVar(static_cast<float>(0),"FEEDBACK");
}

//--------------------------------------------------------------
void pdspDelay::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onSliderEvent(this, &pdspDelay::onSliderEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    time = gui->addSlider("Time", 0,1,0.1);
    time->setUseCustomMouse(true);
    damping = gui->addSlider("Damping", 0,1,0);
    damping->setUseCustomMouse(true);
    feedback = gui->addSlider("Feedback", 0,1,0);
    feedback->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);
}

//--------------------------------------------------------------
void pdspDelay::setupAudioOutObjectContent(pdsp::Engine &engine){

    delay.setMaxTime(1000);

    time_ctrl >> delay.in_time();
    time_ctrl.set(1000);
    time_ctrl.enableSmoothing(50.0f);

    damping_ctrl >> delay.in_damping();
    damping_ctrl.set(0.0f);
    damping_ctrl.enableSmoothing(50.0f);

    feedback_ctrl >> delay.in_feedback();
    feedback_ctrl.set(0.0f);
    feedback_ctrl.enableSmoothing(50.0f);

    this->pdspIn[0] >> delay.in_signal();

    delay.out_signal() >> this->pdspOut[0];

    delay.out_signal() >> scope >> engine.blackhole();
}

//--------------------------------------------------------------
void pdspDelay::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    time->update();
    damping->update();
    feedback->update();

    if(this->inletsConnected[1]){
        time_ctrl.set(ofClamp(*(float *)&_inletParams[1],0.0f,DELAY_MAX_TIME));
        time->setValue(time_ctrl.get()/DELAY_MAX_TIME);
    }

    if(this->inletsConnected[2]){
        damping_ctrl.set(ofClamp(*(float *)&_inletParams[2],0.0f,1.0f));
        damping->setValue(damping_ctrl.get());
    }

    if(this->inletsConnected[3]){
        feedback_ctrl.set(ofClamp(*(float *)&_inletParams[3],0.0f,1.0f));
        feedback->setValue(feedback_ctrl.get());
    }

    if(!loaded){
        loaded = true;
        time->setValue(this->getCustomVar("TIME"));
        time_ctrl.set(ofClamp(time->getValue()*DELAY_MAX_TIME,0.0f,DELAY_MAX_TIME));
        damping->setValue(this->getCustomVar("DAMPING"));
        damping_ctrl.set(ofClamp(damping->getValue(),0.0f,1.0f));
        feedback->setValue(this->getCustomVar("FEEDBACK"));
        feedback_ctrl.set(ofClamp(feedback->getValue(),0.0f,1.0f));
    }

}

//--------------------------------------------------------------
void pdspDelay::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void pdspDelay::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspDelay::loadAudioSettings(){
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
void pdspDelay::audioInObject(ofSoundBuffer &inputBuffer){

}

//--------------------------------------------------------------
void pdspDelay::audioOutObject(ofSoundBuffer &outputBuffer){
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}

//--------------------------------------------------------------
void pdspDelay::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    time->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    damping->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    feedback->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || damping->hitTest(_m-this->getPos()) || feedback->hitTest(_m-this->getPos()) || time->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void pdspDelay::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        time->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        damping->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        feedback->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }else{
        ofNotifyEvent(dragEvent, nId);

        box->setFromCenter(_m.x, _m.y,box->getWidth(),box->getHeight());
        headerBox->set(box->getPosition().x,box->getPosition().y,box->getWidth(),headerHeight);

        x = box->getPosition().x;
        y = box->getPosition().y;

        for(int j=0;j<static_cast<int>(outPut.size());j++){
            // (outPut[j]->posFrom.x,outPut[j]->posFrom.y);
            // (outPut[j]->posFrom.x+20,outPut[j]->posFrom.y);
        }
    }
}

//--------------------------------------------------------------
void pdspDelay::onSliderEvent(ofxDatGuiSliderEvent e){
    if(e.target == time){
        this->setCustomVar(static_cast<float>(e.value),"TIME");
        time_ctrl.set(static_cast<float>(e.value)*DELAY_MAX_TIME);
    }else if(e.target == damping){
        this->setCustomVar(static_cast<float>(e.value),"DAMPING");
        damping_ctrl.set(ofClamp(static_cast<float>(e.value),0.0f,1.0f));
    }else if(e.target == feedback){
        this->setCustomVar(static_cast<float>(e.value),"FEEDBACK");
        feedback_ctrl.set(ofClamp(static_cast<float>(e.value),0.0f,1.0f));
    }
    
}

OBJECT_REGISTER( pdspDelay, "delay", OFXVP_OBJECT_CAT_SOUND)

#endif