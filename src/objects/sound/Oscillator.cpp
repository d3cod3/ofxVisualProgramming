/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2018 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "Oscillator.h"

//--------------------------------------------------------------
Oscillator::Oscillator() : PatchObject("oscillator"){

    this->numInlets  = 7;
    this->numOutlets = 7;

    _inletParams[0] = new float();  // pitch
    *(float *)&_inletParams[0] = 0.0f;

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

    _inletParams[6] = new float();  // noise
    *(float *)&_inletParams[6] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // osc output
    _outletParams[1] = new ofSoundBuffer(); // sine output
    _outletParams[2] = new ofSoundBuffer(); // triangle output
    _outletParams[3] = new ofSoundBuffer(); // saw output
    _outletParams[4] = new ofSoundBuffer(); // pulse output
    _outletParams[5] = new ofSoundBuffer(); // noise output
    _outletParams[6] = new vector<float>(); // audio buffer

    this->initInletsState();

    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    loaded                  = false;

    level_float             = 0.5f;
    pitch_float             = 72.0f;
    detune_float            = 0.0f;
    fine_float              = 0.0f;
    pw_float                = 0.5f;

    sine_float              = 0.0f;
    triangle_float          = 0.0f;
    saw_float               = 0.0f;
    pulse_float             = 0.0f;
    noise_float             = 0.0f;

    this->width *= 2;
    this->height *= 2.7f;

}

//--------------------------------------------------------------
void Oscillator::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"pitch");
    this->addInlet(VP_LINK_NUMERIC,"level");
    this->addInlet(VP_LINK_NUMERIC,"sine level");
    this->addInlet(VP_LINK_NUMERIC,"triangle level");
    this->addInlet(VP_LINK_NUMERIC,"saw level");
    this->addInlet(VP_LINK_NUMERIC,"pulse level");
    this->addInlet(VP_LINK_NUMERIC,"noise level");

    this->addOutlet(VP_LINK_AUDIO,"signal");
    this->addOutlet(VP_LINK_AUDIO,"sine");
    this->addOutlet(VP_LINK_AUDIO,"triangle");
    this->addOutlet(VP_LINK_AUDIO,"saw");
    this->addOutlet(VP_LINK_AUDIO,"pulse");
    this->addOutlet(VP_LINK_AUDIO,"noise");
    this->addOutlet(VP_LINK_ARRAY,"dataBuffer");

    this->setCustomVar(pitch_float,"PITCH");
    this->setCustomVar(level_float,"LEVEL");
    this->setCustomVar(detune_float,"DETUNE_COARSE");
    this->setCustomVar(fine_float,"DETUNE_FINE");
    this->setCustomVar(pw_float,"PULSE_WIDTH");

    this->setCustomVar(sine_float,"SINE_LEVEL");
    this->setCustomVar(triangle_float,"TRIANGLE_LEVEL");
    this->setCustomVar(saw_float,"SAW_LEVEL");
    this->setCustomVar(pulse_float,"PULSE_LEVEL");
    this->setCustomVar(noise_float,"NOISE_LEVEL");
}

//--------------------------------------------------------------
void Oscillator::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();

}

//--------------------------------------------------------------
void Oscillator::setupAudioOutObjectContent(pdsp::Engine &engine){
    // level
    level_ctrl >> level.in_mod();
    level_ctrl.set(level_float);
    level_ctrl.enableSmoothing(50.0f);

    // pw (pulse width for the pulse waveform only)
    pw_ctrl >> osc.in_pw();
    pw_ctrl.set(pw_float); // square wave
    pw_ctrl.enableSmoothing(50.0f);

    // pitch
    pitch_ctrl.set(pitch_float);
    pitch_ctrl.enableSmoothing(50.0f);

    // detune
    detuneCoarse_ctrl.set(detune_float);
    detuneCoarse_ctrl.enableSmoothing(50.0f);
    detuneFine_ctrl.set(fine_float);
    detuneFine_ctrl.enableSmoothing(50.0f);
    pitch_ctrl + detuneCoarse_ctrl + detuneFine_ctrl >> osc.in_pitch();

    // waveform
    sine_ctrl >> sineLevel.in_mod();
    sine_ctrl.set(sine_float);
    sine_ctrl.enableSmoothing(50.0f);
    triangle_ctrl >> triangleLevel.in_mod();
    triangle_ctrl.set(triangle_float);
    triangle_ctrl.enableSmoothing(50.0f);
    saw_ctrl >> sawLevel.in_mod();
    saw_ctrl.set(saw_float);
    saw_ctrl.enableSmoothing(50.0f);
    pulse_ctrl >> pulseLevel.in_mod();
    pulse_ctrl.set(pulse_float);
    pulse_ctrl.enableSmoothing(50.0f);
    noise_ctrl >> noiseLevel.in_mod();
    noise_ctrl.set(noise_float);
    noise_ctrl.enableSmoothing(50.0f);

    osc.out_sine() >> sineLevel >> level;
    osc.out_triangle() >> triangleLevel >> level;
    osc.out_saw() >> sawLevel >> level;
    osc.out_pulse() >> pulseLevel >> level;
    noise >> noiseLevel >> level;


    level >> this->pdspOut[0];
    level >> scope >> engine.blackhole();

    sineLevel >> this->pdspOut[1];
    sineLevel >> sine_scope >> engine.blackhole();
    triangleLevel >> this->pdspOut[2];
    triangleLevel >> triangle_scope >> engine.blackhole();
    sawLevel >> this->pdspOut[3];
    sawLevel >> saw_scope >> engine.blackhole();
    pulseLevel >> this->pdspOut[4];
    pulseLevel >> pulse_scope >> engine.blackhole();
    noiseLevel >> this->pdspOut[5];
    noiseLevel >> noise_scope >> engine.blackhole();

}

//--------------------------------------------------------------
void Oscillator::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[0]){
        pitch_float = ofClamp(*(float *)&_inletParams[0],0,127);
        pitch_ctrl.set(pitch_float);
    }

    if(this->inletsConnected[1]){
        level_float = ofClamp(*(float *)&_inletParams[1],0.0f,1.0f);
        level_ctrl.set(level_float);
    }

    if(this->inletsConnected[2]){
        sine_float = ofClamp(*(float *)&_inletParams[2],0.0f,1.0f);
        sine_ctrl.set(sine_float);
    }

    if(this->inletsConnected[3]){
        triangle_float = ofClamp(*(float *)&_inletParams[3],0.0f,1.0f);
        triangle_ctrl.set(triangle_float);
    }

    if(this->inletsConnected[4]){
        saw_float = ofClamp(*(float *)&_inletParams[4],0.0f,1.0f);
        saw_ctrl.set(saw_float);
    }

    if(this->inletsConnected[5]){
        pulse_float = ofClamp(*(float *)&_inletParams[5],0.0f,1.0f);
        pulse_ctrl.set(pulse_float);
    }

    if(this->inletsConnected[6]){
        noise_float = ofClamp(*(float *)&_inletParams[6],0.0f,1.0f);
        noise_ctrl.set(noise_float);
    }

    if(!loaded){
        loaded = true;
        pitch_float = ofClamp(this->getCustomVar("PITCH"),0,127);
        pitch_ctrl.set(pitch_float);
        level_float = ofClamp(this->getCustomVar("LEVEL"),0.0f,1.0f);
        level_ctrl.set(level_float);
        detune_float = ofClamp(this->getCustomVar("DETUNE_COARSE"),-12.0f, 12.0f);
        detuneCoarse_ctrl.set(detune_float);
        fine_float = ofClamp(this->getCustomVar("DETUNE_FINE"),-1.0f,1.0f);
        detuneFine_ctrl.set(fine_float);
        pw_float = ofClamp(this->getCustomVar("PULSE_WIDTH"),0.0f,1.0f);
        pw_ctrl.set(pw_float);

        sine_float = ofClamp(this->getCustomVar("SINE_LEVEL"),0.0f,1.0f);
        sine_ctrl.set(sine_float);
        triangle_float = ofClamp(this->getCustomVar("TRIANGLE_LEVEL"),0.0f,1.0f);
        triangle_ctrl.set(triangle_float);
        saw_float = ofClamp(this->getCustomVar("SAW_LEVEL"),0.0f,1.0f);
        saw_ctrl.set(saw_float);
        pulse_float = ofClamp(this->getCustomVar("PULSE_LEVEL"),0.0f,1.0f);
        pulse_ctrl.set(pulse_float);
        noise_float = ofClamp(this->getCustomVar("NOISE_LEVEL"),0.0f,1.0f);
        noise_ctrl.set(noise_float);
    }

}

//--------------------------------------------------------------
void Oscillator::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(0);

}

//--------------------------------------------------------------
void Oscillator::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    if(_nodeCanvas.BeginNodeMenu()){
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            ImGuiEx::ObjectInfo(
                        "Oscillator with antialiased waveforms",
                        "https://mosaic.d3cod3.org/reference.php?r=oscillator");

            ImGui::EndMenu();
        }
        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        // draw waveform
        ImGuiEx::drawWaveform(_nodeCanvas.getNodeDrawList(), ImVec2(ImGui::GetWindowSize().x,ImGui::GetWindowSize().y*0.3f), plot_data, 1024, 1.3f, IM_COL32(255,255,120,255));

        char temp[128];
        sprintf(temp,"%.2f Hz", pdsp::PitchToFreq::eval(pitch_float+detune_float+fine_float));
        _nodeCanvas.getNodeDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(ImGui::GetWindowPos().x + (40*_nodeCanvas.GetCanvasScale()), ImGui::GetWindowPos().y + (ImGui::GetWindowSize().y*0.24)), IM_COL32_WHITE,temp, NULL, 0.0f);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-46)/11, IM_COL32(255,255,120,255), "pitch", &pitch_float, 0.0f, 127.0f, 1270.0f)){
            this->setCustomVar(pitch_float,"PITCH");
            pitch_ctrl.set(pitch_float);
        }
        ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-46)/11, IM_COL32(255,255,120,255), "level", &level_float, 0.0f, 1.0f, 100.0f)){
            level_ctrl.set(level_float);
            this->setCustomVar(level_float,"LEVEL");
        }
        ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-46)/11, IM_COL32(255,255,120,255), "detune", &detune_float, -12.0f, 12.0f, 240.0f)){
            detuneCoarse_ctrl.set(detune_float);
            this->setCustomVar(detune_float,"DETUNE_COARSE");
        }
        ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-46)/11, IM_COL32(255,255,120,255), "fine", &fine_float, -1.0f, 1.0f, 2000.0f)){
            detuneFine_ctrl.set(fine_float);
            this->setCustomVar(fine_float,"DETUNE_FINE");
        }
        ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-46)/11, IM_COL32(255,255,120,255), "pw", &pw_float, 0.0f, 1.0f, 100.0f)){
            pw_ctrl.set(pw_float);
            this->setCustomVar(pw_float,"PULSE_WIDTH");
        }

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*8));
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-46)/11, IM_COL32(255,255,120,255), "sine", &sine_float, 0.0f, 1.0f, 100.0f)){
            sine_ctrl.set(sine_float);
            this->setCustomVar(sine_float,"SINE_LEVEL");
        }
        ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-46)/11, IM_COL32(255,255,120,255), "triangle", &triangle_float, 0.0f, 1.0f, 100.0f)){
            triangle_ctrl.set(triangle_float);
            this->setCustomVar(triangle_float,"TRIANGLE_LEVEL");
        }
        ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-46)/11, IM_COL32(255,255,120,255), "saw", &saw_float, 0.0f, 1.0f, 100.0f)){
            saw_ctrl.set(saw_float);
            this->setCustomVar(saw_float,"SAW_LEVEL");
        }
        ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-46)/11, IM_COL32(255,255,120,255), "pulse", &pulse_float, 0.0f, 1.0f, 100.0f)){
            pulse_ctrl.set(pulse_float);
            this->setCustomVar(pulse_float,"PULSE_LEVEL");
        }
        ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-46)/11, IM_COL32(255,255,120,255), "noise", &noise_float, 0.0f, 1.0f, 100.0f)){
            noise_ctrl.set(noise_float);
            this->setCustomVar(noise_float,"NOISE_LEVEL");
        }

    }


}

//--------------------------------------------------------------
void Oscillator::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void Oscillator::loadAudioSettings(){
    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){
        if (XML.pushTag("settings")){
            sampleRate = XML.getValue("sample_rate_in",0);
            bufferSize = XML.getValue("buffer_size",0);

            for(int i=0;i<bufferSize;i++){
                static_cast<vector<float> *>(_outletParams[6])->push_back(0.0f);
                plot_data[i] = 0.0f;
            }

            XML.popTag();
        }
    }
}

//--------------------------------------------------------------
void Oscillator::audioOutObject(ofSoundBuffer &outputBuffer){

    for(size_t i = 0; i < scope.getBuffer().size(); i++) {
        float sample = scope.getBuffer().at(i);
        plot_data[i] = hardClip(sample);

        // SIGNAL BUFFER DATA
        static_cast<vector<float> *>(_outletParams[6])->at(i) = sample;
    }
    // SIGNALS BUFFERS
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[1])->copyFrom(sine_scope.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[2])->copyFrom(triangle_scope.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[3])->copyFrom(saw_scope.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[4])->copyFrom(pulse_scope.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[5])->copyFrom(noise_scope.getBuffer().data(), bufferSize, 1, sampleRate);
}

OBJECT_REGISTER( Oscillator, "oscillator", OFXVP_OBJECT_CAT_SOUND)

#endif
