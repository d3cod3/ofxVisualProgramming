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

#include "LoadBang.h"

//--------------------------------------------------------------
LoadBang::LoadBang() : PatchObject("loadbang"){

    this->numInlets  = 1;
    this->numOutlets = 2;

    _inletParams[0] = new float();  // time
    *(float *)&_inletParams[0] = 1000.0f;

    _outletParams[0] = new float(); // output numeric
    *(float *)&_outletParams[0] = 0.0f;

    _outletParams[1] = new string(); // output string
    *static_cast<string *>(_outletParams[1]) = "";

    this->initInletsState();

    bang                = false;
    loadStart           = false;

    wait                = 1000;
    startTime           = ofGetElapsedTimeMillis();

    loaded              = false;

}

//--------------------------------------------------------------
void LoadBang::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"delay");

    this->addOutlet(VP_LINK_NUMERIC,"bang");
    this->addOutlet(VP_LINK_STRING,"bang");

    this->setCustomVar(static_cast<float>(wait),"TIME");
}

//--------------------------------------------------------------
void LoadBang::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    pressColor = { 250/255.0f, 250/255.0f, 5/255.0f, 1.0f };
    releaseColor = { 0.f, 0.f, 0.f, 0.f };

    currentColor = releaseColor;
}

//--------------------------------------------------------------
void LoadBang::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[0] && static_cast<size_t>(floor(*(float *)&_inletParams[0])) != wait){
        wait = static_cast<int>(floor(*(float *)&_inletParams[0]));
        this->setCustomVar(static_cast<float>(wait),"TIME");
        loadStart = false;
        startTime = ofGetElapsedTimeMillis();
    }

    if(!loadStart && (ofGetElapsedTimeMillis()-startTime > wait)){
        bang = true;
        loadStart = true;
    }else{
        bang = false;
    }
    *(float *)&_outletParams[0] = static_cast<float>(bang);

    if(bang){
        *static_cast<string *>(_outletParams[1]) = "bang";
    }else{
        *static_cast<string *>(_outletParams[1]) = "";
    }

    if(!loaded){
        loaded = true;
        wait = static_cast<int>(floor(this->getCustomVar("TIME")));
    }

}

//--------------------------------------------------------------
void LoadBang::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void LoadBang::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 window_size = ImVec2(this->width*_nodeCanvas.GetCanvasScale(),this->height*_nodeCanvas.GetCanvasScale());

        // BANG (PD Style) button
        ImGuiEx::BangButton("", currentColor, ImVec2(ImGui::GetWindowSize().x,ImGui::GetWindowSize().y));

        if (bang){
            currentColor = pressColor;
        }else{
            currentColor = releaseColor;
        }

        if(!loadStart){
            float percentage = static_cast<float>(ofGetElapsedTimeMillis()-startTime) / static_cast<float>(wait);
            _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(window_pos.x, window_pos.y + window_size.y - (26*this->scaleFactor)),ImVec2(window_pos.x + (window_size.x*percentage),window_pos.y + window_size.y - (26*this->scaleFactor)),IM_COL32(60,60,60,255),8.0f);
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void LoadBang::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(ImGui::InputInt("Delay",&wait)){
        if(wait < 0){
            wait = 0;
        }

    }
    ImGui::SameLine(); ImGuiEx::HelpMarker("Delay in milliseconds.");

    ImGui::Spacing();
    if(ImGui::Button("APPLY",ImVec2(-1,26*this->scaleFactor))){
        this->setCustomVar(static_cast<float>(wait),"TIME");
        loadStart = false;
        startTime = ofGetElapsedTimeMillis();
    }

    ImGuiEx::ObjectInfo(
                "Automatically triggered bang on patch load, with configurable delay time.",
                "https://mosaic.d3cod3.org/reference.php?r=delay-bang", scaleFactor);
}

OBJECT_REGISTER( LoadBang, "loadbang", OFXVP_OBJECT_CAT_LOGIC)

#endif
