    /*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2025 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "AChaos.h"

//--------------------------------------------------------------
AChaos::AChaos() : PatchObject("strange attractors"){

    this->numInlets  = 3;
    this->numOutlets = 2;

    _inletParams[0] = new float();  // X
    _inletParams[1] = new float();  // Y
    _inletParams[2] = new float();  // VOLUME
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]) = 0.0f;
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]) = 0.0f;
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[2]) = 0.0f;

    _outletParams[0] = new ofSoundBuffer();
    _outletParams[1] = new ofTexture();  // texture

    output = new float[6];
    output[0] = 0.0f;
    output[1] = 0.0f;
    output[2] = 0.0f;
    output[3] = 0.0f;
    output[4] = 0.0f;
    output[5] = 0.0f;

    chaosFbo    = new ofFbo();

    chaos = new AChaosBase();
    visualizer = new AChaosVisualizer();

    cNle            = 0;
    newcNle         = 0;

    this->initInletsState();

    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    this->setIsResizable(true);
    this->setIsTextureObj(true);

    volume = 0.57f;
    changeNLE = false;

    posX = posY = drawW = drawH = 0.0f;

    prevW                   = this->width;
    prevH                   = this->height;

    loaded                  = false;

    _x = 0.5f;
    _y = 0.5f;

}

//--------------------------------------------------------------
void AChaos::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"x");
    this->addInlet(VP_LINK_NUMERIC,"y");
    this->addInlet(VP_LINK_NUMERIC,"amp");
    this->addOutlet(VP_LINK_AUDIO,"chaosSignal");
    this->addOutlet(VP_LINK_TEXTURE,"chaosTexture");

    this->setCustomVar(volume,"VOLUME");
    this->setCustomVar(static_cast<float>(cNle),"CHAOS_NLE");

    this->setCustomVar(static_cast<float>(prevW),"WIDTH");
    this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
}

//--------------------------------------------------------------
void AChaos::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadAudioSettings();

    cNleVector.push_back("Clifford");
    cNleVector.push_back("Duffing");
    cNleVector.push_back("Ginger");
    cNleVector.push_back("Ikeda");
    cNleVector.push_back("Jong");
    cNleVector.push_back("Lorenz");
    cNleVector.push_back("NavierStokes");

    ofDisableArbTex();
    chaosFbo = new ofFbo();
    chaosFbo->allocate(STANDARD_TEXTURE_WIDTH, STANDARD_TEXTURE_HEIGHT, GL_RGB, 1);
    ofEnableArbTex();

    p.assign(7,0.0f);

    startTime = ofGetElapsedTimeMillis();

    loaded                  = false;

}

//--------------------------------------------------------------
void AChaos::setupAudioOutObjectContent(pdsp::Engine &engine){
    signalOUT.out_signal() >> this->pdspOut[0];
    signalOUT.out_signal() >> scope >> engine.blackhole();
}

//--------------------------------------------------------------
void AChaos::updateObjectContent(std::map<int,std::shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(this->inletsConnected[0]){
        _x = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]),0.0f,1.0f);
    }

    if(this->inletsConnected[1]){
        _y = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]),0.0f,1.0f);
    }

    if(this->inletsConnected[2]){
        volume = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[2]),0.0f,1.0f);
    }

    if(!loaded){
        loaded = true;
        cNle = newcNle = this->getCustomVar("CHAOS_NLE");
        prevW = this->getCustomVar("WIDTH");
        prevH = this->getCustomVar("HEIGHT");
        this->width             = prevW;
        this->height            = prevH;

        setChaosNLE();
    }

    if(output==NULL) return;

    p.clear();
    switch (cNle) {
    case 0:
        p = {output[0], output[1],ofMap(_x,0.0,1.0,0.75, 1.1), ofMap(_y,0.0,1.0,0.75, 1.1),0.4,7.7}; // CLIFFORD
        break;
    case 1:
        p = {ofClamp(output[0],-0.7f,0.7f), ofClamp(output[1],-0.7f,0.7f),ofMap(_x,0.0,1.0,0.1, 0.5), ofMap(_y,0.0,1.0,0.1, 0.5),0.4,7.7,0.0}; // DUFFING
        break;
    case 2:
        p = {ofClamp(output[0],-0.7f,0.7f), ofClamp(output[1],-0.7f,0.7f),ofMap(_x,0.0,1.0,0.01, 0.3)}; // GINGER
        break;
    case 3:
        p = {output[0], output[1],ofMap(_x,0.0,1.0,0.75, 1.1), ofMap(_y,0.0,1.0,0.75, 1.1),0.4,7.7}; // IKEDA
        break;
    case 4:
        p = {output[0], output[1],ofMap(_x,0.0,1.0,0.75, 1.1), ofMap(_y,0.0,1.0,0.75, 1.1),0.4,7.7}; // JONG
        break;
    case 5:
        p = {10.0,28.0,ofClamp(output[0],0.01,3.0), ofClamp(output[1],0.01,1.0),0.1,0.1,0.01}; // LORENZ
        break;
    case 6:
        p = {output[0], output[1],_x,_y,1.0,ofMap(_x,0.0,1.0,10.0,28.0),ofMap(_y,0.0,1.0,0.001,0.015)}; // NAVIERSTOKES
        break;
    default:
        break;

    }
    chaos->setVector(p);

    if(changeNLE){
        changeNLE = false;
        cNle = newcNle;
        setChaosNLE();
        this->setCustomVar(static_cast<float>(cNle),"CHAOS_NLE");
    }

}

//--------------------------------------------------------------
void AChaos::drawObjectContent(ofTrueTypeFont *font, std::shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

    if(chaosFbo->isAllocated()){
        chaosFbo->begin();
        ofClear(0,0,0,255);
        ofSetColor(255);
        visualizer->draw(STANDARD_TEXTURE_WIDTH, STANDARD_TEXTURE_HEIGHT);
        chaosFbo->end();

        *ofxVP_CAST_PIN_PTR<ofTexture>(_outletParams[1]) = chaosFbo->getTexture();
    }

}

//--------------------------------------------------------------
void AChaos::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImVec2 window_pos = ImGui::GetWindowPos()+ImVec2(IMGUI_EX_NODE_PINS_WIDTH_NORMAL, IMGUI_EX_NODE_HEADER_HEIGHT);
        _nodeCanvas.getNodeDrawList()->AddRectFilled(window_pos,window_pos+ImVec2(scaledObjW*this->scaleFactor*_nodeCanvas.GetCanvasScale(), scaledObjH*this->scaleFactor*_nodeCanvas.GetCanvasScale()),ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)));
        if(ofxVP_CAST_PIN_PTR<ofTexture>(_outletParams[1])->isAllocated()){
            calcTextureDims(*ofxVP_CAST_PIN_PTR<ofTexture>(_outletParams[1]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
            ImGui::SetCursorPos(ImVec2(posX+(IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor), posY+(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor)));
            ImGui::Image((ImTextureID)(uintptr_t)ofxVP_CAST_PIN_PTR<ofTexture>(_outletParams[1])->getTextureData().textureID, ImVec2(drawW, drawH));
        }

        // get imgui node translated/scaled position/dimension for drawing textures in OF
        //objOriginX = (ImGui::GetWindowPos().x + ((IMGUI_EX_NODE_PINS_WIDTH_NORMAL - 1)*this->scaleFactor) - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale();
        //objOriginY = (ImGui::GetWindowPos().y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale();
        scaledObjW = this->width - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL+IMGUI_EX_NODE_PINS_WIDTH_SMALL)*this->scaleFactor/_nodeCanvas.GetCanvasScale();
        scaledObjH = this->height - (IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor/_nodeCanvas.GetCanvasScale();

        if(this->width != prevW){
            prevW = this->width;
            this->setCustomVar(static_cast<float>(prevW),"WIDTH");
        }
        if(this->height != prevH){
            prevH = this->height;
            this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
        }

        _nodeCanvas.EndNodeContent();
    }

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();

}

//--------------------------------------------------------------
void AChaos::drawObjectNodeConfig(){
    ImGui::Spacing();

    if(ImGui::BeginCombo("Chaos NLE", cNleVector.at(newcNle).c_str() )){
        for(int i=0; i < cNleVector.size(); ++i){
            bool is_selected = (newcNle == i );
            if (ImGui::Selectable(cNleVector.at(i).c_str(), is_selected)){
                newcNle = i;
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    if(ImGui::Button("APPLY",ImVec2(224*scaleFactor,26*scaleFactor))){
        changeNLE = true;
    }

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    if(ImGui::SliderFloat("amp",&volume,0.0f,1.0f)){
        this->setCustomVar(volume,"VOLUME");
    }

    ImGuiEx::ObjectInfo(
                "A-Chaos Lib is a systematic approach to emulate the use of up-to-date known strange attractors non-linear equations by Andre Sier.\n\nhttp://s373.net/code/A-Chaos-Lib/A-Chaos.html",
                "https://mosaic.d3cod3.org/reference.php?r=strange-attractors", scaleFactor);
}

//--------------------------------------------------------------
void AChaos::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);

    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void AChaos::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);

    if(ofGetElapsedTimeMillis()-startTime > 1000){
        for(size_t i = 0; i < monoBuffer.getNumFrames(); i++) {

            if(chaos != nullptr) output = chaos->update();

            if(visualizer != nullptr){
                visualizer->update(output);

                monoBuffer.getSample(i,0) = ofMap(output[0], visualizer->channels[0].min, visualizer->channels[0].max, -1, 1) * volume;
                monoBuffer.getSample(i,1) = ofMap(output[1], visualizer->channels[1].min, visualizer->channels[1].max, -1, 1) * volume;
            }

        }

        lastBuffer = monoBuffer;


        signalOUT.copyInput(lastBuffer.getBuffer().data(),lastBuffer.getNumFrames());
        *ofxVP_CAST_PIN_PTR<ofSoundBuffer>(_outletParams[0]) = lastBuffer;
    }

}

//--------------------------------------------------------------
void AChaos::loadAudioSettings(){
    ofxVPXml.loadMosaicPatch(this->patchFile);

    sampleRate = this->ofxVPXml.getMosaicConfigInt("sample_rate_in");
    bufferSize = this->ofxVPXml.getMosaicConfigInt("buffer_size");

    shortBuffer = new short[bufferSize];
    for (int i = 0; i < bufferSize; i++){
        shortBuffer[i] = 0;
    }

    ofSoundBuffer tmpBuffer(shortBuffer,static_cast<size_t>(bufferSize),1,static_cast<unsigned int>(sampleRate));
    monoBuffer = tmpBuffer;

}

//--------------------------------------------------------------
void AChaos::setChaosNLE(){

    output[0] = 0.0f;
    output[1] = 0.0f;
    output[2] = 0.0f;
    output[3] = 0.0f;
    output[4] = 0.0f;
    output[5] = 0.0f;

    chaos = new AChaosBase();
    visualizer = new AChaosVisualizer();

    if(cNle == 0){
        AChaosClifford *temp = new AChaosClifford();
        temp->setup();
        chaos = temp;
    }else if(cNle == 1){
        AChaosDuffing *temp = new AChaosDuffing();
        temp->setup();
        chaos = temp;
    }else if(cNle == 2){
        AChaosGinger *temp = new AChaosGinger();
        temp->setup();
        chaos = temp;
    }else if(cNle == 3){
        AChaosIkeda *temp = new AChaosIkeda();
        temp->setup();
        chaos = temp;
    }else if(cNle == 4){
        AChaosJong *temp = new AChaosJong();
        temp->setup();
        chaos = temp;
    }else if(cNle == 5){
        AChaosLorenz *temp = new AChaosLorenz();
        temp->setup();
        chaos = temp;
    }else if(cNle == 6){
        AChaosNavierStokes *temp = new AChaosNavierStokes();
        temp->setup();
        chaos = temp;
    }
    visualizer->setup(chaos,1024);
}

OBJECT_REGISTER( AChaos, "strange attractors", OFXVP_OBJECT_CAT_GENERATIVE)

#endif
