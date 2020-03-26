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

#include "pdspChorusEffect.h"

//--------------------------------------------------------------
pdspChorusEffect::pdspChorusEffect() : PatchObject(){

    this->numInlets  = 4;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();  // audio input

    _inletParams[1] = new float();          // speed
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();          // depth
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();          // delay
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
void pdspChorusEffect::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"speed");
    this->addInlet(VP_LINK_NUMERIC,"depth");
    this->addInlet(VP_LINK_NUMERIC,"delay");
    this->addOutlet(VP_LINK_AUDIO,"signal");

    this->setCustomVar(static_cast<float>(0.25),"SPEED");
    this->setCustomVar(static_cast<float>(10),"DEPTH");
    this->setCustomVar(static_cast<float>(80),"DELAY");
}

//--------------------------------------------------------------
void pdspChorusEffect::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onSliderEvent(this, &pdspChorusEffect::onSliderEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    speed = gui->addSlider("Speed", 0,1,0.25);
    speed->setUseCustomMouse(true);
    depth = gui->addSlider("Depth", 0,1,0.01);
    depth->setUseCustomMouse(true);
    delay = gui->addSlider("Delay", 0,1,0.08);
    delay->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);
}

//--------------------------------------------------------------
void pdspChorusEffect::setupAudioOutObjectContent(pdsp::Engine &engine){

    speed_ctrl >> chorus.in_speed();
    speed_ctrl.set(0.25);
    speed_ctrl.enableSmoothing(50.0f);

    depth_ctrl >> chorus.in_depth();
    depth_ctrl.set(10.0f);
    depth_ctrl.enableSmoothing(50.0f);

    delay_ctrl >> chorus.in_delay();
    delay_ctrl.set(80.0f);
    delay_ctrl.enableSmoothing(50.0f);

    this->pdspIn[0] >> chorus.ch(0);

    chorus.ch(0) >> this->pdspOut[0];

    chorus.ch(0) >> scope >> engine.blackhole();
}

//--------------------------------------------------------------
void pdspChorusEffect::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    speed->update();
    depth->update();
    delay->update();

    if(this->inletsConnected[1]){
        speed_ctrl.set(ofClamp(*(float *)&_inletParams[1],0.0f,1.0f));
        speed->setValue(speed_ctrl.get());
    }

    if(this->inletsConnected[2]){
        depth_ctrl.set(ofClamp(*(float *)&_inletParams[2],0.0f,1000.0f));
        depth->setValue(depth_ctrl.get()/1000.0f);
    }

    if(this->inletsConnected[3]){
        delay_ctrl.set(ofClamp(*(float *)&_inletParams[3],0.0f,1000.0f));
        delay->setValue(delay_ctrl.get()/1000.0f);
    }

    if(!loaded){
        loaded = true;
        speed->setValue(this->getCustomVar("SPEED"));
        speed_ctrl.set(ofClamp(speed->getValue(),0.0f,1.0f));
        depth->setValue(this->getCustomVar("DEPTH"));
        depth_ctrl.set(ofClamp(depth->getValue()*1000.0f,0.0f,1000.0f));
        delay->setValue(this->getCustomVar("DELAY"));
        delay_ctrl.set(ofClamp(delay->getValue()*1000.0f,0.0f,1000.0f));
    }

}

//--------------------------------------------------------------
void pdspChorusEffect::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void pdspChorusEffect::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspChorusEffect::loadAudioSettings(){
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
void pdspChorusEffect::audioInObject(ofSoundBuffer &inputBuffer){

}

//--------------------------------------------------------------
void pdspChorusEffect::audioOutObject(ofSoundBuffer &outputBuffer){
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}

//--------------------------------------------------------------
void pdspChorusEffect::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    speed->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    depth->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    delay->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || delay->hitTest(_m-this->getPos()) || depth->hitTest(_m-this->getPos()) || speed->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void pdspChorusEffect::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        speed->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        depth->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        delay->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void pdspChorusEffect::onSliderEvent(ofxDatGuiSliderEvent e){
    if(e.target == speed){
        this->setCustomVar(static_cast<float>(e.value),"SPEED");
        speed_ctrl.set(static_cast<float>(e.value));
    }else if(e.target == depth){
        this->setCustomVar(static_cast<float>(e.value),"DEPTH");
        depth_ctrl.set(ofClamp(static_cast<float>(e.value)*1000.0f,0.0f,1000.0f));
    }else if(e.target == delay){
        this->setCustomVar(static_cast<float>(e.value),"DELAY");
        delay_ctrl.set(ofClamp(static_cast<float>(e.value)*1000.0f,0.0f,1000.0f));
    }

}

OBJECT_REGISTER( pdspChorusEffect, "chorus", OFXVP_OBJECT_CAT_SOUND)
