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

//--------------------------------------------------------------
pdspSequencer::pdspSequencer() : PatchObject("sequencer"){

    this->numInlets  = 5;
    this->numOutlets = 21;

    _inletParams[0] = new vector<float>(); // S
    _inletParams[1] = new vector<float>(); // A
    _inletParams[2] = new vector<float>(); // B
    _inletParams[3] = new vector<float>(); // C
    _inletParams[4] = new vector<float>(); // D

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

    step                    = 0;
    chapter                 = 0;
    maxChapter              = Steps_1_16;
    actualSteps             = CHAPTER_STEPS*(maxChapter+1);

    metro                   = false;
    bang                    = false;

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
    seq.code = [&]() noexcept {
        // actual sequencer step
        step = seq.frame()%actualSteps.load();

        // BPM metronome
        if(seq.frame()%4==0) metro = true;

        // raw step bang
        bang = true;

        // fix jitter for outside triggers ( testing )
        time_span = std::chrono::steady_clock::now() - clock_begin;
        clock_begin = std::chrono::steady_clock::now();

        step_millis = static_cast<int>(floor(double(time_span.count()) * 1000 * std::chrono::steady_clock::period::num / std::chrono::steady_clock::period::den));

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
    step_millis = static_cast<int>(floor(60000 / 4 / 108));
    expected_step_millis = static_cast<int>(floor(60000 / 4 / 108));
}

//--------------------------------------------------------------
void pdspSequencer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(bang){

        int timeGap = expected_step_millis.load()-step_millis.load();
        if(timeGap > 1){
            // fix jitter
            std::this_thread::sleep_for(std::chrono::milliseconds(timeGap));
        }

        bang = false;

        // CTRLS
        *(float *)&_outletParams[16] = seqSteps[step];      // S
        *(float *)&_outletParams[17] = ctrl1Steps[step];    // A
        *(float *)&_outletParams[18] = ctrl2Steps[step];    // B
        *(float *)&_outletParams[19] = ctrl3Steps[step];    // C
        *(float *)&_outletParams[20] = ctrl4Steps[step];    // D

        // SEQ
        if(seqSteps[step]>0.0f){
            *(float *)&_outletParams[step - (static_cast<int>(floor(step/16))*CHAPTER_STEPS)] = 1.0f;
        }else{
            *(float *)&_outletParams[step - (static_cast<int>(floor(step/16))*CHAPTER_STEPS)] = 0.0f;
        }
        /*for(size_t i=0;i<SEQUENCER_STEPS;i++){
            if(i==step && seqSteps[i]>0.0f){
                *(float *)&_outletParams[i - (static_cast<int>(floor(step/16))*CHAPTER_STEPS)] = 1.0f;
                //ofLog(OF_LOG_NOTICE, "Chapter %i, step %i, outlet %i",static_cast<int>(floor(step/16)),step,i - (static_cast<int>(floor(step/16))*CHAPTER_STEPS));
                break;
            }else{
                *(float *)&_outletParams[i - (static_cast<int>(floor(step/16))*CHAPTER_STEPS)] = 0.0f;
            }
        }*/
    }else{
        for(size_t i=0;i<CHAPTER_STEPS;i++){
            *(float *)&_outletParams[i] = 0.0f;
        }
    }

    // S
    if(this->inletsConnected[0]){
        for(size_t i=0;i<SEQUENCER_STEPS;i++){
            if(i < static_cast<vector<float> *>(_inletParams[0])->size()){
                seqSteps[i] = static_cast<vector<float> *>(_inletParams[0])->at(i);
            }
        }
    }

    // A
    if(this->inletsConnected[1]){
        for(size_t i=0;i<SEQUENCER_STEPS;i++){
            if(i < static_cast<vector<float> *>(_inletParams[1])->size()){
                ctrl1Steps[i] = static_cast<vector<float> *>(_inletParams[1])->at(i);
            }
        }
    }

    // B
    if(this->inletsConnected[2]){
        for(size_t i=0;i<SEQUENCER_STEPS;i++){
            if(i < static_cast<vector<float> *>(_inletParams[2])->size()){
                ctrl2Steps[i] = static_cast<vector<float> *>(_inletParams[2])->at(i);
            }
        }
    }

    // C
    if(this->inletsConnected[3]){
        for(size_t i=0;i<SEQUENCER_STEPS;i++){
            if(i < static_cast<vector<float> *>(_inletParams[3])->size()){
                ctrl3Steps[i] = static_cast<vector<float> *>(_inletParams[3])->at(i);
            }
        }
    }

    // D
    if(this->inletsConnected[4]){
        for(size_t i=0;i<SEQUENCER_STEPS;i++){
            if(i < static_cast<vector<float> *>(_inletParams[4])->size()){
                ctrl4Steps[i] = static_cast<vector<float> *>(_inletParams[4])->at(i);
            }
        }
    }

    if(!loaded){
        loaded = true;
        maxChapter = static_cast<int>(this->getCustomVar("STEPS"));
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
    expected_step_millis = static_cast<int>(floor(60000 / 4 / static_cast<int>(floor(engine.sequencer.getTempo()))));
}

//--------------------------------------------------------------
void pdspSequencer::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
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

            ImGui::Spacing();
            const char* steps_nums[Steps_COUNT] = { "16", "32", "48", "64" };
            const char* steps_names[Steps_COUNT] = { "1-16", "17-32", "33-48", "49-64" };
            const char* availablesSteps[maxChapter+1];
            for(int i=0;i<maxChapter+1;i++){
                availablesSteps[i] = steps_names[i];
            }
            if(ImGui::SliderInt("steps", &maxChapter, 0, Steps_COUNT - 1, steps_nums[maxChapter])){
                actualSteps = CHAPTER_STEPS*(maxChapter+1);
                this->setCustomVar(static_cast<float>(maxChapter),"STEPS");
            }
            ImGui::Spacing();
            ImGui::ListBox("sections", &chapter, availablesSteps, IM_ARRAYSIZE(availablesSteps), 4);

            ImGuiEx::ObjectInfo(
                        "Up to 64 step sequencer with 5 per-step assignable controls.",
                        "https://mosaic.d3cod3.org/reference.php?r=secuencer");

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
            ImVec2 stepPos = ImVec2(window_pos.x + (window_size.x-40)/16 * (i+1),window_pos.y + 32);
            if((i + (chapter*CHAPTER_STEPS)) == step && seqSteps[i + (chapter*CHAPTER_STEPS)] > 0.0f){
                _nodeCanvas.getNodeDrawList()->AddCircleFilled(stepPos, 5, IM_COL32(255, 255, 120, 140), 40);
            }
        }

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*4));
        for(size_t i=0;i<CHAPTER_STEPS;i++){
            sprintf(temp,"S %s",to_string(i+1+ (chapter*CHAPTER_STEPS)).c_str());
            if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (window_size.x-46)/37, IM_COL32(255,255,120,255), temp, &seqSteps[i + (chapter*CHAPTER_STEPS)], 0.0f, 1.0f, 100.0f)){
                this->setCustomVar(seqSteps[i + (chapter*CHAPTER_STEPS)],"S_"+ofToString(i+1+ (chapter*CHAPTER_STEPS)));
            }
            if(i<15){
                ImGui::SameLine();
            }
        }

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*4));
        for(size_t i=0;i<CHAPTER_STEPS;i++){
            sprintf(temp,"A %s",to_string(i+1+ (chapter*CHAPTER_STEPS)).c_str());
            if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (window_size.x-46)/37, IM_COL32(255,255,120,255), temp, &ctrl1Steps[i + (chapter*CHAPTER_STEPS)], 0.0f, 1.0f, 100.0f)){
                this->setCustomVar(ctrl1Steps[i + (chapter*CHAPTER_STEPS)],"A_"+ofToString(i+1+ (chapter*CHAPTER_STEPS)));
            }
            if(i<15){
                ImGui::SameLine();
            }
        }

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*4));
        for(size_t i=0;i<CHAPTER_STEPS;i++){
            sprintf(temp,"B %s",to_string(i+1+ (chapter*CHAPTER_STEPS)).c_str());
            if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (window_size.x-46)/37, IM_COL32(255,255,120,255), temp, &ctrl2Steps[i + (chapter*CHAPTER_STEPS)], 0.0f, 1.0f, 100.0f)){
                this->setCustomVar(ctrl2Steps[i + (chapter*CHAPTER_STEPS)],"B_"+ofToString(i+1+ (chapter*CHAPTER_STEPS)));
            }
            if(i<15){
                ImGui::SameLine();
            }
        }

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*4));
        for(size_t i=0;i<CHAPTER_STEPS;i++){
            sprintf(temp,"C %s",to_string(i+1+ (chapter*CHAPTER_STEPS)).c_str());
            if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (window_size.x-46)/37, IM_COL32(255,255,120,255), temp, &ctrl3Steps[i + (chapter*CHAPTER_STEPS)], 0.0f, 1.0f, 100.0f)){
                this->setCustomVar(ctrl3Steps[i + (chapter*CHAPTER_STEPS)],"C_"+ofToString(i+1+ (chapter*CHAPTER_STEPS)));
            }
            if(i<15){
                ImGui::SameLine();
            }
        }

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*4));
        for(size_t i=0;i<CHAPTER_STEPS;i++){
            sprintf(temp,"D %s",to_string(i+1+ (chapter*CHAPTER_STEPS)).c_str());
            if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (window_size.x-46)/37, IM_COL32(255,255,120,255), temp, &ctrl4Steps[i + (chapter*CHAPTER_STEPS)], 0.0f, 1.0f, 100.0f)){
                this->setCustomVar(ctrl4Steps[i + (chapter*CHAPTER_STEPS)],"D_"+ofToString(i+1+ (chapter*CHAPTER_STEPS)));
            }
            if(i<15){
                ImGui::SameLine();
            }
        }

        for(size_t i=0;i<CHAPTER_STEPS;i++){
            // step leds
            ImVec2 stepPos = ImVec2(window_pos.x + (window_size.x-40)/16 * (i+1),window_pos.y + window_size.y - 40);
            if((i + (chapter*CHAPTER_STEPS)) == step){
                _nodeCanvas.getNodeDrawList()->AddCircleFilled(stepPos, 6, IM_COL32(182, 30, 41, 255), 40);
            }else{
                _nodeCanvas.getNodeDrawList()->AddCircleFilled(stepPos, 5, IM_COL32(50, 50, 50, 255), 40);
            }
        }

        // BPM
        /*if(metro){
            metro = false;
            ImVec2 pos = ImVec2(window_pos.x + window_size.x - 30, window_pos.y + 40);
            _nodeCanvas.getNodeDrawList()->AddCircleFilled(pos, 6, IM_COL32(255, 255, 120, 255), 40);
        }*/

    }


}

//--------------------------------------------------------------
void pdspSequencer::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void pdspSequencer::audioOutObject(ofSoundBuffer &outputBuffer){

}

OBJECT_REGISTER( pdspSequencer, "sequencer", OFXVP_OBJECT_CAT_SOUND)

#endif
