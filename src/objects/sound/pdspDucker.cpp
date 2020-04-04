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

#include "pdspDucker.h"

//--------------------------------------------------------------
pdspDucker::pdspDucker() : PatchObject(){

    this->numInlets  = 4;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer(); // audio input

    _inletParams[1] = new float();          // bang
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();          // duration
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();          // ducking
    *(float *)&_inletParams[3] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output

    this->initInletsState();

    this->width *= 2;

    isGUIObject             = true;
    this->isOverGUI         = true;

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    loaded                  = false;

    attackDuration          = 0.05f;
    holdDuration            = 0.45f;
    releaseDuration         = 0.5f;

    envelopeDuration        = 100;

    rect.set(120,this->headerHeight+10,this->width-130,this->height-this->headerHeight-40);

}

//--------------------------------------------------------------
void pdspDucker::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"duration");
    this->addInlet(VP_LINK_NUMERIC,"ducking");
    this->addOutlet(VP_LINK_AUDIO,"duckedSignal");

    this->setCustomVar(static_cast<float>(envelopeDuration),"DURATION");
    this->setCustomVar(-20.0f,"DUCKING");
    this->setCustomVar(1.0f,"ATTACK_CURVE");
    this->setCustomVar(1.0f,"RELEASE_CURVE");

    this->setCustomVar(120.0f,"ATTACK_X");
    this->setCustomVar(this->headerHeight+10,"ATTACK_Y");
    this->setCustomVar(120.0f+(this->width-130)/2,"HOLD_X");
    this->setCustomVar(this->headerHeight+10,"HOLD_Y");
    this->setCustomVar(120.0f+(this->width-130),"RELEASE_X");
    this->setCustomVar(this->headerHeight+10+(this->height-this->headerHeight-40),"RELEASE_Y");
}

//--------------------------------------------------------------
void pdspDucker::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();

    controlPoints.push_back(DraggableVertex(this->getCustomVar("ATTACK_X"),this->getCustomVar("ATTACK_Y"))); // A
    controlPoints.push_back(DraggableVertex(this->getCustomVar("HOLD_X"),this->getCustomVar("HOLD_Y"))); // H
    controlPoints.push_back(DraggableVertex(this->getCustomVar("RELEASE_X"),this->getCustomVar("RELEASE_Y"))); // R

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onTextInputEvent(this,&pdspDucker::onTextInputEvent);
    gui->onSliderEvent(this, &pdspDucker::onSliderEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    duration = gui->addTextInput("DURATION (MS)","");
    duration->setUseCustomMouse(true);
    duration->setText(ofToString(this->getCustomVar("DURATION")));
    gui->addBreak();
    ducking = gui->addSlider("Ducking", 0,1,this->getCustomVar("DUCKING"));
    ducking->setUseCustomMouse(true);
    attackHardness = gui->addSlider("Attack Curve", 0,1,this->getCustomVar("ATTACK_CURVE"));
    attackHardness->setUseCustomMouse(true);
    releaseHardness = gui->addSlider("Release Curve", 0,1,this->getCustomVar("RELEASE_CURVE"));
    releaseHardness->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

}

//--------------------------------------------------------------
void pdspDucker::setupAudioOutObjectContent(pdsp::Engine &engine){

    gate_ctrl.out_trig() >> ducker.in_trig();

    duck_ctrl >> ducker.in_ducking(); // -48.0 - 0.0
    duck_ctrl.set(-20.0f);
    duck_ctrl.enableSmoothing(50.0f);

    attack_ctrl >> ducker.in_attack();
    attack_ctrl.set(50.0f);
    attack_ctrl.enableSmoothing(50.0f);

    hold_ctrl >> ducker.in_hold();
    hold_ctrl.set(0.0f);
    hold_ctrl.enableSmoothing(50.0f);

    release_ctrl >> ducker.in_release();
    release_ctrl.set(100.0f);
    release_ctrl.enableSmoothing(50.0f);

    this->pdspIn[0] >> ducker >> this->pdspOut[0];
    this->pdspIn[0] >> ducker >> scope >> engine.blackhole();

}

//--------------------------------------------------------------
void pdspDucker::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){
    gui->update();
    header->update();
    duration->update();
    ducking->update();
    attackHardness->update();
    releaseHardness->update();

    attackDuration          = (controlPoints.at(0).x-rect.x)/rect.width;
    holdDuration            = (((controlPoints.at(1).x-rect.x)/rect.width * 100)-((controlPoints.at(0).x-rect.x)/rect.width * 100))/100;
    releaseDuration         = (((controlPoints.at(2).x-rect.x)/rect.width * 100)-((controlPoints.at(1).x-rect.x)/rect.width * 100))/100;

    attack_ctrl.set(attackDuration*envelopeDuration);
    hold_ctrl.set(holdDuration*envelopeDuration);
    release_ctrl.set(releaseDuration*envelopeDuration);

    // bang --> trigger ducker (sidechain compressor)
    if(this->inletsConnected[1]){
        gate_ctrl.trigger(ofClamp(*(float *)&_inletParams[1],0.0f,1.0f));
    }else{
        gate_ctrl.off();
    }

    // duration
    if(this->inletsConnected[2]){
        duration->setText(ofToString(static_cast<int>(floor(abs(*(float *)&_inletParams[2])))));
        this->setCustomVar(*(float *)&_inletParams[2],"DURATION");
    }

    // ducking
    if(this->inletsConnected[3]){
        ducking->setValue(ofClamp(static_cast<float>(*(float *)&_inletParams[3]),0.0f,1.0f));
        duck_ctrl.set(ofMap(ofClamp(static_cast<float>(*(float *)&_inletParams[3]),0.0f,1.0f),0.0f,1.0f,-48.0f,0.0f,true));
        this->setCustomVar(ofClamp(static_cast<float>(*(float *)&_inletParams[3]),0.0f,1.0f),"DUCKER");
    }

    if(!loaded){
        loaded = true;
        duration->setText(ofToString(static_cast<int>(floor(this->getCustomVar("DURATION")))));
        ducking->setValue(this->getCustomVar("DUCKING"));
        attackHardness->setValue(this->getCustomVar("ATTACK_CURVE"));
        releaseHardness->setValue(this->getCustomVar("RELEASE_CURVE"));
    }
}

//--------------------------------------------------------------
void pdspDucker::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    ofSetColor(255,255,120);
    for(size_t i=0;i<controlPoints.size();i++){
        if(controlPoints.at(i).bOver){
            ofDrawCircle(controlPoints.at(i).x,controlPoints.at(i).y,6);
        }else{
            ofDrawCircle(controlPoints.at(i).x,controlPoints.at(i).y,4);
        }

    }
    ofDrawLine(rect.getBottomLeft().x,rect.getBottomLeft().y,controlPoints.at(0).x,controlPoints.at(0).y);
    ofDrawLine(controlPoints.at(0).x,controlPoints.at(0).y,controlPoints.at(1).x,controlPoints.at(1).y);
    ofDrawLine(controlPoints.at(1).x,controlPoints.at(1).y,controlPoints.at(2).x,controlPoints.at(2).y);

    ofSetColor(160,160,160);
    font->draw("A:",this->fontSize,rect.getBottomLeft().x+6,rect.getBottomLeft().y+8);
    font->draw("H:",this->fontSize,rect.getCenter().x-30,rect.getBottomLeft().y+8);
    font->draw("R:",this->fontSize,rect.getBottomRight().x-70,rect.getBottomLeft().y+8);
    ofSetColor(255,255,120);
    string tempStr = ofToString(attackDuration * 100,1)+"%";
    font->draw(tempStr,this->fontSize,rect.getBottomLeft().x+20,rect.getBottomLeft().y+8);
    tempStr = ofToString(holdDuration * 100,1)+"%";
    font->draw(tempStr,this->fontSize,rect.getCenter().x-16,rect.getBottomLeft().y+8);
    tempStr = ofToString(releaseDuration * 100,1)+"%";
    font->draw(tempStr,this->fontSize,rect.getBottomRight().x-56,rect.getBottomLeft().y+8);

    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void pdspDucker::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspDucker::loadAudioSettings(){
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
void pdspDucker::audioOutObject(ofSoundBuffer &outputBuffer){
    // SIGNAL BUFFER
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}

//--------------------------------------------------------------
void pdspDucker::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    duration->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    ducking->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    attackHardness->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    releaseHardness->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    for(int j=0;j<static_cast<int>(controlPoints.size());j++){
        controlPoints.at(j).over(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || duration->hitTest(_m-this->getPos()) || ducking->hitTest(_m-this->getPos()) || attackHardness->hitTest(_m-this->getPos()) || releaseHardness->hitTest(_m-this->getPos()) || controlPoints.at(0).bOver || controlPoints.at(1).bOver || controlPoints.at(2).bOver;
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos()) || controlPoints.at(0).bOver || controlPoints.at(1).bOver || controlPoints.at(2).bOver;
    }

}

//--------------------------------------------------------------
void pdspDucker::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        duration->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        ducking->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        attackHardness->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        releaseHardness->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

        for(int j=0;j<static_cast<int>(controlPoints.size());j++){
            if(static_cast<int>(_m.x - this->getPos().x) >= rect.getLeft() && static_cast<int>(_m.x - this->getPos().x) <= rect.getRight()){
                controlPoints.at(j).drag(static_cast<int>(_m.x - this->getPos().x),controlPoints.at(j).y);
            }

        }
        this->setCustomVar(controlPoints.at(0).x,"ATTACK_X");
        this->setCustomVar(controlPoints.at(0).y,"ATTACK_Y");
        this->setCustomVar(controlPoints.at(1).x,"HOLD_X");
        this->setCustomVar(controlPoints.at(1).y,"HOLD_Y");
        this->setCustomVar(controlPoints.at(2).x,"RELEASE_X");
        this->setCustomVar(controlPoints.at(2).y,"RELEASE_Y");

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
void pdspDucker::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == duration){
            if(isInteger(e.text)){
                envelopeDuration = ofToInt(e.text);
                this->setCustomVar(ofToFloat(e.text),"DURATION");
            }else{
                duration->setText(ofToString(envelopeDuration));
            }
        }
    }
}

//--------------------------------------------------------------
void pdspDucker::onSliderEvent(ofxDatGuiSliderEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == attackHardness){
            ducker.setAttackCurve(static_cast<float>(e.value));
            this->setCustomVar(static_cast<float>(e.value),"ATTACK_CURVE");
        }else if(e.target == releaseHardness){
            ducker.setReleaseCurve(static_cast<float>(e.value));
            this->setCustomVar(static_cast<float>(e.value),"RELEASE_CURVE");
        }else if(e.target == ducking){
            duck_ctrl.set(ofMap(ofClamp(static_cast<float>(e.value),0.0f,1.0f),0.0f,1.0f,-48.0f,0.0f,true));
            this->setCustomVar(ofClamp(static_cast<float>(e.value),0.0f,1.0f),"DUCKER");
        }
    }
}

OBJECT_REGISTER( pdspDucker, "ducker", OFXVP_OBJECT_CAT_SOUND)

#endif