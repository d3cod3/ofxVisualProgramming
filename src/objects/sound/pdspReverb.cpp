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

#include "pdspReverb.h"

//--------------------------------------------------------------
pdspReverb::pdspReverb() : PatchObject("reverb"){

    this->numInlets  = 6;
    this->numOutlets = 2;

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

    _outletParams[0] = new ofSoundBuffer(); // audio output L
    _outletParams[1] = new ofSoundBuffer(); // audio output R

    this->initInletsState();

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    time                    = 0.0f;
    density                 = 0.5f;
    damping                 = 0.5f;
    modSpeed                = 0.2f;
    modAmount               = 0.8f;

    loaded                  = false;

    this->width *= 2.2f;

}

//--------------------------------------------------------------
void pdspReverb::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"time");
    this->addInlet(VP_LINK_NUMERIC,"density");
    this->addInlet(VP_LINK_NUMERIC,"damping");
    this->addInlet(VP_LINK_NUMERIC,"speed");
    this->addInlet(VP_LINK_NUMERIC,"amount");

    this->addOutlet(VP_LINK_AUDIO,"reverbSignal L");
    this->addOutlet(VP_LINK_AUDIO,"reverbSignal R");

    this->setCustomVar(time,"TIME");
    this->setCustomVar(density,"DENSITY");
    this->setCustomVar(damping,"DAMPING");
    this->setCustomVar(modSpeed,"MODSPEED");
    this->setCustomVar(modAmount,"MODAMOUNT");
}

//--------------------------------------------------------------
void pdspReverb::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();

}

//--------------------------------------------------------------
void pdspReverb::setupAudioOutObjectContent(pdsp::Engine &engine){

    time_ctrl >> reverb.in_time();
    time_ctrl.set(time);
    time_ctrl.enableSmoothing(50.0f);

    density_ctrl >> reverb.in_density();
    density_ctrl.set(density);
    density_ctrl.enableSmoothing(50.0f);

    damping_ctrl >> reverb.in_damping();
    damping_ctrl.set(damping);
    damping_ctrl.enableSmoothing(50.0f);

    modSpeed_ctrl >> reverb.in_mod_freq();
    modSpeed_ctrl.set(modSpeed);
    modSpeed_ctrl.enableSmoothing(50.0f);

    modAmount_ctrl >> reverb.in_mod_amount();
    modAmount_ctrl.set(modAmount);
    modAmount_ctrl.enableSmoothing(50.0f);

    this->pdspIn[0] >> reverb.in_signal();

    reverb.ch(0) >> this->pdspOut[0];
    reverb.ch(1) >> this->pdspOut[1];
    reverb.ch(0) >> scopeL >> engine.blackhole();
    reverb.ch(1) >> scopeR >> engine.blackhole();
}

//--------------------------------------------------------------
void pdspReverb::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[1]){
        time = ofClamp(*(float *)&_inletParams[1],0.0f,60.0f);
    }

    if(this->inletsConnected[2]){
        density = ofClamp(*(float *)&_inletParams[2],0.0f,1.0f);
    }

    if(this->inletsConnected[3]){
        damping = ofClamp(*(float *)&_inletParams[3],0.0f,2.0f);
    }

    if(this->inletsConnected[4]){
        modSpeed = ofClamp(*(float *)&_inletParams[4],0.0f,20.0f);
    }

    if(this->inletsConnected[5]){
        modAmount = ofClamp(*(float *)&_inletParams[5],0.0f,2.0f);
    }

    time_ctrl.set(time);
    density_ctrl.set(density);
    damping_ctrl.set(damping);
    modSpeed_ctrl.set(modSpeed);
    modAmount_ctrl.set(modAmount);

    if(!loaded){
        loaded = true;
        time = ofClamp(this->getCustomVar("TIME"),0.0f,60.0f);
        density = ofClamp(this->getCustomVar("DENSITY"),0.0f,1.0f);
        damping = ofClamp(this->getCustomVar("DAMPING"),0.0f,2.0f);
        modSpeed = ofClamp(this->getCustomVar("MODSPEED"),0.0f,20.0f);
        modAmount = ofClamp(this->getCustomVar("MODAMOUNT"),0.0f,2.0f);
    }

}

//--------------------------------------------------------------
void pdspReverb::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void pdspReverb::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    if(_nodeCanvas.BeginNodeMenu()){
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            ImGuiEx::ObjectInfo(
                        "Dubby metallic reverb with mono input and stereo output",
                        "https://mosaic.d3cod3.org/reference.php?r=reverb", scaleFactor);

            ImGui::EndMenu();
        }
        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/12, IM_COL32(255,255,120,255), "TIME", &time, 0.0f, 60.0f, 2000.0f)){
            this->setCustomVar(time,"TIME");
        }
        ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/12, IM_COL32(255,255,120,255), "DENSITY", &density, 0.0f, 1.0f, 100.0f)){
            this->setCustomVar(density,"DENSITY");
        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(6*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/12, IM_COL32(255,255,120,255), "DAMPING", &damping, 0.0f, 2.0f, 200.0f)){
            this->setCustomVar(damping,"DAMPING");
        }
        ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/12, IM_COL32(255,255,120,255), "SPEED", &modSpeed, 0.0f, 20.0f, 1000.0f)){
            this->setCustomVar(modSpeed,"MODSPEED");
        }
        ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/12, IM_COL32(255,255,120,255), "AMOUNT", &modAmount, 0.0f, 2.0f, 200.0f)){
            this->setCustomVar(modAmount,"MODAMOUNT");
        }

    }


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
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scopeL.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[1])->copyFrom(scopeR.getBuffer().data(), bufferSize, 1, sampleRate);
}


OBJECT_REGISTER( pdspReverb, "reverb", OFXVP_OBJECT_CAT_SOUND)

#endif
