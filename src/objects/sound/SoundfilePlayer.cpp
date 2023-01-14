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

#include "SoundfilePlayer.h"

//--------------------------------------------------------------
SoundfilePlayer::SoundfilePlayer() : PatchObject("soundfile player"){

    this->numInlets  = 5;
    this->numOutlets = 3;

    _inletParams[0] = new string();  // control
    *static_cast<string *>(_inletParams[0]) = "";
    _inletParams[1] = new float();  // playhead
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();  // speed
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();  // volume
    *(float *)&_inletParams[3] = 0.0f;
    _inletParams[4] = new float();  // trigger
    *(float *)&_inletParams[4] = 0.0f;

    _outletParams[0] = new ofSoundBuffer();  // signal
    _outletParams[1] = new vector<float>(); // audio buffer
    _outletParams[2] = new float();  // finish bang
    *(float *)&_outletParams[2] = 0.0f;

    this->initInletsState();

    isAudioOUTObject    = true;

    isNewObject         = true;
    isFileLoaded        = false;
    isPlaying           = false;
    audioWasPlaying     = false;
    lastMessage         = "";

    loop                = false;
    volume              = 1.0f;
    speed               = 1.0f;
    sampleRate          = 44100.0;
    bufferSize          = 256;

    lastSoundfile       = "";
    loadSoundfileFlag   = false;
    soundfileLoaded     = false;
    loadingFile         = false;

    finishSemaphore     = false;
    finishBang          = false;
    isNextCycle         = false;

    isPDSPPatchableObject   = true;

    this->width         *= 2;

}

//--------------------------------------------------------------
void SoundfilePlayer::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_STRING,"control");
    this->addInlet(VP_LINK_NUMERIC,"playhead");
    this->addInlet(VP_LINK_NUMERIC,"speed");
    this->addInlet(VP_LINK_NUMERIC,"volume");
    this->addInlet(VP_LINK_NUMERIC,"bang");

    this->addOutlet(VP_LINK_AUDIO,"audioFileSignal");
    this->addOutlet(VP_LINK_ARRAY,"dataBuffer");
    this->addOutlet(VP_LINK_NUMERIC,"finish");
}

//--------------------------------------------------------------
void SoundfilePlayer::autoloadFile(string _fp){
    lastSoundfile = _fp;

    soundfileLoaded = true;
    loading = false;
    startTime = ofGetElapsedTimeMillis();
}

//--------------------------------------------------------------
void SoundfilePlayer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    fileDialog.setIsRetina(this->isRetina);

    loadSettings();

    loading = true;
}

//--------------------------------------------------------------
void SoundfilePlayer::setupAudioOutObjectContent(pdsp::Engine &engine){

    fileOUT.out_signal() >> this->pdspOut[0];
    fileOUT.out_signal() >> scope >> engine.blackhole();

    startTime   = ofGetElapsedTimeMillis();
}

//--------------------------------------------------------------
void SoundfilePlayer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
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

    if(!isFileLoaded && audiofile.loaded() && audiofile.samplerate() > 100){
        isFileLoaded = true;
        ofLog(OF_LOG_NOTICE,"[verbose] sound file loaded: %s, Sample Rate: %s, Audiofile length: %s",filepath.c_str(), ofToString(audiofile.samplerate()).c_str(), ofToString(audiofile.length()).c_str());
    }

    if(isFileLoaded && audiofile.loaded()){
        // listen to message control (_inletParams[0])
        if(this->inletsConnected[0]){
            if(lastMessage != *static_cast<string *>(_inletParams[0])){
                lastMessage = *static_cast<string *>(_inletParams[0]);

                if(lastMessage == "play"){
                    isPlaying = true;
                    playhead = 0.0;
                    audioWasPlaying = true;
                    finishSemaphore = true;
                }else if(lastMessage == "pause"){
                    isPlaying = false;
                }else if(lastMessage == "unpause"){
                    if(audioWasPlaying){
                        isPlaying = true;
                    }
                }else if(lastMessage == "stop"){
                    isPlaying = false;
                    playhead = 0.0;
                    audioWasPlaying = false;
                }else if(lastMessage == "loop_normal"){
                    loop = true;
                }else if(lastMessage == "loop_none"){
                    loop = false;
                }
            }
        }
        // playhead
        if(this->inletsConnected[1] && *(float *)&_inletParams[1] != -1.0f){
            playhead = static_cast<double>(*(float *)&_inletParams[1]) * audiofile.length();
        }
        // speed
        if(this->inletsConnected[2]){
            speed = ofClamp(*(float *)&_inletParams[2],-10.0f,10.0f);
        }
        // volume
        if(this->inletsConnected[3]){
            volume = ofClamp(*(float *)&_inletParams[3],0.0f,1.0f);
        }

        // outlet finish bang
        if(finishBang){
            *(float *)&_outletParams[2] = 1.0f;
        }else{
            *(float *)&_outletParams[2] = 0.0f;
        }

        if(finishBang){
            finishBang = false;
        }

    }

}

//--------------------------------------------------------------
void SoundfilePlayer::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

    ofSetColor(255);
}

//--------------------------------------------------------------
void SoundfilePlayer::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
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
        if(isFileLoaded && audiofile.loaded()){
            ImVec2 window_pos = ImGui::GetWindowPos();
            ImVec2 window_size = ImGui::GetWindowSize();
            ImVec2 ph_pos = ImVec2(window_pos.x + (20*scaleFactor), window_pos.y + (20*scaleFactor));

            objOriginX = ImGui::GetWindowPos().x + ((IMGUI_EX_NODE_PINS_WIDTH_NORMAL - 1)*this->scaleFactor);
            objOriginY = ImGui::GetWindowPos().y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor);
            scaledObjW = window_size.x - IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor;
            scaledObjH = window_size.y - (IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor;

            // draw Audiofile Waveform plot
            _nodeCanvas.getNodeDrawList()->AddRectFilled(ImVec2(objOriginX,objOriginY),ImVec2(objOriginX+scaledObjW,objOriginY+scaledObjH),IM_COL32_BLACK);
            for( int x=objOriginX; x<objOriginX+scaledObjW; ++x ){
                int n = ofMap( x, objOriginX, objOriginX+scaledObjW, 0, audiofile.length(), true );
                float val = audiofile.sample( n, 0 );
                _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(x, objOriginY + scaledObjH/2 - (val*(scaledObjH*0.5)) ),ImVec2(x, objOriginY + scaledObjH/2 + (val*(scaledObjH*0.5))),IM_COL32(255,255,120,180), 1.0f);
            }

            // draw position (timecode)
            ImGuiEx::drawTimecode(_nodeCanvas.getNodeDrawList(),static_cast<int>(ceil(static_cast<int>(floor(playhead))/audiofile.samplerate())),"",true,ImVec2(window_pos.x +(40*_nodeCanvas.GetCanvasScale()), window_pos.y+window_size.y-(36*_nodeCanvas.GetCanvasScale())),_nodeCanvas.GetCanvasScale()/this->scaleFactor);

            // draw player state
            if(isPlaying){ // play
                _nodeCanvas.getNodeDrawList()->AddTriangleFilled(ImVec2(window_pos.x+window_size.x-(50*_nodeCanvas.GetCanvasScale()),window_pos.y+window_size.y-(40*_nodeCanvas.GetCanvasScale())), ImVec2(window_pos.x+window_size.x-(50*_nodeCanvas.GetCanvasScale()), window_pos.y+window_size.y-(20*_nodeCanvas.GetCanvasScale())), ImVec2(window_pos.x+window_size.x-(30*_nodeCanvas.GetCanvasScale()), window_pos.y+window_size.y-(30*_nodeCanvas.GetCanvasScale())), IM_COL32(255, 255, 255, 120));
            }else if(!isPlaying && playhead > 0.0){ // pause
                _nodeCanvas.getNodeDrawList()->AddRectFilled(ImVec2(window_pos.x+window_size.x-(50*_nodeCanvas.GetCanvasScale()),window_pos.y+window_size.y-(40*_nodeCanvas.GetCanvasScale())),ImVec2(window_pos.x+window_size.x-(42*_nodeCanvas.GetCanvasScale()),window_pos.y+window_size.y-(20*_nodeCanvas.GetCanvasScale())),IM_COL32(255, 255, 255, 120));
                _nodeCanvas.getNodeDrawList()->AddRectFilled(ImVec2(window_pos.x+window_size.x-(38*_nodeCanvas.GetCanvasScale()),window_pos.y+window_size.y-(40*_nodeCanvas.GetCanvasScale())),ImVec2(window_pos.x+window_size.x-(30*_nodeCanvas.GetCanvasScale()),window_pos.y+window_size.y-(20*_nodeCanvas.GetCanvasScale())),IM_COL32(255, 255, 255, 120));
            }else if(!isPlaying && playhead == 0.0){ // stop
                _nodeCanvas.getNodeDrawList()->AddRectFilled(ImVec2(window_pos.x+window_size.x-(50*_nodeCanvas.GetCanvasScale()),window_pos.y+window_size.y-(40*_nodeCanvas.GetCanvasScale())),ImVec2(window_pos.x+window_size.x-(30*_nodeCanvas.GetCanvasScale()),window_pos.y+window_size.y-(20*_nodeCanvas.GetCanvasScale())),IM_COL32(255, 255, 255, 120));
            }

            // draw playhead
            float phx = ofMap( playhead, 0, audiofile.length()*0.98f, 1, (this->width*0.98f*_nodeCanvas.GetCanvasScale())-(31*this->scaleFactor) );
            _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(ph_pos.x + phx, ph_pos.y),ImVec2(ph_pos.x + phx, window_size.y+ph_pos.y-(26*this->scaleFactor)),IM_COL32(255, 255, 255, 160), 2.0f);

        }else if(loadingFile){
            ImGui::Text("LOADING FILE...");
        }else if(!isNewObject && !audiofile.loaded()){
            ImGui::Text("FILE NOT FOUND!");
        }

        _nodeCanvas.EndNodeContent();
    }

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, loadSoundfileFlag, "Select an audio file", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".wav,.mp3,.ogg,.flac", "", scaleFactor)){
        ofFile file (fileDialog.selected_path);
        if (file.exists()){
            lastSoundfile = file.getAbsolutePath();
            soundfileLoaded= true;
            startTime = ofGetElapsedTimeMillis();
        }
    }
}

//--------------------------------------------------------------
void SoundfilePlayer::drawObjectNodeConfig(){
    ofFile tempFilename(filepath);

    loadSoundfileFlag   = false;

    ImGui::Spacing();
    ImGui::Text("Loaded File:");
    if(filepath == "none"){
        ImGui::Text("%s",filepath.c_str());
    }else{
        ImGui::Text("%s",tempFilename.getFileName().c_str());
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",tempFilename.getAbsolutePath().c_str());
        ImGuiEx::drawTimecode(ImGui::GetForegroundDrawList(),static_cast<int>(ceil(audiofile.length()/audiofile.samplerate())),"Duration: ");
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
        isPlaying = true;
        playhead = 0.0;
        audioWasPlaying = true;
        finishSemaphore = true;
    }
    ImGui::SameLine();
    if(ImGui::Button(ICON_FA_STOP,ImVec2(69*scaleFactor,26*scaleFactor))){
        isPlaying = false;
        playhead = 0.0;
        audioWasPlaying = false;
    }
    ImGui::PopStyleColor(3);
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, VHS_YELLOW);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, VHS_YELLOW_OVER);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, VHS_YELLOW_OVER);
    if(ImGui::Button(ICON_FA_PAUSE,ImVec2(69*scaleFactor,26*scaleFactor))){
        isPlaying = !isPlaying;
    }
    ImGui::PopStyleColor(3);

    ImGui::Spacing();
    ImGui::PushItemWidth(130*scaleFactor);
    ImGui::SliderFloat("SPEED",&speed,-1.0f, 1.0f);
    ImGui::SliderFloat("VOLUME",&volume,0.0f, 1.0f);
    ImGui::PopItemWidth();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Checkbox("LOOP " ICON_FA_REDO,&loop);

    ImGuiEx::ObjectInfo(
                "Audiofile player, it can load .wav, .mp3, .ogg, and .flac files.",
                "https://mosaic.d3cod3.org/reference.php?r=soundfile-player", scaleFactor);

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, loadSoundfileFlag, "Select an audio file", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".wav,.mp3,.ogg,.flac", "", scaleFactor)){
        ofFile file (fileDialog.selected_path);
        if (file.exists()){
            lastSoundfile = file.getAbsolutePath();
            soundfileLoaded= true;
            startTime = ofGetElapsedTimeMillis();
        }
    }
}

//--------------------------------------------------------------
void SoundfilePlayer::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);

    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void SoundfilePlayer::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);

    // trigger, this needs to run in audio thread
    if(this->inletsConnected[4]){
        if(ofClamp(*(float *)&_inletParams[4],0.0f,1.0f) == 1.0f && !isNextCycle){
            isNextCycle = true;
            playhead = 0.0;
            isPlaying = true;
            finishSemaphore = true;
        }else if(playhead > 5000){
            isNextCycle = false;
        }

    }

    if(isFileLoaded && audiofile.loaded() && isPlaying){
        for(size_t i = 0; i < monoBuffer.getNumFrames(); i++) {
            int n = static_cast<int>(floor(playhead));

            if(n < static_cast<int>(audiofile.length()-1)){
                float fract = static_cast<float>(playhead - n);
                float isample = audiofile.sample(n, 0)*(1.0f-fract) + audiofile.sample(n+1, 0)*fract; // linear interpolation
                monoBuffer.getSample(i,0) = isample * volume;

                playhead += (step*speed);

            }else{
                monoBuffer.getSample(i,0) = 0.0f;

                if(finishSemaphore){
                    finishSemaphore = false;
                    finishBang = true;
                }

                if(loop){
                    // backword
                    if(speed < 0.0){
                        playhead = audiofile.length()-2;
                    }else if(speed > 0.0){
                        playhead = 0.0;
                    }
                }
            }
        }
        lastBuffer = monoBuffer;
    }else{
        lastBuffer = monoBuffer * 0.0f;
    }

    fileOUT.copyInput(lastBuffer.getBuffer().data(),lastBuffer.getNumFrames());
    *static_cast<ofSoundBuffer *>(_outletParams[0]) = lastBuffer;
    *static_cast<vector<float> *>(_outletParams[1]) = scope.getBuffer();

}

//--------------------------------------------------------------
void SoundfilePlayer::loadSettings(){
    ofxXmlSettings XML;

    if(XML.loadFile(patchFile)){
        if(XML.pushTag("settings")){
            sampleRate = static_cast<double>(XML.getValue("sample_rate_out",0));
            bufferSize = XML.getValue("buffer_size",0);
            XML.popTag();
        }
    }

    for(int i=0;i<bufferSize;i++){
        static_cast<vector<float> *>(_outletParams[1])->push_back(0.0f);
    }

    shortBuffer = new short[bufferSize];
    for (int i = 0; i < bufferSize; i++){
        shortBuffer[i] = 0;
    }

    ofSoundBuffer tmpBuffer(shortBuffer,static_cast<size_t>(bufferSize),1,static_cast<unsigned int>(sampleRate));
    monoBuffer = tmpBuffer;
}

//--------------------------------------------------------------
void SoundfilePlayer::loadAudioFile(string audiofilepath){

    if(!loading){ // when loading previously saved patch
        filepath = copyFileToPatchFolder(this->patchFolderPath,audiofilepath);
    }else{
        filepath = forceCheckMosaicDataPath(audiofilepath);
    }

    loadingFile = true;

    audiofile.free();
    audiofile.load(filepath);
    playhead = std::numeric_limits<int>::max();
    step = audiofile.samplerate() / sampleRate;

    plot_data = new float[bufferSize];
    for( int x=0; x<bufferSize; ++x){
        int n = ofMap( x, 0, bufferSize, 0, audiofile.length(), true );
        plot_data[x] = hardClip(audiofile.sample( n, 0 ));
    }

    ofSoundBuffer tmpBuffer(shortBuffer,static_cast<size_t>(bufferSize),1,static_cast<unsigned int>(sampleRate));
    monoBuffer.clear();
    monoBuffer = tmpBuffer;

    playhead = 0.0;

    this->saveConfig(false);

    isFileLoaded = false;
    loadingFile = false;

}

OBJECT_REGISTER( SoundfilePlayer, "soundfile player", OFXVP_OBJECT_CAT_SOUND)

#endif
