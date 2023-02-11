/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2020 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "pdspSequencer.h"

const char* steps_nums[Steps_COUNT] = { "16", "32", "48", "64" };
const char* steps_names[Steps_COUNT] = { "1-16", "17-32", "33-48", "49-64" };

//--------------------------------------------------------------
pdspSequencer::pdspSequencer() : PatchObject("sequencer"){

    this->numInlets  = 6;
    this->numOutlets = 21;

    _inletParams[0] = new vector<float>(); // S
    _inletParams[1] = new vector<float>(); // A
    _inletParams[2] = new vector<float>(); // B
    _inletParams[3] = new vector<float>(); // C
    _inletParams[4] = new vector<float>(); // D
    _inletParams[5] = new float();         // steps
    *(float *)&_inletParams[5] = 0.0f;

    _outletParams[0] = new float();          // step
    *(float *)&_outletParams[0] = 0.0f;

    _outletParams[1] = new float();          // step
    *(float *)&_outletParams[1] = 0.0f;

    _outletParams[2] = new float();          // step
    *(float *)&_outletParams[2] = 0.0f;

    _outletParams[3] = new float();          // step
    *(float *)&_outletParams[3] = 0.0f;

    _outletParams[4] = new float();          // step
    *(float *)&_outletParams[4] = 0.0f;

    _outletParams[5] = new float();          // step
    *(float *)&_outletParams[5] = 0.0f;

    _outletParams[6] = new float();          // step
    *(float *)&_outletParams[6] = 0.0f;

    _outletParams[7] = new float();          // step
    *(float *)&_outletParams[7] = 0.0f;

    _outletParams[8] = new float();          // step
    *(float *)&_outletParams[8] = 0.0f;

    _outletParams[9] = new float();          // step
    *(float *)&_outletParams[9] = 0.0f;

    _outletParams[10] = new float();          // step
    *(float *)&_outletParams[10] = 0.0f;

    _outletParams[11] = new float();          // step
    *(float *)&_outletParams[11] = 0.0f;

    _outletParams[12] = new float();          // step
    *(float *)&_outletParams[12] = 0.0f;

    _outletParams[13] = new float();          // step
    *(float *)&_outletParams[13] = 0.0f;

    _outletParams[14] = new float();          // step
    *(float *)&_outletParams[14] = 0.0f;

    _outletParams[15] = new float();          // step
    *(float *)&_outletParams[15] = 0.0f;

    _outletParams[16] = new float();          // S
    *(float *)&_outletParams[16] = 0.0f;

    _outletParams[17] = new float();          // A
    *(float *)&_outletParams[17] = 0.0f;

    _outletParams[18] = new float();          // B
    *(float *)&_outletParams[18] = 0.0f;

    _outletParams[19] = new float();          // C
    *(float *)&_outletParams[19] = 0.0f;

    _outletParams[20] = new float();          // D
    *(float *)&_outletParams[20] = 0.0f;

    this->initInletsState();

    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    loaded                  = false;

    meter_step              = 0;
    chapter                 = 0;
    maxChapter              = Steps_1_16;
    manualSteps             = CHAPTER_STEPS*(maxChapter+1);
    actualSteps             = CHAPTER_STEPS*(maxChapter+1);

    this->width  *= 4.5f;
    this->height *= 4.0f;

}

//--------------------------------------------------------------
void pdspSequencer::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"S");
    this->addInlet(VP_LINK_ARRAY,"A");
    this->addInlet(VP_LINK_ARRAY,"B");
    this->addInlet(VP_LINK_ARRAY,"C");
    this->addInlet(VP_LINK_ARRAY,"D");
    this->addInlet(VP_LINK_NUMERIC,"steps");

    this->addOutlet(VP_LINK_NUMERIC,"s1");
    this->addOutlet(VP_LINK_NUMERIC,"s2");
    this->addOutlet(VP_LINK_NUMERIC,"s3");
    this->addOutlet(VP_LINK_NUMERIC,"s4");
    this->addOutlet(VP_LINK_NUMERIC,"s5");
    this->addOutlet(VP_LINK_NUMERIC,"s6");
    this->addOutlet(VP_LINK_NUMERIC,"s7");
    this->addOutlet(VP_LINK_NUMERIC,"s8");

    this->addOutlet(VP_LINK_NUMERIC,"s9");
    this->addOutlet(VP_LINK_NUMERIC,"s10");
    this->addOutlet(VP_LINK_NUMERIC,"s11");
    this->addOutlet(VP_LINK_NUMERIC,"s12");
    this->addOutlet(VP_LINK_NUMERIC,"s13");
    this->addOutlet(VP_LINK_NUMERIC,"s14");
    this->addOutlet(VP_LINK_NUMERIC,"s15");
    this->addOutlet(VP_LINK_NUMERIC,"s16");

    this->addOutlet(VP_LINK_NUMERIC,"S");
    this->addOutlet(VP_LINK_NUMERIC,"A");
    this->addOutlet(VP_LINK_NUMERIC,"B");
    this->addOutlet(VP_LINK_NUMERIC,"C");
    this->addOutlet(VP_LINK_NUMERIC,"D");

    this->setCustomVar(0.0f,"STEPS");
    this->setCustomVar(static_cast<float>(actualSteps),"MANUAL_STEPS");

    for(size_t i=0;i<SEQUENCER_STEPS;i++){
        this->setCustomVar(0.0f,"S_"+ofToString(i+1));
        this->setCustomVar(0.0f,"A_"+ofToString(i+1));
        this->setCustomVar(0.0f,"B_"+ofToString(i+1));
        this->setCustomVar(0.0f,"C_"+ofToString(i+1));
        this->setCustomVar(0.0f,"D_"+ofToString(i+1));
    }

}

//--------------------------------------------------------------
void pdspSequencer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    // ---- this code runs in the audio thread ----
    seq.code = [&]() noexcept {
        // actual sequencer step
        int step = seq.frame()%actualSteps.load();

        meter_step = step;

        // CTRLS
        *(float *)&_outletParams[16] = seqSteps[meter_step];      // S
        *(float *)&_outletParams[17] = ctrl1Steps[meter_step];    // A
        *(float *)&_outletParams[18] = ctrl2Steps[meter_step];    // B
        *(float *)&_outletParams[19] = ctrl3Steps[meter_step];    // C
        *(float *)&_outletParams[20] = ctrl4Steps[meter_step];    // D

        // SEQ
        if(seqSteps[meter_step]>0.0f){
            *(float *)&_outletParams[meter_step - (static_cast<int>(floor(meter_step/16))*CHAPTER_STEPS)] = 1.0f;
        }else{
            *(float *)&_outletParams[meter_step - (static_cast<int>(floor(meter_step/16))*CHAPTER_STEPS)] = 0.0f;
        }

        for(int i=0;i<CHAPTER_STEPS;i++){
            if(i == meter_step){
                *(float *)&_outletParams[i] = seqSteps[meter_step];
            }else{
                *(float *)&_outletParams[i] = 0.0f;
            }
        }

    };

    for(size_t i=0;i<SEQUENCER_STEPS;i++){
        seqSteps[i] = 0.0f;
        ctrl1Steps[i] = 0.0f;
        ctrl2Steps[i] = 0.0f;
        ctrl3Steps[i] = 0.0f;
        ctrl4Steps[i] = 0.0f;

        static_cast<vector<float> *>(_inletParams[0])->push_back(0.0f);
        static_cast<vector<float> *>(_inletParams[1])->push_back(0.0f);
        static_cast<vector<float> *>(_inletParams[2])->push_back(0.0f);
        static_cast<vector<float> *>(_inletParams[3])->push_back(0.0f);
        static_cast<vector<float> *>(_inletParams[4])->push_back(0.0f);
    }
}

//--------------------------------------------------------------
void pdspSequencer::setupAudioOutObjectContent(pdsp::Engine &engine){
    unusedArgs(engine);

}

//--------------------------------------------------------------
void pdspSequencer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    // S
    if(this->inletsConnected[0] && !static_cast<vector<float> *>(_inletParams[0])->empty()){
        for(size_t i=0;i<SEQUENCER_STEPS;i++){
            if(i < static_cast<vector<float> *>(_inletParams[0])->size()){
                seqSteps[i] = static_cast<vector<float> *>(_inletParams[0])->at(i);
            }
        }
    }

    // A
    if(this->inletsConnected[1] && !static_cast<vector<float> *>(_inletParams[1])->empty()){
        for(size_t i=0;i<SEQUENCER_STEPS;i++){
            if(i < static_cast<vector<float> *>(_inletParams[1])->size()){
                ctrl1Steps[i] = static_cast<vector<float> *>(_inletParams[1])->at(i);
            }
        }
    }

    // B
    if(this->inletsConnected[2] && !static_cast<vector<float> *>(_inletParams[2])->empty()){
        for(size_t i=0;i<SEQUENCER_STEPS;i++){
            if(i < static_cast<vector<float> *>(_inletParams[2])->size()){
                ctrl2Steps[i] = static_cast<vector<float> *>(_inletParams[2])->at(i);
            }
        }
    }

    // C
    if(this->inletsConnected[3] && !static_cast<vector<float> *>(_inletParams[3])->empty()){
        for(size_t i=0;i<SEQUENCER_STEPS;i++){
            if(i < static_cast<vector<float> *>(_inletParams[3])->size()){
                ctrl3Steps[i] = static_cast<vector<float> *>(_inletParams[3])->at(i);
            }
        }
    }

    // D
    if(this->inletsConnected[4] && !static_cast<vector<float> *>(_inletParams[4])->empty()){
        for(size_t i=0;i<SEQUENCER_STEPS;i++){
            if(i < static_cast<vector<float> *>(_inletParams[4])->size()){
                ctrl4Steps[i] = static_cast<vector<float> *>(_inletParams[4])->at(i);
            }
        }
    }

    // steps
    if(this->inletsConnected[5]){
        manualSteps = static_cast<int>(ofClamp(*(float *)&_inletParams[5],1.0f,SEQUENCER_STEPS*1.0f));
        actualSteps = manualSteps;
        if(actualSteps <= 16){
            maxChapter = 0;
        }else if(actualSteps > 16 && actualSteps <= 32){
            maxChapter = 1;
        }else if(actualSteps > 32 && actualSteps <= 48){
            maxChapter = 2;
        }else if(actualSteps > 48 && actualSteps <= 64){
            maxChapter = 3;
        }
        meter_step = seq.frame()%actualSteps.load();
    }

    if(!loaded){
        loaded = true;
        maxChapter = static_cast<int>(this->getCustomVar("STEPS"));
        manualSteps = static_cast<int>(this->getCustomVar("MANUAL_STEPS"));
        actualSteps = CHAPTER_STEPS*(maxChapter+1);
        for(size_t i=0;i<SEQUENCER_STEPS;i++){
            seqSteps[i] = this->getCustomVar("S_"+ofToString(i+1));
            ctrl1Steps[i] = this->getCustomVar("A_"+ofToString(i+1));
            ctrl2Steps[i] = this->getCustomVar("B_"+ofToString(i+1));
            ctrl3Steps[i] = this->getCustomVar("C_"+ofToString(i+1));
            ctrl4Steps[i] = this->getCustomVar("D_"+ofToString(i+1));
        }
    }
}

//--------------------------------------------------------------
void pdspSequencer::updateAudioObjectContent(pdsp::Engine &engine){
    unusedArgs(engine);
}

//--------------------------------------------------------------
void pdspSequencer::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

    ofSetColor(255);

}

//--------------------------------------------------------------
void pdspSequencer::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        char temp[32];

        for(size_t i=0;i<CHAPTER_STEPS;i++){
            // gate leds
            ImVec2 stepPos = ImVec2(window_pos.x + (window_size.x-(40*scaleFactor))/16 * (i+1),window_pos.y + (32*scaleFactor));
            if((i + (chapter*CHAPTER_STEPS)) == static_cast<unsigned long>(meter_step) && seqSteps[i + (chapter*CHAPTER_STEPS)] > 0.0f){
                _nodeCanvas.getNodeDrawList()->AddCircleFilled(stepPos, 5*scaleFactor, IM_COL32(255, 255, 120, 140), 40);
            }
        }

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*4*scaleFactor));
        for(size_t i=0;i<CHAPTER_STEPS;i++){
            sprintf_s(temp,"S %s",to_string(i+1+ (chapter*CHAPTER_STEPS)).c_str());
            if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (window_size.x-(46*scaleFactor))/39, IM_COL32(255,255,120,255), temp, &seqSteps[i + (chapter*CHAPTER_STEPS)], 0.0f, 1.0f, 100.0f)){
                this->setCustomVar(seqSteps[i + (chapter*CHAPTER_STEPS)],"S_"+ofToString(i+1+ (chapter*CHAPTER_STEPS)));
            }
            if(i<15){
                ImGui::SameLine();
            }
        }

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*4*scaleFactor));
        for(size_t i=0;i<CHAPTER_STEPS;i++){
            sprintf_s(temp,"A %s",to_string(i+1+ (chapter*CHAPTER_STEPS)).c_str());
            if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (window_size.x-(46*scaleFactor))/39, IM_COL32(255,255,120,255), temp, &ctrl1Steps[i + (chapter*CHAPTER_STEPS)], 0.0f, 1.0f, 100.0f)){
                this->setCustomVar(ctrl1Steps[i + (chapter*CHAPTER_STEPS)],"A_"+ofToString(i+1+ (chapter*CHAPTER_STEPS)));
            }
            if(i<15){
                ImGui::SameLine();
            }
        }

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*4*scaleFactor));
        for(size_t i=0;i<CHAPTER_STEPS;i++){
            sprintf_s(temp,"B %s",to_string(i+1+ (chapter*CHAPTER_STEPS)).c_str());
            if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (window_size.x-(46*scaleFactor))/39, IM_COL32(255,255,120,255), temp, &ctrl2Steps[i + (chapter*CHAPTER_STEPS)], 0.0f, 1.0f, 100.0f)){
                this->setCustomVar(ctrl2Steps[i + (chapter*CHAPTER_STEPS)],"B_"+ofToString(i+1+ (chapter*CHAPTER_STEPS)));
            }
            if(i<15){
                ImGui::SameLine();
            }
        }

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*4*scaleFactor));
        for(size_t i=0;i<CHAPTER_STEPS;i++){
            sprintf_s(temp,"C %s",to_string(i+1+ (chapter*CHAPTER_STEPS)).c_str());
            if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (window_size.x-(46*scaleFactor))/39, IM_COL32(255,255,120,255), temp, &ctrl3Steps[i + (chapter*CHAPTER_STEPS)], 0.0f, 1.0f, 100.0f)){
                this->setCustomVar(ctrl3Steps[i + (chapter*CHAPTER_STEPS)],"C_"+ofToString(i+1+ (chapter*CHAPTER_STEPS)));
            }
            if(i<15){
                ImGui::SameLine();
            }
        }

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*4*scaleFactor));
        for(size_t i=0;i<CHAPTER_STEPS;i++){
            sprintf_s(temp,"D %s",to_string(i+1+ (chapter*CHAPTER_STEPS)).c_str());
            if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (window_size.x-(46*scaleFactor))/39, IM_COL32(255,255,120,255), temp, &ctrl4Steps[i + (chapter*CHAPTER_STEPS)], 0.0f, 1.0f, 100.0f)){
                this->setCustomVar(ctrl4Steps[i + (chapter*CHAPTER_STEPS)],"D_"+ofToString(i+1+ (chapter*CHAPTER_STEPS)));
            }
            if(i<15){
                ImGui::SameLine();
            }
        }

        for(size_t i=0;i<CHAPTER_STEPS;i++){
            // step leds
            ImVec2 stepPos = ImVec2(window_pos.x + (window_size.x-(40*scaleFactor))/16 * (i+1),window_pos.y + window_size.y - (40*scaleFactor));
            if((i + (chapter*CHAPTER_STEPS)) == static_cast<size_t>(meter_step)){
                _nodeCanvas.getNodeDrawList()->AddCircleFilled(stepPos, 6*scaleFactor, IM_COL32(182, 30, 41, 255), 40);
            }else{
                _nodeCanvas.getNodeDrawList()->AddCircleFilled(stepPos, 5*scaleFactor, IM_COL32(50, 50, 50, 255), 40);
            }
        }

        _nodeCanvas.EndNodeContent();

    }

}

//--------------------------------------------------------------
void pdspSequencer::drawObjectNodeConfig(){
    ImGui::Spacing();

    std::vector<char*> vectorSteps(maxChapter+1);

    for(int i=0;i<maxChapter+1;i++){
        vectorSteps[i] = (char *)steps_names[i];
    }

    if(ImGui::SliderInt("steps", &maxChapter, 0, Steps_COUNT - 1, steps_nums[maxChapter])){
        actualSteps = CHAPTER_STEPS*(maxChapter+1);
        manualSteps = actualSteps;
        meter_step = seq.frame()%actualSteps.load();
        this->setCustomVar(static_cast<float>(maxChapter),"STEPS");
        this->setCustomVar(static_cast<float>(manualSteps),"MANUAL_STEPS");
    }
    ImGui::Spacing();
    if(ImGui::SliderInt("manual steps", &manualSteps, 1, 64)){
        actualSteps = manualSteps;
        if(actualSteps <= 16){
            maxChapter = 0;
        }else if(actualSteps > 16 && actualSteps <= 32){
            maxChapter = 1;
        }else if(actualSteps > 32 && actualSteps <= 48){
            maxChapter = 2;
        }else if(actualSteps > 48 && actualSteps <= 64){
            maxChapter = 3;
        }
        meter_step = seq.frame()%actualSteps.load();
        this->setCustomVar(static_cast<float>(manualSteps),"MANUAL_STEPS");
    }
    ImGui::Spacing();
    ImGui::ListBox("sections", &chapter, &vectorSteps[0], vectorSteps.size(), 4);
    ImGuiEx::ObjectInfo(
                "Up to 64 step sequencer with 5 per-step assignable controls.",
                "https://mosaic.d3cod3.org/reference.php?r=sequencer", scaleFactor);
}

//--------------------------------------------------------------
void pdspSequencer::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
}

//--------------------------------------------------------------
void pdspSequencer::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);
}

OBJECT_REGISTER( pdspSequencer, "sequencer", OFXVP_OBJECT_CAT_SOUND)

#endif
