/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2025 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "pdspAddressSequencer.h"

//--------------------------------------------------------------
pdspAddressSequencer::pdspAddressSequencer() : PatchObject("address sequencer"){

    this->numInlets  = 4;
    this->numOutlets = 2;

    _inletParams[0] = new vector<float>();      // values
    _inletParams[1] = new float();              // ratio
    *(float *)&_inletParams[1] = 1.0f;
    _inletParams[2] = new float();              // steps
    *(float *)&_inletParams[2] = 8.0f;
    _inletParams[3] = new float();              // sync
    *(float *)&_inletParams[3] = 0.0f;

    _outletParams[0] = new float();             // bang
    *(float *)&_outletParams[0] = 0.0f;

    _outletParams[1] = new float();             // value
    *(float *)&_outletParams[1] = 0.0f;


    this->initInletsState();

    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    loaded                  = false;

    meter_step              = 0;
    manualSteps             = 8;
    actualSteps             = manualSteps;

    scaleMode               = Scale_Mode_x1;
    scaleMultiplier         = 1.0f;

    rev                     = false;

    this->width  *= 3.5f;
    this->height *= 1.62f;

}

//--------------------------------------------------------------
void pdspAddressSequencer::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"values");
    this->addInlet(VP_LINK_NUMERIC,"ratio");
    this->addInlet(VP_LINK_NUMERIC,"steps");
    this->addInlet(VP_LINK_NUMERIC,"sync");

    this->addOutlet(VP_LINK_NUMERIC,"bang");
    this->addOutlet(VP_LINK_NUMERIC,"value");

    this->setCustomVar(static_cast<float>(actualSteps),"MANUAL_STEPS");
    this->setCustomVar(static_cast<float>(rev),"REVERSE");
    this->setCustomVar(static_cast<float>(scaleMode),"SCALE_MODE");

    for(size_t i=0;i<8;i++){
        this->setCustomVar(0.0f,"S_"+ofToString(i+1));
    }

}

//--------------------------------------------------------------
void pdspAddressSequencer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    for(size_t i=0;i<8;i++){
        seqSteps[i] = 0.0f;
        static_cast<vector<float> *>(_inletParams[0])->push_back(0.0f);
    }

    scaleModesString.push_back("/4");
    scaleModesString.push_back("/3");
    scaleModesString.push_back("/2.5");
    scaleModesString.push_back("/2");
    scaleModesString.push_back("/1.5");
    scaleModesString.push_back("x1");
    scaleModesString.push_back("x1.5");
    scaleModesString.push_back("x2");
    scaleModesString.push_back("x2.5");
    scaleModesString.push_back("x3");
    scaleModesString.push_back("x4");
}

//--------------------------------------------------------------
void pdspAddressSequencer::setupAudioOutObjectContent(pdsp::Engine &engine){
    unusedArgs(engine);

    // ---- this code runs in the audio thread ----    

    seq.timing = 48;

    //
    seq.code = [&]() noexcept {
        // actual sequencer step
        if(rev){
            if(seq.frame()%static_cast<int>(floor(6.0f/scaleMultiplier))==0){
                if(meter_step > 0){
                    meter_step--;
                }else{
                    meter_step = actualSteps.load()-1;
                }
            }
            //meter_step = 7 - (seq.frame()%actualSteps.load());
        }else{
            if(seq.frame()%static_cast<int>(floor(6.0f/scaleMultiplier))==0){
                if(meter_step < actualSteps.load()-1){
                    meter_step++;
                }else{
                    meter_step = 0;
                }
            }
            //meter_step = static_cast<int>(floor(seq.frame()*scaleMultiplier))%actualSteps.load();
        }


        // SEQ bangs
        if(seqSteps[meter_step]>0.0f){
            *(float *)&_outletParams[0] = 1.0f;
        }else{
            *(float *)&_outletParams[0] = 0.0f;
        }

        // CTRLS values
        *(float *)&_outletParams[1] = seqSteps[meter_step];      // value

    };

}

//--------------------------------------------------------------
void pdspAddressSequencer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(this->inletsConnected[1]){ // ratio
        scaleMultiplier = *(float *)&_inletParams[1]; //ofClamp(*(float *)&_inletParams[1],0.25f,4.0f);
    }

    if(this->inletsConnected[2]){ // steps
        manualSteps = static_cast<int>(floor(ofClamp(*(float *)&_inletParams[2],1,8)));
        actualSteps = manualSteps;
    }

    // SYNC
    if(this->inletsConnected[3]){
        if(*(float *)&_inletParams[3] == 1.0f){
            if(rev){
                 meter_step = actualSteps.load()-1;
            }else{
                 meter_step = 0;
            }

        }
    }

    if(!loaded){
        loaded = true;
        manualSteps = static_cast<int>(this->getCustomVar("MANUAL_STEPS"));
        rev = static_cast<bool>(this->getCustomVar("REVERSE"));

        scaleMode = static_cast<int>(this->getCustomVar("SCALE_MODE"));
        if(scaleMode == Scale_Mode_d4){
            scaleMultiplier = 1.0f/4.0f;
        }else if(scaleMode == Scale_Mode_d3){
            scaleMultiplier = 1.0f/3.0f;
        }else if(scaleMode == Scale_Mode_d2_5){
            scaleMultiplier = 1.0f/2.5f;
        }else if(scaleMode == Scale_Mode_d2){
            scaleMultiplier = 1.0f/2.0f;
        }else if(scaleMode == Scale_Mode_d1_5){
            scaleMultiplier = 1.0f/1.5f;
        }else if(scaleMode == Scale_Mode_x1){
            scaleMultiplier = 1.0f;
        }else if(scaleMode == Scale_Mode_x1_5){
            scaleMultiplier = 1.5f;
        }else if(scaleMode == Scale_Mode_x2){
            scaleMultiplier = 2.0f;
        }else if(scaleMode == Scale_Mode_x2_5){
            scaleMultiplier = 2.5f;
        }else if(scaleMode == Scale_Mode_x3){
            scaleMultiplier = 3.0f;
        }else if(scaleMode == Scale_Mode_x4){
            scaleMultiplier = 4.0f;
        }

        actualSteps = manualSteps;
        for(size_t i=0;i<8;i++){
            seqSteps[i] = this->getCustomVar("S_"+ofToString(i+1));
        }
    }
}

//--------------------------------------------------------------
void pdspAddressSequencer::updateAudioObjectContent(pdsp::Engine &engine){
    unusedArgs(engine);
}

//--------------------------------------------------------------
void pdspAddressSequencer::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

}

//--------------------------------------------------------------
void pdspAddressSequencer::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        char temp[32];

        for(size_t i=0;i<8;i++){
            // gate leds
            ImVec2 stepPos = ImVec2(window_pos.x - 10 + (window_size.x-(50*scaleFactor))/8 * (i+1),window_pos.y + (32*scaleFactor));
            if(i == static_cast<unsigned long>(meter_step) && seqSteps[i] > 0.0f){
                _nodeCanvas.getNodeDrawList()->AddCircleFilled(stepPos, 5*scaleFactor, IM_COL32(255, 255, 120, 140), 40);
            }
        }

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*8*scaleFactor));
        for(size_t i=0;i<8;i++){
            sprintf_s(temp,"S %s",to_string(i+1).c_str());
            if(ImGuiKnobs::Knob(temp, &seqSteps[i], 0.0f, 1.0f, 0.001f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
                this->setCustomVar(seqSteps[i],"S_"+ofToString(i+1));
            }
            if(i<7){
                ImGui::SameLine();
            }
        }

        for(size_t i=0;i<8;i++){
            // step leds
            ImVec2 stepPos = ImVec2(window_pos.x - 10 + (window_size.x-(50*scaleFactor))/8 * (i+1),window_pos.y + window_size.y - (20*scaleFactor));
            if(i == static_cast<size_t>(meter_step)){
                _nodeCanvas.getNodeDrawList()->AddCircleFilled(stepPos, 6*scaleFactor, IM_COL32(182, 30, 41, 255), 40);
            }else{
                _nodeCanvas.getNodeDrawList()->AddCircleFilled(stepPos, 5*scaleFactor, IM_COL32(50, 50, 50, 255), 40);
            }
        }

        _nodeCanvas.EndNodeContent();

    }

}

//--------------------------------------------------------------
void pdspAddressSequencer::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(ImGui::SliderInt("steps", &manualSteps, 1, 8)){
        actualSteps = manualSteps;
        this->setCustomVar(static_cast<float>(manualSteps),"MANUAL_STEPS");
    }
    ImGui::Spacing();
    if(ImGui::BeginCombo("ratio", scaleModesString.at(scaleMode).c_str(),ImGuiComboFlags_HeightLargest)){
        for(unsigned int i=0; i < scaleModesString.size(); ++i){
            bool is_selected = (scaleMode == (int)i );
            if (ImGui::Selectable(scaleModesString.at(i).c_str(), is_selected)){
                scaleMode = i;
                this->setCustomVar(static_cast<float>(scaleMode),"SCALE_MODE");
                if(scaleMode == Scale_Mode_d4){
                    scaleMultiplier = 1.0f/4.0f;
                }else if(scaleMode == Scale_Mode_d3){
                    scaleMultiplier = 1.0f/3.0f;
                }else if(scaleMode == Scale_Mode_d2_5){
                    scaleMultiplier = 1.0f/2.5f;
                }else if(scaleMode == Scale_Mode_d2){
                    scaleMultiplier = 1.0f/2.0f;
                }else if(scaleMode == Scale_Mode_d1_5){
                    scaleMultiplier = 1.0f/1.5f;
                }else if(scaleMode == Scale_Mode_x1){
                    scaleMultiplier = 1.0f;
                }else if(scaleMode == Scale_Mode_x1_5){
                    scaleMultiplier = 1.5f;
                }else if(scaleMode == Scale_Mode_x2){
                    scaleMultiplier = 2.0f;
                }else if(scaleMode == Scale_Mode_x2_5){
                    scaleMultiplier = 2.5f;
                }else if(scaleMode == Scale_Mode_x3){
                    scaleMultiplier = 3.0f;
                }else if(scaleMode == Scale_Mode_x4){
                    scaleMultiplier = 4.0f;
                }
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Text("Fwd"); ImGui::SameLine(); ImGuiEx::ToggleButton("seq_direction",&rev); ImGui::SameLine(); ImGui::Text("Rev");
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    if(ImGui::Button("Randomize",ImVec2(224*scaleFactor,26*scaleFactor))){
        for(size_t i=0;i<8;i++){
            seqSteps[i] = ofRandomuf();
            this->setCustomVar(seqSteps[i],"S_"+ofToString(i+1));
        }
    }
    ImGui::Spacing();

    ImGuiEx::ObjectInfo(
                "8 step address sequencer.",
                "https://mosaic.d3cod3.org/reference.php?r=address-sequencer", scaleFactor);
}

//--------------------------------------------------------------
void pdspAddressSequencer::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
}

//--------------------------------------------------------------
void pdspAddressSequencer::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);

    // S
    if(this->inletsConnected[0] && !static_cast<vector<float> *>(_inletParams[0])->empty()){
        for(size_t i=0;i<8;i++){
            if(i < static_cast<vector<float> *>(_inletParams[0])->size()){
                seqSteps[i] = static_cast<vector<float> *>(_inletParams[0])->at(i);
            }
        }
    }

    
}

OBJECT_REGISTER( pdspAddressSequencer, "address sequencer", OFXVP_OBJECT_CAT_SOUND)

#endif
