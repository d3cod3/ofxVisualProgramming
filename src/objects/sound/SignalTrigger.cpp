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

#include "SignalTrigger.h"

//--------------------------------------------------------------
SignalTrigger::SignalTrigger() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();  // audio in
    _inletParams[1] = new float();          // threshold
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new float();         // signal trigger --> bang
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    isGUIObject             = true;
    this->isOverGUI         = true;

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    loaded                  = false;

    bang                    = false;

}

//--------------------------------------------------------------
void SignalTrigger::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"thresh");
    this->addOutlet(VP_LINK_NUMERIC,"bang");

    this->setCustomVar(0.5f,"THRESHOLD");
}

//--------------------------------------------------------------
void SignalTrigger::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onSliderEvent(this, &SignalTrigger::onSliderEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    slider = gui->addSlider("Thresh", 0,1,0);
    slider->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);
}

//--------------------------------------------------------------
void SignalTrigger::setupAudioOutObjectContent(pdsp::Engine &engine){

    thresh_ctrl >> toTrigger.in_threshold();
    thresh_ctrl.set(0.5f);
    thresh_ctrl.enableSmoothing(10.0f);

    this->pdspIn[0] >> peakDetector >> follower.out_signal() >> toTrigger.out_trig() >> engine.blackhole();

}

//--------------------------------------------------------------
void SignalTrigger::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    slider->update();

    if(this->inletsConnected[1]){
        thresh_ctrl.set(ofClamp(*(float *)&_inletParams[1],0.0f,1.0f));
        slider->setValue(thresh_ctrl.get());
    }

    if(toTrigger.meter_gate() > 0.0f){
        // trigger on
        *(float *)&_outletParams[0] = 1.0f;
        bang = true;
    }else{
        // trigger off
        *(float *)&_outletParams[0] = 0.0f;
        bang = false;
    }

    if(!loaded){
        loaded = true;
        slider->setValue(this->getCustomVar("THRESHOLD"));
        thresh_ctrl.set(ofClamp(slider->getValue(),0.0f,1.0f));
    }

}

//--------------------------------------------------------------
void SignalTrigger::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(bang){
        ofSetColor(250,250,5);
        ofDrawRectangle(0,0,this->width,this->height);
    }
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void SignalTrigger::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void SignalTrigger::loadAudioSettings(){
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
void SignalTrigger::audioInObject(ofSoundBuffer &inputBuffer){

}

//--------------------------------------------------------------
void SignalTrigger::audioOutObject(ofSoundBuffer &outputBuffer){

}

//--------------------------------------------------------------
void SignalTrigger::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    slider->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || slider->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void SignalTrigger::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void SignalTrigger::onSliderEvent(ofxDatGuiSliderEvent e){
    this->setCustomVar(static_cast<float>(e.value),"THRESHOLD");
    thresh_ctrl.set(ofClamp(static_cast<float>(e.value),0.0f,1.0f));
}

OBJECT_REGISTER( SignalTrigger, "signal trigger", OFXVP_OBJECT_CAT_SOUND);
