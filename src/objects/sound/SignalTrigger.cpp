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

#include "SignalTrigger.h"

//--------------------------------------------------------------
SignalTrigger::SignalTrigger() : PatchObject("signal trigger"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();  // audio in
    _inletParams[1] = new float();          // threshold
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new float();         // signal trigger --> bang
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    thresh                  = 0.5f;

    loaded                  = false;

    bang                    = false;

}

//--------------------------------------------------------------
void SignalTrigger::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"thresh");

    this->addOutlet(VP_LINK_NUMERIC,"bang");

    this->setCustomVar(thresh,"THRESHOLD");
}

//--------------------------------------------------------------
void SignalTrigger::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadAudioSettings();

    pressColor = { 250/255.0f, 250/255.0f, 5/255.0f, 1.0f };
    releaseColor = { 0.f, 0.f, 0.f, 0.f };

    currentColor = releaseColor;
}

//--------------------------------------------------------------
void SignalTrigger::setupAudioOutObjectContent(pdsp::Engine &engine){

    thresh_ctrl >> toTrigger.in_threshold();
    thresh_ctrl.set(thresh);
    thresh_ctrl.enableSmoothing(10.0f);

    this->pdspIn[0] >> peakDetector >> follower.out_signal() >> toTrigger.out_trig() >> engine.blackhole();

}

//--------------------------------------------------------------
void SignalTrigger::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[1]){
        thresh = ofClamp(*(float *)&_inletParams[1],0.0f,1.0f);
        thresh_ctrl.set(thresh);
    }

    if(toTrigger.meter_gate() > 0.0f){
        // trigger on
        *(float *)&_outletParams[0] = 1.0f;
        bang = true;
    }else{
        // trigger off
        *(float *)&_outletParams[0] = 0.0f;
        bang = false;
    }

    if(!loaded){
        loaded = true;
        thresh = ofClamp(this->getCustomVar("THRESHOLD"),0.0f,1.0f);
        thresh_ctrl.set(thresh);
    }

}

//--------------------------------------------------------------
void SignalTrigger::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);
}

//--------------------------------------------------------------
void SignalTrigger::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        // BANG (PD Style) button
        ImGuiEx::BangButton("", currentColor, ImVec2(ImGui::GetWindowSize().x,ImGui::GetWindowSize().y));

        if (bang){
            currentColor = pressColor;
        }else{
            currentColor = releaseColor;
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void SignalTrigger::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(ImGui::SliderFloat("Threshold",&thresh,0.0f,1.0f)){
        thresh_ctrl.set(thresh);
        this->setCustomVar(thresh,"THRESHOLD");
    }

    ImGuiEx::ObjectInfo(
                "Triggers a bang when the sound signal level exceeds the threshold.",
                "https://mosaic.d3cod3.org/reference.php?r=signal-trigger", scaleFactor);
}

//--------------------------------------------------------------
void SignalTrigger::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void SignalTrigger::loadAudioSettings(){
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
void SignalTrigger::audioInObject(ofSoundBuffer &inputBuffer){

}

//--------------------------------------------------------------
void SignalTrigger::audioOutObject(ofSoundBuffer &outputBuffer){

}


OBJECT_REGISTER( SignalTrigger, "signal trigger", OFXVP_OBJECT_CAT_SOUND)

#endif
