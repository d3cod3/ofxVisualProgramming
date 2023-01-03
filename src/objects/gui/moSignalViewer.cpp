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

#include "moSignalViewer.h"

//--------------------------------------------------------------
moSignalViewer::moSignalViewer() :
            PatchObject("signal viewer")

{

    this->numInlets  = 1;
    this->numOutlets = 4;

    _inletParams[0] = new ofSoundBuffer();  // signal

    _outletParams[0] = new ofSoundBuffer();     // signal
    _outletParams[1] = new ofSoundBuffer();     // signal
    _outletParams[2] = new vector<float>();     // audio buffer
    _outletParams[3] = new float();             // RMS
    *(float *)&_outletParams[3] = 0.0f;

    this->initInletsState();

    this->width             *= 2;

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;
}

//--------------------------------------------------------------
void moSignalViewer::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addOutlet(VP_LINK_AUDIO,"signal");
    this->addOutlet(VP_LINK_AUDIO,"signal");
    this->addOutlet(VP_LINK_ARRAY,"dataBuffer");
    this->addOutlet(VP_LINK_NUMERIC,"RMSAmplitude");
}

//--------------------------------------------------------------
void moSignalViewer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadAudioSettings();
}

//--------------------------------------------------------------
void moSignalViewer::setupAudioOutObjectContent(pdsp::Engine &engine){
    this->pdspIn[0] >> this->pdspOut[0];
    this->pdspIn[0] >> this->pdspOut[1];
}

//--------------------------------------------------------------
void moSignalViewer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    if(this->inletsConnected[0]){
        *(float *)&_outletParams[3] = ofClamp(static_cast<ofSoundBuffer *>(_inletParams[0])->getRMSAmplitude(),0.0,1.0);
    }else{
        *(float *)&_outletParams[3] = 0;
    }
}

//--------------------------------------------------------------
void moSignalViewer::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){

}

//--------------------------------------------------------------
void moSignalViewer::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        // draw waveform
        ImGuiEx::drawWaveform(_nodeCanvas.getNodeDrawList(), ImGui::GetWindowSize(), plot_data, bufferSize, 1.3f, IM_COL32(255,255,120,255), this->scaleFactor);

        // draw signal RMS amplitude
        _nodeCanvas.getNodeDrawList()->AddRectFilled(ImGui::GetWindowPos()+ImVec2(0,ImGui::GetWindowSize().y),ImGui::GetWindowPos()+ImVec2(ImGui::GetWindowSize().x,ImGui::GetWindowSize().y * (1.0f - ofClamp(static_cast<ofSoundBuffer *>(_inletParams[0])->getRMSAmplitude(),0.0,1.0))),IM_COL32(255,255,120,12));

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void moSignalViewer::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Audio signal display, also byapass it through its outlets, plus the data buffer and the RMS amplitude.",
                "https://mosaic.d3cod3.org/reference.php?r=signal-viewer", scaleFactor);
}

//--------------------------------------------------------------
void moSignalViewer::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void moSignalViewer::loadAudioSettings(){
    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){
        if (XML.pushTag("settings")){
            sampleRate = XML.getValue("sample_rate_in",0);
            bufferSize = XML.getValue("buffer_size",0);

            plot_data = new float[bufferSize];
            for(int i=0;i<bufferSize;i++){
                static_cast<vector<float> *>(_outletParams[2])->push_back(0.0f);
                plot_data[i] = 0.0f;
            }

            XML.popTag();
        }
    }
}

//--------------------------------------------------------------
void moSignalViewer::audioInObject(ofSoundBuffer &inputBuffer){

}

//--------------------------------------------------------------
void moSignalViewer::audioOutObject(ofSoundBuffer &outBuffer){
    if(this->inletsConnected[0]){
        *static_cast<ofSoundBuffer *>(_outletParams[0]) = *static_cast<ofSoundBuffer *>(_inletParams[0]);
        *static_cast<ofSoundBuffer *>(_outletParams[1]) = *static_cast<ofSoundBuffer *>(_inletParams[0]);

        for(size_t i = 0; i < static_cast<ofSoundBuffer *>(_inletParams[0])->getNumFrames(); i++) {
            float sample = static_cast<ofSoundBuffer *>(_inletParams[0])->getSample(i,0);
            plot_data[i] = hardClip(sample);

            // SIGNAL BUFFER DATA
            static_cast<vector<float> *>(_outletParams[2])->at(i) = sample;
        }
    }else{
        *static_cast<ofSoundBuffer *>(_outletParams[0]) *= 0.0f;
        *static_cast<ofSoundBuffer *>(_outletParams[1]) *= 0.0f;
    }

}

OBJECT_REGISTER( moSignalViewer, "signal viewer", OFXVP_OBJECT_CAT_GUI)

#endif
