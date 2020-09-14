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

#include "TimedSemaphore.h"

//--------------------------------------------------------------
TimedSemaphore::TimedSemaphore() : PatchObject("timed semaphore"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;

    _inletParams[1] = new float();  // ms
    *(float *)&_inletParams[1] = 1000.0f;

    _outletParams[0] = new float(); // output numeric
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    bang                = false;

    loadStart           = true;

    wait                = 1000;
    startTime           = ofGetElapsedTimeMillis();

}

//--------------------------------------------------------------
void TimedSemaphore::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"ms");

    this->addOutlet(VP_LINK_NUMERIC,"bang");

    this->setCustomVar(static_cast<float>(wait),"MS");
}

//--------------------------------------------------------------
void TimedSemaphore::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    pressColor = { 250/255.0f, 250/255.0f, 5/255.0f, 1.0f };
    releaseColor = { 0.f, 0.f, 0.f, 0.f };

    currentColor = releaseColor;
}

//--------------------------------------------------------------
void TimedSemaphore::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[1]){
      wait = static_cast<int>(floor(*(float *)&_inletParams[1]));
    }

    if(this->inletsConnected[0] && loadStart){
        if(*(float *)&_inletParams[0] == 1.0 && !bang){
            bang        = true;
            loadStart   = false;
            startTime   = ofGetElapsedTimeMillis();
        }
    }else{
      bang        = false;
    }

    if(!loadStart && (ofGetElapsedTimeMillis()-startTime > wait)){
        loadStart   = true;
    }
    
    *(float *)&_outletParams[0] = static_cast<float>(bang);

    if(!loaded){
        loaded = true;
        wait   = static_cast<int>(floor(this->getCustomVar("MS")));
    }

}

//--------------------------------------------------------------
void TimedSemaphore::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void TimedSemaphore::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){

        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            drawObjectNodeConfig();

            ImGui::EndMenu();
        }

        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 window_size = ImGui::GetWindowSize();
        ImVec2 pos = ImVec2(window_pos.x + window_size.x - (30*this->scaleFactor), window_pos.y + (40*this->scaleFactor));

        // BANG (PD Style) button
        ImGuiEx::BangButton("", currentColor, ImVec2(ImGui::GetWindowSize().x,ImGui::GetWindowSize().y));

        if (bang){
            currentColor = pressColor;
        }else{
            currentColor = releaseColor;
        }

        if(loadStart){
            _nodeCanvas.getNodeDrawList()->AddCircleFilled(pos, 10*this->scaleFactor, IM_COL32(5, 250, 5, 255), 40);
        }else{
            _nodeCanvas.getNodeDrawList()->AddCircleFilled(pos, 10*this->scaleFactor, IM_COL32(250, 5, 5, 255), 40);
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void TimedSemaphore::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(ImGui::InputInt("Time",&wait)){
        if(wait < 0){
            wait = 0;
        }
        this->setCustomVar(static_cast<float>(wait),"MS");
    }
    ImGui::SameLine(); ImGuiEx::HelpMarker("Interrupt time in milliseconds.");

    ImGuiEx::ObjectInfo(
                "Interrupts, for a certain time, the continuous data flow to generate a discrete action.",
                "https://mosaic.d3cod3.org/reference.php?r=timed-semaphore", scaleFactor);
}

//--------------------------------------------------------------
void TimedSemaphore::removeObjectContent(bool removeFileFromData){

}


OBJECT_REGISTER( TimedSemaphore, "timed semaphore", OFXVP_OBJECT_CAT_LOGIC)

#endif
