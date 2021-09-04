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

#include "VideoPlayer.h"

//--------------------------------------------------------------
VideoPlayer::VideoPlayer() : PatchObject("video player"){

    this->numInlets  = 4;
    this->numOutlets = 2;

    _inletParams[0] = new string();  // control
    *static_cast<string *>(_inletParams[0]) = "";
    _inletParams[1] = new float();  // playhead
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();  // speed
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();  // trigger
    *(float *)&_inletParams[3] = 0.0f;

    _outletParams[0] = new ofTexture(); // output
    _outletParams[1] = new float();  // finish bang
    *(float *)&_outletParams[1] = 0.0f;

    this->initInletsState();

    #ifndef TARGET_WIN32
        video = new ofxHapPlayer();
    #else
        video = new ofVideoPlayer();
    #endif

    lastMessage         = "";
    isNewObject         = false;
    isFileLoaded        = false;
    nameLabelLoaded     = false;

    loop                = false;
    speed               = 1.0f;

    posX = posY = drawW = drawH = 0.0f;

    needToLoadVideo     = true;
    lastPlayhead        = 0.0f;
    loadVideoFlag       = false;
    videoWasPlaying     = false;
    isPaused            = false;

    videoName           = "";
    videoRes            = "";
    videoPath           = "";

    finishBang          = false;

    preloadFirstFrame   = false;

    this->setIsResizable(true);
    this->setIsTextureObj(true);

}

//--------------------------------------------------------------
void VideoPlayer::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_STRING,"control");
    this->addInlet(VP_LINK_NUMERIC,"playhead");
    this->addInlet(VP_LINK_NUMERIC,"speed");
    this->addInlet(VP_LINK_NUMERIC,"bang");

    this->addOutlet(VP_LINK_TEXTURE,"output");
    this->addOutlet(VP_LINK_NUMERIC,"finish");
}

//--------------------------------------------------------------
void VideoPlayer::autoloadFile(string _fp){
    //this->filepath = _fp;
    this->filepath = copyFileToPatchFolder(this->patchFolderPath,_fp);
    isFileLoaded = false;
    needToLoadVideo = true;
}

//--------------------------------------------------------------
void VideoPlayer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    fileDialog.setIsRetina(this->isRetina);

    if(filepath == "none"){
        isNewObject = true;
    }

}

//--------------------------------------------------------------
void VideoPlayer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(needToLoadVideo){
        needToLoadVideo = false;
        loadVideoFile();
    }

}

//--------------------------------------------------------------
void VideoPlayer::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

    if(!isFileLoaded && video->isLoaded()){
        video->setLoopState(OF_LOOP_NONE);
        video->setVolume(0);
        video->play();
        preloadFirstFrame = true;

        ofLog(OF_LOG_NOTICE,"[verbose] video file loaded: %s",filepath.c_str());
        //ofLog(OF_LOG_NOTICE,"Internal texture data type: %i",video->getTexture().getTextureData().glInternalFormat);

        isFileLoaded = true;
    }

    if(isFileLoaded && video->isLoaded()){

        if(video->getWidth() != static_cast<ofTexture *>(_outletParams[0])->getWidth() || video->getHeight() != static_cast<ofTexture *>(_outletParams[0])->getHeight()){
            _outletParams[0] = new ofTexture();
            static_cast<ofTexture *>(_outletParams[0])->allocate(video->getWidth(),video->getHeight(),GL_RGB);
            static_cast<ofTexture *>(_outletParams[0])->clear();
        }else{
            //static_cast<ofTexture *>(_outletParams[0])->loadData(video->getPixels());
            #ifndef TARGET_WIN32
                *static_cast<ofTexture *>(_outletParams[0]) = *video->getTexture();
            #else
                static_cast<ofTexture *>(_outletParams[0])->loadData(video->getPixels());
            #endif
        }

        // listen to message control (_inletParams[0])
        if(this->inletsConnected[0]){
            if(lastMessage != *static_cast<string *>(_inletParams[0])){
                lastMessage = *static_cast<string *>(_inletParams[0]);

                if(lastMessage == "play"){
                    video->firstFrame();
                    video->play();
                    videoWasPlaying = true;
                }else if(lastMessage == "pause"){
                    video->setPaused(true);
                }else if(lastMessage == "unpause"){
                    video->setPaused(false);
                    if(!videoWasPlaying){
                        video->stop();
                    }
                }else if(lastMessage == "stop"){
                    video->stop();
                    videoWasPlaying = false;
                }else if(lastMessage == "loop_normal"){
                    loop = true;
                    video->setLoopState(OF_LOOP_NORMAL);
                }else if(lastMessage == "loop_none"){
                    loop = false;
                    video->setLoopState(OF_LOOP_NONE);
                }

                //ofLog(OF_LOG_NOTICE,"%s",lastMessage.c_str());
            }
        }

        // playhead
        if(this->inletsConnected[1] && *(float *)&_inletParams[1] != -1.0f && *(float *)&_inletParams[1] != lastPlayhead){
            video->setPosition(*(float *)&_inletParams[1]);
            lastPlayhead = *(float *)&_inletParams[1];
        }
        // speed
        if(this->inletsConnected[2]){
            speed = *(float *)&_inletParams[2];
            video->setSpeed(speed);
        }
        // trigger
        if(this->inletsConnected[3]){
            if(ofClamp(*(float *)&_inletParams[3],0.0f,1.0f) == 1.0f){
                video->firstFrame();
                video->play();
            }
        }

        if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
            if(video->isPlaying()){ // play
               video->update();

               //ofLog(OF_LOG_NOTICE,"%s: duration = %f",this->getName().c_str(),video->getDuration());

               // preload first video frame into outlet texture
               if(preloadFirstFrame && video->getCurrentFrame() > 0){
                   preloadFirstFrame = false;
                   video->stop();
               }

               // bang on video end -- TODO FIX
               if(video->getCurrentFrame() == video->getTotalNumFrames()){
                   finishBang = true;
               }

               // outlet finish bang
               if(finishBang){
                   finishBang = false;
                   *(float *)&_outletParams[1] = 1.0f;
               }else{
                   *(float *)&_outletParams[1] = 0.0f;
               }

            }

            // draw node texture preview with OF
            if(scaledObjW*canvasZoom > 90.0f){
                #ifndef TARGET_WIN32
                if(video->getShader()){
                    video->getShader()->begin();
                    drawNodeOFTexture(*static_cast<ofTexture *>(_outletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
                    video->getShader()->end();
                }else{
                    drawNodeOFTexture(*static_cast<ofTexture *>(_outletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
                }
                #else
                    drawNodeOFTexture(*static_cast<ofTexture *>(_outletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
                #endif
            }
        }

    }else{
        // background
        if(scaledObjW*canvasZoom > 90.0f){
            ofSetColor(34,34,34);
            ofDrawRectangle(objOriginX - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/canvasZoom), objOriginY-(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor/canvasZoom),scaledObjW + (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/canvasZoom),scaledObjH + (((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor)/canvasZoom) );
        }
    }

    ///////////////////////////////////////////
}

//--------------------------------------------------------------
void VideoPlayer::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    loadVideoFlag = false;

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

        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 window_size = ImGui::GetWindowSize();
        ImVec2 ph_pos = ImVec2(window_pos.x + (20*this->scaleFactor), window_pos.y + (20*this->scaleFactor));

        // get imgui node translated/scaled position/dimension for drawing textures in OF
        objOriginX = (ImGui::GetWindowPos().x + ((IMGUI_EX_NODE_PINS_WIDTH_NORMAL - 1)*this->scaleFactor) - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale();
        objOriginY = (ImGui::GetWindowPos().y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale();
        scaledObjW = this->width - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/_nodeCanvas.GetCanvasScale());
        scaledObjH = this->height - ((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor/_nodeCanvas.GetCanvasScale());

        if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){

            // draw position (timecode)
            ImGuiEx::drawTimecode(_nodeCanvas.getNodeDrawList(),static_cast<int>(ofClamp(floor(video->getPosition()*video->getDuration()),0,video->getDuration())),"",true,ImVec2(window_pos.x +(40*_nodeCanvas.GetCanvasScale()), window_pos.y+window_size.y-(36*_nodeCanvas.GetCanvasScale())),_nodeCanvas.GetCanvasScale()/this->scaleFactor);

            // draw player state
            if(video->isPlaying()){ // play
                _nodeCanvas.getNodeDrawList()->AddTriangleFilled(ImVec2(window_pos.x+window_size.x-(50*_nodeCanvas.GetCanvasScale()),window_pos.y+window_size.y-(40*_nodeCanvas.GetCanvasScale())), ImVec2(window_pos.x+window_size.x-(50*_nodeCanvas.GetCanvasScale()), window_pos.y+window_size.y-(20*_nodeCanvas.GetCanvasScale())), ImVec2(window_pos.x+window_size.x-(30*_nodeCanvas.GetCanvasScale()), window_pos.y+window_size.y-(30*_nodeCanvas.GetCanvasScale())), IM_COL32(255, 255, 255, 120));
            }else if(!video->isPlaying() && video->isPaused() && video->getCurrentFrame() > 1){ // pause
                _nodeCanvas.getNodeDrawList()->AddRectFilled(ImVec2(window_pos.x+window_size.x-(50*_nodeCanvas.GetCanvasScale()),window_pos.y+window_size.y-(40*_nodeCanvas.GetCanvasScale())),ImVec2(window_pos.x+window_size.x-(42*_nodeCanvas.GetCanvasScale()),window_pos.y+window_size.y-(20*_nodeCanvas.GetCanvasScale())),IM_COL32(255, 255, 255, 120));
                _nodeCanvas.getNodeDrawList()->AddRectFilled(ImVec2(window_pos.x+window_size.x-(38*_nodeCanvas.GetCanvasScale()),window_pos.y+window_size.y-(40*_nodeCanvas.GetCanvasScale())),ImVec2(window_pos.x+window_size.x-(30*_nodeCanvas.GetCanvasScale()),window_pos.y+window_size.y-(20*_nodeCanvas.GetCanvasScale())),IM_COL32(255, 255, 255, 120));
            }else if(!video->isPlaying() && video->getCurrentFrame() <= 1){ // stop
                _nodeCanvas.getNodeDrawList()->AddRectFilled(ImVec2(window_pos.x+window_size.x-(50*_nodeCanvas.GetCanvasScale()),window_pos.y+window_size.y-(40*_nodeCanvas.GetCanvasScale())),ImVec2(window_pos.x+window_size.x-(30*_nodeCanvas.GetCanvasScale()),window_pos.y+window_size.y-(20*_nodeCanvas.GetCanvasScale())),IM_COL32(255, 255, 255, 120));
            }

            // draw playhead
            float phx = ofMap( ofClamp(video->getPosition(),0.0f,1.0f), 0.0f, 1.0f, 1, (this->width*0.98f*_nodeCanvas.GetCanvasScale())-(31*this->scaleFactor) );
            _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(ph_pos.x + phx, ph_pos.y),ImVec2(ph_pos.x + phx, window_size.y+ph_pos.y-(26*this->scaleFactor)),IM_COL32(255, 255, 255, 160), 2.0f);
        }
        _nodeCanvas.EndNodeContent();
    }

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();

    // file dialog
    #ifndef TARGET_WIN32
    if(ImGuiEx::getFileDialog(fileDialog, loadVideoFlag, "Select a video file encoded with the HAP codec", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".mov,.mp4,.mpg,.mpeg,.avi", "", scaleFactor)){
        ofFile file (fileDialog.selected_path);
        if (file.exists()){
            filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
            isFileLoaded = false;
            needToLoadVideo = true;
        }
    }
    #else
    if(ImGuiEx::getFileDialog(fileDialog, loadVideoFlag, "Select a video file", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".mov,.mp4,.mpg,.mpeg,.avi", "", scaleFactor)){
        ofFile file (fileDialog.selected_path);
        if (file.exists()){
            filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
            isFileLoaded = false;
            needToLoadVideo = true;
        }
    }
    #endif


}

//--------------------------------------------------------------
void VideoPlayer::drawObjectNodeConfig(){
    ofFile tempFilename(filepath);

    loadVideoFlag = false;

    ImGui::Spacing();
    ImGui::Text("Loaded File:");
    if(filepath == "none"){
        ImGui::Text("%s",filepath.c_str());
    }else{
        ImGui::Text("%s",tempFilename.getFileName().c_str());
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",tempFilename.getAbsolutePath().c_str());
        ImGuiEx::drawTimecode(ImGui::GetForegroundDrawList(),static_cast<int>(ceil(video->getDuration())),"Duration: ");
        ImGui::Text("Resolution %.0fx%.0f",video->getWidth(),video->getHeight());
    }

    ImGui::Spacing();
    if(ImGui::Button(ICON_FA_FILE,ImVec2(224*this->scaleFactor,26*this->scaleFactor))){
        loadVideoFlag = true;
    }
    if (ImGui::IsItemHovered()){
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        #ifndef TARGET_WIN32
            ImGui::TextUnformatted("Open a video file. Compatible extensions: .mov .mp4 .mpg .mpeg .avi | Compatible codec: HAP");
        #else
            ImGui::TextUnformatted("Open a video file. Compatible extensions: .mov .mp4 .mpg .mpeg .avi");
        #endif

        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button, VHS_BLUE);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, VHS_BLUE_OVER);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, VHS_BLUE_OVER);
    if(ImGui::Button(ICON_FA_PLAY,ImVec2(69*this->scaleFactor,26*this->scaleFactor))){
        video->firstFrame();
        video->play();
        videoWasPlaying = true;
    }
    ImGui::SameLine();
    if(ImGui::Button(ICON_FA_STOP,ImVec2(69*this->scaleFactor,26*this->scaleFactor))){
        video->stop();
        videoWasPlaying = false;
    }
    ImGui::PopStyleColor(3);
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, VHS_YELLOW);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, VHS_YELLOW_OVER);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, VHS_YELLOW_OVER);
    if(ImGui::Button(ICON_FA_PAUSE,ImVec2(69*this->scaleFactor,26*this->scaleFactor))){
        isPaused = !isPaused;
        video->setPaused(isPaused);
        if(!isPaused){
            if(!videoWasPlaying){
                video->stop();
            }
        }
    }
    ImGui::PopStyleColor(3);

    ImGui::Spacing();
    ImGui::PushItemWidth(130*this->scaleFactor);
    if(ImGui::SliderFloat("SPEED",&speed,-1.0f, 1.0f)){
        video->setSpeed(speed);
    }
    ImGui::Spacing();
    ImGui::Spacing();
    if(ImGui::Checkbox("LOOP " ICON_FA_REDO,&loop)){
        if(loop){
            video->setLoopState(OF_LOOP_NORMAL);
        }else{
            video->setLoopState(OF_LOOP_NONE);
        }
    }

    #ifndef TARGET_WIN32
        ImGuiEx::ObjectInfo(
                "Simple object for playing video files. It use the HAP codec as standard, so you can use only videos encoded with HAP. More info here: https://hap.video/using-hap.html",
                "https://mosaic.d3cod3.org/reference.php?r=video-player", scaleFactor);
        // file dialog
        if(ImGuiEx::getFileDialog(fileDialog, loadVideoFlag, "Select a video file encoded with the HAP codec", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, "*.*", "", scaleFactor)){
            ofFile file (fileDialog.selected_path);
            if (file.exists()){
                filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
                isFileLoaded = false;
                needToLoadVideo = true;
            }
        }
    #else
        ImGuiEx::ObjectInfo(
                "Simple object for playing video files. In mac OSX you can upload .mov and .mp4 files; in linux .mp4, .mpeg and .mpg, while in windows .mp4 and .avi can be used.",
                "https://mosaic.d3cod3.org/reference.php?r=video-player", scaleFactor);
        // file dialog
        if(ImGuiEx::getFileDialog(fileDialog, loadVideoFlag, "Select a video file", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, "*.*", "", scaleFactor)){
            ofFile file (fileDialog.selected_path);
            if (file.exists()){
                filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
                isFileLoaded = false;
                needToLoadVideo = true;
            }
        }
    #endif


}

//--------------------------------------------------------------
void VideoPlayer::removeObjectContent(bool removeFileFromData){
    if(video->isLoaded()){
        video->stop();
        video->setVolume(0);
        video->close();
    }
    if(removeFileFromData){
        //removeFile(filepath);
    }
}

//--------------------------------------------------------------
void VideoPlayer::loadVideoFile(){
    if(filepath != "none"){
        filepath = forceCheckMosaicDataPath(filepath);
        isNewObject = false;
        //video->setUseTexture(false);
        video->load(filepath);

        ofFile tempFile(filepath);
        videoName = tempFile.getFileName();
        videoPath = tempFile.getAbsolutePath();
        videoRes = ofToString(video->getWidth())+"x"+ofToString(video->getHeight());

        this->saveConfig(false);
    }
}

OBJECT_REGISTER( VideoPlayer, "video player", OFXVP_OBJECT_CAT_TEXTURE)

#endif
