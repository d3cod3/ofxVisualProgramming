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

#include "pdspCompressor.h"

//--------------------------------------------------------------
pdspCompressor::pdspCompressor() : PatchObject(){

    this->numInlets  = 6;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer(); // audio input

    _inletParams[1] = new float();          // attack
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();          // release
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();          // thresh
    *(float *)&_inletParams[3] = 0.0f;
    _inletParams[4] = new float();          // ratio
    *(float *)&_inletParams[4] = 0.0f;
    _inletParams[5] = new float();          // knee
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
void pdspCompressor::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"attack");
    this->addInlet(VP_LINK_NUMERIC,"release");
    this->addInlet(VP_LINK_NUMERIC,"thresh");
    this->addInlet(VP_LINK_NUMERIC,"ratio");
    this->addInlet(VP_LINK_NUMERIC,"knee");
    this->addOutlet(VP_LINK_AUDIO,"compressedSignal");

    this->setCustomVar(0.0f,"ATTACK");
    this->setCustomVar(0.0f,"RELEASE");
    this->setCustomVar(0.0f,"THRESH");
    this->setCustomVar(0.0f,"RATIO");
    this->setCustomVar(0.0f,"KNEE");
}

//--------------------------------------------------------------
void pdspCompressor::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onSliderEvent(this, &pdspCompressor::onSliderEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    attack = gui->addSlider("Attack", 0,1,this->getCustomVar("ATTACK"));
    attack->setUseCustomMouse(true);
    release = gui->addSlider("Release", 0,1,this->getCustomVar("RELEASE"));
    release->setUseCustomMouse(true);
    thresh = gui->addSlider("Thresh", 0,1,this->getCustomVar("THRESH"));
    thresh->setUseCustomMouse(true);
    ratio = gui->addSlider("Ratio", 0,1,this->getCustomVar("RATIO"));
    ratio->setUseCustomMouse(true);
    knee = gui->addSlider("Knee", 0,1,this->getCustomVar("KNEE"));
    knee->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

}

//--------------------------------------------------------------
void pdspCompressor::setupAudioOutObjectContent(pdsp::Engine &engine){

    attack_ctrl >> compressor.in_attack(); // 1 - 100 ms
    attack_ctrl.set(12);
    attack_ctrl.enableSmoothing(50.0f);

    release_ctrl >> compressor.in_release();// 1 - 100 ms
    release_ctrl.set(12);
    release_ctrl.enableSmoothing(50.0f);

    thresh_ctrl >> compressor.in_threshold(); // -48dB - 0dB
    thresh_ctrl.set(dB(-20.0f));
    thresh_ctrl.enableSmoothing(50.0f);

    ratio_ctrl >> compressor.in_ratio(); // 1 - 100
    ratio_ctrl.set(12);
    ratio_ctrl.enableSmoothing(50.0f);

    knee_ctrl >> compressor.in_knee(); // -48dB - 0dB
    knee_ctrl.set(dB(0.0f));
    knee_ctrl.enableSmoothing(50.0f);


    this->pdspIn[0] >> compressor.ch(0) >> this->pdspOut[0];
    this->pdspIn[0] >> compressor.ch(0) >> scope >> engine.blackhole();

}

//--------------------------------------------------------------
void pdspCompressor::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    gui->update();
    header->update();
    attack->update();
    release->update();
    thresh->update();
    ratio->update();
    knee->update();

    // attack
    if(this->inletsConnected[1]){
        if(attack->getValue() != *(float *)&_inletParams[1]){
            attack->setValue(ofMap(ofClamp(*(float *)&_inletParams[1],0.0f,1.0f),0.0f,1.0f,1.0f,100.0f,true));
            attack_ctrl.set(attack->getValue());
            this->setCustomVar(ofClamp(*(float *)&_inletParams[1],0.0f,1.0f),"ATTACK");
        }
    }

    // release
    if(this->inletsConnected[2]){
        if(release->getValue() != *(float *)&_inletParams[2]){
            release->setValue(ofMap(ofClamp(*(float *)&_inletParams[2],0.0f,1.0f),0.0f,1.0f,1.0f,100.0f,true));
            release_ctrl.set(release->getValue());
            this->setCustomVar(ofClamp(*(float *)&_inletParams[2],0.0f,1.0f),"RELEASE");
        }
    }

    // thresh
    if(this->inletsConnected[3]){
        if(thresh->getValue() != *(float *)&_inletParams[3]){
            thresh->setValue(ofMap(ofClamp(*(float *)&_inletParams[3],0.0f,1.0f),0.0f,1.0f,-48.0f,0.0f,true));
            thresh_ctrl.set(thresh->getValue());
            this->setCustomVar(ofClamp(*(float *)&_inletParams[3],0.0f,1.0f),"THRESH");
        }
    }

    // ratio
    if(this->inletsConnected[4]){
        if(ratio->getValue() != *(float *)&_inletParams[4]){
            ratio->setValue(ofMap(ofClamp(*(float *)&_inletParams[4],0.0f,1.0f),0.0f,1.0f,1.0f,100.0f,true));
            ratio_ctrl.set(ratio->getValue());
            this->setCustomVar(ofClamp(*(float *)&_inletParams[4],0.0f,1.0f),"RATIO");
        }
    }

    // knee
    if(this->inletsConnected[5]){
        if(knee->getValue() != *(float *)&_inletParams[5]){
            knee->setValue(ofMap(ofClamp(*(float *)&_inletParams[5],0.0f,1.0f),0.0f,1.0f,-48.0f,0.0f,true));
            knee_ctrl.set(knee->getValue());
            this->setCustomVar(ofClamp(*(float *)&_inletParams[5],0.0f,1.0f),"KNEE");
        }
    }

    if(!loaded){
        loaded = true;
        attack->setValue(this->getCustomVar("ATTACK"));
        release->setValue(this->getCustomVar("RELEASE"));
        thresh->setValue(this->getCustomVar("THRESH"));
        ratio->setValue(this->getCustomVar("RATIO"));
        knee->setValue(this->getCustomVar("KNEE"));
    }
}

//--------------------------------------------------------------
void pdspCompressor::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void pdspCompressor::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspCompressor::loadAudioSettings(){
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
void pdspCompressor::audioOutObject(ofSoundBuffer &outputBuffer){
    // SIGNAL BUFFER
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}

//--------------------------------------------------------------
void pdspCompressor::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    attack->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    release->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    thresh->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    ratio->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    knee->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));


    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || attack->hitTest(_m-this->getPos()) || release->hitTest(_m-this->getPos())
                                        || thresh->hitTest(_m-this->getPos()) || ratio->hitTest(_m-this->getPos()) || knee->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void pdspCompressor::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        attack->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        release->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        thresh->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        ratio->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        knee->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    }else{
        

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
void pdspCompressor::onSliderEvent(ofxDatGuiSliderEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == attack){
            attack_ctrl.set(ofMap(ofClamp(static_cast<float>(e.value),0.0f,1.0f),0.0f,1.0f,1.0f,100.0f,true));
            this->setCustomVar(ofClamp(static_cast<float>(e.value),0.0f,1.0f),"ATTACK");
        }else if(e.target == release){
            release_ctrl.set(ofMap(ofClamp(static_cast<float>(e.value),0.0f,1.0f),0.0f,1.0f,1.0f,100.0f,true));
            this->setCustomVar(ofClamp(static_cast<float>(e.value),0.0f,1.0f),"RELEASE");
        }else if(e.target == thresh){
            thresh_ctrl.set(ofMap(ofClamp(static_cast<float>(e.value),0.0f,1.0f),0.0f,1.0f,-48.0f,0.0f,true));
            this->setCustomVar(ofClamp(static_cast<float>(e.value),0.0f,1.0f),"THRESH");
        }else if(e.target == ratio){
            ratio_ctrl.set(ofMap(ofClamp(static_cast<float>(e.value),0.0f,1.0f),0.0f,1.0f,1.0f,100.0f,true));
            this->setCustomVar(ofClamp(static_cast<float>(e.value),0.0f,1.0f),"RATIO");
        }else if(e.target == knee){
            knee_ctrl.set(ofMap(ofClamp(static_cast<float>(e.value),0.0f,1.0f),0.0f,1.0f,-48.0f,0.0f,true));
            this->setCustomVar(ofClamp(static_cast<float>(e.value),0.0f,1.0f),"KNEEE");
        }
    }
}

OBJECT_REGISTER( pdspCompressor, "compressor", OFXVP_OBJECT_CAT_SOUND)

#endif