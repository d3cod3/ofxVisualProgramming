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

    this->numInlets  = 1;
    this->numOutlets = 0;

    _inletParams[0] = new ofSoundBuffer(); // input

    this->initInletsState();

    isAudioINObject     = true;

    exportAudioFlag     = false;
    audioSaved          = false;

    audioFPS            = 0.0f;
    audioCounter        = 0;
    lastAudioTimeReset  = ofGetElapsedTimeMillis();

    recButtonLabel      = "REC";
}

//--------------------------------------------------------------
void AudioExporter::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"input");
}

//--------------------------------------------------------------
void AudioExporter::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
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

    if(audioSaved){
        audioSaved = false;
        recorder.setOutputPath(filepath);
        recorder.startCustomAudioRecord();
    }

}

//--------------------------------------------------------------
void AudioExporter::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();

    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void AudioExporter::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    exportAudioFlag = false;

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {
            ofFile tempFilename(filepath);
            ImGui::Text("Export File:");
            ImGui::Text("%s",tempFilename.getFileName().c_str());
            ImGui::Spacing();
            if(ImGui::Button(recButtonLabel.c_str(),ImVec2(200,20))){
                if(!this->inletsConnected[0]){
                    ofLog(OF_LOG_WARNING,"There is no ofSoundBuffer connected to the object inlet, connect something if you want to export it as audio!");
                }else{
                    if(!recorder.isRecording()){
                        exportAudioFlag = true;
                        recButtonLabel = "STOP";
                        ofLog(OF_LOG_NOTICE,"START EXPORTING AUDIO");
                    }else if(recorder.isRecording()){
                        recorder.stop();
                        recButtonLabel = "REC";
                        ofLog(OF_LOG_NOTICE,"FINISHED EXPORTING AUDIO");
                    }
                }
            }

            ImGui::Spacing();
            if (ImGui::CollapsingHeader("INFO", ImGuiTreeNodeFlags_None)){
                ImGui::TextWrapped("Export audio from every sound buffer cable (yellow ones). Export format is fixed to 320 kb mp3.");
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.5f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.129f, 0.0f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.5f, 1.0f, 1.0f));
                if(ImGui::Button("Reference")){
                    ofLaunchBrowser("https://mosaic.d3cod3.org/reference.php?r=audio-exporter");
                }
                ImGui::PopStyleColor(3);
            }

            ImGui::EndMenu();
        }
        _nodeCanvas.EndNodeMenu();
    }


    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        if(this->inletsConnected[0]){
            // Waveform plot
            ImGui::PlotConfig conf;
            conf.values.ys = plot_data;
            conf.values.count = 1024;
            conf.values.color = IM_COL32(255,255,120,255);
            conf.scale.min = -1;
            conf.scale.max = 1;
            conf.tooltip.show = false;
            conf.tooltip.format = "x=%.2f, y=%.2f";
            conf.grid_x.show = false;
            conf.grid_y.show = false;
            conf.frame_size = ImVec2(this->width*0.98f*_nodeCanvas.GetCanvasScale(), this->height*0.7f*_nodeCanvas.GetCanvasScale());
            conf.line_thickness = 1.3f;

            ImGui::Plot("plot", conf);
        }

        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 window_size = ImGui::GetWindowSize();
        ImVec2 pos = ImVec2(window_pos.x + window_size.x - 20, window_pos.y + 40);
        if (recorder.isRecording()){
            ImGui::GetForegroundDrawList()->AddCircleFilled(pos, 10, IM_COL32(255, 0, 0, 255), 40);
        }else if(recorder.isPaused() && recorder.isRecording()){
            ImGui::GetForegroundDrawList()->AddCircleFilled(pos, 10, IM_COL32(255, 255, 0, 255), 40);
        }else{
            ImGui::GetForegroundDrawList()->AddCircleFilled(pos, 10, IM_COL32(0, 255, 0, 255), 40);
        }

        _nodeCanvas.EndNodeContent();
    }

    // file dialog
    if(this->inletsConnected[0]){
        #if defined(TARGET_WIN32)
        if(ImGuiEx::getFileDialog(fileDialog, exportAudioFlag, "Export audio", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ".mp3")){
            filepath = fileDialog.selected_path;
            // check extension
            if(fileDialog.ext != "mp3"){
                filepath += ".mp3";
            }
            audioSaved = true;
        }
        #else
        if(ImGuiEx::getFileDialog(fileDialog, exportAudioFlag, "Export audio", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ".mp3")){
            filepath = fileDialog.selected_path;
            // check extension
            if(fileDialog.ext != "mp3"){
                filepath += ".mp3";
            }
            audioSaved = true;
        }
        #endif

    }

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
