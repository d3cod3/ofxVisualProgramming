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

#include "SummingMixer.h"

//--------------------------------------------------------------
SummingMixer::SummingMixer() : PatchObject("summing mixer"){

    this->numInlets  = 6;
    this->numOutlets = 1;

    for(size_t i=0;i<32;i++){
        _inletParams[i] = new ofSoundBuffer();
    }

    _outletParams[0] = new ofSoundBuffer(); // audio output

    this->initInletsState();

    signalInlets    = 6;

    levels          = new pdsp::Amp[signalInlets];
    gain_ctrl       = new pdsp::ValueControl[signalInlets];

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    needReset       = false;
    loaded          = false;

    this->setIsResizable(true);

    prevW                   = this->width;
    prevH                   = this->height;

}

//--------------------------------------------------------------
void SummingMixer::newObject(){
    PatchObject::setName( this->objectName );

    for(size_t i=0;i<32;i++){
        this->addInlet(VP_LINK_AUDIO,"s"+ofToString(i+1));
    }

    this->addOutlet(VP_LINK_AUDIO,"output");

    this->setCustomVar(static_cast<float>(signalInlets),"NUM_INLETS");

    this->setCustomVar(static_cast<float>(prevW),"WIDTH");
    this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
}

//--------------------------------------------------------------
void SummingMixer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadAudioSettings();

    initInlets();
}

//--------------------------------------------------------------
void SummingMixer::setupAudioOutObjectContent(pdsp::Engine &engine){
    for(int i=0;i<signalInlets;i++){
        gain_ctrl[i] >> levels[i].in_mod();
        gain_ctrl[i].set(1.0f/signalInlets);
        gain_ctrl[i].enableSmoothing(50.0f);

        this->pdspIn[i] >> levels[i] >> mix;
    }

    mix >> this->pdspOut[0];
    mix >> scope >> engine.blackhole();

}

//--------------------------------------------------------------
void SummingMixer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(needReset){
        needReset = false;
        resetInletsSettings();
    }

    if(!loaded){
        loaded  = true;

        prevW = this->getCustomVar("WIDTH");
        prevH = this->getCustomVar("HEIGHT");
        this->width             = prevW;
        this->height            = prevH;
    }
    
}

//--------------------------------------------------------------
void SummingMixer::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

}

//--------------------------------------------------------------
void SummingMixer::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 window_size = ImVec2(this->width*_nodeCanvas.GetCanvasScale(),this->height*_nodeCanvas.GetCanvasScale());
        float pinDistance = (window_size.y-((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor))/this->numInlets;

        for(int i=0;i<this->numInlets;i++){
            _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(window_pos.x,window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2) + pinDistance*i),ImVec2(window_pos.x+window_size.x,window_pos.y+(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor)+((window_size.y-((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor))/2)),IM_COL32(255,255,120,60),2.0f);
        }

        // save object dimensions (for resizable ones)
        if(this->width != prevW){
            prevW = this->width;
            this->setCustomVar(static_cast<float>(prevW),"WIDTH");
        }
        if(this->height != prevH){
            prevH = this->height;
            this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void SummingMixer::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(ImGui::InputInt("Signal Inlets",&signalInlets)){
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
                "Sum/mix up to 32 audio signals.",
                "https://mosaic.d3cod3.org/reference.php?r=summing-mixer", scaleFactor);
}

//--------------------------------------------------------------
void SummingMixer::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);

    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void SummingMixer::audioInObject(ofSoundBuffer &inputBuffer){
    unusedArgs(inputBuffer);


}

//--------------------------------------------------------------
void SummingMixer::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);

    // SIGNAL BUFFER
    ofxVP_CAST_PIN_PTR<ofSoundBuffer>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}

//--------------------------------------------------------------
void SummingMixer::loadAudioSettings(){
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
void SummingMixer::initInlets(){
    signalInlets = this->getCustomVar("NUM_INLETS");

    resetInletsSettings();
}

//--------------------------------------------------------------
void SummingMixer::resetInletsSettings(){

    mix.disconnectIn();

    for(int i=0;i<signalInlets;i++){
        this->pdspIn[i].disconnectAll();
    }

    vector<bool> tempInletsConn;
    for(int i=0;i<this->numInlets;i++){
        if(this->inletsConnected[i]){
            tempInletsConn.push_back(true);
        }else{
            tempInletsConn.push_back(false);
        }
    }

    this->numInlets = signalInlets;

    for(int i=0;i<this->numInlets;i++){
        _inletParams[i] = new ofSoundBuffer();
    }

    this->inletsType.clear();
    this->inletsNames.clear();
    this->inletsIDs.clear();
    this->inletsWirelessReceive.clear();

    for(int i=0;i<this->numInlets;i++){
        this->addInlet(VP_LINK_AUDIO,"s"+ofToString(i+1));
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

    levels          = new pdsp::Amp[signalInlets];
    gain_ctrl       = new pdsp::ValueControl[signalInlets];

    for(int i=0;i<signalInlets;i++){
        gain_ctrl[i] >> levels[i].in_mod();
        gain_ctrl[i].set(1.0f/signalInlets);
        gain_ctrl[i].enableSmoothing(50.0f);

        this->pdspIn[i] >> levels[i] >> mix;
    }

    ofNotifyEvent(this->resetEvent, this->nId);

    this->saveConfig(false);

}

OBJECT_REGISTER( SummingMixer, "summing mixer", OFXVP_OBJECT_CAT_SOUND)

#endif
