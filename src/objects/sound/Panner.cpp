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

#include "Panner.h"

//--------------------------------------------------------------
Panner::Panner() : PatchObject("panner"){

    this->numInlets  = 2;
    this->numOutlets = 2;

    _inletParams[0] = new ofSoundBuffer();  // audio input
    _inletParams[1] = new float();          // gain
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output L
    _outletParams[1] = new ofSoundBuffer(); // audio output R

    this->initInletsState();

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    pan                     = 0.0f;

    loaded                  = false;

}

//--------------------------------------------------------------
void Panner::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"pan");

    this->addOutlet(VP_LINK_AUDIO,"left");
    this->addOutlet(VP_LINK_AUDIO,"right");

    this->setCustomVar(pan,"PAN");
}

//--------------------------------------------------------------
void Panner::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadAudioSettings();
}

//--------------------------------------------------------------
void Panner::setupAudioOutObjectContent(pdsp::Engine &engine){

    pan_ctrl >> panner.in_pan();
    pan_ctrl.set(0.0f);
    pan_ctrl.enableSmoothing(50.0f);

    this->pdspIn[0] >> panner.in_signal();

    panner.out_L() >> this->pdspOut[0];
    panner.out_R() >> this->pdspOut[1];

    panner.out_L() >> scopeL >> engine.blackhole();
    panner.out_R() >> scopeR >> engine.blackhole();
}

//--------------------------------------------------------------
void Panner::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(this->inletsConnected[1]){
        pan = ofClamp(*(float *)&_inletParams[1],-1.0f,1.0f);
        pan_ctrl.set(pan);
    }

    if(!loaded){
        loaded = true;
        pan = this->getCustomVar("PAN");
        pan_ctrl.set(ofClamp(pan,-1.0f,1.0f));
    }

}

//--------------------------------------------------------------
void Panner::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

    ofSetColor(255);
}

//--------------------------------------------------------------
void Panner::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGui::Dummy(ImVec2(-1,ImGui::GetWindowSize().y/2 - (26*scaleFactor))); // Padding top
        ImGui::PushItemWidth(-1);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(255,255,120,30));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(255,255,120,60));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(255,255,120,60));
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(255,255,120,160));
        if(ImGui::SliderFloat("",&pan,-1.0f, 1.0f)){
            pan_ctrl.set(pan);
            this->setCustomVar(pan,"PAN");
        }
        ImGui::PopStyleColor(4);
        ImGui::PopItemWidth();

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void Panner::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Basic stereo panner",
                "https://mosaic.d3cod3.org/reference.php?r=panner", scaleFactor);
}

//--------------------------------------------------------------
void Panner::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);

    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void Panner::loadAudioSettings(){
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
void Panner::audioInObject(ofSoundBuffer &inputBuffer){
    unusedArgs(inputBuffer);
}

//--------------------------------------------------------------
void Panner::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);

    // STEREO SIGNAL BUFFERS
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scopeL.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[1])->copyFrom(scopeR.getBuffer().data(), bufferSize, 1, sampleRate);
}


OBJECT_REGISTER( Panner, "panner", OFXVP_OBJECT_CAT_SOUND)

#endif
