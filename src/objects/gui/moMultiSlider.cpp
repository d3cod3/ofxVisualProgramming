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

#include "moMultiSlider.h"

//--------------------------------------------------------------
moMultiSlider::moMultiSlider() : PatchObject("multislider"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new vector<float>(); // values

    _outletParams[0] = new vector<float>(); // output

    this->initInletsState();

    numSliders      = 6;
    newNumSliders   = numSliders;

    sliderW         = 30.0f;
    this->width     *= 1.7;

    needReset       = false;
    loaded          = false;

}

//--------------------------------------------------------------
void moMultiSlider::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"values");
    this->addOutlet(VP_LINK_ARRAY,"values");

    this->setCustomVar(static_cast<float>(numSliders),"NUM_SLIDERS");

    static_cast<vector<float> *>(_inletParams[0])->clear();
    static_cast<vector<float> *>(_inletParams[0])->assign(numSliders,0.0f);
    static_cast<vector<float> *>(_outletParams[0])->clear();
    static_cast<vector<float> *>(_outletParams[0])->assign(numSliders,0.0f);

    values.assign(numSliders,0.0f);

    for(int i=0;i<numSliders;i++){
        this->setCustomVar(values[i],"SLIDERVALUE_"+ofToString(i+1));
    }

}

//--------------------------------------------------------------
void moMultiSlider::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);
}

//--------------------------------------------------------------
void moMultiSlider::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(this->inletsConnected[0]){
        for(size_t i=0;i<numSliders;i++){
            values[i] = static_cast<vector<float> *>(_inletParams[0])->at(i);
        }
    }

    for(size_t i=0;i<numSliders;i++){
        static_cast<vector<float> *>(_outletParams[0])->at(i) = values[i];
    }

    if(needReset){
        needReset = false;

        numSliders = newNumSliders;

        static_cast<vector<float> *>(_inletParams[0])->clear();
        static_cast<vector<float> *>(_inletParams[0])->assign(numSliders,0.0f);
        static_cast<vector<float> *>(_outletParams[0])->clear();
        static_cast<vector<float> *>(_outletParams[0])->assign(numSliders,0.0f);

        values.assign(numSliders,0.0f);

        for(int i=0;i<numSliders;i++){
            if(this->existsCustomVar("SLIDERVALUE_"+ofToString(i+1))){
                values[i] = this->getCustomVar("SLIDERVALUE_"+ofToString(i+1));
            }else{
                this->setCustomVar(values[i],"SLIDERVALUE_"+ofToString(i+1));
            }
        }

        this->width = 20*scaleFactor + numSliders*(sliderW+9.0f)*scaleFactor + 10*scaleFactor;
        if(this->width < OBJECT_WIDTH*scaleFactor){
            this->width = OBJECT_WIDTH*scaleFactor;
        }

        this->saveConfig(false);
    }

    if(!loaded){
        loaded = true;

        numSliders = this->getCustomVar("NUM_SLIDERS");
        newNumSliders   = numSliders;

        static_cast<vector<float> *>(_inletParams[0])->clear();
        static_cast<vector<float> *>(_inletParams[0])->assign(numSliders,0.0f);
        static_cast<vector<float> *>(_outletParams[0])->clear();
        static_cast<vector<float> *>(_outletParams[0])->assign(numSliders,0.0f);

        values.assign(numSliders,0.0f);

        for(int i=0;i<numSliders;i++){
            if(this->existsCustomVar("SLIDERVALUE_"+ofToString(i+1))){
                values[i] = this->getCustomVar("SLIDERVALUE_"+ofToString(i+1));
            }else{
                this->setCustomVar(values[i],"SLIDERVALUE_"+ofToString(i+1));
            }
        }

        this->saveConfig(false);
    }


}

//--------------------------------------------------------------
void moMultiSlider::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);
}

//--------------------------------------------------------------
void moMultiSlider::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
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

        ImGui::Dummy(ImVec2(-1,2)); // Padding top
        for(int i=0;i<numSliders;i++){
            if (i > 0) ImGui::SameLine();

            ImGui::PushID(i);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(120,120,120,30));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(120,120,120,60));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(120,120,120,60));
            ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(120,120,120,160));
            ImGui::VSliderFloat("##v", ImVec2(sliderW*scaleFactor, this->height - 36.0f*scaleFactor), &values[i], 0.0f, 1.0f, "");
            if (ImGui::IsItemActive() || ImGui::IsItemHovered()){
                ImGui::SetTooltip("s%i %.2f", i+1, values[i]);
                this->setCustomVar(values[i],"SLIDERVALUE_"+ofToString(i+1));
            }
            ImGui::PopStyleColor(4);
            ImGui::PopID();
        }

        _nodeCanvas.EndNodeContent();
    }
}

//--------------------------------------------------------------
void moMultiSlider::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(ImGui::InputInt("Inlets",&newNumSliders)){
        if(newNumSliders > MAX_INLETS){
            newNumSliders = MAX_INLETS;
        }
        if(newNumSliders < 2){
            newNumSliders = 2;
        }
    }
    ImGui::SameLine(); ImGuiEx::HelpMarker("You can set 32 inlets max.");
    ImGui::Spacing();
    if(ImGui::Button("APPLY",ImVec2(224*scaleFactor,26*scaleFactor))){
        this->setCustomVar(static_cast<float>(numSliders),"NUM_SLIDERS");
        needReset = true;
    }

    ImGuiEx::ObjectInfo(
                "Multiple slider gui object.",
                "https://mosaic.d3cod3.org/reference.php?r=multislider", scaleFactor);
}

//--------------------------------------------------------------
void moMultiSlider::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
}

OBJECT_REGISTER( moMultiSlider, "multislider", OFXVP_OBJECT_CAT_GUI)

#endif
