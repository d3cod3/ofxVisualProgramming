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

#include "Metronome.h"

//--------------------------------------------------------------
Metronome::Metronome() :
        PatchObject("metronome"),

        // define default values
        timeSetting(1000,"time")
{

    this->numInlets  = 2;
    this->numOutlets = 2;

    *(float *)&_inletParams[0] = timeSetting.get();

    _inletParams[1] = new float(); // bang
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new float(); // bang
    *(float *)&_outletParams[0] = 0.0f;

    _outletParams[1] = new float(); // system bpm bang
    *(float *)&_outletParams[1] = 0.0f;

    this->initInletsState();

    resetTime = ofGetElapsedTimeMillis();
    metroTime = ofGetElapsedTimeMillis();

    sync                = false;

    bpmMetro            = false;

    loaded              = false;

}

//--------------------------------------------------------------
void Metronome::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"time");
    this->addInlet(VP_LINK_NUMERIC,"sync");

    this->addOutlet(VP_LINK_NUMERIC,"bang");
    this->addOutlet(VP_LINK_NUMERIC,"system bpm bang");

    this->setCustomVar(static_cast<float>(timeSetting.get()),"TIME");
}

//--------------------------------------------------------------
void Metronome::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    systemBPM.code = [&]() noexcept {
        // BPM metronome
        if(systemBPM.frame()%4==0) bpmMetro = true;
    };
}

//--------------------------------------------------------------
void Metronome::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    metroTime = ofGetElapsedTimeMillis();

    if(this->inletsConnected[0] && static_cast<int>(floor(*(float *)&_inletParams[0])) != timeSetting.get()){
        timeSetting.get() = static_cast<int>(floor(*(float *)&_inletParams[0]));
        this->setCustomVar(static_cast<float>(timeSetting.get()),"TIME");
    }

    if(this->inletsConnected[1]){
        sync = static_cast<bool>(floor(*(float *)&_inletParams[1]));
    }

    if(sync){
        resetTime = ofGetElapsedTimeMillis();
    }

    if(metroTime-resetTime > timeSetting.get()){
        resetTime = ofGetElapsedTimeMillis();
        *(float *)&_outletParams[0] = 1.0f;
    }else{
        *(float *)&_outletParams[0] = 0.0f;
    }

    if(bpmMetro){
        bpmMetro = false;
        *(float *)&_outletParams[1] = 1.0f;
    }else{
        *(float *)&_outletParams[1] = 0.0f;
    }

    if(!loaded){
        loaded = true;
        timeSetting.set(static_cast<int>(floor(this->getCustomVar("TIME"))));
    }

}

//--------------------------------------------------------------
void Metronome::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void Metronome::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGuiEx::plotValue(*(float *)&_outletParams[0], 0.f, 1.f, IM_COL32(255,255,255,255), this->scaleFactor);

        _nodeCanvas.EndNodeContent();
    }
}

//--------------------------------------------------------------
void Metronome::drawObjectNodeConfig(){
    ImGui::Spacing();
    ImGui::PushItemWidth(130);
    if(ImGui::DragInt("time (ms)", &timeSetting.get())){
        this->setCustomVar(static_cast<float>(timeSetting.get()),"TIME");
    }

    ImGuiEx::ObjectInfo(
                "Sends a bang with the time periodicity you specify in milliseconds.",
                "https://mosaic.d3cod3.org/reference.php?r=metronome", scaleFactor);
}

//--------------------------------------------------------------
void Metronome::removeObjectContent(bool removeFileFromData){
    
}

OBJECT_REGISTER( Metronome, "metronome", OFXVP_OBJECT_CAT_MATH)

#endif
