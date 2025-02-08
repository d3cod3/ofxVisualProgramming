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

#include "BooleanOperator.h"

//--------------------------------------------------------------
BooleanOperator::BooleanOperator() : PatchObject("boolean operator"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // b1
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]) = 0.0f;
    _inletParams[1] = new float();  // b2
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]) = 0.0f;

    _outletParams[0] = new float(); // output
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0.0f;

    this->initInletsState();

    _operator           = Bool_Operator_AND;
    bang                = false;

    loaded              = false;

}

//--------------------------------------------------------------
void BooleanOperator::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"b1");
    this->addInlet(VP_LINK_NUMERIC,"b2");

    this->addOutlet(VP_LINK_NUMERIC,"result");

    this->setCustomVar(_operator,"OPERATOR");
}

//--------------------------------------------------------------
void BooleanOperator::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    operators_string.push_back("&&");
    operators_string.push_back("||");
}

//--------------------------------------------------------------
void BooleanOperator::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[0] && this->inletsConnected[1]){
        if(_operator == Bool_Operator_AND){
            if(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]) >= 1.0 && *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]) >= 1.0){
                *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 1;
                bang                = true;
            }else{
                *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0;
                bang                = false;
            }
        }else if(_operator == Bool_Operator_OR){
            if(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]) >= 1.0 || *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]) >= 1.0){
                *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 1;
                bang                = true;
            }else{
                *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0;
                bang                = false;
            }
        }
    }else{
        *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0;
        bang                = false;
    }

    if(!loaded){
        loaded = true;
        _operator = this->getCustomVar("OPERATOR");
    }
}

//--------------------------------------------------------------
void BooleanOperator::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void BooleanOperator::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGui::SetCursorPos(ImVec2(IMGUI_EX_NODE_PINS_WIDTH_NORMAL+(4*scaleFactor), this->height/2 *_nodeCanvas.GetCanvasScale()));

        ImGui::PushItemWidth(-50*scaleFactor);
        if(ImGui::BeginCombo("operator", operators_string.at(_operator).c_str() )){
            for(int i=0; i < operators_string.size(); ++i){
                bool is_selected = (_operator == i );
                if (ImGui::Selectable(operators_string.at(i).c_str(), is_selected)){
                    _operator = i;
                    this->setCustomVar(_operator,"OPERATOR");
                }
                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if(bang){
            ImVec2 window_pos = ImGui::GetWindowPos();
            ImVec2 window_size = ImVec2(this->width*_nodeCanvas.GetCanvasScale(),this->height*_nodeCanvas.GetCanvasScale());
            ImVec2 pos = ImVec2(window_pos.x + window_size.x - (ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,10,60)*scaleFactor), window_pos.y + IMGUI_EX_NODE_HEADER_HEIGHT + (ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,1,40)*scaleFactor));
            float radius = ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,1,20)*scaleFactor;
            _nodeCanvas.getNodeDrawList()->AddCircleFilled(pos, radius, IM_COL32(250, 250, 5, 255), 40);
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void BooleanOperator::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Basic boolean operator",
                "https://mosaic.d3cod3.org/reference.php?r=boolean-operator", scaleFactor);
}

//--------------------------------------------------------------
void BooleanOperator::removeObjectContent(bool removeFileFromData){

}

OBJECT_REGISTER( BooleanOperator, "boolean operator", OFXVP_OBJECT_CAT_LOGIC)

#endif
