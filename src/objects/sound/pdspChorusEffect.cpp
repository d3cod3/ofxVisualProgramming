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

#include "pdspChorusEffect.h"

//--------------------------------------------------------------
pdspChorusEffect::pdspChorusEffect() : PatchObject("dimension chorus"){

    this->numInlets  = 4;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();  // audio input

    _inletParams[1] = new float();          // speed
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();          // depth
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();          // delay
    *(float *)&_inletParams[3] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output

    this->initInletsState();

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    speed                   = 0.25f;
    depth                   = 10.0f;
    delay                   = 80.0f;

    loaded                  = false;

    this->width *= 2.0f;

}

//--------------------------------------------------------------
void pdspChorusEffect::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"speed");
    this->addInlet(VP_LINK_NUMERIC,"depth");
    this->addInlet(VP_LINK_NUMERIC,"delay");

    this->addOutlet(VP_LINK_AUDIO,"signal");

    this->setCustomVar(speed,"SPEED");
    this->setCustomVar(depth,"DEPTH");
    this->setCustomVar(delay,"DELAY");
}

//--------------------------------------------------------------
void pdspChorusEffect::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();
}

//--------------------------------------------------------------
void pdspChorusEffect::setupAudioOutObjectContent(pdsp::Engine &engine){

    speed_ctrl >> chorus.in_speed();
    speed_ctrl.set(speed);
    speed_ctrl.enableSmoothing(50.0f);

    depth_ctrl >> chorus.in_depth();
    depth_ctrl.set(depth);
    depth_ctrl.enableSmoothing(50.0f);

    delay_ctrl >> chorus.in_delay();
    delay_ctrl.set(delay);
    delay_ctrl.enableSmoothing(50.0f);

    this->pdspIn[0] >> chorus.ch(0);

    chorus.ch(0) >> this->pdspOut[0];

    chorus.ch(0) >> scope >> engine.blackhole();
}

//--------------------------------------------------------------
void pdspChorusEffect::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[1]){
        speed = ofClamp(*(float *)&_inletParams[1],0.0f,1.0f);
        speed_ctrl.set(speed);
    }

    if(this->inletsConnected[2]){
        depth = ofClamp(*(float *)&_inletParams[2],0.0f,1000.0f);
        depth_ctrl.set(depth);
    }

    if(this->inletsConnected[3]){
        delay = ofClamp(*(float *)&_inletParams[3],0.0f,1000.0f);
        delay_ctrl.set(delay);
    }

    if(!loaded){
        loaded = true;
        speed = ofClamp(this->getCustomVar("SPEED"),0.0f,1.0f);
        speed_ctrl.set(speed);
        depth = ofClamp(this->getCustomVar("DEPTH"),0.0f,1000.0f);
        depth_ctrl.set(depth);
        delay = ofClamp(this->getCustomVar("DELAY"),0.0f,1000.0f);
        delay_ctrl.set(delay);
    }

}

//--------------------------------------------------------------
void pdspChorusEffect::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void pdspChorusEffect::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    if(_nodeCanvas.BeginNodeMenu()){
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            ImGuiEx::ObjectInfo(
                        "Chorus loosely based on Roland Dimension C-D models",
                        "https://mosaic.d3cod3.org/reference.php?r=chorus", scaleFactor);

            ImGui::EndMenu();
        }
        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/11, IM_COL32(255,255,120,255), "speed", &speed, 0.0f, 1.0f, 100.0f)){
            speed_ctrl.set(speed);
            this->setCustomVar(speed,"SPEED");

        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(40*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/11, IM_COL32(255,255,120,255), "depth", &depth, 0.0f, 1000.0f, 1000.0f)){
            depth_ctrl.set(depth);
            this->setCustomVar(depth,"DEPTH");


        }
        ImGui::SameLine();ImGui::Dummy(ImVec2(40*scaleFactor,-1));ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/11, IM_COL32(255,255,120,255), "delay", &delay, 0.0f, 1000.0f, 1000.0f)){
            delay_ctrl.set(delay);
            this->setCustomVar(delay,"DELAY");
        }

    }

}

//--------------------------------------------------------------
void pdspChorusEffect::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspChorusEffect::loadAudioSettings(){
    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){
        if (XML.pushTag("settings")){
            sampleRate = XML.getValue("sample_rate_in",0);
            bufferSize = XML.getValue("buffer_size",0);
            XML.popTag();
        }
    }
}

//--------------------------------------------------------------
void pdspChorusEffect::audioInObject(ofSoundBuffer &inputBuffer){

}

//--------------------------------------------------------------
void pdspChorusEffect::audioOutObject(ofSoundBuffer &outputBuffer){
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}



OBJECT_REGISTER( pdspChorusEffect, "dimension chorus", OFXVP_OBJECT_CAT_SOUND)

#endif
