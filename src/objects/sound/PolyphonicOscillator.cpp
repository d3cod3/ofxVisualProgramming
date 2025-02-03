/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2025 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "PolyphonicOscillator.h"

//--------------------------------------------------------------
PolyphonicOscillator::PolyphonicOscillator() : PatchObject("polyphonic oscillator"){

    this->numInlets  = 6;
    this->numOutlets = 6;

    _inletParams[0] = new vector<float>();  // pitch

    _inletParams[1] = new float();  // level
    *(float *)&_inletParams[1] = 0.0f;

    _inletParams[2] = new float();  // sine
    *(float *)&_inletParams[2] = 0.0f;

    _inletParams[3] = new float();  // triangle
    *(float *)&_inletParams[3] = 0.0f;

    _inletParams[4] = new float();  // saw
    *(float *)&_inletParams[4] = 0.0f;

    _inletParams[5] = new float();  // pulse
    *(float *)&_inletParams[5] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // osc output
    _outletParams[1] = new ofSoundBuffer(); // sine output
    _outletParams[2] = new ofSoundBuffer(); // triangle output
    _outletParams[3] = new ofSoundBuffer(); // saw output
    _outletParams[4] = new ofSoundBuffer(); // pulse output
    _outletParams[5] = new vector<float>(); // audio buffer

    this->initInletsState();

    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    loaded                  = false;

    level_float             = 0.5f;

    detune_float            = 0.0f;
    fine_float              = 0.0f;
    pw_float                = 0.5f;

    sine_float              = 0.0f;
    triangle_float          = 0.0f;
    saw_float               = 0.0f;
    pulse_float             = 0.0f;

    this->width *= 1.85f;
    this->height *= 3.1f;

}

//--------------------------------------------------------------
void PolyphonicOscillator::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"notes");
    this->addInlet(VP_LINK_NUMERIC,"level");
    this->addInlet(VP_LINK_NUMERIC,"sine level");
    this->addInlet(VP_LINK_NUMERIC,"triangle level");
    this->addInlet(VP_LINK_NUMERIC,"saw level");
    this->addInlet(VP_LINK_NUMERIC,"pulse level");

    this->addOutlet(VP_LINK_AUDIO,"signal");
    this->addOutlet(VP_LINK_AUDIO,"sine");
    this->addOutlet(VP_LINK_AUDIO,"triangle");
    this->addOutlet(VP_LINK_AUDIO,"saw");
    this->addOutlet(VP_LINK_AUDIO,"pulse");
    this->addOutlet(VP_LINK_ARRAY,"dataBuffer");

    this->setCustomVar(level_float,"LEVEL");
    this->setCustomVar(detune_float,"DETUNE_COARSE");
    this->setCustomVar(fine_float,"DETUNE_FINE");
    this->setCustomVar(pw_float,"PULSE_WIDTH");

    this->setCustomVar(sine_float,"SINE_LEVEL");
    this->setCustomVar(triangle_float,"TRIANGLE_LEVEL");
    this->setCustomVar(saw_float,"SAW_LEVEL");
    this->setCustomVar(pulse_float,"PULSE_LEVEL");
}

//--------------------------------------------------------------
void PolyphonicOscillator::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadAudioSettings();

}

//--------------------------------------------------------------
void PolyphonicOscillator::setupAudioOutObjectContent(pdsp::Engine &engine){

    for(size_t i=0;i<MAX_OSC_VOICES;i++){
        pitch_float.push_back(72.0f);
        voice_float.push_back(1.0f);

        pdsp::VAOscillator _osc;
        osc.push_back(_osc);
        pdsp::ValueControl pc;
        pitch_ctrl.push_back(pc);

        pdsp::ValueControl _lc;
        level_ctrl.push_back(_lc);
        pdsp::Amp _la;
        level.push_back(_la);

        pdsp::ValueControl _vc;
        voice_ctrl.push_back(_vc);
        pdsp::Amp va;
        voice_amp_sine.push_back(va);
        voice_amp_triangle.push_back(va);
        voice_amp_saw.push_back(va);
        voice_amp_pulse.push_back(va);

        pdsp::ValueControl sinec;
        sine_ctrl.push_back(sinec);
        pdsp::ValueControl _tc;
        triangle_ctrl.push_back(_tc);
        pdsp::ValueControl _sc;
        saw_ctrl.push_back(_sc);
        pdsp::ValueControl _pc;
        pulse_ctrl.push_back(_pc);

        pdsp::Amp _sa;
        sineLevel.push_back(_sa);
        pdsp::Amp _ta;
        triangleLevel.push_back(_ta);
        pdsp::Amp _sawa;
        sawLevel.push_back(_sawa);
        pdsp::Amp _pa;
        pulseLevel.push_back(_pa);
    }

    for(size_t i=0;i<MAX_OSC_VOICES;i++){
        // level
        level_ctrl.at(i) >> level.at(i).in_mod();
        level_ctrl.at(i).set(level_float);
        level_ctrl.at(i).enableSmoothing(50.0f);

        pw_ctrl >> osc.at(i).in_pw(); // pw (pulse width for the pulse waveform only)

        // pitches
        pitch_ctrl.at(i).set(pitch_float.at(i));
        pitch_ctrl.at(i).enableSmoothing(50.0f);

        // mutes
        voice_ctrl.at(i) >> voice_amp_sine.at(i).in_mod();
        voice_ctrl.at(i) >> voice_amp_triangle.at(i).in_mod();
        voice_ctrl.at(i) >> voice_amp_saw.at(i).in_mod();
        voice_ctrl.at(i) >> voice_amp_pulse.at(i).in_mod();
        voice_ctrl.at(i).set(voice_float.at(i));
        voice_ctrl.at(i).enableSmoothing(50.0f);
    }
    pw_ctrl.set(pw_float); // square wave
    pw_ctrl.enableSmoothing(50.0f);

    // detune
    detuneCoarse_ctrl.set(detune_float);
    detuneCoarse_ctrl.enableSmoothing(50.0f);
    detuneFine_ctrl.set(fine_float);
    detuneFine_ctrl.enableSmoothing(50.0f);

    for(size_t i=0;i<MAX_OSC_VOICES;i++){
        pitch_ctrl.at(i) + detuneCoarse_ctrl + detuneFine_ctrl >> osc.at(i).in_pitch();
    }


    for(size_t i=0;i<MAX_OSC_VOICES;i++){
        // waveform
        sine_ctrl.at(i) >> sineLevel.at(i).in_mod();
        sine_ctrl.at(i).set(sine_float);
        sine_ctrl.at(i).enableSmoothing(50.0f);
        triangle_ctrl.at(i) >> triangleLevel.at(i).in_mod();
        triangle_ctrl.at(i).set(triangle_float);
        triangle_ctrl.at(i).enableSmoothing(50.0f);
        saw_ctrl.at(i) >> sawLevel.at(i).in_mod();
        saw_ctrl.at(i).set(saw_float);
        saw_ctrl.at(i).enableSmoothing(50.0f);
        pulse_ctrl.at(i) >> pulseLevel.at(i).in_mod();
        pulse_ctrl.at(i).set(pulse_float);
        pulse_ctrl.at(i).enableSmoothing(50.0f);

        osc.at(i).out_sine() >> voice_amp_sine.at(i) >> sineLevel.at(i) >> level.at(i);
        osc.at(i).out_triangle() >> voice_amp_triangle.at(i) >> triangleLevel.at(i) >> level.at(i);
        osc.at(i).out_saw() >> voice_amp_saw.at(i) >> sawLevel.at(i) >> level.at(i);
        osc.at(i).out_pulse() >> voice_amp_pulse.at(i) >> pulseLevel.at(i) >> level.at(i);

        level.at(i) >> this->pdspOut[0];
        level.at(i) >> scope >> engine.blackhole();

        sineLevel.at(i) >> this->pdspOut[1];
        sineLevel.at(i) >> sine_scope >> engine.blackhole();
        triangleLevel.at(i) >> this->pdspOut[2];
        triangleLevel.at(i) >> triangle_scope >> engine.blackhole();
        sawLevel.at(i) >> this->pdspOut[3];
        sawLevel.at(i) >> saw_scope >> engine.blackhole();
        pulseLevel.at(i) >> this->pdspOut[4];
        pulseLevel.at(i) >> pulse_scope >> engine.blackhole();

    }



}

//--------------------------------------------------------------
void PolyphonicOscillator::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(this->inletsConnected[0] && static_cast<vector<float> *>(_inletParams[0])->size()>0){
        size_t counter = 0;
        size_t activeVoices = 0;
        for(size_t i=0;i<static_cast<vector<float> *>(_inletParams[0])->size();i++){
            if(static_cast<vector<float> *>(_inletParams[0])->at(i) > 0){
                activeVoices++;
            }
        }
        if(activeVoices > MAX_OSC_VOICES){
            activeVoices = MAX_OSC_VOICES;
        }
        for(size_t i=0;i<static_cast<vector<float> *>(_inletParams[0])->size();i++){
            if(static_cast<vector<float> *>(_inletParams[0])->at(i) > 0){
                if(counter<MAX_OSC_VOICES){
                    pitch_float.at(counter) = i;
                    pitch_ctrl.at(counter).set(pitch_float.at(counter));
                    voice_float.at(counter) = 1.0f/activeVoices;
                    voice_ctrl.at(counter).set(voice_float.at(counter));
                    counter++;
                }
            }
        }
        if(counter < MAX_OSC_VOICES){
            for(size_t i=counter;i<MAX_OSC_VOICES;i++){
                pitch_float.at(i) = 0.0f;
                pitch_ctrl.at(i).set(pitch_float.at(i));
                voice_float.at(i) = 0.0f;
                voice_ctrl.at(i).set(voice_float.at(i));
            }
        }
    }else{
        for(size_t i=0;i<MAX_OSC_VOICES;i++){
            pitch_float.at(i) = 0.0f;
            pitch_ctrl.at(i).set(pitch_float.at(i));
            voice_float.at(i) = 0.0f;
            voice_ctrl.at(i).set(voice_float.at(i));
        }
    }

    if(this->inletsConnected[1]){
        level_float = ofClamp(*(float *)&_inletParams[1],0.0f,1.0f);
        for(size_t i=0;i<MAX_OSC_VOICES;i++){
            level_ctrl.at(i).set(level_float);
        }
    }

    if(this->inletsConnected[2]){
        sine_float = ofClamp(*(float *)&_inletParams[2],0.0f,1.0f);
        for(size_t i=0;i<MAX_OSC_VOICES;i++){
            sine_ctrl.at(i).set(sine_float);
        }
    }

    if(this->inletsConnected[3]){
        triangle_float = ofClamp(*(float *)&_inletParams[3],0.0f,1.0f);
        for(size_t i=0;i<MAX_OSC_VOICES;i++){
            triangle_ctrl.at(i).set(triangle_float);
        }
    }

    if(this->inletsConnected[4]){
        saw_float = ofClamp(*(float *)&_inletParams[4],0.0f,1.0f);
        for(size_t i=0;i<MAX_OSC_VOICES;i++){
            saw_ctrl.at(i).set(saw_float);
        }
    }

    if(this->inletsConnected[5]){
        pulse_float = ofClamp(*(float *)&_inletParams[5],0.0f,1.0f);
        for(size_t i=0;i<MAX_OSC_VOICES;i++){
            pulse_ctrl.at(i).set(pulse_float);
        }
    }

    if(!loaded){
        loaded = true;

        level_float = ofClamp(this->getCustomVar("LEVEL"),0.0f,1.0f);

        detune_float = ofClamp(this->getCustomVar("DETUNE_COARSE"),-12.0f, 12.0f);
        detuneCoarse_ctrl.set(detune_float);
        fine_float = ofClamp(this->getCustomVar("DETUNE_FINE"),-1.0f,1.0f);
        detuneFine_ctrl.set(fine_float);
        pw_float = ofClamp(this->getCustomVar("PULSE_WIDTH"),0.0f,1.0f);
        pw_ctrl.set(pw_float);

        sine_float = ofClamp(this->getCustomVar("SINE_LEVEL"),0.0f,1.0f);
        triangle_float = ofClamp(this->getCustomVar("TRIANGLE_LEVEL"),0.0f,1.0f);
        saw_float = ofClamp(this->getCustomVar("SAW_LEVEL"),0.0f,1.0f);
        pulse_float = ofClamp(this->getCustomVar("PULSE_LEVEL"),0.0f,1.0f);

        for(size_t i=0;i<MAX_OSC_VOICES;i++){
            level_ctrl.at(i).set(level_float);
            sine_ctrl.at(i).set(sine_float);
            triangle_ctrl.at(i).set(triangle_float);
            saw_ctrl.at(i).set(saw_float);
            pulse_ctrl.at(i).set(pulse_float);
        }
    }

}

//--------------------------------------------------------------
void PolyphonicOscillator::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

    ofSetColor(0);

}

//--------------------------------------------------------------
void PolyphonicOscillator::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        // draw waveform
        ImGuiEx::drawWaveform(_nodeCanvas.getNodeDrawList(), ImVec2(ImGui::GetWindowSize().x,ImGui::GetWindowSize().y*0.3f), plot_data, bufferSize, 1.3f, IM_COL32(255,255,120,255), this->scaleFactor);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*scaleFactor));

        if(ImGuiKnobs::Knob("level", &level_float, 0.0f, 1.0f, 0.01f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            for(size_t i=0;i<MAX_OSC_VOICES;i++){
                level_ctrl.at(i).set(level_float);
            }
            this->setCustomVar(level_float,"LEVEL");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("detune", &detune_float, -12.0f, 12.0f, 0.005f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            detuneCoarse_ctrl.set(detune_float);
            this->setCustomVar(detune_float,"DETUNE_COARSE");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("fine", &fine_float, -1.0f, 1.0f, 0.005f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            detuneFine_ctrl.set(fine_float);
            this->setCustomVar(fine_float,"DETUNE_FINE");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("pw", &pw_float, 0.0f, 1.0f, 0.01f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            pw_ctrl.set(pw_float);
            this->setCustomVar(pw_float,"PULSE_WIDTH");
        }

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*8*scaleFactor));
        if(ImGuiKnobs::Knob("sine", &sine_float, 0.0f, 1.0f, 0.01f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            for(size_t i=0;i<MAX_OSC_VOICES;i++){
                sine_ctrl.at(i).set(sine_float);
            }
            this->setCustomVar(sine_float,"SINE_LEVEL");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("triangle", &triangle_float, 0.0f, 1.0f, 0.01f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            for(size_t i=0;i<MAX_OSC_VOICES;i++){
                triangle_ctrl.at(i).set(triangle_float);
            }
            this->setCustomVar(triangle_float,"TRIANGLE_LEVEL");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("saw", &saw_float, 0.0f, 1.0f, 0.01f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            for(size_t i=0;i<MAX_OSC_VOICES;i++){
                saw_ctrl.at(i).set(saw_float);
            }
            this->setCustomVar(saw_float,"SAW_LEVEL");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("pulse", &pulse_float, 0.0f, 1.0f, 0.01f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            for(size_t i=0;i<MAX_OSC_VOICES;i++){
                pulse_ctrl.at(i).set(pulse_float);
            }
            this->setCustomVar(pulse_float,"PULSE_LEVEL");
        }

        _nodeCanvas.EndNodeContent();

    }

}

//--------------------------------------------------------------
void PolyphonicOscillator::drawObjectNodeConfig(){

    ImGui::Spacing();

    ImGuiEx::ObjectInfo(
                "Polyphonic Oscillator, with up to 16 voices, with antialiased waveforms",
                "https://mosaic.d3cod3.org/reference.php?r=polyphonic-oscillator", scaleFactor);
}

//--------------------------------------------------------------
void PolyphonicOscillator::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);

    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void PolyphonicOscillator::loadAudioSettings(){
    ofxXmlSettings XML;

#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
    if (XML.loadFile(patchFile)){
#else
    if (XML.load(patchFile)){
#endif
        if (XML.pushTag("settings")){
            sampleRate = XML.getValue("sample_rate_in",0);
            bufferSize = XML.getValue("buffer_size",0);

            plot_data = new float[bufferSize];
            for(int i=0;i<bufferSize;i++){
                static_cast<vector<float> *>(_outletParams[5])->push_back(0.0f);
                plot_data[i] = 0.0f;
            }

            XML.popTag();
        }
    }
}

//--------------------------------------------------------------
void PolyphonicOscillator::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);

    for(size_t i = 0; i < scope.getBuffer().size(); i++) {
        float sample = scope.getBuffer().at(i);
        plot_data[i] = hardClip(sample);

        // SIGNAL BUFFER DATA
        static_cast<vector<float> *>(_outletParams[5])->at(i) = sample;
    }
    // SIGNALS BUFFERS
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[1])->copyFrom(sine_scope.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[2])->copyFrom(triangle_scope.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[3])->copyFrom(saw_scope.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[4])->copyFrom(pulse_scope.getBuffer().data(), bufferSize, 1, sampleRate);
}

OBJECT_REGISTER( PolyphonicOscillator, "polyphonic oscillator", OFXVP_OBJECT_CAT_SOUND)

#endif
