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

#include "pdspCompressor.h"

//--------------------------------------------------------------
pdspCompressor::pdspCompressor() : PatchObject("compressor"){

    this->numInlets  = 6;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer(); // audio input

    _inletParams[1] = new float();          // attack
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]) = 0.0f;
    _inletParams[2] = new float();          // release
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[2]) = 0.0f;
    _inletParams[3] = new float();          // thresh
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[3]) = 0.0f;
    _inletParams[4] = new float();          // ratio
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[4]) = 0.0f;
    _inletParams[5] = new float();          // knee
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[5]) = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output

    this->initInletsState();

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    loaded                  = false;

    attack                  = 12.0f;
    release                 = 12.0f;
    thresh                  = -20.0f;
    ratio                   = 12.0f;
    knee                    = 0.0f;

    this->width *= 2.25f;
    this->height *= 1.12f;

}

//--------------------------------------------------------------
void pdspCompressor::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"attack");
    this->addInlet(VP_LINK_NUMERIC,"release");
    this->addInlet(VP_LINK_NUMERIC,"thresh");
    this->addInlet(VP_LINK_NUMERIC,"ratio");
    this->addInlet(VP_LINK_NUMERIC,"knee");

    this->addOutlet(VP_LINK_AUDIO,"compressedSignal");

    this->setCustomVar(attack,"ATTACK");
    this->setCustomVar(release,"RELEASE");
    this->setCustomVar(thresh,"THRESH");
    this->setCustomVar(ratio,"RATIO");
    this->setCustomVar(knee,"KNEE");
}

//--------------------------------------------------------------
void pdspCompressor::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadAudioSettings();

}

//--------------------------------------------------------------
void pdspCompressor::setupAudioOutObjectContent(pdsp::Engine &engine){

    attack_ctrl >> compressor.in_attack(); // 1 - 100 ms
    attack_ctrl.set(attack);
    attack_ctrl.enableSmoothing(50.0f);

    release_ctrl >> compressor.in_release();// 1 - 100 ms
    release_ctrl.set(release);
    release_ctrl.enableSmoothing(50.0f);

    thresh_ctrl >> compressor.in_threshold(); // -48dB - 0dB
    thresh_ctrl.set(thresh);
    thresh_ctrl.enableSmoothing(50.0f);

    ratio_ctrl >> compressor.in_ratio(); // 1 - 100
    ratio_ctrl.set(ratio);
    ratio_ctrl.enableSmoothing(50.0f);

    knee_ctrl >> compressor.in_knee(); // -48dB - 0dB
    knee_ctrl.set(knee);
    knee_ctrl.enableSmoothing(50.0f);

    compressor.analog();

    this->pdspIn[0] >> compressor >> this->pdspOut[0];
    this->pdspIn[0] >> compressor >> scope >> engine.blackhole();

}

//--------------------------------------------------------------
void pdspCompressor::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    // attack
    if(this->inletsConnected[1]){
        if(attack != *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1])){
            attack = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]),1.0f,100.0f);
            attack_ctrl.set(attack);
            this->setCustomVar(attack,"ATTACK");
        }
    }

    // release
    if(this->inletsConnected[2]){
        if(release != *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[2])){
            release = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[2]),1.0f,100.0f);
            release_ctrl.set(release);
            this->setCustomVar(release,"RELEASE");
        }
    }

    // thresh
    if(this->inletsConnected[3]){
        if(thresh != *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[3])){
            thresh = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[3]),-48.0f,0.0f);
            thresh_ctrl.set(thresh);
            this->setCustomVar(thresh,"THRESH");
        }
    }

    // ratio
    if(this->inletsConnected[4]){
        if(ratio != *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[4])){
            ratio = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[4]),1.0f,100.0f);
            ratio_ctrl.set(ratio);
            this->setCustomVar(ratio,"RATIO");
        }
    }

    // knee
    if(this->inletsConnected[5]){
        if(knee != *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[5])){
            knee = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[5]),-48.0f,0.0f);
            knee_ctrl.set(knee);
            this->setCustomVar(knee,"KNEE");
        }
    }

    if(!loaded){
        loaded = true;
        attack = this->getCustomVar("ATTACK");
        release = this->getCustomVar("RELEASE");
        thresh = this->getCustomVar("THRESH");
        ratio = this->getCustomVar("RATIO");
        knee = this->getCustomVar("KNEE");
    }
}

//--------------------------------------------------------------
void pdspCompressor::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void pdspCompressor::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*scaleFactor));

        if(ImGuiKnobs::Knob("attack", &attack, 1.0f, 100.0f, 0.1f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            attack_ctrl.set(attack);
            this->setCustomVar(attack,"ATTACK");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("release", &release, 1.0f, 100.0f, 0.1f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            release_ctrl.set(release);
            this->setCustomVar(release,"RELEASE");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("thresh", &thresh, -48.0f, 0.0f, 0.1f, "%.2fdb", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            thresh_ctrl.set(thresh);
            this->setCustomVar(thresh,"THRESH");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("ratio", &ratio, 1.0f, 100.0f, 0.1f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            ratio_ctrl.set(ratio);
            this->setCustomVar(ratio,"RATIO");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("knee", &knee, -48.0f, 0.0f, 0.1f, "%.2fdb", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            knee_ctrl.set(knee);
            this->setCustomVar(knee,"KNEE");
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void pdspCompressor::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Feed-forward compressor.",
                "https://mosaic.d3cod3.org/reference.php?r=compressor", scaleFactor);
}

//--------------------------------------------------------------
void pdspCompressor::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspCompressor::loadAudioSettings(){
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
void pdspCompressor::audioOutObject(ofSoundBuffer &outputBuffer){
    // SIGNAL BUFFER
    ofxVP_CAST_PIN_PTR<ofSoundBuffer>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}

OBJECT_REGISTER( pdspCompressor, "compressor", OFXVP_OBJECT_CAT_SOUND)

#endif
