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

#include "pdspLowCut.h"

//--------------------------------------------------------------
pdspLowCut::pdspLowCut() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();  // audio input
    _inletParams[1] = new float();          // gain
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output

    this->initInletsState();

    this->width *= 2;

    freqINFO                = new ofImage();

    isGUIObject             = true;
    this->isOverGUI         = true;

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    loaded                  = false;
}

//--------------------------------------------------------------
void pdspLowCut::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"freq");
    this->addOutlet(VP_LINK_AUDIO,"filteredSignal");

    this->setCustomVar(static_cast<float>(20),"FREQUENCY");
}

//--------------------------------------------------------------
void pdspLowCut::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onSliderEvent(this, &pdspLowCut::onSliderEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    slider = gui->addSlider("freq",20,20000,20);
    slider->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    freqINFO->load("images/freq_graph.png");
}

//--------------------------------------------------------------
void pdspLowCut::setupAudioOutObjectContent(pdsp::Engine &engine){
    freq_ctrl >> filter.in_freq();
    freq_ctrl.set(20.0f);
    freq_ctrl.enableSmoothing(50.0f);

    this->pdspIn[0] >> filter >> this->pdspOut[0];
    this->pdspIn[0] >> filter >> scope >> engine.blackhole();
}

//--------------------------------------------------------------
void pdspLowCut::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    slider->update();

    if(this->inletsConnected[1]){
        freq_ctrl.set(ofClamp(*(float *)&_inletParams[1],20.0f,20000.0f));
        slider->setValue(freq_ctrl.get());
    }

    if(!loaded){
        loaded = true;
        slider->setValue(this->getCustomVar("FREQUENCY"));
        freq_ctrl.set(ofClamp(slider->getValue(),20.0f,20000.0f));
    }

}

//--------------------------------------------------------------
void pdspLowCut::drawObjectContent(ofxFontStash *font){
    ofSetColor(0);
    ofDrawRectangle(0,0,this->width,this->height);
    ofEnableAlphaBlending();
    ofSetColor(255,255,120);
    ofSetLineWidth(3);
    ofDrawLine(0,this->height-20,ofMap(freq_ctrl.get(),20.0f,20000.0f,10,this->width),this->height/3);
    ofDrawLine(ofMap(freq_ctrl.get(),20.0f,20000.0f,10,this->width),this->height/3,this->width,this->height/3);
    ofSetLineWidth(1);
    ofSetColor(255);
    freqINFO->draw(0,this->height/2);
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void pdspLowCut::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspLowCut::loadAudioSettings(){
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
void pdspLowCut::audioInObject(ofSoundBuffer &inputBuffer){

}

//--------------------------------------------------------------
void pdspLowCut::audioOutObject(ofSoundBuffer &outputBuffer){
    // SIGNAL BUFFER
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}

//--------------------------------------------------------------
void pdspLowCut::mouseMovedObjectContent(ofVec3f _m){
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
void pdspLowCut::dragGUIObject(ofVec3f _m){
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
void pdspLowCut::onSliderEvent(ofxDatGuiSliderEvent e){
    this->setCustomVar(static_cast<float>(e.value),"FREQUENCY");
    freq_ctrl.set(ofClamp(static_cast<float>(e.value),20.0f,20000.0f));
}

OBJECT_REGISTER( pdspLowCut, "hi pass", OFXVP_OBJECT_CAT_SOUND);

#endif
