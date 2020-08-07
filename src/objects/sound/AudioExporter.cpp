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

#include "AudioExporter.h"

//--------------------------------------------------------------
AudioExporter::AudioExporter() : PatchObject("audio exporter"){

    this->numInlets  = 2;
    this->numOutlets = 0;

    _inletParams[0] = new ofSoundBuffer(); // input

    _inletParams[1] = new float();  // bang
    *(float *)&_inletParams[1] = 0.0f;

    this->initInletsState();

    bang                = false;
    isAudioINObject     = true;

    exportAudioFlag     = false;

    audioFPS            = 0.0f;
    audioCounter        = 0;
    lastAudioTimeReset  = ofGetElapsedTimeMillis();

    recButtonLabel      = "REC";
}

//--------------------------------------------------------------
void AudioExporter::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"input");
    this->addInlet(VP_LINK_NUMERIC,"bang");
}

//--------------------------------------------------------------
void AudioExporter::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    fileDialog.setIsRetina(this->isRetina);

    loadAudioSettings();

    recorder.setup(true, false, glm::vec2(1280, 720));
    recorder.setAudioConfig(bufferSize,sampleRate);
    recorder.setOverWrite(true);

#if defined(TARGET_OSX)
    recorder.setFFmpegPath(ofToDataPath("ffmpeg/osx/ffmpeg",true));
#elif defined(TARGET_WIN32)
    recorder.setFFmpegPath(ofToDataPath("ffmpeg/win/ffmpeg.exe",true));
#endif

}

//--------------------------------------------------------------
void AudioExporter::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[1]){
        if(*(float *)&_inletParams[1] < 1.0){
            bang = false;
        }else{
            bang = true;
        }
    }

    if(this->inletsConnected[0] && filepath != "none" && bang){
        if(!recorder.isRecording()){
            recorder.startCustomAudioRecord();
            recButtonLabel = "STOP";
            ofLog(OF_LOG_NOTICE,"START EXPORTING AUDIO");
        }else if(recorder.isRecording()){
            recorder.stop();
            recButtonLabel = "REC";
            ofLog(OF_LOG_NOTICE,"FINISHED EXPORTING AUDIO");
        }
    }

}

//--------------------------------------------------------------
void AudioExporter::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void AudioExporter::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    ofFile tempFilename(filepath);

    exportAudioFlag = false;

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {
            ImGui::Spacing();
            ImGui::Text("Export to File:");
            if(filepath == "none"){
                ImGui::Text("none");
            }else{
                ImGui::Text("%s",tempFilename.getFileName().c_str());
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",tempFilename.getAbsolutePath().c_str());
            }
            ImGui::Spacing();
            if(ImGui::Button(ICON_FA_FILE_UPLOAD,ImVec2(84*scaleFactor,26*scaleFactor))){
                exportAudioFlag = true;
            }
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, VHS_RED);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, VHS_RED_OVER);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, VHS_RED_OVER);
            char tmp[256];
            sprintf(tmp,"%s %s",ICON_FA_CIRCLE, recButtonLabel.c_str());
            if(ImGui::Button(tmp,ImVec2(84*scaleFactor,26*scaleFactor))){
                if(!this->inletsConnected[0]){
                    ofLog(OF_LOG_WARNING,"There is no ofSoundBuffer connected to the object inlet, connect something if you want to export it as audio!");
                }else if(filepath == "none"){
                    ofLog(OF_LOG_WARNING,"No file selected. Please select one before recording!");
                }else{
                    if(!recorder.isRecording()){
                        recorder.startCustomAudioRecord();
                        recButtonLabel = "STOP";
                        ofLog(OF_LOG_NOTICE,"START EXPORTING AUDIO");
                    }else if(recorder.isRecording()){
                        recorder.stop();
                        recButtonLabel = "REC";
                        ofLog(OF_LOG_NOTICE,"FINISHED EXPORTING AUDIO");
                    }
                }
            }
            ImGui::PopStyleColor(3);

            ImGuiEx::ObjectInfo(
                        "Export audio from every sound buffer cable (yellow ones). Export format is fixed to 320 kb mp3.",
                        "https://mosaic.d3cod3.org/reference.php?r=audio-exporter", scaleFactor);

            ImGui::EndMenu();
        }
        _nodeCanvas.EndNodeMenu();
    }


    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        if(this->inletsConnected[0]){
            // draw waveform
            ImGuiEx::drawWaveform(_nodeCanvas.getNodeDrawList(), ImGui::GetWindowSize(), plot_data, 1024, 1.3f, IM_COL32(255,255,120,255), this->scaleFactor);

            // draw signal RMS amplitude
            _nodeCanvas.getNodeDrawList()->AddRectFilled(ImGui::GetWindowPos()+ImVec2(0,ImGui::GetWindowSize().y),ImGui::GetWindowPos()+ImVec2(ImGui::GetWindowSize().x,ImGui::GetWindowSize().y * (1.0f - ofClamp(static_cast<ofSoundBuffer *>(_inletParams[0])->getRMSAmplitude(),0.0,1.0))),IM_COL32(255,255,120,12));

        }

        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 window_size = ImGui::GetWindowSize();
        ImVec2 pos = ImVec2(window_pos.x + window_size.x - 20, window_pos.y + 40);
        if (recorder.isRecording()){
            _nodeCanvas.getNodeDrawList()->AddCircleFilled(pos, 10, IM_COL32(255, 0, 0, 255), 40);
        }else if(recorder.isPaused() && recorder.isRecording()){
            _nodeCanvas.getNodeDrawList()->AddCircleFilled(pos, 10, IM_COL32(255, 255, 0, 255), 40);
        }else{
            _nodeCanvas.getNodeDrawList()->AddCircleFilled(pos, 10, IM_COL32(0, 255, 0, 255), 40);
        }

        _nodeCanvas.EndNodeContent();
    }

    // file dialog
#if defined(TARGET_WIN32)
    if(ImGuiEx::getFileDialog(fileDialog, exportAudioFlag, "Export audio", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ".mp3", "audioExport.mp3", scaleFactor)){
        filepath = fileDialog.selected_path;
        // check extension
        if(fileDialog.ext != "mp3"){
            filepath += ".mp3";
        }
        recorder.setOutputPath(filepath);
        // prepare blank audio file
        recorder.startCustomAudioRecord();
        recorder.stop();
    }
#else
    if(ImGuiEx::getFileDialog(fileDialog, exportAudioFlag, "Export audio", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ".mp3", "audioExport.mp3", scaleFactor)){
        filepath = fileDialog.selected_path;
        // check extension
        if(fileDialog.ext != "mp3"){
            filepath += ".mp3";
        }
        recorder.setOutputPath(filepath);
        // prepare blank audio file
        recorder.startCustomAudioRecord();
        recorder.stop();
    }
#endif

}

//--------------------------------------------------------------
void AudioExporter::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void AudioExporter::loadAudioSettings(){
    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){
        if (XML.pushTag("settings")){
            sampleRate = XML.getValue("sample_rate_in",0);
            bufferSize = XML.getValue("buffer_size",0);

            for(int i=0;i<bufferSize;i++){
                plot_data[i] = 0.0f;
            }

            XML.popTag();
        }
    }
}

//--------------------------------------------------------------
void AudioExporter::audioInObject(ofSoundBuffer &inputBuffer){
    if(ofGetElapsedTimeMillis()-lastAudioTimeReset >= 1000){
        lastAudioTimeReset = ofGetElapsedTimeMillis();
        audioFPS = audioCounter;
        audioCounter = 0;
    }else{
        audioCounter++;
    }

    if(this->inletsConnected[0]){
        if(recorder.isRecording()){
            recorder.addBuffer(*static_cast<ofSoundBuffer *>(_inletParams[0]),audioFPS);
        }

        for(size_t i = 0; i < static_cast<ofSoundBuffer *>(_inletParams[0])->getNumFrames(); i++) {
            float sample = static_cast<ofSoundBuffer *>(_inletParams[0])->getSample(i,0);
            plot_data[i] = hardClip(sample);
        }
    }
}

OBJECT_REGISTER( AudioExporter, "audio exporter", OFXVP_OBJECT_CAT_SOUND)

#endif
