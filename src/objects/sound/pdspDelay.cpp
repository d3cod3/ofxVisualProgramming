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

#include "pdspDelay.h"

#define DELAY_MAX_TIME 10000

//--------------------------------------------------------------
pdspDelay::pdspDelay() : PatchObject("delay"){

    this->numInlets  = 4;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();  // audio input

    _inletParams[1] = new float();          // time
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();          // damping
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();          // feedback
    *(float *)&_inletParams[3] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output

    this->initInletsState();

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    time                    = 1000.0f;
    damping                 = 0.0f;
    feedback                = 0.0f;

    loaded                  = false;

    this->width *= 2.06f;
    this->height *= 1.12f;

}

//--------------------------------------------------------------
void pdspDelay::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"time");
    this->addInlet(VP_LINK_NUMERIC,"damping");
    this->addInlet(VP_LINK_NUMERIC,"feedback");

    this->addOutlet(VP_LINK_AUDIO,"delayedSignal");

    this->setCustomVar(time,"TIME");
    this->setCustomVar(damping,"DAMPING");
    this->setCustomVar(feedback,"FEEDBACK");
}

//--------------------------------------------------------------
void pdspDelay::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadAudioSettings();
}

//--------------------------------------------------------------
void pdspDelay::setupAudioOutObjectContent(pdsp::Engine &engine){

    //delay.setMaxTime(time);

    time_ctrl >> delay.in_time();
    time_ctrl.set(time);
    time_ctrl.enableSmoothing(50.0f);

    damping_ctrl >> delay.in_damping();
    damping_ctrl.set(damping);
    damping_ctrl.enableSmoothing(50.0f);

    feedback_ctrl >> delay.in_feedback();
    feedback_ctrl.set(feedback);
    feedback_ctrl.enableSmoothing(50.0f);

    this->pdspIn[0] >> delay.in_signal();

    delay.out_signal() >> this->pdspOut[0];

    delay.out_signal() >> scope >> engine.blackhole();
}

//--------------------------------------------------------------
void pdspDelay::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[1]){
        time = ofClamp(*(float *)&_inletParams[1],0.0f,DELAY_MAX_TIME);
    }

    if(this->inletsConnected[2]){
        damping = ofClamp(*(float *)&_inletParams[2],0.0f,1.0f);
    }

    if(this->inletsConnected[3]){
        feedback = ofClamp(*(float *)&_inletParams[3],0.0f,1.0f);
    }

    time_ctrl.set(time);
    damping_ctrl.set(damping);
    feedback_ctrl.set(feedback);

    if(!loaded){
        loaded = true;
        time = ofClamp(this->getCustomVar("TIME"),0.0f,DELAY_MAX_TIME);
        damping = ofClamp(this->getCustomVar("DAMPING"),0.0f,1.0f);
        feedback = ofClamp(this->getCustomVar("FEEDBACK"),0.0f,1.0f);
    }

}

//--------------------------------------------------------------
void pdspDelay::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void pdspDelay::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGui::Dummy(ImVec2(0,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,-20,40)*scaleFactor));
        if(ImGuiKnobs::Knob("TIME", &time, 0.0f, DELAY_MAX_TIME, 10.0f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(time,"TIME");
        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,0,90)*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiKnobs::Knob("DAMPING", &damping, 0.0f, 1.0f, 0.001f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(damping,"DAMPING");
        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,0,90)*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiKnobs::Knob("FEEDBACK", &feedback, 0.0f, 1.0f, 0.001f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(feedback,"FEEDBACK");
        }

        _nodeCanvas.EndNodeContent();

    }

}

//--------------------------------------------------------------
void pdspDelay::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Delay with feedback damping",
                "https://mosaic.d3cod3.org/reference.php?r=delay", scaleFactor);
}

//--------------------------------------------------------------
void pdspDelay::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspDelay::loadAudioSettings(){
    ofxXmlSettings XML;

#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
    if (XML.loadFile(patchFile)){
#else
    if (XML.load(patchFile)){
#endif
        if (XML.pushTag("settings")){
            sampleRate = XML.getValue("sample_rate_in",0);
            bufferSize = XML.getValue("buffer_size",0);
            XML.popTag();
        }
    }
}

//--------------------------------------------------------------
void pdspDelay::audioInObject(ofSoundBuffer &inputBuffer){

}

//--------------------------------------------------------------
void pdspDelay::audioOutObject(ofSoundBuffer &outputBuffer){
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}


OBJECT_REGISTER( pdspDelay, "delay", OFXVP_OBJECT_CAT_SOUND)

#endif
