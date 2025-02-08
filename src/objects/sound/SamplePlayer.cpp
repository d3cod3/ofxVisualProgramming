/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2024 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "SamplePlayer.h"

//--------------------------------------------------------------
SamplePlayer::SamplePlayer() : PatchObject("sample player"){

    this->numInlets  = 4;
    this->numOutlets = 2;

    _inletParams[0] = new float();  // bang
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]) = 0.0f;
    _inletParams[1] = new float();  // pitch
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]) = 0.0f;
    _inletParams[2] = new float();  // gain
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[2]) = 0.0f;
    _inletParams[3] = new float();  // direction
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[3]) = 0.0f;

    _outletParams[0] = new ofSoundBuffer();  // signal
    _outletParams[1] = new vector<float>(); // audio buffer

    this->initInletsState();

    isAudioOUTObject    = true;

    isNewObject         = true;
    isFileLoaded        = false;

    reverse             = false;
    direction           = 1;
    gain                = 1.0f;
    pitch               = 1.0f;
    sampleRate          = 44100.0;
    bufferSize          = 256;

    lastSoundfile       = "";
    loadSoundfileFlag   = false;
    soundfileLoaded     = false;
    loadingFile         = false;
    hasTriggered        = false;

    isPDSPPatchableObject   = true;

    this->width         *= 2;

    loaded              = false;

}

//--------------------------------------------------------------
void SamplePlayer::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"pitch");
    this->addInlet(VP_LINK_NUMERIC,"gain");

    this->addOutlet(VP_LINK_AUDIO,"sampleSignal");
    this->addOutlet(VP_LINK_ARRAY,"dataBuffer");

    this->setCustomVar(static_cast<float>(pitch),"PITCH");
    this->setCustomVar(static_cast<float>(gain),"GAIN");
    this->setCustomVar(static_cast<float>(direction),"DIRECTION");

}

//--------------------------------------------------------------
void SamplePlayer::autoloadFile(string _fp){
    lastSoundfile = _fp;

    soundfileLoaded = true;
    loading = false;
    startTime = ofGetElapsedTimeMillis();
}

//--------------------------------------------------------------
void SamplePlayer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    fileDialog.setIsRetina(this->isRetina);

    loadSettings();

    loading = true;
}

//--------------------------------------------------------------
void SamplePlayer::setupAudioOutObjectContent(pdsp::Engine &engine){

    // ---- this code runs in the audio thread ----
    sseq.code = [&]() noexcept {
        if(this->inletsConnected[0]){
            if(ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]),0.0f,1.0f) == 1.0f && !hasTriggered){
                hasTriggered = true;
                trigger.trigger(1.0f);
            }else{
                hasTriggered = false;
                trigger.off();
            }
        }
    };

    pitch_ctrl >> sampler.in_pitch();
    pitch_ctrl.set(1.0f);
    pitch_ctrl.enableSmoothing(50.0f);

    gain_ctrl >> gainAmp.in_mod();
    gain_ctrl.set(gain);
    gain_ctrl.enableSmoothing(50.0f);

    direction_ctrl >> sampler.in_direction();
    direction_ctrl.set(direction);

    start_ctrl >> sampler.in_start();
    start_ctrl.set(0.0f);

    trigger >> sampler;

    sampler >> gainAmp >> this->pdspOut[0];
    sampler >> gainAmp >> scope >> engine.blackhole();

    startTime   = ofGetElapsedTimeMillis();
}

//--------------------------------------------------------------
void SamplePlayer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(soundfileLoaded && ofGetElapsedTimeMillis()-startTime > 100){
        soundfileLoaded = false;
        ofFile file (lastSoundfile);
        if (file.exists()){
            isNewObject = false;
            loadAudioFile(file.getAbsolutePath());
        }
    }

    if(loading && ofGetElapsedTimeMillis()-startTime > 500){
        if(filepath != "none"){
            isNewObject = false;
            loadAudioFile(filepath);
        }else{
            isNewObject = true;
        }
        loading = false;
    }

    if(!isFileLoaded && sampleBuffer.loaded() && sampleBuffer.fileSampleRate > 100){
        isFileLoaded = true;
        ofLog(OF_LOG_NOTICE,"-- sound file loaded: %s, Sample Rate: %s, Audiofile length: %s",filepath.c_str(), ofToString(sampleBuffer.fileSampleRate).c_str(), ofToString(sampleBuffer.length).c_str());
    }

    if(isFileLoaded && sampleBuffer.loaded()){
        // pitch
        if(this->inletsConnected[1]){
            pitch = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]),1.0f,10.0f);
            pitch_ctrl.set(pitch);
        }
        // gain
        if(this->inletsConnected[2]){
            gain = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[2]),0.0f,10.0f);
            gain_ctrl.set(gain);
        }
        // direction
        if(this->inletsConnected[3]){
            direction = static_cast<int>(ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[3]),-1.0f,1.0f));
            direction_ctrl.set(direction);
            if(direction < 0){
                reverse = true;
                start_ctrl.set(1.0);
            }else{
                reverse = false;
                start_ctrl.set(0.0f);
            }
        }
    }

    if(!loaded){
        loaded = true;

        pitch = static_cast<float>(this->getCustomVar("PITCH"));
        gain = static_cast<float>(this->getCustomVar("GAIN"));
        direction = static_cast<int>(this->getCustomVar("DIRECTION"));

        pitch_ctrl.set(pitch);
        gain_ctrl.set(gain);
        direction_ctrl.set(direction);

        if(direction < 0){
            reverse = true;
            start_ctrl.set(1.0);
        }else{
            reverse = false;
            start_ctrl.set(0.0);
        }

    }

}

//--------------------------------------------------------------
void SamplePlayer::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);
}

//--------------------------------------------------------------
void SamplePlayer::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
    loadSoundfileFlag = false;

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
        if(isFileLoaded && sampleBuffer.loaded()){
            ImVec2 window_pos = ImGui::GetWindowPos();
            ImVec2 window_size = ImVec2(this->width*_nodeCanvas.GetCanvasScale(),this->height*_nodeCanvas.GetCanvasScale());
            ImVec2 ph_pos = ImVec2(window_pos.x + (20*scaleFactor), window_pos.y + (20*scaleFactor));

            objOriginX = ImGui::GetWindowPos().x + ((IMGUI_EX_NODE_PINS_WIDTH_NORMAL - 1)*this->scaleFactor);
            objOriginY = ImGui::GetWindowPos().y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor);
            scaledObjW = window_size.x - IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor;
            scaledObjH = window_size.y - (IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor;

            // draw Audiofile Waveform plot
            _nodeCanvas.getNodeDrawList()->AddRectFilled(ImVec2(objOriginX,objOriginY),ImVec2(objOriginX+scaledObjW,objOriginY+scaledObjH),IM_COL32_BLACK);
            for( int x=objOriginX; x<objOriginX+scaledObjW; ++x ){
                int n = ofMap( x, objOriginX, objOriginX+scaledObjW, 0, sampleBuffer.length, true );
                float val = sampleBuffer.buffer[0][n];
                _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(x, objOriginY + scaledObjH/2 - (val*(scaledObjH*0.5)) ),ImVec2(x, objOriginY + scaledObjH/2 + (val*(scaledObjH*0.5))),IM_COL32(255,255,120,180), 1.0f);
            }

            // draw playhead
            float phx = ofMap( sampler.meter_position(), 0.0f, 1.0f, 1, (this->width*0.98f*_nodeCanvas.GetCanvasScale())-(31*this->scaleFactor) );
            _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(ph_pos.x + phx, ph_pos.y),ImVec2(ph_pos.x + phx, window_size.y+ph_pos.y-(26*this->scaleFactor)),IM_COL32(255, 255, 255, 160), 2.0f);

        }else if(loadingFile){
            ImGui::Text("LOADING SAMPLE...");
        }else if(!isNewObject && !sampleBuffer.loaded()){
            ImGui::Text("SAMPLE NOT FOUND!");
        }

        _nodeCanvas.EndNodeContent();
    }

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, loadSoundfileFlag, "Select an audio sample", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".wav,.mp3,.ogg,.flac,.WAV,.MP3,.OGG,.FLAC", "", scaleFactor)){
        ofFile file (fileDialog.selected_path);
        if (file.exists()){
            lastSoundfile = file.getAbsolutePath();
            soundfileLoaded= true;
            startTime = ofGetElapsedTimeMillis();
        }
    }
}

//--------------------------------------------------------------
void SamplePlayer::drawObjectNodeConfig(){
    ofFile tempFilename(filepath);

    loadSoundfileFlag   = false;

    ImGui::Spacing();
    ImGui::Text("Loaded Sample:");
    if(filepath == "none"){
        ImGui::Text("%s",filepath.c_str());
    }else{
        ImGui::Text("%s",tempFilename.getFileName().c_str());
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",tempFilename.getAbsolutePath().c_str());
        ImGui::Text("Duration (ms): %.0f",sampleBuffer.length/sampleBuffer.fileSampleRate*1000);
    }
    if(ImGui::Button(ICON_FA_FILE,ImVec2(224*scaleFactor,26*scaleFactor))){
        loadSoundfileFlag = true;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button, VHS_BLUE);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, VHS_BLUE_OVER);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, VHS_BLUE_OVER);
    if(ImGui::Button(ICON_FA_PLAY,ImVec2(69*scaleFactor,26*scaleFactor))){
        trigger.trigger(1.0f);
    }
    ImGui::PopStyleColor(3);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::PushItemWidth(130*scaleFactor);
    if(ImGui::InputFloat("PITCH",&pitch,1.0f, 10.0f,"%.2f")){
        pitch = ofClamp(pitch,1.0f, 10.0f);
        this->setCustomVar(pitch,"PITCH");
        pitch_ctrl.set(pitch);
    }
    ImGui::Spacing();
    if(ImGui::InputFloat("GAIN",&gain,0.0f, 10.0f,"%.2f")){
        gain = ofClamp(gain,0.0f, 10.0f);
        this->setCustomVar(gain,"GAIN");
        gain_ctrl.set(gain);
    }
    ImGui::Spacing();
    ImGui::Spacing();
    if(ImGui::Checkbox("REVERSE PLAY " ICON_FA_ARROW_LEFT,&reverse)){
        if(reverse){
            direction = -1;
            start_ctrl.set(1.0);
        }else{
            direction = 1;
            start_ctrl.set(0.0);
        }
        this->setCustomVar(static_cast<float>(direction),"DIRECTION");
        direction_ctrl.set(direction);
    }

    ImGui::PopItemWidth();
    ImGui::Spacing();
    ImGui::Spacing();

    ImGuiEx::ObjectInfo(
                "Sample player, run in audio thread, this must be used instead of Soundfile Player in case we need realtime audio precision.",
                "https://mosaic.d3cod3.org/reference.php?r=sample-player", scaleFactor);

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, loadSoundfileFlag, "Select an audio sample", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".wav,.mp3,.ogg,.flac,.WAV,.MP3,.OGG,.FLAC", "", scaleFactor)){
        ofFile file (fileDialog.selected_path);
        if (file.exists()){
            lastSoundfile = file.getAbsolutePath();
            soundfileLoaded= true;
            startTime = ofGetElapsedTimeMillis();
        }
    }
}

//--------------------------------------------------------------
void SamplePlayer::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);

    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void SamplePlayer::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);

    ofxVP_CAST_PIN_PTR<ofSoundBuffer>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);

    *ofxVP_CAST_PIN_PTR<vector<float>>(_outletParams[1]) = scope.getBuffer();

}

//--------------------------------------------------------------
void SamplePlayer::loadSettings(){
    ofxXmlSettings XML;

#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
    if (XML.loadFile(patchFile)){
#else
    if (XML.load(patchFile)){
#endif
        if(XML.pushTag("settings")){
            sampleRate = static_cast<double>(XML.getValue("sample_rate_out",0));
            bufferSize = XML.getValue("buffer_size",0);
            XML.popTag();
        }
    }

    for(int i=0;i<bufferSize;i++){
        ofxVP_CAST_PIN_PTR<vector<float>>(_outletParams[1])->push_back(0.0f);
    }

}

//--------------------------------------------------------------
void SamplePlayer::loadAudioFile(string audiofilepath){

    if(!loading){ // when loading previously saved patch
        filepath = copyFileToPatchFolder(this->patchFolderPath,audiofilepath);
    }else{
        filepath = forceCheckMosaicDataPath(audiofilepath);
    }

    loadingFile = true;

    sampleBuffer.load(filepath);
    sampler.setSample(&sampleBuffer,0);

    this->saveConfig(false);

    isFileLoaded = false;
    loadingFile = false;

}

OBJECT_REGISTER( SamplePlayer, "sample player", OFXVP_OBJECT_CAT_SOUND)

#endif
