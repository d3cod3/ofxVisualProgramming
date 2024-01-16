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

#include "pdspCombFilter.h"

//--------------------------------------------------------------
pdspCombFilter::pdspCombFilter() : PatchObject("comb filter"){

    this->numInlets  = 4;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();  // audio input

    _inletParams[1] = new float();          // pitch
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();          // damping
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();          // feedback
    *(float *)&_inletParams[3] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output

    this->initInletsState();

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    pitch                   = 12.0f;
    damping                 = 0.0f;
    feedback                = 0.0f;

    loaded                  = false;

    this->width *= 2.0f;
    this->height *= 1.12f;

}

//--------------------------------------------------------------
void pdspCombFilter::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"pitch");
    this->addInlet(VP_LINK_NUMERIC,"damping");
    this->addInlet(VP_LINK_NUMERIC,"feedback");

    this->addOutlet(VP_LINK_AUDIO,"filteredSignal");

    this->setCustomVar(static_cast<float>(pitch),"PITCH");
    this->setCustomVar(static_cast<float>(damping),"DAMPING");
    this->setCustomVar(static_cast<float>(feedback),"FEEDBACK");
}

//--------------------------------------------------------------
void pdspCombFilter::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadAudioSettings();
}

//--------------------------------------------------------------
void pdspCombFilter::setupAudioOutObjectContent(pdsp::Engine &engine){

    pitch_ctrl >> filter.in_pitch();
    pitch_ctrl.set(pitch);
    pitch_ctrl.enableSmoothing(50.0f);

    damping_ctrl >> filter.in_damping();
    damping_ctrl.set(damping);
    damping_ctrl.enableSmoothing(50.0f);

    feedback_ctrl >> filter.in_feedback();
    feedback_ctrl.set(feedback);
    feedback_ctrl.enableSmoothing(50.0f);

    this->pdspIn[0] >> filter.in_signal();

    filter.out_signal() >> this->pdspOut[0];
    filter.out_signal() >> scope >> engine.blackhole();
}

//--------------------------------------------------------------
void pdspCombFilter::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[1]){
        pitch = ofClamp(*(float *)&_inletParams[1],0.0f,127.0f);
        pitch_ctrl.set(pitch);
    }

    if(this->inletsConnected[2]){
        damping = ofClamp(*(float *)&_inletParams[2],0.0f,1.0f);
        damping_ctrl.set(damping);
    }

    if(this->inletsConnected[3]){
        feedback = ofClamp(*(float *)&_inletParams[3],0.0f,1.0f);
        feedback_ctrl.set(feedback);
    }

    if(!loaded){
        loaded = true;
        pitch = ofClamp(this->getCustomVar("PITCH"),0.0f,127.0f);
        pitch_ctrl.set(pitch);
        damping = ofClamp(this->getCustomVar("DAMPING"),0.0f,1.0f);
        damping_ctrl.set(damping);
        feedback = ofClamp(this->getCustomVar("FEEDBACK"),0.0f,1.0f);
        feedback_ctrl.set(feedback);
    }

}

//--------------------------------------------------------------
void pdspCombFilter::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void pdspCombFilter::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGui::Dummy(ImVec2(0,4*scaleFactor));


        if(ImGuiKnobs::Knob("pitch", &pitch, 0.0f, 127.0f, 0.1f, "%.2f", ImGuiKnobVariant_Wiper)){
            pitch_ctrl.set(pitch);
            this->setCustomVar(static_cast<float>(pitch),"PITCH");
        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(40*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiKnobs::Knob("damping", &damping, 0.0f, 1.0f, 0.001f, "%.2f", ImGuiKnobVariant_Wiper)){
            damping_ctrl.set(damping);
            this->setCustomVar(static_cast<float>(damping),"DAMPING");
        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(40*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiKnobs::Knob("feedback", &feedback, 0.0f, 1.0f, 0.001f, "%.2f", ImGuiKnobVariant_Wiper)){
            feedback_ctrl.set(feedback);
            this->setCustomVar(static_cast<float>(feedback),"FEEDBACK");
        }

        _nodeCanvas.EndNodeContent();

    }

}

//--------------------------------------------------------------
void pdspCombFilter::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "A comb filter is a delay tuned to a specific pitch frequency (mix it with the dry signal)",
                "https://mosaic.d3cod3.org/reference.php?r=comb-filter", scaleFactor);
}

//--------------------------------------------------------------
void pdspCombFilter::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspCombFilter::loadAudioSettings(){
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
void pdspCombFilter::audioInObject(ofSoundBuffer &inputBuffer){

}

//--------------------------------------------------------------
void pdspCombFilter::audioOutObject(ofSoundBuffer &outputBuffer){
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}


OBJECT_REGISTER( pdspCombFilter, "comb filter", OFXVP_OBJECT_CAT_SOUND)

#endif
