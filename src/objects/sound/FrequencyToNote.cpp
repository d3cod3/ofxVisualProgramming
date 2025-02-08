/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2023 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "FrequencyToNote.h"

string notation_2[NOTES]	 = {"C1","C#1","D1","D#1","E1","F1","F#1","G1","G#1","A1","A#1","B1",
                            "C2","C#2","D2","D#2","E2","F2","F#2","G2","G#2","A2","A#2","B2",
                            "C3","C#3","D3","D#3","E3","F3","F#3","G3","G#3","A3","A#3","B3",
                            "C4","C#4","D4","D#4","E4","F4","F#4","G4","G#4","A4","A#4","B4",
                            "C5","C#5","D5","D#5","E5","F5","F#5","G5","G#5","A5","A#5","B5",
                            "C6","C#6","D6","D#6","E6","F6","F#6","G6","G#6","A6","A#6","B6",
                            "C7","C#7","D7","D#7","E7","F7","F#7","G7","G#7","A7","A#7","B7",
                            "C8","C#8","D8","D#8","E8","F8","F#8","G8","G#8","A8","A#8","B8",
                            "C9","C#9","D9","D#9","E9","F9","F#9","G9","G#9","A9","A#9","B9",
                            "C10","C#10","D10","D#10","E10","F10","F#10","G10","G#10","A10","A#10","B10",
                            "C11","C#11","D11","D#11","E11","F11","F#11","G11"};


//--------------------------------------------------------------
FrequencyToNote::FrequencyToNote() : PatchObject("frequency to note"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // frequency
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]) = 0.0f;

    _outletParams[0] = new float(); // midi [0 - 127]
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0.0f;

    this->initInletsState();

    frequency           = 440.0f;
    lastNote            = frequencyToPitch(frequency);

    loaded              = false;

    this->width         *= 1.4f;
}

//--------------------------------------------------------------
void FrequencyToNote::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"frequency");

    this->addOutlet(VP_LINK_NUMERIC,"midi note");

}

//--------------------------------------------------------------
void FrequencyToNote::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);
}

//--------------------------------------------------------------
void FrequencyToNote::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(this->inletsConnected[0]){
        frequency = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]),1.0f,13289.0f);
    }

    lastNote = static_cast<int>(frequencyToPitch(frequency));
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = lastNote;

    if(!loaded){
        loaded = true;
        lastNote = static_cast<int>(floor(this->getCustomVar("MIDI_NOTE")));
    }

}

//--------------------------------------------------------------
void FrequencyToNote::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

    ofSetColor(255);
}

//--------------------------------------------------------------
void FrequencyToNote::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*scaleFactor));

        ImGui::PushItemWidth(90*scaleFactor);
        ImGui::InputFloat("frequency",&frequency);
        ImGui::PopItemWidth();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Text("Midi note: %s", ofToString(lastNote).c_str());
        ImGui::Spacing();
        ImGui::Text("Notation: %s", notation_2[static_cast<int>(lastNote)].c_str());

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void FrequencyToNote::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Convert a frequency in Hz to his correspondent quantized note",
                "https://mosaic.d3cod3.org/reference.php?r=frequency-to-note", scaleFactor);
}

//--------------------------------------------------------------
void FrequencyToNote::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
}

//--------------------------------------------------
float FrequencyToNote::frequencyToPitch(float freq){
    return pdsp::f2p(freq);
}

OBJECT_REGISTER( FrequencyToNote, "frequency to note", OFXVP_OBJECT_CAT_SOUND)

#endif
