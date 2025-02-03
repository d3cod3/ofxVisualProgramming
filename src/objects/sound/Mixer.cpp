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
    this->numOutlets = 2;

    _inletParams[0] = new vector<float>();

    for(size_t i=1;i<32;i++){
        _inletParams[i] = new ofSoundBuffer();
    }

    _outletParams[0] = new ofSoundBuffer(); // audio output L
    _outletParams[1] = new ofSoundBuffer(); // audio output R

    this->initInletsState();

    signalInlets    = this->numInlets-1;

    levelsL         = new pdsp::Amp[signalInlets];
    levelsR         = new pdsp::Amp[signalInlets];

    levels_float    = new float[signalInlets];

    gainL_ctrl       = new pdsp::ValueControl[signalInlets];
    gainR_ctrl       = new pdsp::ValueControl[signalInlets];
    pans_float      = new float[signalInlets];

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    needReset       = false;
    loaded          = false;

    sliderW         = 56.0f;

    //this->setIsResizable(true);

    this->height *= 2.36f;


}

//--------------------------------------------------------------
void Mixer::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"control");

    for(size_t i=1;i<32;i++){
        this->addInlet(VP_LINK_AUDIO,"s"+ofToString(i));
    }

    this->addOutlet(VP_LINK_AUDIO,"mainLeft");
    this->addOutlet(VP_LINK_AUDIO,"mainRight");

    this->setCustomVar(static_cast<float>(signalInlets),"NUM_INLETS");

    mainlevel_float = 1.0f;

    this->setCustomVar(mainlevel_float,"MAIN_LEVEL");

    for(int i=0;i<signalInlets;i++){
        levels_float[i] = 1.0f;
        pans_float[i] = 0.0f;
        this->setCustomVar(levels_float[i],"LEVEL_"+ofToString(i+1));
        this->setCustomVar(pans_float[i],"PAN_"+ofToString(i+1));
    }

    static_cast<vector<float> *>(_inletParams[0])->clear();
    static_cast<vector<float> *>(_inletParams[0])->assign(signalInlets*2,0.0f);

}

//--------------------------------------------------------------
void Mixer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadAudioSettings();

    initInlets();

}

//--------------------------------------------------------------
void Mixer::setupAudioOutObjectContent(pdsp::Engine &engine){

    for(int i=0;i<signalInlets;i++){
        gainL_ctrl[i] >> levelsL[i].in_mod();
        gainL_ctrl[i].set(ofMap(ofClamp(pans_float[i],-1.0f,1.0f),-1.0f,1.0f,1.0f,0.0f)*levels_float[i]*mainlevel_float);
        gainL_ctrl[i].enableSmoothing(50.0f);
        gainR_ctrl[i] >> levelsR[i].in_mod();
        gainR_ctrl[i].set(ofMap(ofClamp(pans_float[i],-1.0f,1.0f),-1.0f,1.0f,0.0f,1.0f)*levels_float[i]*mainlevel_float);
        gainR_ctrl[i].enableSmoothing(50.0f);

        this->pdspIn[i+1] >> levelsL[i] >> mixL;
        this->pdspIn[i+1] >> levelsR[i] >> mixR;

    }

    mixL >> this->pdspOut[0];
    mixR >> this->pdspOut[1];
    mixL >> scopeL >> engine.blackhole();
    mixR >> scopeR >> engine.blackhole();
}

//--------------------------------------------------------------
void Mixer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(this->inletsConnected[0] && !static_cast<vector<float> *>(_inletParams[0])->empty()){
        for(int i=0;i<static_cast<int>(static_cast<vector<float> *>(_inletParams[0])->size());i++){
            if(i < signalInlets){ // volumes
                levels_float[i] = static_cast<vector<float> *>(_inletParams[0])->at(i);
            }else if(i >= signalInlets && i < signalInlets*2){ // pans
                pans_float[i-signalInlets] = static_cast<vector<float> *>(_inletParams[0])->at(i);
            }
        }
    }

    for(int i=0;i<this->numInlets-1;i++){
        gainL_ctrl[i].set(ofMap(ofClamp(pans_float[i],-1.0f,1.0f),-1.0f,1.0f,1.0f,0.0f)*levels_float[i]*mainlevel_float);
        gainR_ctrl[i].set(ofMap(ofClamp(pans_float[i],-1.0f,1.0f),-1.0f,1.0f,0.0f,1.0f)*levels_float[i]*mainlevel_float);
    }

    if(needReset){
        needReset = false;
        resetInletsSettings();
    }

    if(!loaded){
        loaded  = true;
    }
}

//--------------------------------------------------------------
void Mixer::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

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

        char temp[32];

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*scaleFactor));

        for(int i=0;i<this->numInlets-1;i++){
            if (i > 0) ImGui::SameLine();

            ImGui::PushID(i);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(255,255,120,30));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(255,255,120,60));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(255,255,120,60));
            ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(255,255,120,160));

            ImGui::VSliderFloat("##v", ImVec2(sliderW*scaleFactor, (150.0f*scaleFactor - (26*scaleFactor + IMGUI_EX_NODE_CONTENT_PADDING*3*scaleFactor))*_nodeCanvas.GetCanvasScale()), &levels_float[i], 0.0f, 1.0f, "");
            if (ImGui::IsItemActive() || ImGui::IsItemHovered()){
                ImGui::SetTooltip("s%i %.2f", i+1, levels_float[i]);
                this->setCustomVar(levels_float[i],"LEVEL_"+ofToString(i+1));
            }
            ImGui::PopStyleColor(4);
            ImGui::PopID();
        }

        ImGui::SameLine();ImGui::Dummy(ImVec2(sliderW*scaleFactor/8.0f,1));ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(255,255,120,30));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(255,255,120,60));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(255,255,120,60));
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(255,255,120,160));
        ImGui::VSliderFloat("##main_volume", ImVec2(sliderW*scaleFactor, (150.0f*scaleFactor - (26*scaleFactor + IMGUI_EX_NODE_CONTENT_PADDING*3*scaleFactor))*_nodeCanvas.GetCanvasScale()), &mainlevel_float, 0.0f, 1.0f, "");
        if(ImGui::IsItemActive() || ImGui::IsItemHovered()){
            ImGui::SetTooltip("Main Volume %.2f", mainlevel_float);
            this->setCustomVar(mainlevel_float,"MAIN_LEVEL");
        }
        ImGui::PopStyleColor(4);

        ImGui::SameLine();

        ImGuiEx::VUMeter(_nodeCanvas.getNodeDrawList(), sliderW*scaleFactor/4.0f, (150.0f*scaleFactor - (26*scaleFactor + IMGUI_EX_NODE_CONTENT_PADDING*3*scaleFactor))*_nodeCanvas.GetCanvasScale(), static_cast<ofSoundBuffer *>(_outletParams[0])->getRMSAmplitude(), false);
        ImGui::SameLine();
        ImGuiEx::VUMeter(_nodeCanvas.getNodeDrawList(), sliderW*scaleFactor/4.0f, (150.0f*scaleFactor - (26*scaleFactor + IMGUI_EX_NODE_CONTENT_PADDING*3*scaleFactor))*_nodeCanvas.GetCanvasScale(), static_cast<ofSoundBuffer *>(_outletParams[1])->getRMSAmplitude(), false);

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*4*scaleFactor*_nodeCanvas.GetCanvasScale()));

        for(int i=0;i<this->numInlets-1;i++){
            sprintf_s(temp,"PAN s%i",i+1);
            if(ImGuiKnobs::Knob(temp, &pans_float[i], -1.0f, 1.0f, 0.01f, "%.2f", ImGuiKnobVariant_Stepped,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){

                this->setCustomVar(pans_float[i],"PAN_"+ofToString(i+1));
            }
            if (i < this->numInlets-1) ImGui::SameLine();

        }

        sliderW = ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,0,126);

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void Mixer::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(ImGui::InputInt("Channels",&signalInlets)){
        if(signalInlets > MAX_INLETS-1){
            signalInlets = MAX_INLETS-1;
        }
        if(signalInlets < 2){
            signalInlets = 2;
        }
    }
    ImGui::SameLine(); ImGuiEx::HelpMarker("You can set 31 channels max.");
    ImGui::Spacing();
    if(ImGui::Button("APPLY",ImVec2(224*scaleFactor,26*scaleFactor))){
        this->setCustomVar(static_cast<float>(signalInlets),"NUM_INLETS");
        needReset = true;
    }

    ImGuiEx::ObjectInfo(
                "Line mixer, mix up to 31 audio signals. Volumes and panning can be controlled from the first data inlet, for example with a 6 channels mixer, the first inlet can receive a data cable ( green ) with 12 numbers, the first 6 will control volumes, and the last 6 will control panning.)",
                "https://mosaic.d3cod3.org/reference.php?r=mixer", scaleFactor);
}

//--------------------------------------------------------------
void Mixer::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);

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
void Mixer::initInlets(){

    signalInlets = this->getCustomVar("NUM_INLETS");

    //this->numInlets = signalInlets+1;

    resetInletsSettings();
}

//--------------------------------------------------------------
void Mixer::resetInletsSettings(){

    mixL.disconnectIn();
    mixR.disconnectIn();

    for(int i=0;i<signalInlets;i++){
        this->pdspIn[i+1].disconnectAll();
    }

    vector<bool> tempInletsConn;
    for(int i=0;i<this->numInlets;i++){
        if(this->inletsConnected[i]){
            tempInletsConn.push_back(true);
        }else{
            tempInletsConn.push_back(false);
        }
    }

    this->numInlets = signalInlets+1;

    this->width = 20*scaleFactor + signalInlets*(56.0f+6.0f)*scaleFactor + 56.0f*scaleFactor/8.0f + 56.0f*scaleFactor + (56.0f*scaleFactor*2) + 10*scaleFactor; // inlets gap + sliders + gap + main + vumeters + outlets gap

    _inletParams[0] = new vector<float>();
    static_cast<vector<float> *>(_inletParams[0])->clear();
    static_cast<vector<float> *>(_inletParams[0])->assign(signalInlets*2,0.0f);

    for(int i=1;i<this->numInlets;i++){
        _inletParams[i] = new ofSoundBuffer();
    }

    this->inletsType.clear();
    this->inletsNames.clear();
    this->inletsIDs.clear();
    this->inletsWirelessReceive.clear();

    this->addInlet(VP_LINK_ARRAY,"control");

    for(int i=1;i<this->numInlets;i++){
        this->addInlet(VP_LINK_AUDIO,"s"+ofToString(i));
    }

    this->inletsConnected.clear();
    for(int i=0;i<this->numInlets;i++){
        if(i<static_cast<int>(tempInletsConn.size())){
            if(tempInletsConn.at(i)){
                this->inletsConnected.push_back(true);
            }else{
                this->inletsConnected.push_back(false);
            }
        }else{
            this->inletsConnected.push_back(false);
        }
    }

    mainlevel_float = this->getCustomVar("MAIN_LEVEL");

    levelsL         = new pdsp::Amp[signalInlets];
    levelsR         = new pdsp::Amp[signalInlets];
    levels_float    = new float[signalInlets];

    gainL_ctrl       = new pdsp::ValueControl[signalInlets];
    gainR_ctrl       = new pdsp::ValueControl[signalInlets];
    pans_float      = new float[signalInlets];

    for(int i=0;i<signalInlets;i++){
        levels_float[i] = 1.0f;
        if(this->existsCustomVar("LEVEL_"+ofToString(i+1))){
            levels_float[i] = this->getCustomVar("LEVEL_"+ofToString(i+1));
        }else{
            this->setCustomVar(levels_float[i],"LEVEL_"+ofToString(i+1));
        }
        pans_float[i] = 0.0f;
        if(this->existsCustomVar("PAN_"+ofToString(i+1))){
            pans_float[i] = this->getCustomVar("PAN_"+ofToString(i+1));
        }else{
            this->setCustomVar(pans_float[i],"PAN_"+ofToString(i+1));
        }


        gainL_ctrl[i] >> levelsL[i].in_mod();
        gainL_ctrl[i].set(ofMap(ofClamp(pans_float[i],-1.0f,1.0f),-1.0f,1.0f,1.0f,0.0f)*levels_float[i]*mainlevel_float);
        gainL_ctrl[i].enableSmoothing(50.0f);
        gainR_ctrl[i] >> levelsR[i].in_mod();
        gainR_ctrl[i].set(ofMap(ofClamp(pans_float[i],-1.0f,1.0f),-1.0f,1.0f,0.0f,1.0f)*levels_float[i]*mainlevel_float);
        gainR_ctrl[i].enableSmoothing(50.0f);

        this->pdspIn[i+1] >> levelsL[i] >> mixL;
        this->pdspIn[i+1] >> levelsR[i] >> mixR;


    }

    ofNotifyEvent(this->resetEvent, this->nId);

    this->saveConfig(false);

}

//--------------------------------------------------------------
void Mixer::audioInObject(ofSoundBuffer &inputBuffer){
    unusedArgs(inputBuffer);
}

//--------------------------------------------------------------
void Mixer::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);

    // SIGNAL BUFFER
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scopeL.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[1])->copyFrom(scopeR.getBuffer().data(), bufferSize, 1, sampleRate);
}

OBJECT_REGISTER( Mixer, "mixer", OFXVP_OBJECT_CAT_SOUND)

#endif
