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

#include "Map.h"

//--------------------------------------------------------------
Map::Map() : PatchObject("map"){

    this->numInlets  = 5;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // value
    *(float *)&_inletParams[0] = 0.0f;
    _inletParams[1] = new float();  // in min
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();  // in max
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();  // out min
    *(float *)&_inletParams[3] = 0.0f;
    _inletParams[4] = new float();  // out max
    *(float *)&_inletParams[4] = 0.0f;

    _outletParams[0] = new float(); // mapped value
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    inMin = 0;
    inMax = 1;
    outMin = 0;
    outMax = 1;

    loaded              = false;

}

//--------------------------------------------------------------
void Map::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"value");
    this->addInlet(VP_LINK_NUMERIC,"in min");
    this->addInlet(VP_LINK_NUMERIC,"in max");
    this->addInlet(VP_LINK_NUMERIC,"out min");
    this->addInlet(VP_LINK_NUMERIC,"out max");

    this->addOutlet(VP_LINK_NUMERIC,"mapped value");

    this->setCustomVar(static_cast<float>(inMin),"IN_MIN");
    this->setCustomVar(static_cast<float>(inMax),"IN_MAX");
    this->setCustomVar(static_cast<float>(outMin),"OUT_MIN");
    this->setCustomVar(static_cast<float>(outMax),"OUT_MAX");

}

//--------------------------------------------------------------
void Map::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void Map::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[0]){
      if(this->inletsConnected[1]){
          inMin = *(float *)&_inletParams[1];
      }
      if(this->inletsConnected[2]){
          inMax = *(float *)&_inletParams[2];
      }
      if(this->inletsConnected[3]){
          outMin = *(float *)&_inletParams[3];
      }
      if(this->inletsConnected[4]){
          outMax = *(float *)&_inletParams[4];
      }
      *(float *)&_outletParams[0] = ofMap(*(float *)&_inletParams[0],inMin, inMax, outMin, outMax,true);
    }else{
      *(float *)&_outletParams[0] = 0.0f;
    }

    if(!loaded){
        loaded = true;
        inMin = this->getCustomVar("IN_MIN");
        inMax = this->getCustomVar("IN_MAX");
        outMin = this->getCustomVar("OUT_MIN");
        outMax = this->getCustomVar("OUT_MAX");
    }

}

//--------------------------------------------------------------
void Map::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void Map::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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
        float valuePercentage = ofMap(*(float *)&_outletParams[0],outMin,outMax,0.0f,1.0f,true);

        // vertical ranges
        _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(window_pos.x + (50*this->scaleFactor),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2) + (pinDistance*1)),ImVec2(window_pos.x + (50*this->scaleFactor),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2) + (pinDistance*3)),IM_COL32(60,60,60,255),2.0f);
        _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(window_pos.x + (130*this->scaleFactor),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2)),ImVec2(window_pos.x + (130*this->scaleFactor),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2) + (pinDistance*4)),IM_COL32(60,60,60,255),2.0f);

        // value
        _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(window_pos.x + (50*this->scaleFactor),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2) + ((pinDistance*2)*valuePercentage) + pinDistance),ImVec2(window_pos.x + (130*this->scaleFactor),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2) + ((pinDistance*4)*valuePercentage)),IM_COL32(90,90,90,255),2.0f);

        _nodeCanvas.getNodeDrawList()->AddCircleFilled(ImVec2(window_pos.x + (50*this->scaleFactor),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2) + ((pinDistance*2)*valuePercentage) + pinDistance),4*scaleFactor,IM_COL32(160,160,160,255),40);
        _nodeCanvas.getNodeDrawList()->AddCircleFilled(ImVec2(window_pos.x + (130*this->scaleFactor),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2) + ((pinDistance*4)*valuePercentage)),4*scaleFactor,IM_COL32(160,160,160,255),40);
    }

}

//--------------------------------------------------------------
void Map::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(ImGui::DragFloat("Input Min",&inMin,0.01f)){
        this->setCustomVar(static_cast<float>(inMin),"IN_MIN");
    }
    ImGui::Spacing();
    if(ImGui::DragFloat("Input Max",&inMax,0.01f)){
        this->setCustomVar(static_cast<float>(inMax),"IN_MAX");
    }
    ImGui::Spacing();
    if(ImGui::DragFloat("Output Min",&outMin,0.01f)){
        this->setCustomVar(static_cast<float>(outMin),"OUT_MIN");
    }
    ImGui::Spacing();
    if(ImGui::DragFloat("Output Max",&outMax,0.01f)){
        this->setCustomVar(static_cast<float>(outMax),"OUT_MAX");
    }

    ImGuiEx::ObjectInfo(
                "Map a number range to another one.",
                "https://mosaic.d3cod3.org/reference.php?r=map", scaleFactor);
}

//--------------------------------------------------------------
void Map::removeObjectContent(bool removeFileFromData){

}


OBJECT_REGISTER( Map, "map", OFXVP_OBJECT_CAT_MATH)

#endif
