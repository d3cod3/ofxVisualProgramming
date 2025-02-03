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

#include "pdspBitNoise.h"

//--------------------------------------------------------------
pdspBitNoise::pdspBitNoise() : PatchObject("bit noise"){

    this->numInlets  = 3;
    this->numOutlets = 3;

    _inletParams[0] = new float();          // pitch
    *(float *)&_inletParams[0] = 0.0f;
    _inletParams[1] = new float();          // decimation
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();          // bits
    *(float *)&_inletParams[2] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output L
    _outletParams[1] = new ofSoundBuffer(); // audio output R
    _outletParams[2] = new vector<float>(); // audio buffer

    this->initInletsState();

    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    pitch                   = -100.0f;
    decimation              = 151.0f;
    bits                    = 8.0f;

    loaded                  = false;

    this->width *= 2.12f;
    this->height *= 1.12f;

}

//--------------------------------------------------------------
void pdspBitNoise::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"pitch");
    this->addInlet(VP_LINK_NUMERIC,"decimation");
    this->addInlet(VP_LINK_NUMERIC,"bits");

    this->addOutlet(VP_LINK_AUDIO,"signalL");
    this->addOutlet(VP_LINK_AUDIO,"signalR");
    this->addOutlet(VP_LINK_ARRAY,"dataBuffer");

    this->setCustomVar(pitch,"PITCH");
    this->setCustomVar(decimation,"DECIMATION");
    this->setCustomVar(bits,"BITS");
}

//--------------------------------------------------------------
void pdspBitNoise::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadAudioSettings();
}

//--------------------------------------------------------------
void pdspBitNoise::setupAudioOutObjectContent(pdsp::Engine &engine){

    pitch_ctrl >> noise.in_pitch();
    pitch_ctrl.set(pitch);
    pitch_ctrl.enableSmoothing(50.0f);

    decimation_ctrl >> noise.in_decimation();
    decimation_ctrl.set(decimation);
    decimation_ctrl.enableSmoothing(50.0f);

    bits_ctrl >> noise.in_bits();
    bits_ctrl.set(bits);
    bits_ctrl.enableSmoothing(50.0f);

    noise.ch(0) >> this->pdspOut[0];
    noise.ch(1) >> this->pdspOut[1];
    noise.ch(0) >> scopeL >> engine.blackhole();
    noise.ch(1) >> scopeR >> engine.blackhole();
}

//--------------------------------------------------------------
void pdspBitNoise::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[0]){
        pitch = ofClamp(*(float *)&_inletParams[0],-100,150);
        pitch_ctrl.set(pitch);
    }

    if(this->inletsConnected[1]){
        decimation = ofClamp(*(float *)&_inletParams[1],1,200);
        decimation_ctrl.set(decimation);
    }

    if(this->inletsConnected[2]){
        bits = ofClamp(*(float *)&_inletParams[2],0.0f,8.0f);
        bits_ctrl.set(bits);
    }

    if(!loaded){
        loaded = true;
        pitch = ofClamp(this->getCustomVar("PITCH"),-100,150);
        pitch_ctrl.set(pitch);
        decimation = ofClamp(this->getCustomVar("DECIMATION"),1,200);
        decimation_ctrl.set(decimation);
        bits = ofClamp(this->getCustomVar("BITS"),0.0f,8.0f);
        bits_ctrl.set(bits);
    }

}

//--------------------------------------------------------------
void pdspBitNoise::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(0);

}

//--------------------------------------------------------------
void pdspBitNoise::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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
        if (ImGuiKnobs::Knob("pitch", &pitch, -100.0f, 150.0f, 1.0f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)) {
            pitch_ctrl.set(pitch);
            this->setCustomVar(pitch,"PITCH");
        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,0,90)*scaleFactor,-1));ImGui::SameLine();
        if (ImGuiKnobs::Knob("decimation", &decimation, 1.0f, 200.0f, 1.0f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)) {
            decimation_ctrl.set(decimation);
            this->setCustomVar(decimation,"DECIMATION");
        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,0,90)*scaleFactor,-1));ImGui::SameLine();
        if (ImGuiKnobs::Knob("bits", &bits, 0.0f, 8.0f, 0.07f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)) {
            bits_ctrl.set(bits);
            this->setCustomVar(bits,"BITS");
        }

        _nodeCanvas.EndNodeContent();

    }

}

//--------------------------------------------------------------
void pdspBitNoise::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Stereo Digital noise generator",
                "https://mosaic.d3cod3.org/reference.php?r=bit-noise", scaleFactor);
}

//--------------------------------------------------------------
void pdspBitNoise::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspBitNoise::loadAudioSettings(){
    ofxXmlSettings XML;

#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
    if (XML.loadFile(patchFile)){
#else
    if (XML.load(patchFile)){
#endif
        if (XML.pushTag("settings")){
            sampleRate = XML.getValue("sample_rate_in",0);
            bufferSize = XML.getValue("buffer_size",0);

            for(int i=0;i<bufferSize;i++){
                static_cast<vector<float> *>(_outletParams[2])->push_back(0.0f);
            }

            XML.popTag();
        }
    }
}

//--------------------------------------------------------------
void pdspBitNoise::audioOutObject(ofSoundBuffer &outputBuffer){
    for(size_t i = 0; i < scopeL.getBuffer().size(); i++) {
        float sample = (scopeL.getBuffer().at(i) + scopeR.getBuffer().at(i))/2;

        // SIGNAL BUFFER DATA
        static_cast<vector<float> *>(_outletParams[2])->at(i) = sample;
    }
    // SIGNAL BUFFER
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scopeL.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[1])->copyFrom(scopeR.getBuffer().data(), bufferSize, 1, sampleRate);
}


OBJECT_REGISTER( pdspBitNoise, "bit noise", OFXVP_OBJECT_CAT_SOUND)

#endif
