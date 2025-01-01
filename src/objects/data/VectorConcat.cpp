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

#include "VectorConcat.h"

//--------------------------------------------------------------
VectorConcat::VectorConcat() : PatchObject("vector concat"){

    this->numInlets  = 6;
    this->numOutlets = 1;

    for(size_t i=0;i<32;i++){
        _inletParams[i] = new vector<float>();
    }

    _outletParams[0] = new vector<float>();  // final vector

    this->initInletsState();

    dataInlets     = 6;

    needReset       = false;
    loaded          = false;

    this->setIsResizable(true);

    prevW                   = this->width;
    prevH                   = this->height;

}

//--------------------------------------------------------------
void VectorConcat::newObject(){
    PatchObject::setName( this->objectName );

    for(size_t i=0;i<32;i++){
        this->addInlet(VP_LINK_ARRAY,"v"+ofToString(i+1));
    }

    this->addOutlet(VP_LINK_ARRAY,"output");

    this->setCustomVar(static_cast<float>(dataInlets),"NUM_INLETS");

    this->setCustomVar(static_cast<float>(prevW),"WIDTH");
    this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
}

//--------------------------------------------------------------
void VectorConcat::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    initInlets();
}

//--------------------------------------------------------------
void VectorConcat::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    static_cast<vector<float> *>(_outletParams[0])->clear();
    for(int i=0;i<this->numInlets;i++){
        if(this->inletsConnected[i] && !static_cast<vector<float> *>(_inletParams[i])->empty()){
            for(size_t s=0;s<static_cast<size_t>(static_cast<vector<float> *>(_inletParams[i])->size());s++){
                static_cast<vector<float> *>(_outletParams[0])->push_back(static_cast<vector<float> *>(_inletParams[i])->at(s));
            }
        }
    }

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
void VectorConcat::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

}

//--------------------------------------------------------------
void VectorConcat::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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
            _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(window_pos.x + (14*this->scaleFactor),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2) + pinDistance*i),ImVec2(window_pos.x + (114*this->scaleFactor*_nodeCanvas.GetCanvasScale()),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2) + pinDistance*i),IM_COL32(120,255,120,60),2.0f);
            if(i==this->numInlets-1){
                _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(window_pos.x + (114*this->scaleFactor*_nodeCanvas.GetCanvasScale()),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2) + pinDistance*i),ImVec2(window_pos.x + (114*this->scaleFactor*_nodeCanvas.GetCanvasScale()),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2)),IM_COL32(120,255,120,60),2.0f);
                _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(window_pos.x + (114*this->scaleFactor*_nodeCanvas.GetCanvasScale()),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2) + pinDistance*i),ImVec2(window_pos.x+window_size.x,window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + ((window_size.y-((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor))/2)),IM_COL32(120,255,120,60),2.0f);
            }
        }

        ImGui::Dummy(ImVec2(-1,10*scaleFactor));
        if(static_cast<int>(static_cast<vector<float> *>(_outletParams[0])->size()) > 0){
            ImGui::Text("size\nrange");
            ImGui::SameLine();
            ImGui::Text("= %i\n= [0 - %i]",static_cast<int>(static_cast<vector<float> *>(_outletParams[0])->size()),static_cast<int>(static_cast<vector<float> *>(_outletParams[0])->size())-1);
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
void VectorConcat::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(ImGui::InputInt("Inlets",&dataInlets)){
        if(dataInlets > MAX_INLETS){
            dataInlets = MAX_INLETS;
        }
        if(dataInlets < 2){
            dataInlets = 2;
        }
    }
    ImGui::SameLine(); ImGuiEx::HelpMarker("You can set 32 inlets max.");
    ImGui::Spacing();
    if(ImGui::Button("APPLY",ImVec2(224*scaleFactor,26*scaleFactor))){
        this->setCustomVar(static_cast<float>(dataInlets),"NUM_INLETS");
        needReset = true;
    }

    ImGuiEx::ObjectInfo(
                "receive up to 32 data vectors, and concatenates them as a single vector",
                "https://mosaic.d3cod3.org/reference.php?r=vector-concat", scaleFactor);
}

//--------------------------------------------------------------
void VectorConcat::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
}

//--------------------------------------------------------------
void VectorConcat::initInlets(){
    dataInlets = this->getCustomVar("NUM_INLETS");

    //this->numInlets = dataInlets;

    resetInletsSettings();
}

//--------------------------------------------------------------
void VectorConcat::resetInletsSettings(){

    vector<bool> tempInletsConn;
    for(int i=0;i<this->numInlets;i++){
        if(this->inletsConnected[i]){
            tempInletsConn.push_back(true);
        }else{
            tempInletsConn.push_back(false);
        }
    }

    this->numInlets = dataInlets;

    for(int i=0;i<dataInlets;i++){
        _inletParams[i] = new vector<float>();
    }

    this->inletsType.clear();
    this->inletsNames.clear();
    this->inletsIDs.clear();
    this->inletsWirelessReceive.clear();

    for(int i=0;i<dataInlets;i++){
        this->addInlet(VP_LINK_ARRAY,"v"+ofToString(i+1));
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

OBJECT_REGISTER( VectorConcat, "vector concat", OFXVP_OBJECT_CAT_DATA)

#endif
