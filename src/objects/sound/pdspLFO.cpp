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

#include "pdspLFO.h"

//--------------------------------------------------------------
pdspLFO::pdspLFO() : PatchObject("lfo"){

    this->numInlets  = 3;
    this->numOutlets = 5;

    _inletParams[0] = new float();  // retrig (bang)
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]) = 0.0f;
    _inletParams[1] = new float();  // frequency
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]) = 0.0f;
    _inletParams[2] = new float();  // phase
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[2]) = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // triangle LFO
    _outletParams[1] = new ofSoundBuffer(); // sine     LFO
    _outletParams[2] = new ofSoundBuffer(); // saw      LFO
    _outletParams[3] = new ofSoundBuffer(); // square   LFO
    _outletParams[4] = new ofSoundBuffer(); // random   LFO

    this->initInletsState();

    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    pitch                   = 0.5f;
    phase                   = 0.0f;

    loaded                  = false;

    this->width             *= 1.3;
    this->height            *= 1.1;

}

//--------------------------------------------------------------
void pdspLFO::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"frequency");
    this->addInlet(VP_LINK_NUMERIC,"phase");

    this->addOutlet(VP_LINK_AUDIO,"triangle");
    this->addOutlet(VP_LINK_AUDIO,"sine");
    this->addOutlet(VP_LINK_AUDIO,"saw");
    this->addOutlet(VP_LINK_AUDIO,"square");
    this->addOutlet(VP_LINK_AUDIO,"random");

    this->setCustomVar(pitch,"FREQUENCY");
    this->setCustomVar(phase,"PHASE");
}

//--------------------------------------------------------------
void pdspLFO::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadAudioSettings();
}

//--------------------------------------------------------------
void pdspLFO::setupAudioOutObjectContent(pdsp::Engine &engine){

    retrig_ctrl.out_trig() >> lfo.in_retrig();

    pitch_ctrl >> lfo.in_freq();
    pitch_ctrl.set(0.5f);
    pitch_ctrl.enableSmoothing(50.0f);

    phase_ctrl >> lfo.in_phase_start();
    phase_ctrl.set(0.0f);
    phase_ctrl.enableSmoothing(50.0f);

    lfo.out_triangle() >> this->pdspOut[0];
    lfo.out_triangle() >> scope_tri >> engine.blackhole();

    lfo.out_sine() >> this->pdspOut[1];
    lfo.out_sine() >> scope_sine >> engine.blackhole();

    lfo.out_saw() >> this->pdspOut[2];
    lfo.out_saw() >> scope_saw >> engine.blackhole();

    lfo.out_square() >> this->pdspOut[3];
    lfo.out_square() >> scope_square >> engine.blackhole();

    lfo.out_random() >> this->pdspOut[4];
    lfo.out_random() >> scope_random >> engine.blackhole();
}

//--------------------------------------------------------------
void pdspLFO::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);
    // retrig
    if(this->inletsConnected[0]){
        retrig_ctrl.trigger(ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]),0.0f,1.0f));
    }else{
        retrig_ctrl.off();
    }

    // frequency
    if(this->inletsConnected[1]){
        pitch = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]),0.0f,10.0f);
        pitch_ctrl.set(pitch);
    }

    // phase
    if(this->inletsConnected[2]){
        phase = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[2]),-1.0f,1.0f);
        phase_ctrl.set(phase);
    }

    if(!loaded){
        loaded = true;
        pitch = ofClamp(this->getCustomVar("FREQUENCY"),0.0f,10.0f);
        phase = ofClamp(this->getCustomVar("PHASE"),-1.0,1.0);
        pitch_ctrl.set(pitch);
        phase_ctrl.set(phase);
    }

}

//--------------------------------------------------------------
void pdspLFO::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);
    ofSetColor(0);
}

//--------------------------------------------------------------
void pdspLFO::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGui::Dummy(ImVec2(0,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,-20,40)*scaleFactor));
        if(ImGuiKnobs::Knob("pitch", &pitch, 0.0f, 10.0f, 0.01f, "%.5f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            pitch_ctrl.set(pitch);
            this->setCustomVar(pitch,"FREQUENCY");

        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,0,80)*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiKnobs::Knob("phase", &phase, -1.0f, 1.0f, 0.001f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            phase_ctrl.set(phase);
            this->setCustomVar(phase,"PHASE");
        }

        _nodeCanvas.EndNodeContent();

    }

}

//--------------------------------------------------------------
void pdspLFO::drawObjectNodeConfig(){

    ImGui::Spacing();
    ImGui::Text("%.3f Hz",pitch);
    ImGui::Spacing();

    ImGuiEx::ObjectInfo(
                "Low Frequency Oscillator",
                "https://mosaic.d3cod3.org/reference.php?r=lfo", scaleFactor);
}

//--------------------------------------------------------------
void pdspLFO::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspLFO::loadAudioSettings(){
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
void pdspLFO::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);
    // SIGNAL BUFFER
    ofxVP_CAST_PIN_PTR<ofSoundBuffer>(_outletParams[0])->copyFrom(scope_tri.getBuffer().data(), bufferSize, 1, sampleRate);
    ofxVP_CAST_PIN_PTR<ofSoundBuffer>(_outletParams[1])->copyFrom(scope_sine.getBuffer().data(), bufferSize, 1, sampleRate);
    ofxVP_CAST_PIN_PTR<ofSoundBuffer>(_outletParams[2])->copyFrom(scope_saw.getBuffer().data(), bufferSize, 1, sampleRate);
    ofxVP_CAST_PIN_PTR<ofSoundBuffer>(_outletParams[3])->copyFrom(scope_square.getBuffer().data(), bufferSize, 1, sampleRate);
    ofxVP_CAST_PIN_PTR<ofSoundBuffer>(_outletParams[4])->copyFrom(scope_random.getBuffer().data(), bufferSize, 1, sampleRate);
}



OBJECT_REGISTER( pdspLFO, "lfo", OFXVP_OBJECT_CAT_SOUND)

#endif
