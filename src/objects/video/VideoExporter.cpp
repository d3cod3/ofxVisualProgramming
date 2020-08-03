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

    this->numInlets  = 2;
    this->numOutlets = 0;

    _inletParams[0] = new ofTexture(); // input

    _inletParams[1] = new float();  // bang
    *(float *)&_inletParams[1] = 0.0f;

    this->initInletsState();

    isNewObject         = false;

    posX = posY = drawW = drawH = 0.0f;

    bang                = false;
    needToGrab          = false;
    exportVideoFlag     = false;

    codecsList = {"hevc","libx264","jpeg2000","mjpeg","mpeg4"};
    selectedCodec = 4;
    recButtonLabel = "REC";

    this->setIsTextureObj(true);

}

//--------------------------------------------------------------
void VideoExporter::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"input");
    this->addInlet(VP_LINK_NUMERIC,"bang");
}

//--------------------------------------------------------------
void VideoExporter::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    fileDialog.setIsRetina(this->isRetina);

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

    if(this->inletsConnected[1]){
        if(*(float *)&_inletParams[1] < 1.0){
            bang = false;
        }else{
            bang = true;
        }
    }

    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated() && filepath != "none" && bang){
        if(!recorder.isRecording()){
            recorder.setBitRate(20000);
            recorder.startCustomRecord();
            recButtonLabel = "STOP";
            ofLog(OF_LOG_NOTICE,"START EXPORTING VIDEO");
        }else if(recorder.isRecording()){
            recorder.stop();
            recButtonLabel = "REC";
            ofLog(OF_LOG_NOTICE,"FINISHED EXPORTING VIDEO");
        }
    }

}

//--------------------------------------------------------------
void VideoExporter::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

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
        if(this->width > 118.0f){
            // draw node texture preview with OF
            drawNodeOFTexture(*static_cast<ofTexture *>(_inletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, IMGUI_EX_NODE_FOOTER_HEIGHT);
        }
    }else{
        captureFbo.begin();
        ofClear(0,0,0,255);
        captureFbo.end();

        needToGrab = false;
    }

}

//--------------------------------------------------------------
void VideoExporter::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    ofFile tempFilename(filepath);

    exportVideoFlag = false;

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
            if(ImGui::Button(ICON_FA_FILE_UPLOAD,ImVec2(84,26))){
                exportVideoFlag = true;
            }
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, VHS_RED);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, VHS_RED_OVER);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, VHS_RED_OVER);
            char tmp[256];
            sprintf(tmp,"%s %s",ICON_FA_CIRCLE, recButtonLabel.c_str());
            if(ImGui::Button(tmp,ImVec2(84,26))){
                if(!this->inletsConnected[0] || !static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
                    ofLog(OF_LOG_WARNING,"There is no ofTexture connected to the object inlet, connect something if you want to export it as video!");
                }else if(filepath == "none"){
                    ofLog(OF_LOG_WARNING,"No file selected. Please select one before recording!");
                }else{
                    if(!recorder.isRecording()){
                        recorder.setBitRate(20000);
                        recorder.startCustomRecord();
                        recButtonLabel = "STOP";
                        ofLog(OF_LOG_NOTICE,"START EXPORTING VIDEO");
                    }else if(recorder.isRecording()){
                        recorder.stop();
                        recButtonLabel = "REC";
                        ofLog(OF_LOG_NOTICE,"FINISHED EXPORTING VIDEO");
                    }
                }
            }
            ImGui::PopStyleColor(3);
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

            ImGuiEx::ObjectInfo(
                        "Export video from every texture cable (blue ones). You can choose the video codec: mpeg4, mjpeg, jpg2000, libx264, or hevc.",
                        "https://mosaic.d3cod3.org/reference.php?r=video-exporter");

            ImGui::EndMenu();
        }
        _nodeCanvas.EndNodeMenu();
    }


    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        // get imgui node translated/scaled position/dimension for drawing textures in OF
        objOriginX = (ImGui::GetWindowPos().x + IMGUI_EX_NODE_PINS_WIDTH_NORMAL - 1 - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale();
        objOriginY = (ImGui::GetWindowPos().y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale();
        scaledObjW = this->width - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL/_nodeCanvas.GetCanvasScale());
        scaledObjH = this->height - ((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)/_nodeCanvas.GetCanvasScale());



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

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();

    // file dialog
#if defined(TARGET_WIN32)
    if(ImGuiEx::getFileDialog(fileDialog, exportVideoFlag, "Export video", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ".avi", "videoExport.avi", scaleFactor)){
        filepath = fileDialog.selected_path;
        // check extension
        if(fileDialog.ext != "avi"){
            filepath += ".avi";
        }
        recorder.setOutputPath(filepath);
        // prepare blank video file
        recorder.startCustomRecord();
        recorder.stop();
    }
#else
    if(ImGuiEx::getFileDialog(fileDialog, exportVideoFlag, "Export video", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ".mp4", "videoExport.mp4", scaleFactor)){
        filepath = fileDialog.selected_path;
        // check extension
        if(fileDialog.ext != "mp4"){
            filepath += ".mp4";
        }
        recorder.setOutputPath(filepath);
        // prepare blank video file
        recorder.startCustomRecord();
        recorder.stop();
    }
#endif
}

//--------------------------------------------------------------
void VideoExporter::removeObjectContent(bool removeFileFromData){

}


OBJECT_REGISTER( VideoExporter, "video exporter", OFXVP_OBJECT_CAT_VIDEO)

#endif
