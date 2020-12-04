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

#include "pdspDecimator.h"

//--------------------------------------------------------------
pdspDecimator::pdspDecimator() : PatchObject("decimator"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();  // audio input
    _inletParams[1] = new float();          // sample rate freq
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output

    this->initInletsState();

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    freq                    = 800.0f;

    loaded                  = false;

}

//--------------------------------------------------------------
void pdspDecimator::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"freq");

    this->addOutlet(VP_LINK_AUDIO,"decimatedSignal");

    this->setCustomVar(freq,"FREQUENCY");
}

//--------------------------------------------------------------
void pdspDecimator::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();
}

//--------------------------------------------------------------
void pdspDecimator::setupAudioOutObjectContent(pdsp::Engine &engine){
    freq_ctrl >> decimator.in_freq();
    freq_ctrl.set(freq);
    freq_ctrl.enableSmoothing(50.0f);

    this->pdspIn[0] >> decimator >> this->pdspOut[0];
    this->pdspIn[0] >> decimator >> scope >> engine.blackhole();
}

//--------------------------------------------------------------
void pdspDecimator::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[1]){
        freq = ofClamp(*(float *)&_inletParams[1],2.0f,1600.0f);
        freq_ctrl.set(freq);
    }

    if(!loaded){
        loaded = true;
        freq = ofClamp(this->getCustomVar("FREQUENCY"),2.0f,1600.0f);
        freq_ctrl.set(freq);
    }

}

//--------------------------------------------------------------
void pdspDecimator::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void pdspDecimator::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    // CONFIG GUI inside Menu
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

        ImGui::Dummy(ImVec2(-1,ImGui::GetWindowSize().y/2 - (26*scaleFactor))); // Padding top
        ImGui::PushItemWidth(-1);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(255,255,120,30));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(255,255,120,60));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(255,255,120,60));
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(255,255,120,160));
        if(ImGui::SliderFloat("",&freq,2.0f,1600.0f)){
            freq_ctrl.set(freq);
            this->setCustomVar(freq,"FREQUENCY");
        }
        ImGui::PopStyleColor(4);
        ImGui::PopItemWidth();

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void pdspDecimator::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Sample Rate Decimator, reduce the sample rate of the input signal",
                "https://mosaic.d3cod3.org/reference.php?r=decimator", scaleFactor);
}

//--------------------------------------------------------------
void pdspDecimator::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspDecimator::loadAudioSettings(){
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
void pdspDecimator::audioInObject(ofSoundBuffer &inputBuffer){

}

//--------------------------------------------------------------
void pdspDecimator::audioOutObject(ofSoundBuffer &outputBuffer){
    // SIGNAL BUFFER
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}


OBJECT_REGISTER( pdspDecimator, "decimator", OFXVP_OBJECT_CAT_SOUND)

#endif
