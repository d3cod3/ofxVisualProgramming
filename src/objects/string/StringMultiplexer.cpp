/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2020 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "StringMultiplexer.h"

//--------------------------------------------------------------
StringMultiplexer::StringMultiplexer() : PatchObject("string multiplexer"){

    this->numInlets  = 6;
    this->numOutlets = 1;

    _inletParams[0] = new string();  // string1
    _inletParams[1] = new string();  // string2
    _inletParams[2] = new string();  // string3
    _inletParams[3] = new string();  // string4
    _inletParams[4] = new string();  // string5
    _inletParams[5] = new string();  // string6

    _outletParams[0] = new string();  // output

    stringInlets     = 6;

    needReset       = false;
    loaded          = false;

    this->initInletsState();

    this->setIsResizable(true);

    prevW                   = this->width;
    prevH                   = this->height;

}

//--------------------------------------------------------------
void StringMultiplexer::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_STRING,"s1");
    this->addInlet(VP_LINK_STRING,"s2");
    this->addInlet(VP_LINK_STRING,"s3");
    this->addInlet(VP_LINK_STRING,"s4");
    this->addInlet(VP_LINK_STRING,"s5");
    this->addInlet(VP_LINK_STRING,"s6");

    this->addOutlet(VP_LINK_STRING,"string");

    this->setCustomVar(static_cast<float>(stringInlets),"NUM_INLETS");

    this->setCustomVar(static_cast<float>(prevW),"WIDTH");
    this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
}

//--------------------------------------------------------------
void StringMultiplexer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    inletsMemory.assign(this->numInlets,"");
}

//--------------------------------------------------------------
void StringMultiplexer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    for(int i=0;i<this->numInlets;i++){
        if(this->inletsConnected[i]){
            if(*static_cast<string *>(_inletParams[i]) != inletsMemory.at(i)){
                inletsMemory.at(i) = *static_cast<string *>(_inletParams[i]);
                *static_cast<string *>(_outletParams[0]) = *static_cast<string *>(_inletParams[i]);
                //break;
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
void StringMultiplexer::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void StringMultiplexer::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        for(int i=0;i<this->numInlets;i++){
            _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(window_pos.x + (50*this->scaleFactor),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2) + pinDistance*i),ImVec2(window_pos.x + (90*this->scaleFactor),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2) + pinDistance*i),IM_COL32(200,180,255,60),2.0f);
            _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(window_pos.x + (90*this->scaleFactor),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2) + pinDistance*i),ImVec2(window_pos.x+window_size.x,window_pos.y+(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor)+((window_size.y-((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor))/2)),IM_COL32(200,180,255,60),2.0f);
            //_nodeCanvas.getNodeDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(window_pos.x+(20*this->scaleFactor),window_pos.y + ((IMGUI_EX_NODE_HEADER_HEIGHT-7)*this->scaleFactor) + (pinDistance/2) + pinDistance*i), IM_COL32_WHITE, static_cast<string *>(_inletParams[i])->c_str(), NULL, 0.0f);
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
void StringMultiplexer::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(ImGui::InputInt("Inlets",&stringInlets)){
        if(stringInlets > MAX_INLETS){
            stringInlets = MAX_INLETS;
        }
    }
    ImGui::SameLine(); ImGuiEx::HelpMarker("You can set 32 inlets max.");
    ImGui::Spacing();
    if(ImGui::Button("APPLY",ImVec2(224*scaleFactor,26*scaleFactor))){
        this->setCustomVar(static_cast<float>(stringInlets),"NUM_INLETS");
        needReset = true;
    }

    ImGuiEx::ObjectInfo(
                "Simultaneously receive up to 32 strings and transmits the last one changed",
                "https://mosaic.d3cod3.org/reference.php?r=string-multiplexer", scaleFactor);
}

//--------------------------------------------------------------
void StringMultiplexer::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void StringMultiplexer::initInlets(){
    stringInlets = this->getCustomVar("NUM_INLETS");

    this->numInlets = stringInlets;

    resetInletsSettings();
}

//--------------------------------------------------------------
void StringMultiplexer::resetInletsSettings(){

    vector<bool> tempInletsConn;
    for(int i=0;i<this->numInlets;i++){
        if(this->inletsConnected[i]){
            tempInletsConn.push_back(true);
        }else{
            tempInletsConn.push_back(false);
        }
    }

    this->numInlets = stringInlets;

    inletsMemory.clear();
    inletsMemory.assign(this->numInlets,"");

    for(size_t i=0;i<stringInlets;i++){
        _inletParams[i] = new string();
    }

    this->inletsType.clear();
    this->inletsNames.clear();

    for(size_t i=0;i<stringInlets;i++){
        this->addInlet(VP_LINK_STRING,"f"+ofToString(i+1));
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

OBJECT_REGISTER( StringMultiplexer, "string multiplexer", OFXVP_OBJECT_CAT_STRING)

#endif
