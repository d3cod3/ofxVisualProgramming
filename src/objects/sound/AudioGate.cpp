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

#include "AudioGate.h"

//--------------------------------------------------------------
AudioGate::AudioGate() : PatchObject("signal gate"){

    this->numInlets  = 7;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // open
    *(float *)&_inletParams[0] = 0.0f;

    _inletParams[1] = new ofSoundBuffer();  // sig1
    _inletParams[2] = new ofSoundBuffer();  // sig2
    _inletParams[3] = new ofSoundBuffer();  // sig3
    _inletParams[4] = new ofSoundBuffer();  // sig4
    _inletParams[5] = new ofSoundBuffer();  // sig5
    _inletParams[6] = new ofSoundBuffer();  // sig6

    _outletParams[0] = new ofSoundBuffer(); // audio output

    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    this->initInletsState();

    openInlet           = 0;
    changedOpenInlet    = false;

    dataInlets      = 6;

    needReset       = false;
    loaded          = false;

    this->setIsResizable(true);

    prevW                   = this->width;
    prevH                   = this->height;

}

//--------------------------------------------------------------
void AudioGate::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"open");
    this->addInlet(VP_LINK_AUDIO,"s1");
    this->addInlet(VP_LINK_AUDIO,"s2");
    this->addInlet(VP_LINK_AUDIO,"s3");
    this->addInlet(VP_LINK_AUDIO,"s4");
    this->addInlet(VP_LINK_AUDIO,"s5");

    this->addOutlet(VP_LINK_AUDIO,"output");

    this->setCustomVar(static_cast<float>(openInlet),"OPEN");
    this->setCustomVar(static_cast<float>(dataInlets),"NUM_INLETS");

    this->setCustomVar(static_cast<float>(prevW),"WIDTH");
    this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
}

//--------------------------------------------------------------
void AudioGate::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    this->unusedArgs(mainWindow);

    for(int i=1;i<this->numInlets;i++){
        static_cast<ofSoundBuffer *>(_inletParams[i])->set(0.0f);
    }

    static_cast<ofSoundBuffer *>(_outletParams[0])->set(0.0f);
}

//--------------------------------------------------------------
void AudioGate::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    
    if(this->inletsConnected[0]){
        if(static_cast<int>(floor(*(float *)&_inletParams[0])) != openInlet){
            changedOpenInlet = true;
        }
        openInlet = static_cast<int>(floor(*(float *)&_inletParams[0]));
    }

    if(changedOpenInlet){
        changedOpenInlet = false;
        this->pdspOut[0].disconnectIn();
        if(openInlet >= 1 && openInlet < this->numInlets){
            this->pdspIn[openInlet] >> this->pdspOut[0];
        }
    }

    if(needReset){
        needReset = false;
        resetInletsSettings();
    }

    if(!loaded){
        loaded  = true;
        initInlets();
        prevW = this->getCustomVar("WIDTH");
        prevH = this->getCustomVar("HEIGHT");
        this->width             = prevW;
        this->height            = prevH;
        openInlet = this->getCustomVar("OPEN");
    }
    
}

//--------------------------------------------------------------
void AudioGate::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void AudioGate::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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
        ImVec2 window_size = ImGui::GetWindowSize();
        float pinDistance = (window_size.y-((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor))/this->numInlets;

        for(int i=1;i<this->numInlets;i++){
            if(i == openInlet){
                _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(window_pos.x,window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2) + pinDistance*i),ImVec2(window_pos.x + (90*scaleFactor),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2) + pinDistance*i),IM_COL32(255,255,120,60),2.0f);
            }
            _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(window_pos.x + (90*this->scaleFactor),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2) + pinDistance*i),ImVec2(window_pos.x+window_size.x,window_pos.y+(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor)+((window_size.y-((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor))/2)),IM_COL32(255,255,120,60),2.0f);
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
void AudioGate::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(ImGui::InputInt("Open",&openInlet)){
        if(openInlet < 0){
            openInlet = 0;
        }else if(openInlet > dataInlets){
            openInlet = dataInlets;
        }
        this->setCustomVar(static_cast<float>(openInlet),"OPEN");
    }
    if(ImGui::InputInt("Signal Inlets",&dataInlets)){
        if(dataInlets > MAX_INLETS-1){
            dataInlets = MAX_INLETS-1;
        }
    }
    ImGui::SameLine(); ImGuiEx::HelpMarker("You can set 31 inlets max.");
    ImGui::Spacing();
    if(ImGui::Button("APPLY",ImVec2(224*scaleFactor,26*scaleFactor))){
        this->setCustomVar(static_cast<float>(dataInlets),"NUM_INLETS");
        needReset = true;
    }

    ImGuiEx::ObjectInfo(
                "Receives up to 31 audio signals, and transmits only the one indicated in its first inlet: open.",
                "https://mosaic.d3cod3.org/reference.php?r=audio-gate", scaleFactor);
}

//--------------------------------------------------------------
void AudioGate::removeObjectContent(bool removeFileFromData){
    
}

//--------------------------------------------------------------
void AudioGate::audioOutObject(ofSoundBuffer &outputBuffer){
    if(openInlet >= 1 && openInlet < this->numInlets){
        *static_cast<ofSoundBuffer *>(_outletParams[0]) = *static_cast<ofSoundBuffer *>(_inletParams[openInlet]);
    }else if(openInlet == 0){
        *static_cast<ofSoundBuffer *>(_outletParams[0]) *= 0.0f;
    }
}

//--------------------------------------------------------------
void AudioGate::initInlets(){
    dataInlets = this->getCustomVar("NUM_INLETS");

    this->numInlets = dataInlets+1;

    resetInletsSettings();
}

//--------------------------------------------------------------
void AudioGate::resetInletsSettings(){

    vector<bool> tempInletsConn;
    for(int i=0;i<this->numInlets;i++){
        if(this->inletsConnected[i]){
            tempInletsConn.push_back(true);
        }else{
            tempInletsConn.push_back(false);
        }
    }

    this->numInlets = dataInlets+1;

    _inletParams[0] = new float();  // open
    *(float *)&_inletParams[0] = 0.0f;

    for(size_t i=1;i<this->numInlets;i++){
        _inletParams[i] = new ofSoundBuffer();
        static_cast<ofSoundBuffer *>(_inletParams[i])->set(0.0f);
    }

    this->inletsType.clear();
    this->inletsNames.clear();

    this->addInlet(VP_LINK_NUMERIC,"open");

    for(size_t i=1;i<this->numInlets;i++){
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

    ofNotifyEvent(this->resetEvent, this->nId);

    this->saveConfig(false);

}

OBJECT_REGISTER( AudioGate, "signal gate", OFXVP_OBJECT_CAT_SOUND)

#endif
