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

#include "Mixer.h"

//--------------------------------------------------------------
Mixer::Mixer() : PatchObject("mixer"){

    this->numInlets  = 7;
    this->numOutlets = 1;

    _inletParams[0] = new vector<float>();

    _inletParams[1] = new ofSoundBuffer();  // audio input 1
    _inletParams[2] = new ofSoundBuffer();  // audio input 2
    _inletParams[3] = new ofSoundBuffer();  // audio input 3
    _inletParams[4] = new ofSoundBuffer();  // audio input 4
    _inletParams[5] = new ofSoundBuffer();  // audio input 5
    _inletParams[6] = new ofSoundBuffer();  // audio input 6

    _outletParams[0] = new ofSoundBuffer(); // audio output

    this->initInletsState();

    signalInlets    = this->numInlets-1;

    levels          = new pdsp::Amp[signalInlets];
    levels_ctrl     = new pdsp::ValueControl[signalInlets];
    levels_float    = new float[signalInlets];

    needReset       = false;
    loaded          = false;

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;


}

//--------------------------------------------------------------
void Mixer::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"control");
    this->addInlet(VP_LINK_AUDIO,"s1");
    this->addInlet(VP_LINK_AUDIO,"s2");
    this->addInlet(VP_LINK_AUDIO,"s3");
    this->addInlet(VP_LINK_AUDIO,"s4");
    this->addInlet(VP_LINK_AUDIO,"s5");
    this->addInlet(VP_LINK_AUDIO,"s6");

    this->addOutlet(VP_LINK_AUDIO,"mainOutput");

    this->setCustomVar(static_cast<float>(signalInlets),"NUM_INLETS");

    for(int i=0;i<signalInlets;i++){
        levels_float[i] = 1.0f;
        this->setCustomVar(levels_float[i],"LEVEL_"+ofToString(i+1));
    }

    static_cast<vector<float> *>(_inletParams[0])->clear();
    static_cast<vector<float> *>(_inletParams[0])->assign(signalInlets,0.0f);

}

//--------------------------------------------------------------
void Mixer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadAudioSettings();

}

//--------------------------------------------------------------
void Mixer::setupAudioOutObjectContent(pdsp::Engine &engine){

    for(int i=0;i<signalInlets;i++){
        levels_ctrl[i] >> levels[i].in_mod();
        levels_ctrl[i].set(levels_float[i]);
        levels_ctrl[i].enableSmoothing(50.0f);

        this->pdspIn[i+1] >> levels[i] >> mix;
    }

    mix >> this->pdspOut[0];
    mix >> scope >> engine.blackhole();
}

//--------------------------------------------------------------
void Mixer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[0]){
        for(size_t i=0;i<static_cast<vector<float> *>(_inletParams[0])->size();i++){
            if(i < signalInlets){
                levels_float[i] = static_cast<vector<float> *>(_inletParams[0])->at(i);
                levels_ctrl[i].set(levels_float[i]);
            }
        }
    }

    if(needReset){
        needReset = false;
        for(int i=0;i<signalInlets;i++){
            this->pdspIn[i+1].disconnectAll();
        }
        resetInletsSettings();
    }

    if(!loaded){
        loaded  = true;
        initInlets();
    }
}

//--------------------------------------------------------------
void Mixer::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void Mixer::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*scaleFactor));

        float sliderW = (this->width*_nodeCanvas.GetCanvasScale() - (30*scaleFactor) - (8*scaleFactor*(this->numInlets-1)))/this->numInlets;

        for(int i=0;i<this->numInlets-1;i++){
            if (i > 0) ImGui::SameLine();

            ImGui::PushID(i);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(255,255,120,30));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(255,255,120,60));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(255,255,120,60));
            ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(255,255,120,160));

            ImGui::VSliderFloat("##v", ImVec2(sliderW, this->height*_nodeCanvas.GetCanvasScale() - (26*scaleFactor + IMGUI_EX_NODE_CONTENT_PADDING*3*scaleFactor)), &levels_float[i], 0.0f, 1.0f, "");
            if (ImGui::IsItemActive() || ImGui::IsItemHovered()){
                ImGui::SetTooltip("s%i %.2f", i+1, levels_float[i]);
                levels_ctrl[i].set(levels_float[i]);
                this->setCustomVar(levels_float[i],"LEVEL_"+ofToString(i+1));
            }
            ImGui::PopStyleColor(4);
            ImGui::PopID();
        }

        ImGui::SameLine();ImGui::Dummy(ImVec2(sliderW/4.0f,1));ImGui::SameLine();
        ImGuiEx::VUMeter(_nodeCanvas.getNodeDrawList(), sliderW/2.0f, this->height*_nodeCanvas.GetCanvasScale() - (26*scaleFactor + IMGUI_EX_NODE_CONTENT_PADDING*3*scaleFactor), static_cast<ofSoundBuffer *>(_outletParams[0])->getRMSAmplitude(), false);

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void Mixer::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(ImGui::InputInt("Inlets",&signalInlets)){
        if(signalInlets > MAX_INLETS){
            signalInlets = MAX_INLETS;
        }
        if(signalInlets < 2){
            signalInlets = 2;
        }
    }
    ImGui::SameLine(); ImGuiEx::HelpMarker("You can set 32 inlets max.");
    ImGui::Spacing();
    if(ImGui::Button("APPLY",ImVec2(224*scaleFactor,26*scaleFactor))){
        this->setCustomVar(static_cast<float>(signalInlets),"NUM_INLETS");
        needReset = true;
    }

    ImGuiEx::ObjectInfo(
                "Line mixer, mix up to 32 audio signals",
                "https://mosaic.d3cod3.org/reference.php?r=mixer", scaleFactor);
}

//--------------------------------------------------------------
void Mixer::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void Mixer::loadAudioSettings(){
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
void Mixer::initInlets(){

    signalInlets = this->getCustomVar("NUM_INLETS");

    resetInletsSettings();
}

//--------------------------------------------------------------
void Mixer::resetInletsSettings(){

    mix.disconnectIn();

    vector<bool> tempInletsConn;
    for(int i=0;i<this->numInlets;i++){
        if(this->inletsConnected[i]){
            tempInletsConn.push_back(true);
        }else{
            tempInletsConn.push_back(false);
        }
    }

    this->numInlets = signalInlets+1;

    this->width = 20 + signalInlets*42 + 42 + 10; // inlets gap + sliders + vumeter + outlets gap

    if(signalInlets <= 8){
        this->height = OBJECT_HEIGHT;
    }else if(signalInlets > 8 && this->numInlets <= 32){
        this->height = OBJECT_HEIGHT*2;
    }

    _inletParams[0] = new vector<float>();

    for(int i=0;i<signalInlets;i++){
        _inletParams[i+1] = new ofSoundBuffer();
    }

    this->inletsType.clear();
    this->inletsNames.clear();

    this->addInlet(VP_LINK_ARRAY,"control");

    for(int i=0;i<signalInlets;i++){
        this->addInlet(VP_LINK_AUDIO,"s"+ofToString(i+1));
    }

    this->inletsConnected.clear();
    for(size_t i=0;i<tempInletsConn.size();i++){
        if(tempInletsConn.at(i)){
            this->inletsConnected.push_back(true);
        }else{
            this->inletsConnected.push_back(false);
        }
    }

    levels          = new pdsp::Amp[signalInlets];
    levels_ctrl     = new pdsp::ValueControl[signalInlets];
    levels_float    = new float[signalInlets];

    for(int i=0;i<signalInlets;i++){
        levels_float[i] = 1.0f;
        if(this->existsCustomVar("LEVEL_"+ofToString(i+1))){
            levels_float[i] = this->getCustomVar("LEVEL_"+ofToString(i+1));
        }else{
            this->setCustomVar(levels_float[i],"LEVEL_"+ofToString(i+1));
        }

        levels_ctrl[i] >> levels[i].in_mod();
        levels_ctrl[i].set(levels_float[i]);
        levels_ctrl[i].enableSmoothing(50.0f);

        this->pdspIn[i+1] >> levels[i] >> mix;
    }

    static_cast<vector<float> *>(_inletParams[0])->clear();
    static_cast<vector<float> *>(_inletParams[0])->assign(signalInlets,0.0f);

    ofNotifyEvent(this->resetEvent, this->nId);

    this->saveConfig(false);

}

//--------------------------------------------------------------
void Mixer::audioInObject(ofSoundBuffer &inputBuffer){

}

//--------------------------------------------------------------
void Mixer::audioOutObject(ofSoundBuffer &outputBuffer){
    // SIGNAL BUFFER
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}

OBJECT_REGISTER( Mixer, "mixer", OFXVP_OBJECT_CAT_SOUND)

#endif
