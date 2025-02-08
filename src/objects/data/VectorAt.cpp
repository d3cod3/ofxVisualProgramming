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

#include "VectorAt.h"

//--------------------------------------------------------------
VectorAt::VectorAt() : PatchObject("vector at"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new vector<float>();  // input vector
    _inletParams[1] = new float();          // at
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]) = 0.0f;

    _outletParams[0] = new float();         // output
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0.0f;

    this->initInletsState();

    vectorAt            = 0;
    loaded              = false;

}

//--------------------------------------------------------------
void VectorAt::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"vector");
    this->addInlet(VP_LINK_NUMERIC,"at");

    this->addOutlet(VP_LINK_NUMERIC,"value");

    this->setCustomVar(static_cast<float>(vectorAt),"AT");
}

//--------------------------------------------------------------
void VectorAt::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);
}

//--------------------------------------------------------------
void VectorAt::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(!loaded){
        loaded = true;
        vectorAt = static_cast<int>(floor(this->getCustomVar("AT")));
    }

    if(this->inletsConnected[1]){
        vectorAt = static_cast<int>(floor(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1])));
    }

    if(this->inletsConnected[0] && _inletParams[0]){
        if(!ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->empty()){
            if(vectorAt < static_cast<int>(ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->size()) && vectorAt >= 0){
                *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->at(vectorAt);
            }else{
                *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0.0f;
            }
        }else{
            *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0.0f;
        }
    }else{
        *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0.0f;
    }


}

//--------------------------------------------------------------
void VectorAt::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

    ofSetColor(255);

}

//--------------------------------------------------------------
void VectorAt::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGui::SetCursorPos(ImVec2(IMGUI_EX_NODE_PINS_WIDTH_NORMAL+(4*scaleFactor), (this->height/2 *_nodeCanvas.GetCanvasScale()) - (6*scaleFactor)));
        ImGui::PushItemWidth(-1);
        if(ImGui::DragInt("", &vectorAt)){
            if(vectorAt < 0){
                vectorAt = 0;
            }
            this->setCustomVar(static_cast<float>(vectorAt),"AT");
        }
        ImGui::Spacing();
        ImGui::Text("vector[%i] = %.2f", vectorAt,*ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]));
        ImGui::PopItemWidth();

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void VectorAt::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Extracts the value at a particular index from a vector",
                "https://mosaic.d3cod3.org/reference.php?r=vector-at", scaleFactor);
}

//--------------------------------------------------------------
void VectorAt::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
}


OBJECT_REGISTER( VectorAt, "vector at", OFXVP_OBJECT_CAT_DATA)

#endif
