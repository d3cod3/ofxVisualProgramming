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

#include "Quantizer.h"

static bool has_black(int key) {
    return (key == 0 || key == 1 || key == 3 || key == 4 || key == 5);
}

//--------------------------------------------------------------
Quantizer::Quantizer() : PatchObject("quantizer"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // pitch
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]) = 0.0f;

    _inletParams[1] = new float();  // octave
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]) = 4;

    _outletParams[0] = new float();  // quantized pitch
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0.0f;

    this->initInletsState();

    selectedScale  = 0;
    octave      = 4;

    this->width *= 1.48f;

    loaded      = false;

}

//--------------------------------------------------------------
void Quantizer::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"pitch");
    this->addInlet(VP_LINK_NUMERIC,"octave");

    this->addOutlet(VP_LINK_NUMERIC,"quantized pitch");

    this->setCustomVar(static_cast<float>(selectedScale),"SCALE");
    this->setCustomVar(static_cast<float>(octave),"OCTAVE");

}

//--------------------------------------------------------------
void Quantizer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    quant.Init();
    quantScale = {1,0,0,0,0,0,0,0,0,0,0,0};
    quant.UpdateScale(quantScale);
    quant.SetBaseFrequency(440.0f);

    scalesOptions.push_back("None");
    scalesOptions.push_back("Major");
    scalesOptions.push_back("Dorian");
    scalesOptions.push_back("Phrygian");
    scalesOptions.push_back("Lydian");
    scalesOptions.push_back("Myxolodian");
    scalesOptions.push_back("Aeolian - natural Minor");
    scalesOptions.push_back("Locrian");
    scalesOptions.push_back("Acoustic");
    scalesOptions.push_back("Altered");
    scalesOptions.push_back("Augmented");
    scalesOptions.push_back("Bebop dom.");
    scalesOptions.push_back("Blues");
    scalesOptions.push_back("Chromatic");
    scalesOptions.push_back("Enigmatic");
    scalesOptions.push_back("Flamenco");
    scalesOptions.push_back("Gypsy");
    scalesOptions.push_back("Half diminished");
    scalesOptions.push_back("harmonic Major");
    scalesOptions.push_back("harmonic Minor");
    scalesOptions.push_back("Hirajoshi");
    scalesOptions.push_back("Hungarian");
    scalesOptions.push_back("Miyako-bushi");
    scalesOptions.push_back("Insen");
    scalesOptions.push_back("Iwato");
    scalesOptions.push_back("Lydian augmented");
    scalesOptions.push_back("Bebop Major");
    scalesOptions.push_back("Locrian Major");
    scalesOptions.push_back("Pentatonic Major");
    scalesOptions.push_back("melodic Minor");
    scalesOptions.push_back("Pentatonic Minor");
    scalesOptions.push_back("Neapolitan Major");
    scalesOptions.push_back("Neapolitan Minor");
    scalesOptions.push_back("Octatonic 1");
    scalesOptions.push_back("Octatonic 2");
    scalesOptions.push_back("Persian");
    scalesOptions.push_back("Phrygian dominant");
    scalesOptions.push_back("Prometheus");
    scalesOptions.push_back("Harmonics");
    scalesOptions.push_back("Tritone");
    scalesOptions.push_back("Tritone 2S");
    scalesOptions.push_back("Ukranian Dorian");
    scalesOptions.push_back("Wholetone");

}

//--------------------------------------------------------------
void Quantizer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(this->inletsConnected[0]){
        *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = pdsp::f2p(quant.Process(pdsp::p2f(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]))));
    }else{   
        *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0;
    }

    if(this->inletsConnected[1]){
        if(ofClamp(static_cast<int>(floor(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]))),0,8) != octave){
            quant.SetOctaveModifier(octave-4);
        }
        octave = ofClamp(static_cast<int>(floor(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]))),0,8);

    }

    if(!loaded){
        loaded = true;

        selectedScale = static_cast<int>(this->getCustomVar("SCALE"));
        octave = static_cast<int>(this->getCustomVar("OCTAVE"));

        quant.SetOctaveModifier(octave-4);
        updateScale(scalesOptions.at(selectedScale));
    }

}

//--------------------------------------------------------------
void Quantizer::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);
}

//--------------------------------------------------------------
void Quantizer::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();     

        int width = 28*_nodeCanvas.GetCanvasScale()*scaleFactor;
        int cur_key = 0;
        for (int key = 0; key < 7; key++) {
            ImU32 col;
            ImRect tecla = ImRect(ImVec2(p.x + key * width + (width/4), p.y),ImVec2(p.x + key * width + width - (width/4), p.y + ((this->height*_nodeCanvas.GetCanvasScale()*scaleFactor)-IMGUI_EX_NODE_HEADER_HEIGHT-IMGUI_EX_NODE_FOOTER_HEIGHT)));
            bool is_segment_hovered = tecla.Contains(ImGui::GetIO().MousePos) && !this->inletsConnected[0];

            if(quantScale[cur_key]) {
                col = Yellow;
            }else{
                col = Gray;
            }

            if(is_segment_hovered){
                if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
                    quantScale[cur_key] = !quantScale[cur_key];
                    quant.UpdateScale(quantScale);
                }
            }
            draw_list->AddRectFilled(
                        ImVec2(p.x + key * width, p.y),
                        ImVec2(p.x + key * width + width, p.y + ((this->height*_nodeCanvas.GetCanvasScale()*scaleFactor)-IMGUI_EX_NODE_HEADER_HEIGHT-IMGUI_EX_NODE_FOOTER_HEIGHT)),
                        col, 0, ImDrawFlags_RoundCornersAll);
            draw_list->AddRect(
                        ImVec2(p.x + key * width, p.y),
                        ImVec2(p.x + key * width + width, p.y + ((this->height*_nodeCanvas.GetCanvasScale()*scaleFactor)-IMGUI_EX_NODE_HEADER_HEIGHT-IMGUI_EX_NODE_FOOTER_HEIGHT)),
                        Black, 0, ImDrawFlags_RoundCornersAll);
            cur_key++;
            if (has_black(key)) {
                cur_key++;
            }

        }
        cur_key = 1;
        for (int key = 0; key < 7; key++) {
            if (has_black(key)) {
                ImU32 col;
                ImRect tecla = ImRect(ImVec2(p.x + key * width + width * 3 / 4, p.y),ImVec2(p.x + key * width + width * 5 / 4 + 1, p.y + (((this->height*_nodeCanvas.GetCanvasScale()*scaleFactor)-IMGUI_EX_NODE_HEADER_HEIGHT-IMGUI_EX_NODE_FOOTER_HEIGHT)/3*2)));
                bool is_segment_hovered = tecla.Contains(ImGui::GetIO().MousePos) && !this->inletsConnected[0];

                if(quantScale[cur_key]) {
                    col = Yellow;
                }else{
                    col = Gray;
                }

                if(is_segment_hovered){
                    if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
                        quantScale[cur_key] = !quantScale[cur_key];
                        quant.UpdateScale(quantScale);
                    }
                }
                draw_list->AddRectFilled(
                            ImVec2(p.x + key * width + width * 3 / 4, p.y),
                            ImVec2(p.x + key * width + width * 5 / 4 + 1, p.y + (((this->height*_nodeCanvas.GetCanvasScale()*scaleFactor)-IMGUI_EX_NODE_HEADER_HEIGHT-IMGUI_EX_NODE_FOOTER_HEIGHT)/3*2)),
                            col, 0, ImDrawFlags_RoundCornersAll);
                draw_list->AddRect(
                            ImVec2(p.x + key * width + width * 3 / 4, p.y),
                            ImVec2(p.x + key * width + width * 5 / 4 + 1, p.y + (((this->height*_nodeCanvas.GetCanvasScale()*scaleFactor)-IMGUI_EX_NODE_HEADER_HEIGHT-IMGUI_EX_NODE_FOOTER_HEIGHT)/3*2)),
                            Black, 0, ImDrawFlags_RoundCornersAll);

                cur_key += 2;
            } else {
                cur_key++;
            }
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void Quantizer::drawObjectNodeConfig(){
    ImGui::Spacing();
    ImGui::Text("Using scale: %s",scalesOptions.at(selectedScale).c_str());
    ImGui::Spacing();
    if(ImGui::BeginCombo("Scales", scalesOptions.at(selectedScale).c_str(),ImGuiComboFlags_HeightLargest )){
        for(int i=0; i < static_cast<int>(scalesOptions.size()); ++i){
            bool is_selected = (selectedScale == i );
            if (ImGui::Selectable(scalesOptions.at(i).c_str(), is_selected)){
                selectedScale = i;
                updateScale(scalesOptions.at(selectedScale));
                this->setCustomVar(static_cast<float>(selectedScale),"SCALE");
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ImGui::Spacing();
    if(ImGui::SliderInt("Octave",&octave,0,8)){
        quant.SetOctaveModifier(octave-4);
        this->setCustomVar(static_cast<float>(octave),"OCTAVE");
    }

    ImGuiEx::ObjectInfo(
                "Pitch quantizer, select your desired scale and octave to quantize notes",
                "https://mosaic.d3cod3.org/reference.php?r=quantizer", scaleFactor);
}

//--------------------------------------------------------------
void Quantizer::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
}

//--------------------------------------------------------------
void Quantizer::updateScale(string scalename){
    if(scalename == "None"){
        quantScale = {1,0,0,0,0,0,0,0,0,0,0,0};
    }else if(scalename == "Major"){
        quantScale = {1,0,1,0,1,1,0,1,0,1,0,1};
    }else if(scalename == "Dorian"){
        quantScale = {1,0,1,1,0,1,0,1,0,1,1,0};
    }else if(scalename == "Phrygian"){
        quantScale = {1,1,0,1,0,1,0,1,1,0,1,0};
    }else if(scalename == "Lydian"){
        quantScale = {1,0,1,0,1,0,1,1,0,1,0,1};
    }else if(scalename == "Myxolodian"){
        quantScale = {1,0,1,0,1,1,0,1,0,1,1,0};
    }else if(scalename == "Aeolian - natural Minor"){
        quantScale = {1,0,1,1,0,1,0,1,1,0,1,0};
    }else if(scalename == "Locrian"){
        quantScale = {1,1,0,1,0,1,1,0,1,0,1,0};
    }else if(scalename == "Acoustic"){
        quantScale = {1,0,1,0,1,0,1,1,0,1,1,0};
    }else if(scalename == "Altered"){
        quantScale = {1,1,0,1,1,0,1,0,1,0,1,0};
    }else if(scalename == "Augmented"){
        quantScale = {1,0,0,1,1,0,0,1,1,0,0,1};
    }else if(scalename == "Bebop dom."){
        quantScale = {1,0,1,0,1,1,0,1,0,1,1,1};
    }else if(scalename == "Blues"){
        quantScale = {1,0,0,1,0,1,1,1,0,0,1,0};
    }else if(scalename == "Chromatic"){
        quantScale = {1,1,1,1,1,1,1,1,1,1,1,1};
    }else if(scalename == "Enigmatic"){
        quantScale = {1,1,0,0,1,0,1,0,1,0,1,1};
    }else if(scalename == "Flamenco"){
        quantScale = {1,1,0,0,1,1,0,1,1,0,0,1};
    }else if(scalename == "Gypsy"){
        quantScale = {1,0,1,1,0,0,1,1,1,0,1,0};
    }else if(scalename == "Half diminished"){
        quantScale = {1,0,1,1,0,1,1,0,1,0,1,0};
    }else if(scalename == "harmonic Major"){
        quantScale = {1,0,1,0,1,1,0,1,1,0,0,1};
    }else if(scalename == "harmonic Minor"){
        quantScale = {1,0,1,1,0,1,0,1,1,0,0,1};
    }else if(scalename == "Hirajoshi"){
        quantScale = {1,0,0,0,1,0,1,1,0,0,0,1};
    }else if(scalename == "Hungarian"){
        quantScale = {1,0,1,1,0,0,1,1,1,0,0,1};
    }else if(scalename == "Miyako-bushi"){
        quantScale = {1,1,0,0,0,1,0,1,1,0,0,0};
    }else if(scalename == "Insen"){
        quantScale = {1,1,0,0,0,1,0,1,0,0,1,0};
    }else if(scalename == "Iwato"){
        quantScale = {1,1,0,0,0,1,1,0,0,0,1,0};
    }else if(scalename == "Lydian augmented"){
        quantScale = {1,0,1,0,1,0,1,0,1,1,0,1};
    }else if(scalename == "Bebop Major"){
        quantScale = {1,0,1,0,1,1,0,1,1,0,1,1};
    }else if(scalename == "Locrian Major"){
        quantScale = {1,0,1,0,1,1,1,0,1,0,1,0};
    }else if(scalename == "Pentatonic Major"){
        quantScale = {1,0,1,0,1,0,0,1,0,1,0,0};
    }else if(scalename == "melodic Minor"){
        quantScale = {1,0,1,1,0,1,0,1,0,1,0,1};
    }else if(scalename == "Pentatonic Minor"){
        quantScale = {1,0,0,1,0,1,0,1,0,0,1,0};
    }else if(scalename == "Neapolitan Major"){
        quantScale = {1,1,0,1,0,1,0,1,0,1,0,1};
    }else if(scalename == "Neapolitan Minor"){
        quantScale = {1,1,0,1,0,1,0,1,1,0,0,1};
    }else if(scalename == "Octatonic 1"){
        quantScale = {1,0,1,1,0,1,1,0,1,1,0,1};
    }else if(scalename == "Octatonic 2"){
        quantScale = {1,1,0,1,1,0,1,1,0,1,1,0};
    }else if(scalename == "Persian"){
        quantScale = {1,1,0,0,1,1,1,0,1,0,0,1};
    }else if(scalename == "Phrygian dominant"){
        quantScale = {1,1,0,0,1,1,0,1,1,0,1,0};
    }else if(scalename == "Prometheus"){
        quantScale = {1,0,1,0,1,0,1,0,0,1,1,0};
    }else if(scalename == "Harmonics"){
        quantScale = {1,0,0,1,1,1,0,1,0,1,0,0};
    }else if(scalename == "Tritone"){
        quantScale = {1,1,0,0,1,0,1,1,0,0,1,0};
    }else if(scalename == "Tritone 2S"){
        quantScale = {1,1,1,0,0,0,1,1,1,0,0,0};
    }else if(scalename == "Ukranian Dorian"){
        quantScale = {1,0,1,1,0,0,1,1,0,1,1,0};
    }else if(scalename == "Wholetone"){
        quantScale = {1,0,1,0,1,0,1,0,1,0,1,0};
    }

    quant.UpdateScale(quantScale);
}

OBJECT_REGISTER( Quantizer, "quantizer", OFXVP_OBJECT_CAT_SOUND)

#endif
