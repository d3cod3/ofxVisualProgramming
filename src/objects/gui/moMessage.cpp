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

#include "moMessage.h"

//--------------------------------------------------------------
moMessage::moMessage() : PatchObject("message"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;

    _inletParams[1] = new string();  // message
    *static_cast<string *>(_inletParams[1]) = "";

    _outletParams[0] = new string(); // output
    *static_cast<string *>(_outletParams[0]) = "";

    this->initInletsState();

    actualMessage   = "";

    this->setIsResizable(true);

}

//--------------------------------------------------------------
void moMessage::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_STRING,"message");
    this->addOutlet(VP_LINK_STRING,"message");
}

//--------------------------------------------------------------
void moMessage::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void moMessage::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[0] && *(float *)&_inletParams[0] >= 1.0){
        if(this->inletsConnected[1]){
            *static_cast<string *>(_outletParams[0]) = "";
            *static_cast<string *>(_outletParams[0]) = *static_cast<string *>(_inletParams[1]);
            actualMessage = *static_cast<string *>(_inletParams[1]);
        }else{
            *static_cast<string *>(_outletParams[0]) = "";
            *static_cast<string *>(_outletParams[0]) = actualMessage;
        }

    }
}

//--------------------------------------------------------------
void moMessage::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void moMessage::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){

        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            ImGuiEx::ObjectInfo(
                        "A basic text string sending object. Useful to use with string command controlled objects.",
                        "https://mosaic.d3cod3.org/reference.php?r=message");

            ImGui::EndMenu();
        }

        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        ImGui::Dummy(ImVec2(-1,ImGui::GetWindowSize().y/2 - 40)); // Padding top
        ImGui::PushItemWidth(-24);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, VHS_DGRAY);
        ImGui::InputText("##source", &actualMessage);
        ImGui::PopStyleColor(1);
        ImGui::PopItemWidth();
        ImGui::SameLine(); ImGuiEx::HelpMarker("Always check objects reference for UPPERCASE/LOWERCASE messages.");

        ImGui::Spacing();
        if(ImGui::Button("SEND",ImVec2(-1,20))){
            *static_cast<string *>(_outletParams[0]) = "";
            *static_cast<string *>(_outletParams[0]) = actualMessage;
        }


        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void moMessage::removeObjectContent(bool removeFileFromData){
    
}


OBJECT_REGISTER( moMessage, "message", OFXVP_OBJECT_CAT_GUI)

#endif
