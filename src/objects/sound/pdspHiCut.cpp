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

#include "pdspHiCut.h"

//--------------------------------------------------------------
pdspHiCut::pdspHiCut() : PatchObject("low pass"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();  // audio input
    _inletParams[1] = new float();          // cut frequency
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output

    this->initInletsState();

    freqINFO                = new ofImage();
    posX = posY = drawW = drawH = 0.0f;

    this->setIsTextureObj(true);

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    freq                    = 19000.0f;

    loaded                  = false;

    this->width *= 2;

}

//--------------------------------------------------------------
void pdspHiCut::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"freq");

    this->addOutlet(VP_LINK_AUDIO,"filteredSignal");

    this->setCustomVar(freq,"FREQUENCY");
}

//--------------------------------------------------------------
void pdspHiCut::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadAudioSettings();

    freqINFO->load("images/freq_graph.png");
}

//--------------------------------------------------------------
void pdspHiCut::setupAudioOutObjectContent(pdsp::Engine &engine){
    freq_ctrl >> filter.in_freq();
    freq_ctrl.set(freq);
    freq_ctrl.enableSmoothing(50.0f);

    this->pdspIn[0] >> filter >> this->pdspOut[0];
    this->pdspIn[0] >> filter >> scope >> engine.blackhole();
}

//--------------------------------------------------------------
void pdspHiCut::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(this->inletsConnected[1]){
        freq = ofClamp(*(float *)&_inletParams[1],20.0f,20000.0f);
        freq_ctrl.set(freq);
    }

    if(!loaded){
        loaded = true;
        freq = ofClamp(this->getCustomVar("FREQUENCY"),20.0f,20000.0f);
        freq_ctrl.set(freq);
    }

}

//--------------------------------------------------------------
void pdspHiCut::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);
    // draw node texture preview with OF
    ofSetColor(255);
    if(scaledObjW*canvasZoom > 90.0f){
        drawNodeOFTexture(freqINFO->getTexture(), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
    }
}

//--------------------------------------------------------------
void pdspHiCut::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){

        if (ImGui::BeginMenu("CONFIG"))
        {

            drawObjectNodeConfig(); this->configMenuWidth = ImGui::GetWindowWidth();

            ImGui::EndMenu();
        }
        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 window_size = ImGui::GetWindowSize();
        float pinDistance = (window_size.y-((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor))/this->numInlets;

        _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(window_pos.x ,window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2)),ImVec2(window_pos.x + ofMap(freq,20.0f,20000.0f,20*scaleFactor,window_size.x-(20*scaleFactor)), window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2)),IM_COL32(255,255,120,160),2.0f);
        _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(window_pos.x + ofMap(freq,20.0f,20000.0f,20*scaleFactor,window_size.x-(20*scaleFactor)),window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (pinDistance/2)),ImVec2(window_pos.x + window_size.x,window_pos.y + window_size.y),IM_COL32(255,255,120,160),2.0f);

        // get imgui node translated/scaled position/dimension for drawing textures in OF
        objOriginX = (ImGui::GetWindowPos().x + ((IMGUI_EX_NODE_PINS_WIDTH_NORMAL - 1)*this->scaleFactor) - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale();
        objOriginY = (ImGui::GetWindowPos().y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale();
        scaledObjW = this->width - ((IMGUI_EX_NODE_PINS_WIDTH_NORMAL+IMGUI_EX_NODE_PINS_WIDTH_SMALL-2)*this->scaleFactor/_nodeCanvas.GetCanvasScale());
        scaledObjH = this->height - ((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor/_nodeCanvas.GetCanvasScale());

        _nodeCanvas.EndNodeContent();
    }

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();

}

//--------------------------------------------------------------
void pdspHiCut::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(ImGui::SliderFloat("Frequency",&freq,20.0f,20000.0f)){
        freq_ctrl.set(freq);
        this->setCustomVar(freq,"FREQUENCY");
    }
    ImGui::Spacing();

    ImGuiEx::ObjectInfo(
                "12 dB Low Pass (aka High Cut filter). Non-resonant.",
                "https://mosaic.d3cod3.org/reference.php?r=low-pass", scaleFactor);
}

//--------------------------------------------------------------
void pdspHiCut::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspHiCut::loadAudioSettings(){
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
void pdspHiCut::audioInObject(ofSoundBuffer &inputBuffer){
    unusedArgs(inputBuffer);
}

//--------------------------------------------------------------
void pdspHiCut::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);
    // SIGNAL BUFFER
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}



OBJECT_REGISTER( pdspHiCut, "low pass", OFXVP_OBJECT_CAT_SOUND)

#endif
