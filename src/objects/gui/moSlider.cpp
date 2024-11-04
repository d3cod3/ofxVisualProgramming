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

#include "moSlider.h"

//--------------------------------------------------------------
moSlider::moSlider() : PatchObject("slider"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // value
    *(float *)&_inletParams[0] = 0.0f;


    _outletParams[0] = new float(); // output
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    this->height        /= 2;

    loaded              = false;
    value               = 0.0f;

}

//--------------------------------------------------------------
void moSlider::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"value");
    this->addOutlet(VP_LINK_NUMERIC,"value");

    this->setCustomVar(value,"VALUE");
}

//--------------------------------------------------------------
void moSlider::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);
}

//--------------------------------------------------------------
void moSlider::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){


    if(this->inletsConnected[0]){
        value = *(float *)&_inletParams[0];
    }

    *(float *)&_outletParams[0] = value;

    if(!loaded){
        loaded = true;
        value = this->getCustomVar("VALUE");
    }

    this->setCustomVar(static_cast<float>(value),"VALUE");
}

//--------------------------------------------------------------
void moSlider::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void moSlider::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
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
        ImGui::PushItemWidth(-1);
        ImGui::SliderFloat("",&value,0.0f, 1.0f);
        ImGui::PopItemWidth();

        _nodeCanvas.EndNodeContent();
    }
}

//--------------------------------------------------------------
void moSlider::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Basic slider. Interactively adjust a numeric float value (with a range from 0.0 to 1.0) to send it to other objects.",
                "https://mosaic.d3cod3.org/reference.php?r=slider", scaleFactor);
}

//--------------------------------------------------------------
void moSlider::removeObjectContent(bool removeFileFromData){
    
}

OBJECT_REGISTER( moSlider, "slider", OFXVP_OBJECT_CAT_GUI)

#endif
