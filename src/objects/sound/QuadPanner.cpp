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

#include "QuadPanner.h"

//--------------------------------------------------------------
QuadPanner::QuadPanner() : PatchObject("quad panner"){

    this->numInlets  = 3;
    this->numOutlets = 4;

    _inletParams[0] = new ofSoundBuffer();  // audio input
    _inletParams[1] = new float();          // pan X
    _inletParams[2] = new float();          // pan Y
    *(float *)&_inletParams[1] = 0.0f;
    *(float *)&_inletParams[2] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output 1
    _outletParams[1] = new ofSoundBuffer(); // audio output 2
    _outletParams[2] = new ofSoundBuffer(); // audio output 3
    _outletParams[3] = new ofSoundBuffer(); // audio output 4

    this->initInletsState();

    padX                    = 0.5f;
    padY                    = 0.5f;

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    loaded                  = false;

}

//--------------------------------------------------------------
void QuadPanner::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"pan X");
    this->addInlet(VP_LINK_NUMERIC,"pan Y");

    this->addOutlet(VP_LINK_AUDIO,"channel1");
    this->addOutlet(VP_LINK_AUDIO,"channel2");
    this->addOutlet(VP_LINK_AUDIO,"channel3");
    this->addOutlet(VP_LINK_AUDIO,"channel4");

    this->setCustomVar(padX,"XPOS");
    this->setCustomVar(padY,"YPOS");
}

//--------------------------------------------------------------
void QuadPanner::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();
}

//--------------------------------------------------------------
void QuadPanner::setupAudioOutObjectContent(pdsp::Engine &engine){

    gain_ctrl1 >> gain1.in_mod();
    gain_ctrl1.set(0.0f);
    gain_ctrl1.enableSmoothing(50.0f);

    gain_ctrl2 >> gain2.in_mod();
    gain_ctrl2.set(0.0f);
    gain_ctrl2.enableSmoothing(50.0f);

    gain_ctrl3 >> gain3.in_mod();
    gain_ctrl3.set(0.0f);
    gain_ctrl3.enableSmoothing(50.0f);

    gain_ctrl4 >> gain4.in_mod();
    gain_ctrl4.set(0.0f);
    gain_ctrl4.enableSmoothing(50.0f);

    this->pdspIn[0] >> gain1 >> this->pdspOut[0];
    this->pdspIn[0] >> gain2 >> this->pdspOut[1];
    this->pdspIn[0] >> gain3 >> this->pdspOut[2];
    this->pdspIn[0] >> gain4 >> this->pdspOut[3];

    this->pdspIn[0] >> gain1 >> scope1 >> engine.blackhole();
    this->pdspIn[0] >> gain2 >> scope2 >> engine.blackhole();
    this->pdspIn[0] >> gain3 >> scope3 >> engine.blackhole();
    this->pdspIn[0] >> gain4 >> scope4 >> engine.blackhole();
}

//--------------------------------------------------------------
void QuadPanner::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[1]){
        padX    = ofClamp(*(float *)&_inletParams[1],0.0f,1.0f);
    }
    if(this->inletsConnected[2]){
        padY    = ofClamp(*(float *)&_inletParams[2],0.0f,1.0f);
    }

    gain_ctrl1.set(ofClamp(ofMap(padY,0.0,1.0,1.0,0.0),0.0f,1.0f) * ofClamp(ofMap(padX,0.0,1.0,1.0,0.0),0.0f,1.0f));
    gain_ctrl2.set(ofClamp(ofMap(padY,0.0,1.0,1.0,0.0),0.0f,1.0f) * padX);
    gain_ctrl3.set(ofClamp(padY,0.0f,1.0f) * padX);
    gain_ctrl4.set(ofClamp(padY,0.0f,1.0f) * ofClamp(ofMap(padX,0.0,1.0,1.0,0.0),0.0f,1.0f));

    if(!loaded){
        loaded = true;
        padX   = this->getCustomVar("XPOS");
        padY   = this->getCustomVar("YPOS");
        gain_ctrl1.set(ofClamp(ofMap(this->getCustomVar("YPOS"),0.0,1.0,1.0,0.0),0.0f,1.0f) * ofClamp(ofMap(this->getCustomVar("XPOS"),0.0,1.0,1.0,0.0),0.0f,1.0f));
        gain_ctrl2.set(ofClamp(ofMap(this->getCustomVar("YPOS"),0.0,1.0,1.0,0.0),0.0f,1.0f) * this->getCustomVar("XPOS"));
        gain_ctrl3.set(ofClamp(this->getCustomVar("YPOS"),0.0f,1.0f) * this->getCustomVar("XPOS"));
        gain_ctrl4.set(ofClamp(this->getCustomVar("YPOS"),0.0f,1.0f) * ofClamp(ofMap(this->getCustomVar("XPOS"),0.0,1.0,1.0,0.0),0.0f,1.0f));
    }

}

//--------------------------------------------------------------
void QuadPanner::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void QuadPanner::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        if(ImGuiEx::Pad2D(_nodeCanvas.getNodeDrawList(), 0, ImGui::GetWindowSize().y - 26,&padX,&padY)){
            this->setCustomVar(padX,"XPOS");
            this->setCustomVar(padY,"YPOS");
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void QuadPanner::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "A quadraphonic panning controlled by 2D pad interface. Channels order is clockwise starting from top left (ch 1), top right (ch 2), bottom right (ch 3) and bottom left (ch 4).",
                "https://mosaic.d3cod3.org/reference.php?r=quad-panner", scaleFactor);
}

//--------------------------------------------------------------
void QuadPanner::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void QuadPanner::loadAudioSettings(){
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
void QuadPanner::audioInObject(ofSoundBuffer &inputBuffer){

}

//--------------------------------------------------------------
void QuadPanner::audioOutObject(ofSoundBuffer &outputBuffer){
    // QUAD SIGNAL BUFFERS
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope1.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[1])->copyFrom(scope2.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[2])->copyFrom(scope3.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[3])->copyFrom(scope4.getBuffer().data(), bufferSize, 1, sampleRate);
}


OBJECT_REGISTER( QuadPanner, "quad panner", OFXVP_OBJECT_CAT_SOUND)

#endif
