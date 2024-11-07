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

#include "VideoGrabber.h"

//--------------------------------------------------------------
VideoGrabber::VideoGrabber() :
            PatchObject("video grabber")

{

    this->numInlets  = 0;
    this->numOutlets = 1;

    _outletParams[0] = new ofTexture(); // output

    this->initInletsState();

    vidGrabber  = new ofVideoGrabber();
    colorImage  = new ofxCvColorImage();

    isNewObject         = false;

    posX = posY = drawW = drawH = 0.0f;

    camWidth            = 1280;
    camHeight           = 720;
    temp_width          = camWidth;
    temp_height         = camHeight;

    deviceID            = 0;
    deviceName          = "NO DEVICE AVAILABLE";

    hMirror             = false;
    vMirror             = false;

    needReset               = false;
    isOneDeviceAvailable    = false;

    loaded                  = false;

    this->setIsResizable(true);
    this->setIsTextureObj(true);

    prevW                   = this->width;
    prevH                   = this->height;
}

//--------------------------------------------------------------
void VideoGrabber::newObject(){
    PatchObject::setName( this->objectName );

    this->addOutlet(VP_LINK_TEXTURE,"deviceImage");

    this->setCustomVar(static_cast<float>(camWidth),"CAM_WIDTH");
    this->setCustomVar(static_cast<float>(camHeight),"CAM_HEIGHT");
    this->setCustomVar(static_cast<float>(deviceID),"DEVICE_ID");
    this->setCustomVar(static_cast<float>(0.0),"MIRROR_H");
    this->setCustomVar(static_cast<float>(0.0),"MIRROR_V");

    this->setCustomVar(prevW,"OBJ_WIDTH");
    this->setCustomVar(prevH,"OBJ_HEIGHT");
}

//--------------------------------------------------------------
void VideoGrabber::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    wdevices = vidGrabber->listDevices();
    for(int i=0;i<static_cast<int>(wdevices.size());i++){
        if(wdevices[i].bAvailable){
            isOneDeviceAvailable = true;
            devicesVector.push_back(wdevices[i].deviceName);
            devicesID.push_back(i);

            for(size_t f=0;f<wdevices[i].formats.size();f++){
                ofLog(OF_LOG_NOTICE,"Capture Device format vailable: %ix%i",wdevices[i].formats.at(f).width,wdevices[i].formats.at(f).height);
            }

        }
    }

}

//--------------------------------------------------------------
void VideoGrabber::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(needReset && isOneDeviceAvailable){
        resetCameraSettings(deviceID);
    }

    if(vidGrabber->isInitialized()){
        vidGrabber->update();
        if(vidGrabber->isFrameNew()){
            colorImage->setFromPixels(vidGrabber->getPixels());
            colorImage->mirror(vMirror,hMirror);
            colorImage->updateTexture();

            *static_cast<ofTexture *>(_outletParams[0]) = colorImage->getTexture();
        }
    }

    if(!loaded){
        loaded = true;
        camWidth = static_cast<int>(floor(this->getCustomVar("CAM_WIDTH")));
        camHeight = static_cast<int>(floor(this->getCustomVar("CAM_HEIGHT")));
        deviceID = static_cast<int>(floor(this->getCustomVar("DEVICE_ID")));
        hMirror = static_cast<bool>(floor(this->getCustomVar("MIRROR_H")));
        vMirror = static_cast<bool>(floor(this->getCustomVar("MIRROR_V")));
        prevW = this->getCustomVar("OBJ_WIDTH");
        prevH = this->getCustomVar("OBJ_HEIGHT");
        this->width             = prevW;
        this->height            = prevH;
        if(isOneDeviceAvailable){

            loadCameraSettings();

            vidGrabber->setDeviceID(deviceID);
            vidGrabber->setup(camWidth, camHeight);
            resetCameraSettings(deviceID);

        }
    }

}

//--------------------------------------------------------------
void VideoGrabber::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);
}

//--------------------------------------------------------------
void VideoGrabber::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        calcTextureDims(*static_cast<ofTexture *>(_outletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
        if(isOneDeviceAvailable){
            if(vidGrabber->isInitialized() && !needReset){
                if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
                    _nodeCanvas.getNodeDrawList()->AddRectFilled(window_pos,window_pos+ImVec2(scaledObjW*this->scaleFactor*_nodeCanvas.GetCanvasScale(), scaledObjH*this->scaleFactor*_nodeCanvas.GetCanvasScale()),ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)));
                    ImGui::SetCursorPos(ImVec2(posX+(IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor), posY+(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor)));
                    ImGui::Image((ImTextureID)(uintptr_t)static_cast<ofTexture *>(_outletParams[0])->getTextureData().textureID, ImVec2(drawW, drawH));
                }else{
                    _nodeCanvas.getNodeDrawList()->AddRectFilled(window_pos,window_pos+ImVec2(scaledObjW*this->scaleFactor*_nodeCanvas.GetCanvasScale(), scaledObjH*this->scaleFactor*_nodeCanvas.GetCanvasScale()),ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)));
                }
            }
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
void VideoGrabber::drawObjectNodeConfig(){
    ImGui::Spacing();
    ImGui::Text("%s",deviceName.c_str());
    ImGui::Text("Format: %ix%i",camWidth,camHeight);

    ImGui::Spacing();
    if(ImGui::BeginCombo("Device", static_cast<int>(devicesVector.size())>deviceID ? devicesVector.at(deviceID).c_str() : deviceName.c_str() )){
        for(int i=0; i < static_cast<int>(devicesVector.size()); ++i){
            bool is_selected = (deviceID == i );
            if (ImGui::Selectable(devicesVector.at(i).c_str(), is_selected)){
                resetCameraSettings(i);
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    ImGui::Spacing();
    if(ImGui::Checkbox("HORIZONTAL MIRROR",&hMirror)){
        this->setCustomVar(static_cast<float>(hMirror),"MIRROR_H");
    }
    ImGui::Spacing();
    if(ImGui::Checkbox("VERTICAL MIRROR",&vMirror)){
        this->setCustomVar(static_cast<float>(vMirror),"MIRROR_V");
    }

    ImGui::Spacing();
    if(ImGui::InputInt("Width",&temp_width)){
        if(temp_width > CAM_MAX_WIDTH){
            temp_width = camWidth;
        }
    }
    ImGui::SameLine(); ImGuiEx::HelpMarker("You can set a supported resolution WxH (limited for now at max. 1920x1080)");

    if(ImGui::InputInt("Height",&temp_height)){
        if(temp_height > CAM_MAX_HEIGHT){
            temp_height = camHeight;
        }
    }
    ImGui::Spacing();
    if(ImGui::Button("APPLY",ImVec2(224*scaleFactor,26*scaleFactor))){
        needReset = true;
    }

    ImGuiEx::ObjectInfo(
                "Opens a compatible video input/webcam device",
                "https://mosaic.d3cod3.org/reference.php?r=video-grabber", scaleFactor);
}

//--------------------------------------------------------------
void VideoGrabber::removeObjectContent(bool removeFileFromData){
    vidGrabber->close();
}

//--------------------------------------------------------------
void VideoGrabber::loadCameraSettings(){
    if(static_cast<int>(floor(this->getCustomVar("CAM_WIDTH"))) > 0){
        camWidth = static_cast<int>(floor(this->getCustomVar("CAM_WIDTH")));
        camHeight = static_cast<int>(floor(this->getCustomVar("CAM_HEIGHT")));

        temp_width      = camWidth;
        temp_height     = camHeight;

        ofDisableArbTex();

        colorImage    = new ofxCvColorImage();
        colorImage->allocate(camWidth,camHeight);

        _outletParams[0] = new ofTexture();
        static_cast<ofTexture *>(_outletParams[0])->allocate(camWidth,camHeight,GL_RGB);

        ofEnableArbTex();

    }

    if(static_cast<int>(floor(this->getCustomVar("DEVICE_ID"))) >= 0 && static_cast<int>(floor(this->getCustomVar("DEVICE_ID"))) < static_cast<int>(devicesVector.size())){
        deviceID = static_cast<int>(floor(this->getCustomVar("DEVICE_ID")));
    }else{
        deviceID = 0;
        this->setCustomVar(static_cast<float>(deviceID),"DEVICE_ID");
    }

    if(isOneDeviceAvailable){
        deviceName = devicesVector[deviceID];
    }

}

//--------------------------------------------------------------
void VideoGrabber::resetCameraSettings(int devID){

    if(camWidth != temp_width || camHeight != temp_height || devID!=deviceID){

        if(devID!=deviceID){
            ofLog(OF_LOG_NOTICE,"Changing Device to: %s",devicesVector[devID].c_str());

            deviceID = devID;
            deviceName = devicesVector[deviceID];
            this->setCustomVar(static_cast<float>(deviceID),"DEVICE_ID");
        }


        if(camWidth != temp_width || camHeight != temp_height){
            camWidth = temp_width;
            camHeight = temp_height;

            ofLog(OF_LOG_NOTICE,"Changing %s Capture dimensions to: %ix%i",devicesVector[devID].c_str(),camWidth,camHeight);

            this->setCustomVar(static_cast<float>(camWidth),"CAM_WIDTH");
            this->setCustomVar(static_cast<float>(camHeight),"CAM_HEIGHT");

            ofDisableArbTex();

            colorImage    = new ofxCvColorImage();
            colorImage->allocate(camWidth,camHeight);

            _outletParams[0] = new ofTexture();
            static_cast<ofTexture *>(_outletParams[0])->allocate(camWidth,camHeight,GL_RGB);

            ofEnableArbTex();

        }

        if(vidGrabber->isInitialized()){
            vidGrabber->close();

            vidGrabber = new ofVideoGrabber();
            vidGrabber->setDeviceID(deviceID);
            vidGrabber->setup(camWidth, camHeight);
        }
    }

    this->saveConfig(false);

    needReset = false;
}

OBJECT_REGISTER( VideoGrabber, "video grabber", OFXVP_OBJECT_CAT_TEXTURE)

#endif
