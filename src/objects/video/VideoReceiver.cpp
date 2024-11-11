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

#if defined(TARGET_WIN32)
    // Unavailable on windows.
#elif !defined(OFXVP_BUILD_WITH_MINIMAL_OBJECTS)

#include "VideoReceiver.h"

//--------------------------------------------------------------
VideoReceiver::VideoReceiver() : PatchObject("video receiver"){

    this->numInlets  = 0;
    this->numOutlets = 1;

    _outletParams[0] = new ofTexture(); // input

    this->initInletsState();

    posX = posY = drawW = drawH = 0.0f;

    sourceID            = 0;
    sourceName          = "NO SOURCE AVAILABLE";

    needToGrab              = false;
    isOneSourceAvailable    = false;

    loaded                  = false;

    this->setIsTextureObj(true);
    this->setIsResizable(true);

    prevW                   = this->width;
    prevH                   = this->height;

}

//--------------------------------------------------------------
void VideoReceiver::newObject(){
    PatchObject::setName( this->objectName );

    this->addOutlet(VP_LINK_TEXTURE,"textureReceived");

    this->setCustomVar(static_cast<float>(sourceID),"SOURCE_ID");

    this->setCustomVar(prevW,"OBJ_WIDTH");
    this->setCustomVar(prevH,"OBJ_HEIGHT");
}

//--------------------------------------------------------------
void VideoReceiver::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    NDIlib_initialize();
    finder_.watchSources();

}

//--------------------------------------------------------------
void VideoReceiver::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    // Update SourcesNDI
    auto sources = finder_.getSources();
    sourcesVector = std::accumulate(std::begin(sources), std::end(sources), vector<std::string>(), [](vector<std::string> result, const ofxNDI::Source &src) {
            result.push_back(ofToString(result.size()+1, 2, '0')+". "+src.ndi_name+"("+src.url_address+")");
            return result;
    });
    sourcesID.assign(sourcesVector.size(),0);
    for(size_t i=0;i<sourcesVector.size();i++){
        sourcesID.at(i) = i;
    }

    // check if at least one is available
    if(sourcesVector.size()>0){
        isOneSourceAvailable = true;
    }else{
        isOneSourceAvailable = false;
    }

    // connect source
    if(needToGrab && ndiReceiver.isConnected() && isOneSourceAvailable){
        needToGrab = false;
        auto sources = finder_.getSources();
        if(sources.size() > sourceID) {
            if(ndiReceiver.isSetup() ? (ndiReceiver.changeConnection(sources[sourceID]), true) : ndiReceiver.setup(sources[sourceID])) {
                video_.setup(ndiReceiver);
            }
        }

    }

    // receive source data
    if(ndiReceiver.isConnected()) {
        video_.update();
        if(video_.isFrameNew()) {
            video_.decodeTo(pixels_);
            if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
                static_cast<ofTexture *>(_outletParams[0])->loadData(pixels_);
            }else{
                static_cast<ofTexture *>(_outletParams[0])->allocate(pixels_.getWidth(), pixels_.getHeight(), GL_RGB );
            }
        }
    }

    // init object vars
    if(!loaded){
        loaded = true;

        sourceID = static_cast<int>(floor(this->getCustomVar("SOURCE_ID")));

        prevW = this->getCustomVar("OBJ_WIDTH");
        prevH = this->getCustomVar("OBJ_HEIGHT");
        this->width             = prevW;
        this->height            = prevH;
    }
}

//--------------------------------------------------------------
void VideoReceiver::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

}

//--------------------------------------------------------------
void VideoReceiver::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
            calcTextureDims(*static_cast<ofTexture *>(_outletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
            _nodeCanvas.getNodeDrawList()->AddRectFilled(window_pos,window_pos+ImVec2(scaledObjW*this->scaleFactor*_nodeCanvas.GetCanvasScale(), scaledObjH*this->scaleFactor*_nodeCanvas.GetCanvasScale()),ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)));
            ImGui::SetCursorPos(ImVec2(posX+(IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor), posY+(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor)));
            ImGui::Image((ImTextureID)(uintptr_t)static_cast<ofTexture *>(_outletParams[0])->getTextureData().textureID, ImVec2(drawW, drawH));
        }else{
            _nodeCanvas.getNodeDrawList()->AddRectFilled(window_pos,window_pos+ImVec2(scaledObjW*this->scaleFactor*_nodeCanvas.GetCanvasScale(), scaledObjH*this->scaleFactor*_nodeCanvas.GetCanvasScale()),ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)));
        }

        // get imgui node translated/scaled position/dimension for drawing textures in OF
        //objOriginX = (ImGui::GetWindowPos().x + ((IMGUI_EX_NODE_PINS_WIDTH_NORMAL - 1)*this->scaleFactor) - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale();
        //objOriginY = (ImGui::GetWindowPos().y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale();
        scaledObjW = this->width - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL+IMGUI_EX_NODE_PINS_WIDTH_SMALL)*this->scaleFactor/_nodeCanvas.GetCanvasScale();
        scaledObjH = this->height - (IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor/_nodeCanvas.GetCanvasScale();

        // save object dimensions (for resizable ones)
        if(this->width != prevW){
            prevW = this->width;
            this->setCustomVar(prevW,"OBJ_WIDTH");
        }
        if(this->height != prevH){
            prevH = this->height;
            this->setCustomVar(prevH,"OBJ_HEIGHT");
        }

        _nodeCanvas.EndNodeContent();
    }

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();

}

//--------------------------------------------------------------
void VideoReceiver::drawObjectNodeConfig(){
    ImGui::Spacing();

    if(ndiReceiver.isConnected() && static_cast<ofTexture *>(_outletParams[0])->isAllocated()) {
        ImGui::Text("%s",sourceName.c_str());
        ImGui::Text("Format: %ix%i",static_cast<int>(static_cast<ofTexture *>(_outletParams[0])->getWidth()),static_cast<int>(static_cast<ofTexture *>(_outletParams[0])->getHeight()));
    }

    ImGui::Spacing();
    if(ImGui::BeginCombo("Source", static_cast<int>(sourcesVector.size())>sourceID ? sourcesVector.at(sourceID).c_str() : sourceName.c_str() )){
        for(int i=0; i < static_cast<int>(sourcesVector.size()); ++i){
            bool is_selected = (sourceID == i );
            if (ImGui::Selectable(sourcesVector.at(i).c_str(), is_selected)){
                sourceName = sourcesVector.at(i);
                sourceID = i;
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    ImGui::Spacing();
    if(ImGui::Button("APPLY",ImVec2(224*scaleFactor,26*scaleFactor))){
        needToGrab = true;
    }

    ImGuiEx::ObjectInfo(
                "Receive broadcast (local network) video from the video sender object through the NDI (Network Device Interface)communication protocol.",
                "https://mosaic.d3cod3.org/reference.php?r=video-sender", scaleFactor);
}

//--------------------------------------------------------------
void VideoReceiver::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
}

OBJECT_REGISTER( VideoReceiver, "video receiver", OFXVP_OBJECT_CAT_TEXTURE)

#endif
