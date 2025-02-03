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

#include "Crossfader.h"

//--------------------------------------------------------------
Crossfader::Crossfader() : PatchObject("crossfader"){

    this->numInlets  = 3;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();  // audio in 1
    _inletParams[1] = new ofSoundBuffer();  // audio in 2
    _inletParams[2] = new float();          // fade
    *(float *)&_inletParams[2] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output L

    this->initInletsState();

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    fade_value              = 0.0f;

    loaded                  = false;

    this->height            /= 2;

}

//--------------------------------------------------------------
void Crossfader::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"in 1");
    this->addInlet(VP_LINK_AUDIO,"in 2");
    this->addInlet(VP_LINK_NUMERIC,"fade");

    this->addOutlet(VP_LINK_AUDIO,"out");

    this->setCustomVar(fade_value,"FADE");
}

//--------------------------------------------------------------
void Crossfader::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadAudioSettings();

}

//--------------------------------------------------------------
void Crossfader::setupAudioOutObjectContent(pdsp::Engine &engine){

    fade_ctrl >> crossfader.in_fade();
    fade_ctrl.set(fade_value);
    fade_ctrl.enableSmoothing(50.0f);

    this->pdspIn[0] >> crossfader.in_A();
    this->pdspIn[1] >> crossfader.in_B();

    crossfader >> this->pdspOut[0];

    crossfader >> scope >> engine.blackhole();
}

//--------------------------------------------------------------
void Crossfader::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(this->inletsConnected[2]){
        fade_value = ofClamp(*(float *)&_inletParams[2],0.0f,1.0f);
    }

    if(!loaded){
        loaded = true;
        fade_value = this->getCustomVar("FADE");
    }

    fade_ctrl.set(ofClamp(fade_value,0.0f,1.0f));

}

//--------------------------------------------------------------
void Crossfader::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

    ofSetColor(255);
}

//--------------------------------------------------------------
void Crossfader::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGui::SetCursorPos(ImVec2(IMGUI_EX_NODE_PINS_WIDTH_NORMAL+(4*scaleFactor), (this->height/2 *_nodeCanvas.GetCanvasScale()) - (6*scaleFactor)));
        ImGui::PushItemWidth(-1);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(255,255,120,30));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(255,255,120,60));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(255,255,120,60));
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(255,255,120,160));
        ImGui::SliderFloat("",&fade_value,0.0f, 1.0f);
        ImGui::PopStyleColor(4);
        ImGui::PopItemWidth();

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void Crossfader::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "A basic linear crossfader",
                "https://mosaic.d3cod3.org/reference.php?r=crossfader", scaleFactor);
}

//--------------------------------------------------------------
void Crossfader::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);

    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void Crossfader::loadAudioSettings(){
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
void Crossfader::audioInObject(ofSoundBuffer &inputBuffer){
    unusedArgs(inputBuffer);
}

//--------------------------------------------------------------
void Crossfader::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);

    // STEREO SIGNAL BUFFERS
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}

OBJECT_REGISTER( Crossfader, "crossfader", OFXVP_OBJECT_CAT_SOUND)

#endif
