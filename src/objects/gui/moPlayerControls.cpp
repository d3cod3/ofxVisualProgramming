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

#include "moPlayerControls.h"

//--------------------------------------------------------------
moPlayerControls::moPlayerControls() : PatchObject("player controls"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;

    _inletParams[1] = new float();  // select
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new string(); // output
    *static_cast<string *>(_outletParams[0]) = "";

    this->initInletsState();

    bang                = false;

    pause               = false;
    loop                = false;

    this->width         *= 1.7f;

}

//--------------------------------------------------------------
void moPlayerControls::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"select");
    this->addOutlet(VP_LINK_STRING,"command");

    this->setCustomVar(static_cast<float>(0),"LOOP");
}

//--------------------------------------------------------------
void moPlayerControls::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void moPlayerControls::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] < 1.0){
            bang = false;
        }else{
            bang = true;
        }
    }

    if(bang && this->inletsConnected[1]){
        int temp = static_cast<int>(floor(*(float *)&_inletParams[1]));
        switch (temp) {
            case 1:
                *static_cast<string *>(_outletParams[0]) = "play";
                break;
            case 2:
                *static_cast<string *>(_outletParams[0]) = "stop";
                break;
            case 3:
                *static_cast<string *>(_outletParams[0]) = "pause";
                break;
            case 4:
                *static_cast<string *>(_outletParams[0]) = "unpause";
                break;
            case 5:
                *static_cast<string *>(_outletParams[0]) = "loop_normal";
                break;
            case 6:
                *static_cast<string *>(_outletParams[0]) = "loop_none";
                break;
            default:
                break;
        }
    }
    
}

//--------------------------------------------------------------
void moPlayerControls::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void moPlayerControls::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Button, VHS_BLUE);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, VHS_BLUE_OVER);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, VHS_BLUE_OVER);
        if(ImGui::Button(ICON_FA_PLAY,ImVec2(69*scaleFactor,26*scaleFactor))){
            *static_cast<string *>(_outletParams[0]) = "play";
        }
        ImGui::SameLine();
        if(ImGui::Button(ICON_FA_STOP,ImVec2(69*scaleFactor,26*scaleFactor))){
            *static_cast<string *>(_outletParams[0]) = "stop";
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, VHS_YELLOW);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, VHS_YELLOW_OVER);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, VHS_YELLOW_OVER);
        if(ImGui::Button(ICON_FA_PAUSE,ImVec2(69*scaleFactor,26*scaleFactor))){
            pause = !pause;
            if(pause){
                *static_cast<string *>(_outletParams[0]) = "pause";
            }else{
                *static_cast<string *>(_outletParams[0]) = "unpause";
            }
        }
        ImGui::PopStyleColor(3);
        ImGui::Spacing();
        ImGui::Spacing();
        if(ImGui::Checkbox("LOOP " ICON_FA_REDO,&loop)){
            if(loop){
                *static_cast<string *>(_outletParams[0]) = "loop_normal";
            }else{
                *static_cast<string *>(_outletParams[0]) = "loop_none";
            }
            this->setCustomVar(static_cast<float>(loop),"LOOP");
            this->saveConfig(false);
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void moPlayerControls::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Control the status of other objects, such as video player, soundfile player, or timeline. Useful for syncronize play/stop.",
                "https://mosaic.d3cod3.org/reference.php?r=player-controls", scaleFactor);
}

//--------------------------------------------------------------
void moPlayerControls::removeObjectContent(bool removeFileFromData){
    
}


OBJECT_REGISTER( moPlayerControls, "player controls", OFXVP_OBJECT_CAT_GUI)

#endif
