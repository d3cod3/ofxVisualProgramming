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

#include "VideoExporter.h"

//--------------------------------------------------------------
VideoExporter::VideoExporter() :
    PatchObject("video exporter")

{

    this->numInlets  = 1;
    this->numOutlets = 0;

    _inletParams[0] = new ofTexture(); // input

    this->initInletsState();

    isNewObject         = false;

    posX = posY = drawW = drawH = 0.0f;

    needToGrab          = false;
    exportVideoFlag     = false;
    videoSaved          = false;

    codecsList = {"hevc","libx264","jpeg2000","mjpeg","mpeg4"};
    selectedCodec = 4;
    recButtonLabel = "REC";

}

//--------------------------------------------------------------
void VideoExporter::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"input");
}

//--------------------------------------------------------------
void VideoExporter::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    captureFbo.allocate( STANDARD_TEXTURE_WIDTH, STANDARD_TEXTURE_HEIGHT, GL_RGB );
    recorder.setup(true, false, glm::vec2(STANDARD_TEXTURE_WIDTH, STANDARD_TEXTURE_HEIGHT)); // record video only for now
    recorder.setOverWrite(true);

#if defined(TARGET_OSX)
    recorder.setFFmpegPath(ofToDataPath("ffmpeg/osx/ffmpeg",true));
#elif defined(TARGET_WIN32)
    recorder.setFFmpegPath(ofToDataPath("ffmpeg/win/ffmpeg.exe",true));
#endif
    
}

//--------------------------------------------------------------
void VideoExporter::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(videoSaved){
        videoSaved = false;
        recorder.setOutputPath(filepath);
        recorder.setBitRate(20000);
        recorder.startCustomRecord();
    }

}

//--------------------------------------------------------------
void VideoExporter::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofSetCircleResolution(50);
    ofEnableAlphaBlending();

    if(this->inletsConnected[0]){
        if(static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
            if(!needToGrab){
                needToGrab = true;
                captureFbo.allocate(static_cast<ofTexture *>(_inletParams[0])->getWidth(), static_cast<ofTexture *>(_inletParams[0])->getHeight(), GL_RGB );
                recorder.setup(true, false, glm::vec2(static_cast<ofTexture *>(_inletParams[0])->getWidth(), static_cast<ofTexture *>(_inletParams[0])->getHeight())); // record video only for now
                recorder.setOverWrite(true);
            }

            captureFbo.begin();
            ofClear(0,0,0,255);
            ofSetColor(255);
            static_cast<ofTexture *>(_inletParams[0])->draw(0,static_cast<ofTexture *>(_inletParams[0])->getHeight(),static_cast<ofTexture *>(_inletParams[0])->getWidth(),-static_cast<ofTexture *>(_inletParams[0])->getHeight());
            captureFbo.end();

            if(recorder.isRecording()) {
                reader.readToPixels(captureFbo, capturePix,OF_IMAGE_COLOR); // ofxFastFboReader
                //captureFbo.readToPixels(capturePix); // standard
                if(capturePix.getWidth() > 0 && capturePix.getHeight() > 0) {
                    recorder.addFrame(capturePix);
                }
            }

        }
    }else{
        captureFbo.begin();
        ofClear(0,0,0,255);
        captureFbo.end();

        needToGrab = false;
    }

    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void VideoExporter::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    exportVideoFlag = false;

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
            if(ImGui::Button(recButtonLabel.c_str(),ImVec2(-1,20))){
                if(!this->inletsConnected[0] || !static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
                    ofLog(OF_LOG_WARNING,"There is no ofTexture connected to the object inlet, connect something if you want to export it as video!");
                }else{
                    if(!recorder.isRecording()){
                        exportVideoFlag = true;
                        recButtonLabel = "STOP";
                        ofLog(OF_LOG_NOTICE,"START EXPORTING VIDEO");
                    }else if(recorder.isRecording()){
                        recorder.stop();
                        recButtonLabel = "REC";
                        ofLog(OF_LOG_NOTICE,"FINISHED EXPORTING VIDEO");
                    }
                }
            }
            ImGui::Spacing();
            if(ImGui::BeginCombo("Codec", codecsList.at(selectedCodec).c_str() )){
                for(int i=0; i < codecsList.size(); ++i){
                    bool is_selected = (selectedCodec == i );
                    if (ImGui::Selectable(codecsList.at(i).c_str(), is_selected)){
                        selectedCodec = i;
                        recorder.setVideoCodec(codecsList.at(selectedCodec));
                    }
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }
            ImGui::Spacing();
            if (ImGui::CollapsingHeader("INFO", ImGuiTreeNodeFlags_None)){
                ImGui::TextWrapped("Export video from every texture cable (blue ones). You can choose the video codec: mpeg4, mjpeg, jpg2000, libx264, or hevc.");
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.5f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.129f, 0.0f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.5f, 1.0f, 1.0f));
                if(ImGui::Button("Reference")){
                    ofLaunchBrowser("https://mosaic.d3cod3.org/reference.php?r=video-exporter");
                }
                ImGui::PopStyleColor(3);
            }

            ImGui::EndMenu();
        }
        _nodeCanvas.EndNodeMenu();
    }


    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
            float _tw = this->width*_nodeCanvas.GetCanvasScale();
            float _th = (this->height*_nodeCanvas.GetCanvasScale()) - (IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT);

            ImGuiEx::drawOFTexture(static_cast<ofTexture *>(_inletParams[0]),_tw,_th,posX,posY,drawW,drawH);
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
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        #if defined(TARGET_WIN32)
        if(ImGuiEx::getFileDialog(fileDialog, exportVideoFlag, "Export video", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ".avi")){
            filepath = fileDialog.selected_path;
            // check extension
            if(fileDialog.ext != "avi"){
                filepath += ".avi";
            }
            videoSaved = true;
        }
        #else
        if(ImGuiEx::getFileDialog(fileDialog, exportVideoFlag, "Export video", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ".mp4")){
            filepath = fileDialog.selected_path;
            // check extension
            if(fileDialog.ext != "mp4"){
                filepath += ".mp4";
            }
            videoSaved = true;
        }
        #endif

    }
}

//--------------------------------------------------------------
void VideoExporter::removeObjectContent(bool removeFileFromData){

}


OBJECT_REGISTER( VideoExporter, "video exporter", OFXVP_OBJECT_CAT_VIDEO)

#endif
