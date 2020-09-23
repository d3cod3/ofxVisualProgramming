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

#include "Conditional.h"

//--------------------------------------------------------------
Conditional::Conditional() : PatchObject("conditional operator"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // input number
    *(float *)&_inletParams[0] = 0.0f;

    _inletParams[1] = new float();  // value
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new float(); // output
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    _operator           = Conditional_Operator_EQUAL;
    number              = 0.0f;
    loaded              = false;

}

//--------------------------------------------------------------
void Conditional::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"number");
    this->addInlet(VP_LINK_NUMERIC,"value");

    this->addOutlet(VP_LINK_NUMERIC,"trigger");

    this->setCustomVar(_operator,"OPERATOR");
    this->setCustomVar(number,"NUMBER");
}

//--------------------------------------------------------------
void Conditional::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    operators_string.push_back("==");
    operators_string.push_back("!=");
    operators_string.push_back("<");
    operators_string.push_back(">");
}

//--------------------------------------------------------------
void Conditional::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[1]){
        number = *(float *)&_inletParams[1];
    }

    if(this->inletsConnected[0]){
        if(_operator == Conditional_Operator_EQUAL){
            if(*(float *)&_inletParams[0] == number){
                *(float *)&_outletParams[0] = 1;
            }else{
                *(float *)&_outletParams[0] = 0;
            }
        }else if(_operator == Conditional_Operator_NOTEQUAL){
            if(*(float *)&_inletParams[0] != number){
                *(float *)&_outletParams[0] = 1;
            }else{
                *(float *)&_outletParams[0] = 0;
            }
        }else if(_operator == Conditional_Operator_LESSTHAN){
            if(*(float *)&_inletParams[0] < number){
                *(float *)&_outletParams[0] = 1;
            }else{
                *(float *)&_outletParams[0] = 0;
            }
        }else if(_operator == Conditional_Operator_BIGGERTHAN){
            if(*(float *)&_inletParams[0] > number){
                *(float *)&_outletParams[0] = 1;
            }else{
                *(float *)&_outletParams[0] = 0;
            }
        }
    }else{
        *(float *)&_outletParams[0] = 0;
    }

    if(!loaded){
        loaded = true;
        _operator = this->getCustomVar("OPERATOR");
        number = this->getCustomVar("NUMBER");
    }

}

//--------------------------------------------------------------
void Conditional::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void Conditional::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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
void Conditional::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Conditional operator",
                "https://mosaic.d3cod3.org/reference.php?r=conditional-operator", scaleFactor);
}

//--------------------------------------------------------------
void Conditional::removeObjectContent(bool removeFileFromData){

}

OBJECT_REGISTER( Conditional, "conditional operator", OFXVP_OBJECT_CAT_LOGIC)

#endif
