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

#include "NoteToFrequency.h"

string notation[NOTES]	 = {"C1","C#1","D1","D#1","E1","F1","F#1","G1","G#1","A1","A#1","B1",
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

// MIDI --> 11 total octaves  --->  assuming A6 (central octave) = LA = 440 Hz
float freq[NOTES]	 = {8.1757989156f, 8.6619572180f, 9.1770239974f, 9.7227182413f, 10.3008611535f, 10.9133822323f, 11.5623257097f, 12.2498573744f, 12.9782717994f, 13.750f, 14.5676175474f, 15.4338531643f,
                        16.3515978313f, 17.3239144361f, 18.3540479948f, 19.4454364826f, 20.6017223071f, 21.8267644646f, 23.1246514195f, 24.4997147489f, 25.9565435987f, 27.50f, 29.1352350949f, 30.8677063285f,
                        32.7031956626f, 34.6478288721f, 36.7080959897f, 38.8908729653f, 41.2034446141f, 43.6535289291f, 46.2493028390f, 48.9994294977f, 51.9130871975f, 55.00f, 58.2704701898f, 61.7354126570f,
                        65.4063913251f, 69.2956577442f, 73.4161919794f, 77.7817459305f, 82.4068892282f, 87.3070578583f, 92.4986056779f, 97.9988589954f, 103.8261743950f, 110.00f, 116.5409403795f, 123.4708253140f,
                        130.8127826503f, 138.5913154884f, 146.8323839587f, 155.5634918610f, 164.8137784564f, 174.6141157165f, 184.9972113558f, 195.9977179909f, 207.6523487900f, 220.00f, 233.0818807590f, 246.9416506281f,
                        261.6255653006f, 277.1826309769f, 293.6647679174f, 311.1269837221f, 329.6275569129f, 349.2282314330f, 369.9944227116f, 391.9954359817f, 415.3046975799f, 440.00f, 466.1637615181f, 493.8833012561f,
                        523.2511306012f, 554.3652619537f, 587.3295358348f, 622.2539674442f, 659.2551138257f, 698.4564628660f, 739.9888454233f, 783.9908719635f, 830.6093951599f, 880.00f, 932.3275230362f, 987.7666025122f,
                        1046.5022612024f, 1108.7305239075f, 1174.6590716696f, 1244.5079348883f, 1318.5102276515f, 1396.9129257320f, 1479.9776908465f, 1567.9817439270f, 1661.2187903198f, 1760.00f, 1864.6550460724f, 1975.5332050245f,
                        2093.0045224048f, 2217.4610478150f, 2349.3181433393f, 2489.0158697766f, 2637.0204553030f, 2793.8258514640f, 2959.9553816931f, 3135.9634878540f, 3322.4375806396f, 3520.00f, 3729.3100921447f, 3951.0664100490f,
                        4186.0090448096f, 4434.9220956300f, 4698.6362866785f, 4978.0317395533f, 5274.0409106059f, 5587.6517029281f, 5919.9107633862f, 6271.9269757080f, 6644.8751612791f, 7040.00f, 7458.6201842894f, 7902.1328200980f,
                        8372.0180896192f, 8869.8441912599f, 9397.2725733570f, 9956.0634791066f, 10548.0818212118f, 11175.3034058561f, 11839.8215267723f, 12543.8539514160f};

// The "Just Scale" (sometimes referred to as "harmonic tuning" or "Helmholtz's scale")
// occurs naturally as a result of the overtone series for simple systems such as vibrating
// strings or air columns. All the notes in the scale are related by rational numbers.
// Unfortunately, with Just tuning, the tuning depends on the scale you are using -
// the tuning for C Major is not the same as for D Major, for example. Just tuning is
// often used by ensembles (such as for choral or orchestra works) as the players match
// pitch with each other "by ear."
// corrected frequency[i] = freq[i] - _harmcorrection[i]
float _harmCorrection[NOTES] = {0.17f, -0.08f, 0.07f, 0.18f, -0.08f, 0.29f, -0.28f, 0.17f, 0.06f, 0.00f, 0.48f, -0.24f,
                                0.33f, -0.17f, 0.14f, 0.37f, -0.16f, 0.58f, -0.55f, 0.35f, 0.12f, 0.00f, 0.96f, -0.49f,
                                0.67f, -0.33f, 0.29f, 0.73f, -0.33f, 1.16f, -1.10f, 0.69f, 0.24f, 0.00f, 1.92f, -0.98f,
                                1.34f, -0.66f, 0.58f, 1.46f, -0.65f, 2.33f, -2.20f, 1.39f, 0.48f, 0.00f, 3.84f, -1.96f,
                                2.67f, -1.32f, 1.16f, 2.93f, -1.30f, 4.66f, -4.40f, 2.78f, 0.97f, 0.00f, 7.67f, -3.91f,
                                5.35f, -2.64f, 2.32f, 5.86f, -2.61f, 9.32f, -8.80f, 5.55f, 1.94f, 0.00f, 15.35f, -7.82f,
                                10.69f, -5.29f, 4.64f, 11.71f, -5.22f, 18.64f, -17.60f, 11.11f, 3.88f, 0.00f, 30.70f, -15.65f,
                                21.38f, -10.57f, 9.28f, 23.42f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f,
                                0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f,
                                0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f,
                                0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f};

//--------------------------------------------------------------
NoteToFrequency::NoteToFrequency() : PatchObject("note to frequency"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // midi [0 - 127]
    *(float *)&_inletParams[0] = 0.0f;

    _outletParams[0] = new float(); // frequency
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    lastNote            = 69; // A4 (central octave) = LA = 440 Hz

    loaded              = false;

    this->width         *= 1.4f;
}

//--------------------------------------------------------------
void NoteToFrequency::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"midi note");

    this->addOutlet(VP_LINK_NUMERIC,"frequency");

    this->setCustomVar(static_cast<float>(lastNote),"MIDI_NOTE");
}

//--------------------------------------------------------------
void NoteToFrequency::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void NoteToFrequency::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[0]){
      lastNote = ofClamp(ofToInt(ofToString(*(float *)&_inletParams[0])),0,127);
    }

    *(float *)&_outletParams[0] = noteToFrequency(static_cast<int>(lastNote));

    if(!loaded){
        loaded = true;
        lastNote = static_cast<int>(floor(this->getCustomVar("MIDI_NOTE")));
    }

}

//--------------------------------------------------------------
void NoteToFrequency::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void NoteToFrequency::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*scaleFactor));

        ImGui::PushItemWidth(90*scaleFactor);
        if(ImGui::InputInt("midi note",&lastNote)){
            if(lastNote < 0 ){
                lastNote = 0;
            }
            if(lastNote > 127 ){
                lastNote = 127;
            }
            this->setCustomVar(static_cast<float>(lastNote),"MIDI_NOTE");
        }
        ImGui::PopItemWidth();
        ImGui::SameLine(); ImGuiEx::HelpMarker("MIDI notes range: [0 - 127]");
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Text("Frequency: %s", ofToString(*(float *)&_outletParams[0],2).c_str());
        ImGui::Spacing();
        ImGui::Text("Notation: %s", notation[static_cast<int>(lastNote)].c_str());

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void NoteToFrequency::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Convert a MIDI note to his correspondent frequency",
                "https://mosaic.d3cod3.org/reference.php?r=note-to-frequency", scaleFactor);
}

//--------------------------------------------------------------
void NoteToFrequency::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------
float NoteToFrequency::noteToFrequency(int _note){
    return freq[_note];
}

//--------------------------------------------------
float NoteToFrequency::noteToHarmonicFrequency(int _note){
    return freq[_note] - _harmCorrection[_note];
}

//--------------------------------------------------
float NoteToFrequency::frequencyToPitch(float freq){
    return pdsp::f2p(freq);
}

OBJECT_REGISTER( NoteToFrequency, "note to frequency", OFXVP_OBJECT_CAT_SOUND)

#endif
