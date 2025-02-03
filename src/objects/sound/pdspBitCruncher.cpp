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

#include "pdspBitCruncher.h"

//--------------------------------------------------------------
pdspBitCruncher::pdspBitCruncher() : PatchObject("bit cruncher"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();  // audio in
    _inletParams[1] = new float();          // bits
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output

    this->initInletsState();

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    bits                    = 4.0f;

    loaded                  = false;

}

//--------------------------------------------------------------
void pdspBitCruncher::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"in");
    this->addInlet(VP_LINK_NUMERIC,"bits");

    this->addOutlet(VP_LINK_AUDIO,"out");

    this->setCustomVar(bits,"BITS");
}

//--------------------------------------------------------------
void pdspBitCruncher::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadAudioSettings();
}

//--------------------------------------------------------------
void pdspBitCruncher::setupAudioOutObjectContent(pdsp::Engine &engine){

    bits_ctrl >> bitcruncher.in_bits();
    bits_ctrl.set(4.0f);
    bits_ctrl.enableSmoothing(50.0f);

    this->pdspIn[0] >> bitcruncher.in_signal();

    bitcruncher.out_signal() >> this->pdspOut[0];

    bitcruncher >> scope >> engine.blackhole();
}

//--------------------------------------------------------------
void pdspBitCruncher::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[1]){
        bits = ofClamp(*(float *)&_inletParams[1],1.0f,8.0f);
        bits_ctrl.set(bits);
    }

    if(!loaded){
        loaded = true;
        bits_ctrl.set(ofClamp(this->getCustomVar("BITS"),1.0f,8.0f));
    }

}

//--------------------------------------------------------------
void pdspBitCruncher::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);
}

//--------------------------------------------------------------
void pdspBitCruncher::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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
        if(ImGui::SliderFloat("",&bits,1.0f, 8.0f)){
            bits_ctrl.set(bits);
            this->setCustomVar(bits,"BITS");
        }
        ImGui::PopStyleColor(4);
        ImGui::PopItemWidth();

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void pdspBitCruncher::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Lo-Fi audio effect, reducing the bit for the signal amplitude.",
                "https://mosaic.d3cod3.org/reference.php?r=bit-cruncher", scaleFactor);
}

//--------------------------------------------------------------
void pdspBitCruncher::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspBitCruncher::loadAudioSettings(){
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
void pdspBitCruncher::audioInObject(ofSoundBuffer &inputBuffer){

}

//--------------------------------------------------------------
void pdspBitCruncher::audioOutObject(ofSoundBuffer &outputBuffer){
    // STEREO SIGNAL BUFFERS
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}



OBJECT_REGISTER( pdspBitCruncher, "bit cruncher", OFXVP_OBJECT_CAT_SOUND)

#endif
