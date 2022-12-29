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

#include "BangMultiplexer.h"

//--------------------------------------------------------------
BangMultiplexer::BangMultiplexer() : PatchObject("bang multiplexer"){

    this->numInlets  = 6;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // float1
    _inletParams[1] = new float();  // float2
    _inletParams[2] = new float();  // float3
    _inletParams[3] = new float();  // float4
    _inletParams[4] = new float();  // float5
    _inletParams[5] = new float();  // float6
    *(float *)&_inletParams[0] = 0.0f;
    *(float *)&_inletParams[1] = 0.0f;
    *(float *)&_inletParams[2] = 0.0f;
    *(float *)&_inletParams[3] = 0.0f;
    *(float *)&_inletParams[4] = 0.0f;
    *(float *)&_inletParams[5] = 0.0f;

    _outletParams[0] = new float();  // output
    *(float *)&_outletParams[0] = 0.0f;

    floatInlets     = 6;

    needReset       = false;
    loaded          = false;

    this->initInletsState();

    this->setIsResizable(true);

    prevW                   = this->width;
    prevH                   = this->height;

}

//--------------------------------------------------------------
void BangMultiplexer::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"f1");
    this->addInlet(VP_LINK_NUMERIC,"f2");
    this->addInlet(VP_LINK_NUMERIC,"f3");
    this->addInlet(VP_LINK_NUMERIC,"f4");
    this->addInlet(VP_LINK_NUMERIC,"f5");
    this->addInlet(VP_LINK_NUMERIC,"f6");

    this->addOutlet(VP_LINK_NUMERIC,"bang");

    this->setCustomVar(static_cast<float>(floatInlets),"NUM_INLETS");

    this->setCustomVar(static_cast<float>(prevW),"WIDTH");
    this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
}

//--------------------------------------------------------------
void BangMultiplexer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void BangMultiplexer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    for(int i=0;i<this->numInlets;i++){
        if(this->inletsConnected[i]){
            if(*(float *)&_inletParams[i] > 0.0){
                *(float *)&_outletParams[0] = 1.0f;
                break;
            }else{
                *(float *)&_outletParams[0] = 0;
            }
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
    }
}

//--------------------------------------------------------------
void BangMultiplexer::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void BangMultiplexer::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        char temp[32];
        for(int i=0;i<this->numInlets;i++){
            _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(window_pos.x + (50*this->scaleFactor),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2) + pinDistance*i),ImVec2(window_pos.x + (90*this->scaleFactor),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2) + pinDistance*i),IM_COL32(60,60,60,255),2.0f);
            _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(window_pos.x + (90*this->scaleFactor),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2) + pinDistance*i),ImVec2(window_pos.x+window_size.x,window_pos.y+(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor)+((window_size.y-((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor))/2)),IM_COL32(60,60,60,255),2.0f);
            sprintf_s(temp,"%.2f",*(float *)&_inletParams[i]);
            _nodeCanvas.getNodeDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(window_pos.x+(20*this->scaleFactor),window_pos.y + ((IMGUI_EX_NODE_HEADER_HEIGHT-7)*this->scaleFactor) + (pinDistance/2) + pinDistance*i), IM_COL32_WHITE, temp, NULL, 0.0f);
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
void BangMultiplexer::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(ImGui::InputInt("Inlets",&floatInlets)){
        if(floatInlets > MAX_INLETS){
            floatInlets = MAX_INLETS;
        }
    }
    ImGui::SameLine(); ImGuiEx::HelpMarker("You can set 32 inlets max.");
    ImGui::Spacing();
    if(ImGui::Button("APPLY",ImVec2(224*scaleFactor,26*scaleFactor))){
        this->setCustomVar(static_cast<float>(floatInlets),"NUM_INLETS");
        needReset = true;
    }

    ImGuiEx::ObjectInfo(
                "receive up to 32 bangs, and process them as a shared transmission medium (multiple socket)",
                "https://mosaic.d3cod3.org/reference.php?r=floats-to-vector", scaleFactor);
}

//--------------------------------------------------------------
void BangMultiplexer::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void BangMultiplexer::initInlets(){
    floatInlets = this->getCustomVar("NUM_INLETS");

    this->numInlets = floatInlets;

    resetInletsSettings();
}

//--------------------------------------------------------------
void BangMultiplexer::resetInletsSettings(){

    vector<bool> tempInletsConn;
    for(int i=0;i<this->numInlets;i++){
        if(this->inletsConnected[i]){
            tempInletsConn.push_back(true);
        }else{
            tempInletsConn.push_back(false);
        }
    }

    this->numInlets = floatInlets;

    for(size_t i=0;i<floatInlets;i++){
        _inletParams[i] = new float();
        *(float *)&_inletParams[i] = 0.0f;
    }

    this->inletsType.clear();
    this->inletsNames.clear();

    for(size_t i=0;i<floatInlets;i++){
        this->addInlet(VP_LINK_NUMERIC,"f"+ofToString(i+1));
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

OBJECT_REGISTER( BangMultiplexer, "bang multiplexer", OFXVP_OBJECT_CAT_DATA)

#endif
