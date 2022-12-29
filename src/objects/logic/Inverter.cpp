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

#include "Inverter.h"

//--------------------------------------------------------------
Inverter::Inverter() : PatchObject("inverter"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;

    _outletParams[0] = new float(); // output numeric
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    trigger = true;

}

//--------------------------------------------------------------
void Inverter::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"trigger");
    this->addOutlet(VP_LINK_NUMERIC,"invertedTrigger");
}

//--------------------------------------------------------------
void Inverter::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    this->unusedArgs(mainWindow);

    pressColor = { 250/255.0f, 250/255.0f, 5/255.0f, 1.0f };
    releaseColor = { 0.f, 0.f, 0.f, 0.f };

    currentColor = releaseColor;
}

//--------------------------------------------------------------
void Inverter::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    
    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] < 1.0f){
            trigger = true;
        }else{
            trigger = false;
        }
    }
    *(float *)&_outletParams[0] = static_cast<float>(trigger);
}

//--------------------------------------------------------------
void Inverter::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void Inverter::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
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

        // TRIGGER button
        ImGuiEx::BangButton("", currentColor, ImVec2(ImGui::GetWindowSize().x,ImGui::GetWindowSize().y));

        if(trigger){
            currentColor = pressColor;
        }else{
            currentColor = releaseColor;
        }

        _nodeCanvas.EndNodeContent();
    }
}

//--------------------------------------------------------------
void Inverter::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Invert the value transmitted by a trigger or bang object.",
                "https://mosaic.d3cod3.org/reference.php?r=inverter", scaleFactor);
}

//--------------------------------------------------------------
void Inverter::removeObjectContent(bool removeFileFromData){
    
}


OBJECT_REGISTER( Inverter, "inverter", OFXVP_OBJECT_CAT_LOGIC)

#endif
