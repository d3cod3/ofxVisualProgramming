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

#include "pdspResonant2PoleFilter.h"

//--------------------------------------------------------------
pdspResonant2PoleFilter::pdspResonant2PoleFilter() : PatchObject("resonant filter"){

    this->numInlets  = 4;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();  // audio input

    _inletParams[1] = new float();          // pitch
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();          // cutoff
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();          // resonance
    *(float *)&_inletParams[3] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output

    this->initInletsState();

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    pitch                   = 12.0f;
    cutoff                  = 12.0f;
    resonance               = 0.0f;
    filterMode              = Filter_Mode_LP;

    loaded                  = false;

    this->width *= 2.0f;

}

//--------------------------------------------------------------
void pdspResonant2PoleFilter::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"pitch");
    this->addInlet(VP_LINK_NUMERIC,"cutoff");
    this->addInlet(VP_LINK_NUMERIC,"resonance");

    this->addOutlet(VP_LINK_AUDIO,"filteredSignal");

    this->setCustomVar(pitch,"PITCH");
    this->setCustomVar(cutoff,"CUTOFF");
    this->setCustomVar(resonance,"RESONANCE");
    this->setCustomVar(static_cast<float>(filterMode),"MODE");
}

//--------------------------------------------------------------
void pdspResonant2PoleFilter::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();

    filterModesString.push_back("Low Pass");
    filterModesString.push_back("Band Pass");
    filterModesString.push_back("Hi Pass");
    filterModesString.push_back("Notch");
}

//--------------------------------------------------------------
void pdspResonant2PoleFilter::setupAudioOutObjectContent(pdsp::Engine &engine){

    pitch_ctrl >> filter.in_pitch();
    pitch_ctrl.set(12);
    pitch_ctrl.enableSmoothing(50.0f);

    cutoff_ctrl >> filter.in_cutoff();
    cutoff_ctrl.set(12);
    cutoff_ctrl.enableSmoothing(50.0f);

    resonance_ctrl >> filter.in_reso();
    resonance_ctrl.set(0.0f);
    resonance_ctrl.enableSmoothing(50.0f);

    mode_ctrl >> filter.in_mode();
    mode_ctrl.set(0.0f);

    this->pdspIn[0] >> filter.in_signal();

    filter.out_signal() >> this->pdspOut[0];

    filter.out_signal() >> scope >> engine.blackhole();
}

//--------------------------------------------------------------
void pdspResonant2PoleFilter::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[1]){
        pitch = ofClamp(*(float *)&_inletParams[1],0.0f,127.0f);
        pitch_ctrl.set(pitch);
    }

    if(this->inletsConnected[2]){
        cutoff = ofClamp(*(float *)&_inletParams[2],0.0f,127.0f);
        cutoff_ctrl.set(cutoff);
    }

    if(this->inletsConnected[3]){
        resonance = ofClamp(*(float *)&_inletParams[3],0.0f,1.0f);
        resonance_ctrl.set(resonance);
    }

    if(!loaded){
        loaded = true;
        pitch = ofClamp(this->getCustomVar("PITCH"),0.0f,127.0f);
        pitch_ctrl.set(pitch);
        cutoff = ofClamp(this->getCustomVar("CUTOFF"),0.0f,127.0f);
        cutoff_ctrl.set(cutoff);
        resonance = ofClamp(this->getCustomVar("RESONANCE"),0.0f,1.0f);
        resonance_ctrl.set(resonance);
        filterMode = static_cast<int>(floor(ofClamp(this->getCustomVar("MODE"),0,3)));
    }

}

//--------------------------------------------------------------
void pdspResonant2PoleFilter::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void pdspResonant2PoleFilter::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/11, IM_COL32(255,255,120,255), "pitch", &pitch, 0.0f, 127.0f, 1270.0f)){
            pitch_ctrl.set(pitch);
            this->setCustomVar(static_cast<float>(pitch),"PITCH");
        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(40*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/11, IM_COL32(255,255,120,255), "cutoff", &cutoff, 0.0f, 127.0f, 1270.0f)){
            cutoff_ctrl.set(cutoff);
            this->setCustomVar(cutoff,"CUTOFF");
        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(40*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/11, IM_COL32(255,255,120,255), "resonance", &resonance, 0.0f, 1.0f, 100.0f)){
            resonance_ctrl.set(resonance);
            this->setCustomVar(resonance,"RESONANCE");
        }

        _nodeCanvas.EndNodeContent();

    }

}

//--------------------------------------------------------------
void pdspResonant2PoleFilter::drawObjectNodeConfig(){

    ImGui::Spacing();
    if(ImGui::BeginCombo("filter mode", filterModesString.at(filterMode).c_str() )){
        for(int i=0; i < filterModesString.size(); ++i){
            bool is_selected = (filterMode == i );
            if (ImGui::Selectable(filterModesString.at(i).c_str(), is_selected)){
                filterMode = i;
                mode_ctrl.set(filterMode);
                this->setCustomVar(static_cast<float>(filterMode),"MODE");
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGuiEx::ObjectInfo(
                "A Resonant 2 pole state variable filter with switchable LP, BP, HP and Notch outputs and pitched cutoff control",
                "https://mosaic.d3cod3.org/reference.php?r=resonant-2pole-filter", scaleFactor);
}

//--------------------------------------------------------------
void pdspResonant2PoleFilter::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspResonant2PoleFilter::loadAudioSettings(){
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
void pdspResonant2PoleFilter::audioInObject(ofSoundBuffer &inputBuffer){

}

//--------------------------------------------------------------
void pdspResonant2PoleFilter::audioOutObject(ofSoundBuffer &outputBuffer){
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}


OBJECT_REGISTER( pdspResonant2PoleFilter, "resonant filter", OFXVP_OBJECT_CAT_SOUND)

#endif
