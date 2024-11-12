/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2024 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "SignalMetronome.h"

//--------------------------------------------------------------
SignalMetronome::SignalMetronome() : PatchObject("signal metronome")
{

    this->numInlets  = 0;
    this->numOutlets = 1;

    _outletParams[0] = new ofSoundBuffer(); // system bpm bang as signal trigger

    isAudioOUTObject        = true;

    this->initInletsState();

    mainBPM = 0;

    this->height /= 2.0f;

    loaded              = false;

}

//--------------------------------------------------------------
void SignalMetronome::newObject(){
    PatchObject::setName( this->objectName );

    this->addOutlet(VP_LINK_AUDIO,"system bpm signal trigger");

}

//--------------------------------------------------------------
void SignalMetronome::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

}

//--------------------------------------------------------------
void SignalMetronome::setupAudioOutObjectContent(pdsp::Engine &engine){
    unusedArgs(engine);

    // ---- this code runs in the audio thread ----
    systemBPM.code = [&]() noexcept {
        mainBPM = static_cast<int>(engine.sequencer.getTempo());
        // System BPM Signal Metronome
        if(systemBPM.frame()%4==0){
            systemBPM.send("bang",1.0f);
        }else{
            systemBPM.send("bang",0.0f);
        }
    };

    systemBPM.out_trig("bang") >> this->pdspOut[0];

}

//--------------------------------------------------------------
void SignalMetronome::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

}

//--------------------------------------------------------------
void SignalMetronome::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);
}

//--------------------------------------------------------------
void SignalMetronome::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        //ImGuiEx::plotValue(*(float *)&_outletParams[0], 0.f, 1.f, IM_COL32(255,255,255,255), this->scaleFactor);
        ImGui::Text("System BPM: %i",mainBPM);

        _nodeCanvas.EndNodeContent();
    }
}

//--------------------------------------------------------------
void SignalMetronome::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Signal trigger from system BPM",
                "https://mosaic.d3cod3.org/reference.php?r=SignalMetronome", scaleFactor);
}

//--------------------------------------------------------------
void SignalMetronome::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
}

//--------------------------------------------------------------
void SignalMetronome::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);

}

OBJECT_REGISTER( SignalMetronome, "signal metronome", OFXVP_OBJECT_CAT_SOUND)

#endif
