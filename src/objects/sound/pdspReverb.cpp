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

#include "pdspReverb.h"

//--------------------------------------------------------------
pdspReverb::pdspReverb() : PatchObject(){

    this->numInlets  = 6;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();  // audio input

    _inletParams[1] = new float();          // time
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();          // density
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();          // damping
    *(float *)&_inletParams[3] = 0.0f;
    _inletParams[4] = new float();          // modSpeed
    *(float *)&_inletParams[4] = 0.0f;
    _inletParams[5] = new float();          // mosAmount
    *(float *)&_inletParams[5] = 0.0f;

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
void pdspReverb::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"time");
    this->addInlet(VP_LINK_NUMERIC,"density");
    this->addInlet(VP_LINK_NUMERIC,"damping");
    this->addInlet(VP_LINK_NUMERIC,"speed");
    this->addInlet(VP_LINK_NUMERIC,"amount");
    this->addOutlet(VP_LINK_AUDIO,"reverbSignal");

    this->setCustomVar(static_cast<float>(0),"TIME");
    this->setCustomVar(static_cast<float>(0.5),"DENSITY");
    this->setCustomVar(static_cast<float>(0.5),"DAMPING");
    this->setCustomVar(static_cast<float>(0.2),"MODSPEED");
    this->setCustomVar(static_cast<float>(0.8),"MODAMOUNT");
}

//--------------------------------------------------------------
void pdspReverb::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onSliderEvent(this, &pdspReverb::onSliderEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    time = gui->addSlider("Time", 0,60,0.0);
    time->setUseCustomMouse(true);
    density = gui->addSlider("Density", 0,1,0.5);
    density->setUseCustomMouse(true);
    damping = gui->addSlider("Damping", 0,2,0.5);
    damping->setUseCustomMouse(true);
    modSpeed = gui->addSlider("Speed", 0,20,0.5);
    modSpeed->setUseCustomMouse(true);
    modAmount = gui->addSlider("Amount", 0,2,0.5);
    modAmount->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);
}

//--------------------------------------------------------------
void pdspReverb::setupAudioOutObjectContent(pdsp::Engine &engine){

    time_ctrl >> reverb.in_time();
    time_ctrl.set(0.0f);
    time_ctrl.enableSmoothing(50.0f);

    density_ctrl >> reverb.in_density();
    density_ctrl.set(0.5f);
    density_ctrl.enableSmoothing(50.0f);

    damping_ctrl >> reverb.in_damping();
    damping_ctrl.set(0.5f);
    damping_ctrl.enableSmoothing(50.0f);

    modSpeed_ctrl >> reverb.in_mod_freq();
    modSpeed_ctrl.set(0.5f);
    modSpeed_ctrl.enableSmoothing(50.0f);

    modAmount_ctrl >> reverb.in_mod_amount();
    modAmount_ctrl.set(0.5f);
    modAmount_ctrl.enableSmoothing(50.0f);

    this->pdspIn[0] >> reverb.in_signal();

    reverb.ch(0) >> this->pdspOut[0];
    reverb.ch(0) >> scope >> engine.blackhole();
}

//--------------------------------------------------------------
void pdspReverb::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    time->update();
    density->update();
    damping->update();
    modSpeed->update();
    modAmount->update();

    if(this->inletsConnected[1]){
        time_ctrl.set(ofClamp(*(float *)&_inletParams[1],0.0f,60.0f));
        time->setValue(time_ctrl.get());
    }

    if(this->inletsConnected[2]){
        density_ctrl.set(ofClamp(*(float *)&_inletParams[2],0.0f,1.0f));
        density->setValue(density_ctrl.get());
    }

    if(this->inletsConnected[3]){
        damping_ctrl.set(ofClamp(*(float *)&_inletParams[3],0.0f,2.0f));
        damping->setValue(damping_ctrl.get());
    }

    if(this->inletsConnected[4]){
        modSpeed_ctrl.set(ofClamp(*(float *)&_inletParams[4],0.0f,20.0f));
        modSpeed->setValue(modSpeed_ctrl.get());
    }

    if(this->inletsConnected[5]){
        modAmount_ctrl.set(ofClamp(*(float *)&_inletParams[5],0.0f,2.0f));
        modAmount->setValue(modAmount_ctrl.get());
    }

    if(!loaded){
        loaded = true;
        time->setValue(this->getCustomVar("TIME"));
        time_ctrl.set(ofClamp(time->getValue(),0.0f,60.0f));
        density->setValue(this->getCustomVar("DENSITY"));
        density_ctrl.set(ofClamp(density->getValue(),0.0f,1.0f));
        damping->setValue(this->getCustomVar("DAMPING"));
        damping_ctrl.set(ofClamp(damping->getValue(),0.0f,2.0f));
        modSpeed->setValue(this->getCustomVar("MODSPEED"));
        modSpeed_ctrl.set(ofClamp(modSpeed->getValue(),0.0f,20.0f));
        modAmount->setValue(this->getCustomVar("MODAMOUNT"));
        modAmount_ctrl.set(ofClamp(modAmount->getValue(),0.0f,2.0f));
    }

}

//--------------------------------------------------------------
void pdspReverb::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void pdspReverb::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspReverb::loadAudioSettings(){
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
void pdspReverb::audioInObject(ofSoundBuffer &inputBuffer){

}

//--------------------------------------------------------------
void pdspReverb::audioOutObject(ofSoundBuffer &outputBuffer){
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}

//--------------------------------------------------------------
void pdspReverb::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    time->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    density->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    damping->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    modSpeed->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    modAmount->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || damping->hitTest(_m-this->getPos()) || density->hitTest(_m-this->getPos()) || time->hitTest(_m-this->getPos())
                                                             || modSpeed->hitTest(_m-this->getPos()) || modAmount->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void pdspReverb::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        time->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        density->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        damping->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        modSpeed->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        modAmount->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void pdspReverb::onSliderEvent(ofxDatGuiSliderEvent e){
    if(e.target == time){
        this->setCustomVar(static_cast<float>(e.value),"TIME");
        time_ctrl.set(static_cast<float>(e.value));
    }else if(e.target == density){
        this->setCustomVar(static_cast<float>(e.value),"DENSITY");
        density_ctrl.set(static_cast<float>(e.value));
    }else if(e.target == damping){
        this->setCustomVar(static_cast<float>(e.value),"DAMPING");
        damping_ctrl.set(static_cast<float>(e.value));
    }else if(e.target == modSpeed){
        this->setCustomVar(static_cast<float>(e.value),"MODSPEED");
        modSpeed_ctrl.set(static_cast<float>(e.value));
    }else if(e.target == modAmount){
        this->setCustomVar(static_cast<float>(e.value),"MODAMOUNT");
        modAmount_ctrl.set(static_cast<float>(e.value));
    }
    
}

OBJECT_REGISTER( pdspReverb, "reverb", OFXVP_OBJECT_CAT_SOUND)
