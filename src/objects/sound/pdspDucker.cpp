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
pdspDucker::pdspDucker() : PatchObject("sidechain compressor"){

    this->numInlets  = 6;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer(); // audio input

    _inletParams[1] = new float();          // bang
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();          // ducking
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();          // A
    *(float *)&_inletParams[3] = 0.0f;
    _inletParams[4] = new float();          // H
    *(float *)&_inletParams[4] = 0.0f;
    _inletParams[5] = new float();          // R
    *(float *)&_inletParams[5] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output

    this->initInletsState();

    this->width *= 2;
    this->height *= 2.9f;

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    loaded                  = false;

    ducking                 = -20.0f;

    attackDuration          = 50.0f;
    holdDuration            = 0.0f;
    releaseDuration         = 100.0f;

    attackHardness          = 0.0f;
    releaseHardness         = 1.0f;


}

//--------------------------------------------------------------
void pdspDucker::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"ducking");
    this->addInlet(VP_LINK_NUMERIC,"A");
    this->addInlet(VP_LINK_NUMERIC,"H");
    this->addInlet(VP_LINK_NUMERIC,"R");

    this->addOutlet(VP_LINK_AUDIO,"duckedSignal");

    this->setCustomVar(ducking,"DUCKING");
    this->setCustomVar(attackDuration,"ATTACK");
    this->setCustomVar(holdDuration,"HOLD");
    this->setCustomVar(releaseDuration,"RELEASE");
    this->setCustomVar(attackHardness,"ATTACK_CURVE");
    this->setCustomVar(releaseHardness,"RELEASE_CURVE");
}

//--------------------------------------------------------------
void pdspDucker::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();
}

//--------------------------------------------------------------
void pdspDucker::setupAudioOutObjectContent(pdsp::Engine &engine){

    gate_ctrl.out_trig() >> ducker.in_trig();

    duck_ctrl >> ducker.in_ducking(); // -48.0 - 0.0
    duck_ctrl.set(ducking);
    duck_ctrl.enableSmoothing(50.0f);

    attack_ctrl >> ducker.in_attack();
    attack_ctrl.set(attackDuration);
    attack_ctrl.enableSmoothing(50.0f);

    hold_ctrl >> ducker.in_hold();
    hold_ctrl.set(holdDuration);
    hold_ctrl.enableSmoothing(50.0f);

    release_ctrl >> ducker.in_release();
    release_ctrl.set(releaseDuration);
    release_ctrl.enableSmoothing(50.0f);

    this->pdspIn[0] >> ducker >> this->pdspOut[0];
    this->pdspIn[0] >> ducker >> scope >> engine.blackhole();

}

//--------------------------------------------------------------
void pdspDucker::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    ducker.setAttackCurve(attackHardness);
    ducker.setReleaseCurve(releaseHardness);

    // bang --> trigger ducker (sidechain compressor)
    if(this->inletsConnected[1]){
        gate_ctrl.trigger(ofClamp(*(float *)&_inletParams[1],0.0f,1.0f));
    }else{
        gate_ctrl.off();
    }

    // ducking
    if(this->inletsConnected[2] && ducking != *(float *)&_inletParams[2]){
        ducking = ofClamp(*(float *)&_inletParams[2],-48.0f,0.0f);
        duck_ctrl.set(ducking);
        this->setCustomVar(ducking,"DUCKER");
    }

    // A
    if(this->inletsConnected[3] && attackDuration != *(float *)&_inletParams[3]){
        attackDuration = ofClamp(*(float *)&_inletParams[3],0.0f,std::numeric_limits<float>::max());
        attack_ctrl.set(attackDuration);
        this->setCustomVar(attackDuration,"ATTACK");
    }

    // H
    if(this->inletsConnected[4] && holdDuration != *(float *)&_inletParams[4]){
        holdDuration = ofClamp(*(float *)&_inletParams[4],0.0f,std::numeric_limits<float>::max());
        hold_ctrl.set(holdDuration);
        this->setCustomVar(holdDuration,"HOLD");
    }

    // R
    if(this->inletsConnected[5] && releaseDuration != *(float *)&_inletParams[5]){
        releaseDuration = ofClamp(*(float *)&_inletParams[5],0.0f,std::numeric_limits<float>::max());
        release_ctrl.set(releaseDuration);
        this->setCustomVar(releaseDuration,"RELEASE");
    }

    if(!loaded){
        loaded = true;
        ducking = this->getCustomVar("DUCKING");
        attackDuration = this->getCustomVar("ATTACK");
        holdDuration = this->getCustomVar("HOLD");
        releaseDuration = this->getCustomVar("RELEASE");
        attackHardness = this->getCustomVar("ATTACK_CURVE");
        releaseHardness = this->getCustomVar("RELEASE_CURVE");
    }
}

//--------------------------------------------------------------
void pdspDucker::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void pdspDucker::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    if(_nodeCanvas.BeginNodeMenu()){
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            drawObjectNodeConfig();

            ImGui::EndMenu();
        }
        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*2*scaleFactor));
        ImGuiEx::EnvelopeEditor(_nodeCanvas.getNodeDrawList(), 0, ImGui::GetWindowSize().y*0.3, &attackDuration, &holdDuration, &releaseDuration, &releaseDuration, ImGuiEnvelopeEditorType_AHR);


        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/11, IM_COL32(255,255,120,255), "duck (db)", &ducking, -48.0f, 0.0f, 960.0f)){
            duck_ctrl.set(ducking);
            this->setCustomVar(ducking,"DUCKING");
        }
        ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/11, IM_COL32(255,255,120,255), "A", &attackDuration, 0.0f, 1000.0f, 1000.0f)){
            attack_ctrl.set(attackDuration);
            this->setCustomVar(attackDuration,"ATTACK");
        }
        ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/11, IM_COL32(255,255,120,255), "H", &holdDuration, 0.0f, 1000.0f, 1000.0f)){
            hold_ctrl.set(holdDuration);
            this->setCustomVar(holdDuration,"HOLD");
        }
        ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/11, IM_COL32(255,255,120,255), "R", &releaseDuration, 0.0f, 1000.0f, 1000.0f)){
            release_ctrl.set(releaseDuration);
            this->setCustomVar(releaseDuration,"RELEASE");
        }

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*8*scaleFactor));
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/11, IM_COL32(255,255,120,255), "A. H.", &attackHardness, 0.0f, 1.0f, 100.0f)){
            this->setCustomVar(attackHardness,"ATTACK_CURVE");
        }
        ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/11, IM_COL32(255,255,120,255), "R. H.", &releaseHardness, 0.0f, 1.0f, 100.0f)){
            this->setCustomVar(releaseHardness,"RELEASE_CURVE");
        }

    }

}

//--------------------------------------------------------------
void pdspDucker::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "A sidechain compression effect",
                "https://mosaic.d3cod3.org/reference.php?r=ducker", scaleFactor);
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


OBJECT_REGISTER( pdspDucker, "sidechain compressor", OFXVP_OBJECT_CAT_SOUND)

#endif
