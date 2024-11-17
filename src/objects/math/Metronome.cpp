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

    isAudioOUTObject        = true;

    this->initInletsState();

    resetTime = ofGetElapsedTimeMicros();
    metroTime = ofGetElapsedTimeMicros();

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
    unusedArgs(mainWindow);

}

//--------------------------------------------------------------
void Metronome::setupAudioOutObjectContent(pdsp::Engine &engine){

    // ---- this code runs in the audio thread ----
    systemBPM.code = [&]() noexcept {
        // metronome
        metroTime = ofGetElapsedTimeMicros();

        if(this->inletsConnected[1]){
            sync = static_cast<bool>(floor(*(float *)&_inletParams[1]));
        }

        if(sync){
            resetTime = ofGetElapsedTimeMicros();
        }

        if(metroTime-resetTime > static_cast<size_t>(timeSetting.get()*1000.0f)){
            resetTime = ofGetElapsedTimeMicros();
            *(float *)&_outletParams[0] = 1.0f;
        }else{
            *(float *)&_outletParams[0] = 0.0f;
        }

        // Mosaic main BPM
        mbpm = engine.sequencer.getTempo();

        // BPM metronome
        if(systemBPM.frame()%4==0){
            *(float *)&_outletParams[1] = 1.0f;
        }else{
            *(float *)&_outletParams[1] = 0.0f;
        }
    };

}

//--------------------------------------------------------------
void Metronome::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(this->inletsConnected[0] && static_cast<int>(floor(*(float *)&_inletParams[0])) != timeSetting.get()){
        timeSetting.get() = static_cast<int>(floor(*(float *)&_inletParams[0]));
        this->setCustomVar(static_cast<float>(timeSetting.get()),"TIME");
    }

    if(!loaded){
        loaded = true;
        timeSetting.set(static_cast<int>(floor(this->getCustomVar("TIME"))));
    }

}

//--------------------------------------------------------------
void Metronome::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);
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

            drawObjectNodeConfig(); this->configMenuWidth = ImGui::GetWindowWidth();

            ImGui::EndMenu();
        }

        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        ImGuiEx::plotValue(*(float *)&_outletParams[0], 0.f, 1.f, IM_COL32(170,170,170,255), this->scaleFactor);

        // draw System BPM
        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 window_size = ImGui::GetWindowSize();
        ImVec2 pos = ImVec2(window_pos.x + window_size.x - 50*scaleFactor, window_pos.y + window_size.y - 38*scaleFactor);
        ImVec2 posFirst = ImVec2(window_pos.x + window_size.x - 50*scaleFactor, window_pos.y + 38*scaleFactor);

        char temp[32];
        sprintf_s(temp,"%.1f",static_cast<float>(60.0f/(timeSetting.get()/1000.0f)));
        _nodeCanvas.getNodeDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), posFirst, IM_COL32_WHITE,temp, NULL, 0.0f);
        sprintf_s(temp,"%i",static_cast<int>(mbpm));
        _nodeCanvas.getNodeDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), pos, IM_COL32_WHITE,temp, NULL, 0.0f);

        if(*(float *)&_outletParams[1] > 0){
            // draw system BPM beat
            _nodeCanvas.getNodeDrawList()->AddCircleFilled(ImVec2(pos.x + (32*scaleFactor),pos.y + (8*scaleFactor)), 6*scaleFactor, IM_COL32(255, 255, 120, 255), 40);
        }

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
                "Sends a bang with the time periodicity you specify in milliseconds. The second outlet return the bang from Mosaic main BPM",
                "https://mosaic.d3cod3.org/reference.php?r=metronome", scaleFactor);
}

//--------------------------------------------------------------
void Metronome::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
}

//--------------------------------------------------------------
void Metronome::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);

}

OBJECT_REGISTER( Metronome, "metronome", OFXVP_OBJECT_CAT_MATH)

#endif
