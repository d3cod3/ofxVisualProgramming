/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2020 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "pdspKick.h"

//--------------------------------------------------------------
pdspKick::pdspKick() : PatchObject("kick"){

    this->numInlets  = 4;
    this->numOutlets = 1;

    _inletParams[0] = new float();          // bang
    *(float *)&_inletParams[0] = 0.0f;
    _inletParams[1] = new float();          // osc freq
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();          // filter freq
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();          // filter res
    *(float *)&_inletParams[3] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output

    this->initInletsState();


    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    loaded                  = false;

    oscFreq                 = 44.0f;
    filterFreq              = 120.0f;
    filterRes               = 0.168f;

    this->width *= 2.0f;

}

//--------------------------------------------------------------
void pdspKick::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"pitch");
    this->addInlet(VP_LINK_NUMERIC,"filter frequency");
    this->addInlet(VP_LINK_NUMERIC,"filter resonance");

    this->addOutlet(VP_LINK_AUDIO,"kick");

    this->setCustomVar(oscFreq,"OSC_FREQ");
    this->setCustomVar(filterFreq,"FILTER_FREQ");
    this->setCustomVar(filterRes,"FILTER_RES");


}

//--------------------------------------------------------------
void pdspKick::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();

}

//--------------------------------------------------------------
void pdspKick::setupAudioOutObjectContent(pdsp::Engine &engine){

    osc_freq_ctrl >> osc.in_pitch();
    osc_freq_ctrl.set(pdsp::f2p(oscFreq));
    osc_freq_ctrl.enableSmoothing(50.0f);

    filter_freq_ctrl >> filter.in_pitch();
    filter_freq_ctrl.set(pdsp::f2p(filterFreq));
    filter_freq_ctrl.enableSmoothing(50.0f);

    filter_res_ctrl >> filter.in_reso();
    filter_res_ctrl.set(filterRes);
    filter_res_ctrl.enableSmoothing(50.0f);

    pdsp::VAFilter::LowPass24 >> filter.in_mode();

    40.0f >> eq.in_freq();
    2.0f >> eq.in_gain();
    2.0f >> eq.in_Q();

    8.0f >> compressor.in_ratio();
    4.16f >> compressor.in_attack();
    50.0f >> compressor.in_release();

    drive.set(4.57f);

    osc >> amp;
    gate_ctrl.out_trig() >> ampEnv.set(0.0f,240.0f,0.0f,0.0f) >> amp.in_mod();
    gate_ctrl.out_trig() >> modEnv.set(0.0f,15.0f,0.0f,0.0f) * osc_freq_ctrl.get() >> osc.in_pitch();

    amp >> filter >> compressor >> drive >> this->pdspOut[0];
    amp >> filter >> compressor >> drive >> scope >> engine.blackhole();

}

//--------------------------------------------------------------
void pdspKick::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    // bang --> trigger envelope
    if(this->inletsConnected[0]){
        gate_ctrl.trigger(ofClamp(*(float *)&_inletParams[0],0.0f,1.0f));
    }else{
        gate_ctrl.off();
    }

    if(this->inletsConnected[1]){
        oscFreq = ofClamp(*(float *)&_inletParams[1],0.0f,100.0f);
    }

    if(this->inletsConnected[2]){
        filterFreq = ofClamp(*(float *)&_inletParams[2],0.0f, 8260.0f);
    }

    if(this->inletsConnected[3]){
        filterRes = ofClamp(*(float *)&_inletParams[3],0.0f,1.0f);
    }


    if(!loaded){
        loaded = true;

        oscFreq     = this->getCustomVar("OSC_FREQ");
        filterFreq  = this->getCustomVar("FILTER_FREQ");
        filterRes   = this->getCustomVar("FILTER_RES");
    }
}

//--------------------------------------------------------------
void pdspKick::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void pdspKick::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/11, IM_COL32(255,255,120,255), "FREQ", &oscFreq, 0.0f, 100.0f, 100.0f)){
            osc_freq_ctrl.set(pdsp::f2p(oscFreq));
            this->setCustomVar(oscFreq,"OSC_FREQ");
        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(40*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/11, IM_COL32(255,255,120,255), "F. FREQ", &filterFreq, 0.0f, 8260.0f, 2065.0f)){
            filter_freq_ctrl.set(pdsp::f2p(filterFreq));
            this->setCustomVar(filterFreq,"FILTER_FREQ");
        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(40*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/11, IM_COL32(255,255,120,255), "F. RES", &filterRes, 0.0f, 1.0f, 100.0f)){
            filter_res_ctrl.set(filterRes);
            this->setCustomVar(filterRes,"FILTER_RES");
        }

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*8*scaleFactor));

        _nodeCanvas.EndNodeContent();

    }

}

//--------------------------------------------------------------
void pdspKick::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "A pretty simple kick synth.",
                "https://mosaic.d3cod3.org/reference.php?r=kick", scaleFactor);
}

//--------------------------------------------------------------
void pdspKick::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspKick::loadAudioSettings(){
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
void pdspKick::audioOutObject(ofSoundBuffer &outputBuffer){
    // SIGNAL BUFFER
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);

}

OBJECT_REGISTER( pdspKick, "kick", OFXVP_OBJECT_CAT_SOUND)

#endif
