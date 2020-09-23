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

#include "DelayFloat.h"

//--------------------------------------------------------------
DelayFloat::DelayFloat() : PatchObject("delay float"){

    this->numInlets  = 3;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;

    _inletParams[1] = new float();  // number
    *(float *)&_inletParams[1] = 0.0f;

    _inletParams[2] = new float();  // ms
    *(float *)&_inletParams[2] = 1000.0f;

    _outletParams[0] = new float(); // output numeric
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    bang                = false;
    delayBang           = false;
    number              = 0.0f;

    loadStart           = false;
    loaded              = false;

    wait                = 1000;
    startTime           = ofGetElapsedTimeMillis();

}

//--------------------------------------------------------------
void DelayFloat::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"number");
    this->addInlet(VP_LINK_NUMERIC,"ms");

    this->addOutlet(VP_LINK_NUMERIC,"number");

    this->setCustomVar(number,"NUMBER");
    this->setCustomVar(static_cast<float>(wait),"MS");
}

//--------------------------------------------------------------
void DelayFloat::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void DelayFloat::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] == 1.0 && !bang){
            bang        = true;
            loadStart   = false;
            startTime   = ofGetElapsedTimeMillis();
        }
    }

    if(this->inletsConnected[1]){
        number = *(float *)&_inletParams[1];
    }

    if(this->inletsConnected[2]){
        wait                = static_cast<size_t>(floor(*(float *)&_inletParams[2]));
    }

    if(!loadStart && (ofGetElapsedTimeMillis()-startTime > wait)){
        bang        = false;
        loadStart   = true;
        delayBang   = true;
    }else{
        delayBang   = false;
    }

    if(delayBang){
        *(float *)&_outletParams[0] = number;
    }

    if(!loaded){
        loaded = true;
        number = this->getCustomVar("NUMBER");
        wait = static_cast<int>(floor(this->getCustomVar("MS")));
    }

}

//--------------------------------------------------------------
void DelayFloat::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void DelayFloat::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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
        ImVec2 window_size = ImGui::GetWindowSize();

        ImGui::Dummy(ImVec2(-1,ImGui::GetWindowSize().y/2 - (28*scaleFactor))); // Padding top
        ImGui::PushItemWidth(-1);
        if(ImGui::DragFloat("", &number,0.01f)){
            this->setCustomVar(number,"NUMBER");
        }
        ImGui::PopItemWidth();

        if(bang){
            float percentage = static_cast<float>(ofGetElapsedTimeMillis()-startTime) / static_cast<float>(wait);
            _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(window_pos.x, window_pos.y + window_size.y - (26*this->scaleFactor)),ImVec2(window_pos.x + (window_size.x*percentage),window_pos.y + window_size.y - (26*this->scaleFactor)),IM_COL32(60,60,60,255),8.0f);
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void DelayFloat::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(ImGui::InputInt("Delay",&wait)){
        if(wait < 0){
            wait = 0;
        }
        this->setCustomVar(static_cast<float>(wait),"MS");
    }
    ImGui::SameLine(); ImGuiEx::HelpMarker("Delay in milliseconds.");

    ImGuiEx::ObjectInfo(
                "Time delayed number transmission.",
                "https://mosaic.d3cod3.org/reference.php?r=delay-float", scaleFactor);
}

//--------------------------------------------------------------
void DelayFloat::removeObjectContent(bool removeFileFromData){

}


OBJECT_REGISTER( DelayFloat, "delay float", OFXVP_OBJECT_CAT_LOGIC)

#endif
