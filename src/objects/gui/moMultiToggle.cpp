/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2024 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "moMultiToggle.h"

//--------------------------------------------------------------
moMultiToggle::moMultiToggle() : PatchObject("multitoggle"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new vector<float>(); // values

    _outletParams[0] = new vector<float>(); // output

    this->initInletsState();

    numToggles      = 6;
    newNumToggles   = numToggles;

    values = new bool[numToggles];

    toggleW         = 26.0f;
    this->width     *= 1.54;
    this->height    /= 2.0f;

    needReset       = false;
    loaded          = false;

}

//--------------------------------------------------------------
void moMultiToggle::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"values");
    this->addOutlet(VP_LINK_ARRAY,"values");

    this->setCustomVar(static_cast<float>(numToggles),"NUM_TOGGLES");

    ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->clear();
    ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->assign(numToggles,0.0f);
    ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->clear();
    ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->assign(numToggles,0.0f);

    for(size_t i=0;i<numToggles;i++){
        values[i] = false;
    }

    for(int i=0;i<numToggles;i++){
        this->setCustomVar(static_cast<float>(values[i]),"TOGGLEVALUE_"+ofToString(i+1));
    }

}

//--------------------------------------------------------------
void moMultiToggle::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);
}

//--------------------------------------------------------------
void moMultiToggle::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(this->inletsConnected[0]){
        for(size_t i=0;i<numToggles;i++){
            if(ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->at(i) >= 1.0f){
                values[i] = true;
            }else{
                values[i] = false;
            }
        }
    }

    for(size_t i=0;i<numToggles;i++){
        if(values[i]){
            ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(i) = 1.0f;
        }else{
            ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(i) = 0.0f;
        }
    }

    if(needReset){
        needReset = false;

        numToggles = newNumToggles;

        ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->clear();
        ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->assign(numToggles,0.0f);
        ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->clear();
        ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->assign(numToggles,0.0f);

        values = new bool[numToggles];
        for(size_t i=0;i<numToggles;i++){
            values[i] = false;
        }

        for(int i=0;i<numToggles;i++){
            if(this->existsCustomVar("TOGGLEVALUE_"+ofToString(i+1))){
                values[i] = static_cast<bool>(this->getCustomVar("TOGGLEVALUE_"+ofToString(i+1)));
            }else{
                this->setCustomVar(static_cast<float>(values[i]),"TOGGLEVALUE_"+ofToString(i+1));
            }
        }

        this->width = 20*scaleFactor + numToggles*(toggleW+8.0f)*scaleFactor + 10*scaleFactor;
        if(this->width < OBJECT_WIDTH*scaleFactor){
            this->width = OBJECT_WIDTH*scaleFactor;
        }

        this->saveConfig(false);
    }

    if(!loaded){
        loaded = true;

        numToggles = this->getCustomVar("NUM_TOGGLES");
        newNumToggles   = numToggles;

        ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->clear();
        ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->assign(numToggles,0.0f);
        ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->clear();
        ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->assign(numToggles,0.0f);

        values = new bool[numToggles];
        for(size_t i=0;i<numToggles;i++){
            values[i] = false;
        }

        for(int i=0;i<numToggles;i++){
            if(this->existsCustomVar("TOGGLEVALUE_"+ofToString(i+1))){
                values[i] = static_cast<bool>(this->getCustomVar("TOGGLEVALUE_"+ofToString(i+1)));
            }else{
                this->setCustomVar(static_cast<float>(values[i]),"TOGGLEVALUE_"+ofToString(i+1));
            }
        }

        this->saveConfig(false);
    }


}

//--------------------------------------------------------------
void moMultiToggle::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);
}

//--------------------------------------------------------------
void moMultiToggle::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
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

        ImGui::SetCursorPos(ImVec2(((this->width*_nodeCanvas.GetCanvasScale())/2.0f)-(((toggleW+8.0)*scaleFactor)*numToggles/2.0f)+(8.0f*scaleFactor),(IMGUI_EX_NODE_HEADER_HEIGHT*scaleFactor)+(this->height*_nodeCanvas.GetCanvasScale()/2.0f)-(toggleW*scaleFactor)));

        for(int i=0;i<numToggles;i++){
            if (i > 0) ImGui::SameLine();

            ImGui::PushID(i);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(120,120,120,30));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(120,120,120,60));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(120,120,120,60));
            ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(120,120,120,160));
            if(ImGui::Checkbox("##t",&values[i])){
                this->setCustomVar(static_cast<float>(values[i]),"TOGGLEVALUE_"+ofToString(i+1));
            }

            ImGui::PopStyleColor(4);
            ImGui::PopID();
        }

        _nodeCanvas.EndNodeContent();
    }
}

//--------------------------------------------------------------
void moMultiToggle::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(ImGui::InputInt("Inlets",&newNumToggles)){
        if(newNumToggles > MAX_INLETS){
            newNumToggles = MAX_INLETS;
        }
        if(newNumToggles < 2){
            newNumToggles = 2;
        }
    }
    ImGui::SameLine(); ImGuiEx::HelpMarker("You can set 32 inlets max.");
    ImGui::Spacing();
    if(ImGui::Button("APPLY",ImVec2(224*scaleFactor,26*scaleFactor))){
        this->setCustomVar(static_cast<float>(numToggles),"NUM_TOGGLES");
        needReset = true;
    }

    ImGuiEx::ObjectInfo(
                "Multiple toggles gui object.",
                "https://mosaic.d3cod3.org/reference.php?r=multitoggle", scaleFactor);
}

//--------------------------------------------------------------
void moMultiToggle::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
}

OBJECT_REGISTER( moMultiToggle, "multitoggle", OFXVP_OBJECT_CAT_GUI)

#endif
