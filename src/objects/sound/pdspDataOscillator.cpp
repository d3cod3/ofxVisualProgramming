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

#include "pdspDataOscillator.h"

//--------------------------------------------------------------
pdspDataOscillator::pdspDataOscillator() : PatchObject("data oscillator"){

    this->numInlets  = 2;
    this->numOutlets = 2;

    _inletParams[0] = new float();  // pitch
    *(float *)&_inletParams[0] = 0.0f;

    _inletParams[1] = new vector<float>(); // data

    _outletParams[0] = new ofSoundBuffer(); // audio output
    _outletParams[1] = new vector<float>(); // audio buffer

    this->initInletsState();

    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    pitch                   = 72.0f;

    loaded                  = false;
    reinitDataTable         = false;

    this->height            *= 1.5f;

}

//--------------------------------------------------------------
void pdspDataOscillator::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"pitch");
    this->addInlet(VP_LINK_ARRAY,"data");

    this->addOutlet(VP_LINK_AUDIO,"signal");
    this->addOutlet(VP_LINK_ARRAY,"dataBuffer");

    this->setCustomVar(pitch,"PITCH");
}

//--------------------------------------------------------------
void pdspDataOscillator::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadAudioSettings();
}

//--------------------------------------------------------------
void pdspDataOscillator::setupAudioOutObjectContent(pdsp::Engine &engine){
    // we filter the frequency below 20 hz (not audible) just to remove DC offsets
    20.0f >> leakDC.in_freq();

    pitch_ctrl >> osc.in_pitch();
    pitch_ctrl.set(pitch);
    pitch_ctrl.enableSmoothing(50.0f);

    datatable.setup(256,256);
    datatable.smoothing(0.5f);
    osc.setTable( datatable );

    osc >> leakDC >> this->pdspOut[0];
    osc >> leakDC >> scope >> engine.blackhole();
}

//--------------------------------------------------------------
void pdspDataOscillator::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    // PITCH
    if(this->inletsConnected[0]){
        pitch = ofClamp(*(float *)&_inletParams[0],0,127);
        pitch_ctrl.set(pitch);
        //oscInfo->setLabel(ofToString() + " Hz");
    }

    // DATA
    if(datatable.ready()){
        if(this->inletsConnected[1]){
            if(datatable.ready()){
                datatable.begin();
                if(static_cast<vector<float> *>(_inletParams[1])->size() <= datatable.getTableLength()){
                    for(int n=0; n<static_cast<int>(static_cast<vector<float> *>(_inletParams[1])->size()); n++){
                        int pos = static_cast<int>(floor(ofMap(n,0,static_cast<int>(static_cast<vector<float> *>(_inletParams[1])->size()),0,datatable.getTableLength())));
                        float sample = ofMap(static_cast<vector<float> *>(_inletParams[1])->at(n), 0.0f, 1.0f, -0.5f, 0.5f);
                        datatable.data(pos, sample);
                    }
                }else{
                    for(int n=0; n<datatable.getTableLength(); ++n){
                        int pos = static_cast<int>(floor(ofMap(n,0,datatable.getTableLength(),0,static_cast<int>(static_cast<vector<float> *>(_inletParams[1])->size()))));
                        float sample = ofMap(static_cast<vector<float> *>(_inletParams[1])->at(pos), 0.0f, 1.0f, -0.5f, 0.5f);
                        datatable.data(n, sample);
                    }
                }

                datatable.end(false);
            }
        }else{
            if(datatable.ready()){
                datatable.begin();
                for(int n=0; n<datatable.getTableLength(); ++n){
                    float sample = 0.0f;
                    datatable.data(n, sample);
                }
                datatable.end(false);
            }
        }

    }

    if(!loaded){
        loaded = true;
        pitch = ofClamp(this->getCustomVar("PITCH"),0,127);
        pitch_ctrl.set(pitch);
    }

}

//--------------------------------------------------------------
void pdspDataOscillator::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void pdspDataOscillator::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        // draw waveform
        ImGuiEx::drawWaveform(_nodeCanvas.getNodeDrawList(), ImVec2(ImGui::GetWindowSize().x,ImGui::GetWindowSize().y*0.5f), plot_data, bufferSize, 1.3f, IM_COL32(255,255,120,255), this->scaleFactor);

        char temp[128];
        sprintf_s(temp,"%.2f Hz", pdsp::PitchToFreq::eval(ofClamp(pitch,0,127)));
        _nodeCanvas.getNodeDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(ImGui::GetWindowPos().x + ((40*scaleFactor)*_nodeCanvas.GetCanvasScale()), ImGui::GetWindowPos().y + (ImGui::GetWindowSize().y*0.34)), IM_COL32_WHITE,temp, NULL, 0.0f);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*scaleFactor));
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/6, IM_COL32(255,255,120,255), "pitch", &pitch, 0.0f, 127.0f, 1270.0f)){
            this->setCustomVar(pitch,"PITCH");
            pitch_ctrl.set(pitch);
        }

        _nodeCanvas.EndNodeContent();

    }

}

//--------------------------------------------------------------
void pdspDataOscillator::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Data table based oscillator",
                "https://mosaic.d3cod3.org/reference.php?r=data-oscillator", scaleFactor);
}

//--------------------------------------------------------------
void pdspDataOscillator::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspDataOscillator::loadAudioSettings(){
    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){
        if (XML.pushTag("settings")){
            sampleRate = XML.getValue("sample_rate_in",0);
            bufferSize = XML.getValue("buffer_size",0);

            plot_data = new float[bufferSize];
            for(int i=0;i<bufferSize;i++){
                static_cast<vector<float> *>(_outletParams[1])->push_back(0.0f);
                plot_data[i] = 0.0f;
            }

            XML.popTag();
        }
    }
}

//--------------------------------------------------------------
void pdspDataOscillator::audioOutObject(ofSoundBuffer &outputBuffer){

    for(size_t i = 0; i < scope.getBuffer().size(); i++) {
        float sample = scope.getBuffer().at(i);
        plot_data[i] = hardClip(sample);

        // SIGNAL BUFFER DATA
        static_cast<vector<float> *>(_outletParams[1])->at(i) = sample;
    }
    // SIGNAL BUFFER
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}


OBJECT_REGISTER( pdspDataOscillator, "data oscillator", OFXVP_OBJECT_CAT_SOUND)

#endif
