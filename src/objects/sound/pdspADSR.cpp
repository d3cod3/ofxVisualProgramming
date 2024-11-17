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

#include "pdspADSR.h"

//--------------------------------------------------------------
pdspADSR::pdspADSR() : PatchObject("ADSR envelope"){

    this->numInlets  = 6;
    this->numOutlets = 2;

    _inletParams[0] = new ofSoundBuffer(); // audio input

    _inletParams[1] = new float();          // bang
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();          // A
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();          // D
    *(float *)&_inletParams[3] = 0.0f;
    _inletParams[4] = new float();          // S
    *(float *)&_inletParams[4] = 0.0f;
    _inletParams[5] = new float();          // R
    *(float *)&_inletParams[5] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output
    _outletParams[1] = new float();         // ADSR func
    *(float *)&_outletParams[1] = 0.0f;

    this->initInletsState();

    this->width *= 1.8;
    this->height *= 1.7f;

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    loaded                  = false;

    attackDuration          = 0.0f;
    decayDuration           = 50.0f;
    sustainLevel            = 0.5f;
    releaseDuration         = 50.0f;

    attackHardness          = 0.0f;
    releaseHardness         = 1.0f;


}

//--------------------------------------------------------------
void pdspADSR::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"A");
    this->addInlet(VP_LINK_NUMERIC,"D");
    this->addInlet(VP_LINK_NUMERIC,"S");
    this->addInlet(VP_LINK_NUMERIC,"R");
    this->addOutlet(VP_LINK_AUDIO,"envelopedSignal");
    this->addOutlet(VP_LINK_NUMERIC,"envelope");

    this->setCustomVar(attackHardness,"ATTACK_CURVE");
    this->setCustomVar(releaseHardness,"RELEASE_CURVE");

    this->setCustomVar(attackDuration,"ATTACK");
    this->setCustomVar(decayDuration,"DECAY");
    this->setCustomVar(sustainLevel,"SUSTAIN");
    this->setCustomVar(releaseDuration,"RELEASE");
}

//--------------------------------------------------------------
void pdspADSR::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadAudioSettings();

}

//--------------------------------------------------------------
void pdspADSR::setupAudioOutObjectContent(pdsp::Engine &engine){

    gate_ctrl.out_trig() >> env;
    env >> amp.in_mod();

    this->pdspIn[0] >> amp >> this->pdspOut[0];
    this->pdspIn[0] >> amp >> scope >> engine.blackhole();

}

//--------------------------------------------------------------
void pdspADSR::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    // A
    if(this->inletsConnected[2]){
        attackDuration = ofClamp(*(float *)&_inletParams[2],0.0f,std::numeric_limits<float>::max());
        this->setCustomVar(attackDuration,"ATTACK");
    }

    // D
    if(this->inletsConnected[3]){
        decayDuration = ofClamp(*(float *)&_inletParams[3],0.0f,std::numeric_limits<float>::max());
        this->setCustomVar(decayDuration,"DECAY");
    }

    // S
    if(this->inletsConnected[4]){
        sustainLevel = ofClamp(*(float *)&_inletParams[4],0.0f,1.0f);
        this->setCustomVar(sustainLevel,"SUSTAIN");
    }

    // R
    if(this->inletsConnected[5]){
        releaseDuration = ofClamp(*(float *)&_inletParams[5],0.0f,std::numeric_limits<float>::max());
        this->setCustomVar(releaseDuration,"RELEASE");
    }

    if(!loaded){
        loaded = true;
        attackDuration = this->getCustomVar("ATTACK");
        decayDuration = this->getCustomVar("DECAY");
        sustainLevel = this->getCustomVar("SUSTAIN");
        releaseDuration = this->getCustomVar("RELEASE");
        attackHardness = this->getCustomVar("ATTACK_CURVE");
        releaseHardness = this->getCustomVar("RELEASE_CURVE");
    }
}

//--------------------------------------------------------------
void pdspADSR::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

    ofSetColor(255);

}

//--------------------------------------------------------------
void pdspADSR::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    if(_nodeCanvas.BeginNodeMenu()){
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            drawObjectNodeConfig(); this->configMenuWidth = ImGui::GetWindowWidth();

            ImGui::EndMenu();
        }
        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*2*scaleFactor));
        ImGuiEx::EnvelopeEditor(_nodeCanvas.getNodeDrawList(), 0, ImGui::GetWindowSize().y*0.3, &attackDuration, &decayDuration, &sustainLevel, &releaseDuration, ImGuiEnvelopeEditorType_ADSR);

        ImGui::Dummy(ImVec2(0,4*scaleFactor));
        if(ImGuiKnobs::Knob("A", &attackDuration, 0.0f, 1000.0f, 1.0f, "%.2f", ImGuiKnobVariant_Wiper)){
            this->setCustomVar(attackDuration,"ATTACK");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("D", &decayDuration, 0.0f, 1000.0f, 1.0f, "%.2f", ImGuiKnobVariant_Wiper)){
            this->setCustomVar(decayDuration,"DECAY");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("S", &sustainLevel, 0.0f, 1.0f, 0.001f, "%.2f", ImGuiKnobVariant_Wiper)){
            this->setCustomVar(sustainLevel,"SUSTAIN");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("R", &releaseDuration, 0.0f, 1000.0f, 1.0f, "%.2f", ImGuiKnobVariant_Wiper)){
            this->setCustomVar(releaseDuration,"RELEASE");
        }

        _nodeCanvas.EndNodeContent();

    }

}

//--------------------------------------------------------------
void pdspADSR::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Standard ADSR envelope",
                "https://mosaic.d3cod3.org/reference.php?r=adsr-envelope", scaleFactor);
}

//--------------------------------------------------------------
void pdspADSR::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);

    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspADSR::loadAudioSettings(){
    ofxXmlSettings XML;

#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
    if (XML.loadFile(patchFile)){
#else
    if (XML.load(patchFile)){
#endif
        if (XML.pushTag("settings")){
            sampleRate = XML.getValue("sample_rate_in",0);
            bufferSize = XML.getValue("buffer_size",0);

            XML.popTag();
        }
    }
}

//--------------------------------------------------------------
void pdspADSR::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);

    env.set(attackDuration,decayDuration,sustainLevel,releaseDuration);
    env.setAttackCurve(attackHardness);
    env.setReleaseCurve(releaseHardness);

    // bang --> trigger envelope
    if(this->inletsConnected[1]){
        if(*(float *)&_inletParams[1] == 1.0f){
            gate_ctrl.trigger(ofClamp(*(float *)&_inletParams[1],0.0f,1.0f));
        }else{
            gate_ctrl.off();
        }
    }else{
        gate_ctrl.off();
    }

    // output envelope func
    *(float *)&_outletParams[1] = env.meter_output();

    // SIGNAL BUFFER
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}

OBJECT_REGISTER( pdspADSR, "ADSR envelope", OFXVP_OBJECT_CAT_SOUND)

#endif
