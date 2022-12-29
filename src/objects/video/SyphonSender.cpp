/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2021 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#if defined(TARGET_WIN32) || defined(TARGET_LINUX)
    // Unavailable on windows and linux
#elif !defined(OFXVP_BUILD_WITH_MINIMAL_OBJECTS)

#include "SyphonSender.h"

//--------------------------------------------------------------
SyphonSender::SyphonSender() : PatchObject("syphon sender"){

    this->numInlets  = 2;
    this->numOutlets = 0;

    _inletParams[0] = new ofTexture(); // input
    _inletParams[1] = new float();      // bang
    *(float *)&_inletParams[1] = 0.0f;

    this->initInletsState();

    isNewObject         = false;

    posX = posY = drawW = drawH = 0.0f;

    needToGrab          = false;

    isSending           = false;

    recButtonLabel      = "BROADCAST";

    this->setIsTextureObj(true);
}

//--------------------------------------------------------------
void SyphonSender::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"input");
}

//--------------------------------------------------------------
void SyphonSender::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    this->unusedArgs(mainWindow);

    individualTextureSyphonServer.setName("Mosaic Texture");

}

//--------------------------------------------------------------
void SyphonSender::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[1] && *(float *)&_inletParams[1] == 1.0f){
        if(!isSending){
            isSending = true;
            recButtonLabel = "STOP";
            ofLog(OF_LOG_NOTICE,"START NDI VIDEO SENDER");
        }else{
            isSending = false;
            recButtonLabel = "BROADCAST";
            ofLog(OF_LOG_NOTICE,"STOP NDI VIDEO SENDER");
        }

    }

}

//--------------------------------------------------------------
void SyphonSender::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

    if(this->inletsConnected[0]){
        if(static_cast<ofTexture *>(_inletParams[0])->isAllocated()){

            if(isSending) {
                individualTextureSyphonServer.publishTexture(static_cast<ofTexture *>(_inletParams[0]));
            }

            if(scaledObjW*canvasZoom > 90.0f){
                // draw node texture preview with OF
                drawNodeOFTexture(*static_cast<ofTexture *>(_inletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
            }

        }
    }else{

        if(scaledObjW*canvasZoom > 90.0f){
            ofSetColor(34,34,34);
            ofDrawRectangle(objOriginX - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/canvasZoom), objOriginY-(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor/canvasZoom),scaledObjW + (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/canvasZoom),scaledObjH + (((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor)/canvasZoom) );
        }

        needToGrab = false;
    }

}

//--------------------------------------------------------------
void SyphonSender::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        // get imgui node translated/scaled position/dimension for drawing textures in OF
        objOriginX = (ImGui::GetWindowPos().x + ((IMGUI_EX_NODE_PINS_WIDTH_NORMAL - 1)*this->scaleFactor) - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale();
        objOriginY = (ImGui::GetWindowPos().y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale();
        scaledObjW = this->width - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/_nodeCanvas.GetCanvasScale());
        scaledObjH = this->height - ((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor/_nodeCanvas.GetCanvasScale());


        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 window_size = ImGui::GetWindowSize();
        ImVec2 pos = ImVec2(window_pos.x + window_size.x - (30*this->scaleFactor), window_pos.y + (40*this->scaleFactor));
        if (isSending){
            _nodeCanvas.getNodeDrawList()->AddCircleFilled(pos, 10*this->scaleFactor, IM_COL32(255, 0, 0, 255), 40);
        }else{
            _nodeCanvas.getNodeDrawList()->AddCircleFilled(pos, 10*this->scaleFactor, IM_COL32(0, 255, 0, 255), 40);
        }

        _nodeCanvas.EndNodeContent();
    }

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();

}

//--------------------------------------------------------------
void SyphonSender::drawObjectNodeConfig(){
    ImGui::PushStyleColor(ImGuiCol_Button, VHS_RED);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, VHS_RED_OVER);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, VHS_RED_OVER);

    char tmp[256];
    sprintf_s(tmp,"%s %s",ICON_FA_CIRCLE, recButtonLabel.c_str());
    if(ImGui::Button(tmp,ImVec2(224*scaleFactor,26*scaleFactor))){
        if(!this->inletsConnected[0] || !static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
            ofLog(OF_LOG_WARNING,"There is no ofTexture connected to the object inlet, connect something if you want to export it as video!");
        }else{
            if(!isSending){
                isSending = true;
                recButtonLabel = "STOP";
                ofLog(OF_LOG_NOTICE,"START SYPHON VIDEO SENDER");
            }else{
                isSending = false;
                recButtonLabel = "BROADCAST";
                ofLog(OF_LOG_NOTICE,"STOP SYPHON VIDEO SENDER");
            }
        }
    }
    ImGui::PopStyleColor(3);

    ImGuiEx::ObjectInfo(
                "Sends a video broadcast (local network) to the video receiver object through Syphon.",
                "https://mosaic.d3cod3.org/reference.php?r=syphon-sender", scaleFactor);
}

//--------------------------------------------------------------
void SyphonSender::removeObjectContent(bool removeFileFromData){

}



OBJECT_REGISTER( SyphonSender, "syphon sender", OFXVP_OBJECT_CAT_TEXTURE)

#endif
