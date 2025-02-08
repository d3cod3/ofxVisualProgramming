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
    this->numOutlets = 3;

    _inletParams[0] = new float();          // bang
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]) = 0.0f;
    _inletParams[1] = new float();          // osc freq
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]) = 0.0f;
    _inletParams[2] = new float();          // filter freq
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[2]) = 0.0f;
    _inletParams[3] = new float();          // filter res
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[3]) = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output
    _outletParams[1] = new float();         // ADSR func
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[1]) = 0.0f;
    _outletParams[2] = new float();          // Freq. ADSR func
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[2]) = 0.0f;

    this->initInletsState();


    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    loaded                  = false;

    attackDuration          = 0.0f;
    decayDuration           = 240.0f;
    sustainLevel            = 0.0f;
    releaseDuration         = 0.0f;

    f_attackDuration        = 0.0f;
    f_decayDuration         = 15.0f;
    f_sustainLevel          = 0.0f;
    f_releaseDuration       = 0.0f;

    drivePower              = 4.57f;

    oscFreq                 = 44.0f;
    filterFreq              = 120.0f;
    filterRes               = 0.168f;

    compRatio               = 8.0f;
    compAttack              = 2.0f;
    compRelease             = 2.0f;

    this->width *= 2.1f;
    this->height *= 4.6f;

}

//--------------------------------------------------------------
void pdspKick::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"pitch");
    this->addInlet(VP_LINK_NUMERIC,"filter frequency");
    this->addInlet(VP_LINK_NUMERIC,"filter resonance");

    this->addOutlet(VP_LINK_AUDIO,"kick");
    this->addOutlet(VP_LINK_NUMERIC,"envelope");
    this->addOutlet(VP_LINK_NUMERIC,"frequency envelope");

    this->setCustomVar(attackDuration,"ATTACK");
    this->setCustomVar(decayDuration,"DECAY");
    this->setCustomVar(sustainLevel,"SUSTAIN");
    this->setCustomVar(releaseDuration,"RELEASE");

    this->setCustomVar(f_attackDuration,"FREQ_ATTACK");
    this->setCustomVar(f_decayDuration,"FREQ_DECAY");
    this->setCustomVar(f_sustainLevel,"FREQ_SUSTAIN");
    this->setCustomVar(f_releaseDuration,"FREQ_RELEASE");

    this->setCustomVar(oscFreq,"OSC_FREQ");
    this->setCustomVar(filterFreq,"FILTER_FREQ");
    this->setCustomVar(filterRes,"FILTER_RES");

    this->setCustomVar(compRatio,"COMP_RATIO");
    this->setCustomVar(compAttack,"COMP_A");
    this->setCustomVar(compRelease,"COMP_R");

}

//--------------------------------------------------------------
void pdspKick::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

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

    comp_ratio_ctrl >> compressor.in_ratio();
    comp_ratio_ctrl.set(compRatio);
    comp_ratio_ctrl.enableSmoothing(50.0f);

    comp_A_ctrl >> compressor.in_attack();
    comp_A_ctrl.set(compAttack);
    comp_A_ctrl.enableSmoothing(50.0f);

    comp_R_ctrl >> compressor.in_release();
    comp_R_ctrl.set(compRelease);
    comp_R_ctrl.enableSmoothing(50.0f);

    drive.set(drivePower);

    pdsp::VAFilter::LowPass24 >> filter.in_mode();

    compressor.analog();

    osc >> amp;
    gate_ctrl.out_trig() >> ampEnv >> amp.in_mod();
    gate_ctrl.out_trig() >> modEnv * osc_freq_ctrl.get() >> osc.in_pitch();

    amp >> filter >> drive >> compressor >> this->pdspOut[0];
    amp >> filter >> drive >> compressor >> scope >> engine.blackhole();

}

//--------------------------------------------------------------
void pdspKick::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    ampEnv.set(attackDuration,decayDuration,sustainLevel,releaseDuration);
    modEnv.set(f_attackDuration,f_decayDuration,f_sustainLevel,f_releaseDuration);

    if(this->inletsConnected[1]){
        oscFreq = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]),0.0f,100.0f);
    }

    if(this->inletsConnected[2]){
        filterFreq = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[2]),0.0f, 8260.0f);
    }

    if(this->inletsConnected[3]){
        filterRes = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[3]),0.0f,1.0f);
    }


    if(!loaded){
        loaded = true;

        attackDuration = this->getCustomVar("ATTACK");
        decayDuration = this->getCustomVar("DECAY");
        sustainLevel = this->getCustomVar("SUSTAIN");
        releaseDuration = this->getCustomVar("RELEASE");

        f_attackDuration = this->getCustomVar("FREQ_ATTACK");
        f_decayDuration = this->getCustomVar("FREQ_DECAY");
        f_sustainLevel = this->getCustomVar("FREQ_SUSTAIN");
        f_releaseDuration = this->getCustomVar("FREQ_RELEASE");

        oscFreq     = this->getCustomVar("OSC_FREQ");
        filterFreq  = this->getCustomVar("FILTER_FREQ");
        filterRes   = this->getCustomVar("FILTER_RES");

        compRatio   = this->getCustomVar("COMP_RATIO");
        compAttack  = this->getCustomVar("COMP_A");
        compRelease = this->getCustomVar("COMP_R");
    }
}

//--------------------------------------------------------------
void pdspKick::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

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

        ImGui::Dummy(ImVec2(0,4*scaleFactor));
        if(ImGuiKnobs::Knob( "A", &attackDuration, 0.0f, 1000.0f, 1.0f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(attackDuration,"ATTACK");
        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(6*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiKnobs::Knob( "D", &decayDuration, 0.0f, 1000.0f, 1.0f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(decayDuration,"DECAY");
        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(6*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiKnobs::Knob( "S", &sustainLevel, 0.0f, 1.0f, 0.001f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(sustainLevel,"SUSTAIN");
        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(6*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiKnobs::Knob( "R", &releaseDuration, 0.0f, 1000.0f, 1.0f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(releaseDuration,"RELEASE");
        }

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*8*scaleFactor));

        if(ImGuiKnobs::Knob( "FREQ A", &f_attackDuration, 0.0f, 1000.0f, 1.0f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(f_attackDuration,"FREQ_ATTACK");
        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(6*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiKnobs::Knob( "FREQ D", &f_decayDuration, 0.0f, 1000.0f, 1.0f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(f_decayDuration,"FREQ_DECAY");
        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(6*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiKnobs::Knob( "FREQ S", &f_sustainLevel, 0.0f, 1.0f, 0.001f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(f_sustainLevel,"FREQ_SUSTAIN");
        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(6*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiKnobs::Knob( "FREQ R", &f_releaseDuration, 0.0f, 1000.0f, 1.0f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(f_releaseDuration,"FREQ_RELEASE");
        }

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*8*scaleFactor));

        if(ImGuiKnobs::Knob( "FREQ", &oscFreq, 0.0f, 100.0f, 0.1f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            osc_freq_ctrl.set(pdsp::f2p(oscFreq));
            this->setCustomVar(oscFreq,"OSC_FREQ");
        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(6*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiKnobs::Knob( "F. FREQ", &filterFreq, 0.0f, 8260.0f, 10.0f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            filter_freq_ctrl.set(pdsp::f2p(filterFreq));
            this->setCustomVar(filterFreq,"FILTER_FREQ");
        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(6*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiKnobs::Knob( "F. RES", &filterRes, 0.0f, 1.0f, 0.001f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            filter_res_ctrl.set(filterRes);
            this->setCustomVar(filterRes,"FILTER_RES");
        }

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*8*scaleFactor));

        if(ImGuiKnobs::Knob( "RATIO", &compRatio, 1.0f, 40.0f, 0.01f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            comp_ratio_ctrl.set(compRatio);
            this->setCustomVar(compRatio,"COMP_RATIO");
        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(6*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiKnobs::Knob( "COMP A", &compAttack, 0.0f, 1000.0f, 1.0f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            comp_A_ctrl.set(compAttack);
            this->setCustomVar(compAttack,"COMP_A");
        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(6*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiKnobs::Knob( "COMP R", &compRelease, 0.0f, 1000.0f, 1.0f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            comp_R_ctrl.set(compRelease);
            this->setCustomVar(compRelease,"COMP_R");
        }

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
    unusedArgs(removeFileFromData);

    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspKick::loadAudioSettings(){
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
void pdspKick::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);

    // bang --> trigger envelope
    if(this->inletsConnected[0]){
        gate_ctrl.trigger(ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]),0.0f,1.0f));
    }else{
        gate_ctrl.off();
    }

    // output envelope func
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[1]) = ampEnv.meter_output();
    // output freq. envelope func
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[2]) = modEnv.meter_output();

    // SIGNAL BUFFER
    ofxVP_CAST_PIN_PTR<ofSoundBuffer>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);

}

OBJECT_REGISTER( pdspKick, "kick", OFXVP_OBJECT_CAT_SOUND)

#endif
