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

#include "pdspAHR.h"

//--------------------------------------------------------------
pdspAHR::pdspAHR() : PatchObject("AHR envelope"){

    this->numInlets  = 5;
    this->numOutlets = 2;

    _inletParams[0] = new ofSoundBuffer(); // audio input

    _inletParams[1] = new float();          // bang
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]) = 0.0f;
    _inletParams[2] = new float();          // A
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[2]) = 0.0f;
    _inletParams[3] = new float();          // H
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[3]) = 0.0f;
    _inletParams[4] = new float();          // R
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[4]) = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output
    _outletParams[1] = new float();         // AHR func
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[1]) = 0.0f;

    this->initInletsState();

    this->width *= 1.4;
    this->height *= 1.7f;

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    loaded                  = false;

    attackDuration          = 0.0f;
    holdDuration            = 50.0f;
    releaseDuration         = 50.0f;

    attackHardness          = 0.0f;
    releaseHardness         = 1.0f;

}

//--------------------------------------------------------------
void pdspAHR::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"A");
    this->addInlet(VP_LINK_NUMERIC,"H");
    this->addInlet(VP_LINK_NUMERIC,"R");
    this->addOutlet(VP_LINK_AUDIO,"envelopedSignal");
    this->addOutlet(VP_LINK_NUMERIC,"envelope");

    this->setCustomVar(attackHardness,"ATTACK_CURVE");
    this->setCustomVar(releaseHardness,"RELEASE_CURVE");

    this->setCustomVar(attackDuration,"ATTACK");
    this->setCustomVar(holdDuration,"HOLD");
    this->setCustomVar(releaseDuration,"RELEASE");
}

//--------------------------------------------------------------
void pdspAHR::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadAudioSettings();

}

//--------------------------------------------------------------
void pdspAHR::setupAudioOutObjectContent(pdsp::Engine &engine){

    gate_ctrl.out_trig() >> env;
    env >> amp.in_mod();

    this->pdspIn[0] >> amp >> this->pdspOut[0];
    this->pdspIn[0] >> amp >> scope >> engine.blackhole();

}

//--------------------------------------------------------------
void pdspAHR::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    // A
    if(this->inletsConnected[2]){
        attackDuration = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[2]),0.0f,std::numeric_limits<float>::max());
        this->setCustomVar(attackDuration,"ATTACK");
    }

    // H
    if(this->inletsConnected[3]){
        holdDuration = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[3]),0.0f,std::numeric_limits<float>::max());
        this->setCustomVar(holdDuration,"HOLD");
    }

    // R
    if(this->inletsConnected[4]){
        releaseDuration = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[4]),0.0f,std::numeric_limits<float>::max());
        this->setCustomVar(releaseDuration,"RELEASE");
    }


    if(!loaded){
        loaded = true;
        attackDuration = this->getCustomVar("ATTACK");
        holdDuration = this->getCustomVar("HOLD");
        releaseDuration = this->getCustomVar("RELEASE");
        attackHardness = this->getCustomVar("ATTACK_CURVE");
        releaseHardness = this->getCustomVar("RELEASE_CURVE");
    }
}

//--------------------------------------------------------------
void pdspAHR::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

    ofSetColor(255);

}

//--------------------------------------------------------------
void pdspAHR::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*2*scaleFactor));
        ImGuiEx::EnvelopeEditor(_nodeCanvas.getNodeDrawList(), 0, ImGui::GetWindowSize().y*0.3, &attackDuration, &holdDuration, &releaseDuration, &releaseDuration, ImGuiEnvelopeEditorType_AHR);


        if(ImGuiKnobs::Knob("A", &attackDuration, 0.0f, 1000.0f, 1.0f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(attackDuration,"ATTACK");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("H", &holdDuration, 0.0f, 1000.0f, 1.0f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(holdDuration,"HOLD");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("R", &releaseDuration, 0.0f, 1000.0f, 1.0f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(releaseDuration,"RELEASE");
        }

        _nodeCanvas.EndNodeContent();

    }

}

//--------------------------------------------------------------
void pdspAHR::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Standard AHR envelope.",
                "https://mosaic.d3cod3.org/reference.php?r=ahr-envelope", scaleFactor);
}

//--------------------------------------------------------------
void pdspAHR::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);

    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspAHR::loadAudioSettings(){
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
void pdspAHR::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);

    env.set(attackDuration,holdDuration,releaseDuration);
    env.setAttackCurve(attackHardness);
    env.setReleaseCurve(releaseHardness);

    // bang --> trigger envelope
    if(this->inletsConnected[1]){
        gate_ctrl.trigger(ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]),0.0f,1.0f));
    }else{
        gate_ctrl.off();
    }

    // output envelope func
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[1]) = env.meter_output();

    // SIGNAL BUFFER
    ofxVP_CAST_PIN_PTR<ofSoundBuffer>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}

OBJECT_REGISTER( pdspAHR, "AHR envelope", OFXVP_OBJECT_CAT_SOUND)

#endif
