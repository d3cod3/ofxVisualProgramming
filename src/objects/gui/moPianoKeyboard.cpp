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

#include "moPianoKeyboard.h"

static bool has_black(int key) {
    return (!((key - 1) % 7 == 0 || (key - 1) % 7 == 3) && key != 51);
}

//--------------------------------------------------------------
moPianoKeyboard::moPianoKeyboard() : PatchObject("piano keyboard"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new vector<float>(); // midi notes array ( polyphony )

    _outletParams[0] = new vector<float>(); // notes

    this->initInletsState();

    pitch       = 0;
    loaded      = false;

    this->width *= 6.74f;

}

//--------------------------------------------------------------
void moPianoKeyboard::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"midi notes");

    this->addOutlet(VP_LINK_ARRAY,"notes");

}

//--------------------------------------------------------------
void moPianoKeyboard::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->assign(128,0.0f);
}

//--------------------------------------------------------------
void moPianoKeyboard::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(this->inletsConnected[0]){
        for(size_t i =0;i<256;i++){
            key_states[i] = 0;
        }
        for(size_t i=0;i<ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->size();i++){
            ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(i) = 0;
        }
        if(ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->size()>2){
            for(size_t i=0;i<ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->size()-2;i++){
                key_states[static_cast<int>(ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->at(i+1))] = static_cast<int>(ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->at(i+2));
                if(static_cast<int>(ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->at(i+1)) >= 21 && static_cast<int>(ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->at(i+1)) <= 108){
                    ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(static_cast<int>(ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->at(i+1))) = static_cast<int>(ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->at(i+2));
                }else{
                    ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(static_cast<int>(ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->at(i+1))) = 0;
                }
                i++;
            }
        }

    }else{
        for(size_t i=0;i<ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->size();i++){
            if(i == pitch && pitch >= 21 && pitch <= 108){
                ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(i) = 127;
            }else{
                ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0])->at(i) = 0;
            }
        }

    }

    if(!loaded){
        loaded = true;

    }

}

//--------------------------------------------------------------
void moPianoKeyboard::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);
}

//--------------------------------------------------------------
void moPianoKeyboard::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        int width = 20*_nodeCanvas.GetCanvasScale()*scaleFactor;
        int cur_key = 21;
        for (int key = 0; key < 52; key++) {
            ImU32 col = White;
            ImRect tecla = ImRect(ImVec2(p.x + key * width + (width/4), p.y),ImVec2(p.x + key * width + width - (width/4), p.y + ((this->height*_nodeCanvas.GetCanvasScale()*scaleFactor)-IMGUI_EX_NODE_HEADER_HEIGHT-IMGUI_EX_NODE_FOOTER_HEIGHT)));
            bool is_segment_hovered = tecla.Contains(ImGui::GetIO().MousePos) && !this->inletsConnected[0];

            if ((key_states[cur_key] || is_segment_hovered)) {
                if(ofGetMousePressed(OF_MOUSE_BUTTON_LEFT) && is_segment_hovered){
                    col = Yellow;
                    pitch = cur_key;

                }else{
                    col = Gray;
                    pitch = 0;
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
        cur_key = 22;
        for (int key = 0; key < 52; key++) {
            if (has_black(key)) {
                ImU32 col = Black;
                ImRect tecla = ImRect(ImVec2(p.x + key * width + width * 3 / 4, p.y),ImVec2(p.x + key * width + width * 5 / 4 + 1, p.y + (((this->height*_nodeCanvas.GetCanvasScale()*scaleFactor)-IMGUI_EX_NODE_HEADER_HEIGHT-IMGUI_EX_NODE_FOOTER_HEIGHT)/3*2)));
                bool is_segment_hovered = tecla.Contains(ImGui::GetIO().MousePos) && !this->inletsConnected[0];

                if ((key_states[cur_key] || is_segment_hovered)) {
                    if(ofGetMousePressed(OF_MOUSE_BUTTON_LEFT) && is_segment_hovered){
                        col = Yellow;
                        pitch = cur_key;

                    }else{
                        col = Gray;
                        pitch = 0;
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
void moPianoKeyboard::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "An interactive piano keyboard, it can be used to play notes or to visualize MIDI notes from an external MIDI device ( via midi receiver object )",
                "https://mosaic.d3cod3.org/reference.php?r=piano-keyboard", scaleFactor);
}

//--------------------------------------------------------------
void moPianoKeyboard::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
}

//--------------------------------------------------------------
void moPianoKeyboard::keyup(int key) {
    key_states[key] = 0;
}

//--------------------------------------------------------------
void moPianoKeyboard::keydown(int key, int velocity) {
    key_states[key] = velocity;
}
//--------------------------------------------------------------
std::vector<int> moPianoKeyboard::current_notes() {
    std::vector<int> result{};
    for (int i = 0; i < 256; i++) {
        if (key_states[i]) {
            result.push_back(i);
        }
    }
    return result;
}


OBJECT_REGISTER( moPianoKeyboard, "piano keyboard", OFXVP_OBJECT_CAT_GUI)

#endif
