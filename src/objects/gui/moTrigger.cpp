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

#include "moTrigger.h"

//--------------------------------------------------------------
moTrigger::moTrigger() : PatchObject("trigger"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // bang
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]) = 0.0f;

    _outletParams[0] = new float(); // output numeric
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0.0f;

    this->initInletsState();

    trigger = false;

    loaded              = false;

}

//--------------------------------------------------------------
void moTrigger::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"trigger");
    this->addOutlet(VP_LINK_NUMERIC,"trigger");

    this->setCustomVar(static_cast<float>(trigger),"VALUE");
}

//--------------------------------------------------------------
void moTrigger::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    pressColor = { 250/255.0f, 250/255.0f, 5/255.0f, 1.0f };
    releaseColor = { 0.f, 0.f, 0.f, 0.f };

    currentColor = releaseColor;
}

//--------------------------------------------------------------
void moTrigger::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);
    
    if(this->inletsConnected[0]){
        if(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]) < 1.0f){
            trigger = false;
            currentColor = releaseColor;
        }else{
            trigger = true;
            currentColor = pressColor;
        }
    }
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = static_cast<float>(trigger);

    if(!loaded){
        loaded = true;

        trigger = static_cast<bool>(this->getCustomVar("VALUE"));
        if(trigger){
            currentColor = pressColor;
        }else{
            currentColor = releaseColor;
        }
    }

    this->setCustomVar(static_cast<float>(trigger),"VALUE");
}

//--------------------------------------------------------------
void moTrigger::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

    ofSetColor(255);

}

//--------------------------------------------------------------
void moTrigger::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
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
        auto state = ImGuiEx::BangButton("", currentColor, ImVec2(ImGui::GetWindowSize().x,ImGui::GetWindowSize().y));

        if (state == SmartButtonState_Released){
            trigger = !trigger;

            if(trigger){
                currentColor = pressColor;
            }else{
                currentColor = releaseColor;
            }
        }

        _nodeCanvas.EndNodeContent();
    }
}

//--------------------------------------------------------------
void moTrigger::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Basic trigger. Useful to keep an action active, or inactive, continuously over time.",
                "https://mosaic.d3cod3.org/reference.php?r=trigger", scaleFactor);
}

//--------------------------------------------------------------
void moTrigger::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
}


OBJECT_REGISTER( moTrigger, "trigger", OFXVP_OBJECT_CAT_GUI)

#endif
