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

#include "VectorOperator.h"

//--------------------------------------------------------------
VectorOperator::VectorOperator() : PatchObject("vector operator"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new vector<float>();  // input data
    _inletParams[1] = new float();  // multiplier
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new vector<float>(); // output

    this->initInletsState();

    _operator           = Vec_Operator_ADD;
    number              = 0.0f;
    loaded              = false;
}

//--------------------------------------------------------------
void VectorOperator::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"data");
    this->addInlet(VP_LINK_NUMERIC,"value");

    this->addOutlet(VP_LINK_ARRAY,"result");

    this->setCustomVar(_operator,"OPERATOR");
    this->setCustomVar(number,"NUMBER");

}

//--------------------------------------------------------------
void VectorOperator::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    operators_string.push_back("+");
    operators_string.push_back("-");
    operators_string.push_back("*");
    operators_string.push_back("/");
}

//--------------------------------------------------------------
void VectorOperator::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(!loaded){
        loaded = true;
        _operator = this->getCustomVar("OPERATOR");
        number = this->getCustomVar("NUMBER");
    }

    if(this->inletsConnected[1]){
        number = *(float *)&_inletParams[1];
    }

    static_cast<vector<float> *>(_outletParams[0])->clear();
    if(this->inletsConnected[0]){
        for(size_t s=0;s<static_cast<size_t>(static_cast<vector<float> *>(_inletParams[0])->size());s++){
            if(_operator == Vec_Operator_ADD){
                static_cast<vector<float> *>(_outletParams[0])->push_back(static_cast<vector<float> *>(_inletParams[0])->at(s)+number);
            }else if(_operator == Vec_Operator_SUBTRACT){
                static_cast<vector<float> *>(_outletParams[0])->push_back(static_cast<vector<float> *>(_inletParams[0])->at(s)-number);
            }else if(_operator == Vec_Operator_MULTIPLY){
                static_cast<vector<float> *>(_outletParams[0])->push_back(static_cast<vector<float> *>(_inletParams[0])->at(s)*number);
            }else if(_operator == Vec_Operator_DIVIDE){
                static_cast<vector<float> *>(_outletParams[0])->push_back(static_cast<vector<float> *>(_inletParams[0])->at(s)/number);
            }
        }
    }
}

//--------------------------------------------------------------
void VectorOperator::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void VectorOperator::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGui::Dummy(ImVec2(-1,ImGui::GetWindowSize().y/2 - (46*scaleFactor))); // Padding top

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

        ImGui::Spacing();

        ImGui::PushItemWidth(-1);
        if(ImGui::DragFloat("", &number,0.1f)){
            this->setCustomVar(number,"NUMBER");
        }
        ImGui::PopItemWidth();

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void VectorOperator::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Operates on all the values of a float vector, with selected operator and value",
                "https://mosaic.d3cod3.org/reference.php?r=vector-multiply", scaleFactor);
}

//--------------------------------------------------------------
void VectorOperator::removeObjectContent(bool removeFileFromData){

}

OBJECT_REGISTER( VectorOperator, "vector operator", OFXVP_OBJECT_CAT_DATA)

#endif
