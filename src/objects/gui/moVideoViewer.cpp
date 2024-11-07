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

#include "moVideoViewer.h"

//--------------------------------------------------------------
moVideoViewer::moVideoViewer() : PatchObject("video viewer"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();  // texture

    _outletParams[0] = new ofTexture();  // texture

    kuro        = new ofImage();

    posX = posY = drawW = drawH = 0.0f;

    this->width             *= 2;
    this->height            *= 2;

    this->initInletsState();

    this->setIsResizable(true);
    this->setIsTextureObj(true);

    prevW                   = this->width;
    prevH                   = this->height;

    loaded                  = false;

}

//--------------------------------------------------------------
void moVideoViewer::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"texture");
    this->addOutlet(VP_LINK_TEXTURE,"texture");

    this->setCustomVar(static_cast<float>(prevW),"WIDTH");
    this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
}

//--------------------------------------------------------------
void moVideoViewer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    // load kuro
    ofDisableArbTex();
    kuro->load("images/kuro.jpg");
    ofEnableArbTex();
}

//--------------------------------------------------------------
void moVideoViewer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(!loaded){
        loaded = true;
        prevW = this->getCustomVar("WIDTH");
        prevH = this->getCustomVar("HEIGHT");
        this->width             = prevW;
        this->height            = prevH;
    }
}

//--------------------------------------------------------------
void moVideoViewer::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

    ofSetColor(255);

    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        *static_cast<ofTexture *>(_outletParams[0]) = *static_cast<ofTexture *>(_inletParams[0]);
    }else{
        *static_cast<ofTexture *>(_outletParams[0]) = kuro->getTexture();
    }

}

//--------------------------------------------------------------
void moVideoViewer::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGui::SetCursorPos(ImVec2(IMGUI_EX_NODE_PINS_WIDTH_NORMAL, IMGUI_EX_NODE_HEADER_HEIGHT));
        if(this->inletsConnected[0] && static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
            calcTextureDims(*static_cast<ofTexture *>(_outletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
            ImGui::Image(kuro->getTexture().getTextureData().textureID, ImVec2(this->width-IMGUI_EX_NODE_PINS_WIDTH_NORMAL-IMGUI_EX_NODE_PINS_WIDTH_SMALL,this->height-IMGUI_EX_NODE_HEADER_HEIGHT-IMGUI_EX_NODE_FOOTER_HEIGHT));
            ImGui::SetCursorPos(ImVec2(posX+(IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor), posY+(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor)));
            ImGui::Image((ImTextureID)(uintptr_t)static_cast<ofTexture *>(_outletParams[0])->getTextureData().textureID, ImVec2(drawW, drawH));
        }else{
            ImGui::Image(kuro->getTexture().getTextureData().textureID, ImVec2(this->width-IMGUI_EX_NODE_PINS_WIDTH_NORMAL-IMGUI_EX_NODE_PINS_WIDTH_SMALL,this->height-IMGUI_EX_NODE_HEADER_HEIGHT-IMGUI_EX_NODE_FOOTER_HEIGHT));
        }

        // get imgui node translated/scaled position/dimension for drawing textures in OF
        //objOriginX = (ImGui::GetWindowPos().x + ((IMGUI_EX_NODE_PINS_WIDTH_NORMAL - 1)*this->scaleFactor) - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale();
        //objOriginY = (ImGui::GetWindowPos().y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale();
        scaledObjW = this->width - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL+IMGUI_EX_NODE_PINS_WIDTH_SMALL)*this->scaleFactor/_nodeCanvas.GetCanvasScale();
        scaledObjH = this->height - (IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor/_nodeCanvas.GetCanvasScale();

        if(this->width != prevW){
            prevW = this->width;
            this->setCustomVar(static_cast<float>(prevW),"WIDTH");
        }
        if(this->height != prevH){
            prevH = this->height;
            this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
        }

        _nodeCanvas.EndNodeContent();
    }

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();

}

//--------------------------------------------------------------
void moVideoViewer::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "A basic video viewer, the object visualizes it, and bypasses it by its outlet.",
                "https://mosaic.d3cod3.org/reference.php?r=video-viewer", scaleFactor);
}

//--------------------------------------------------------------
void moVideoViewer::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
}

OBJECT_REGISTER( moVideoViewer, "video viewer", OFXVP_OBJECT_CAT_GUI)

#endif
