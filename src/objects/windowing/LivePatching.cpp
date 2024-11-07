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

#include "LivePatching.h"

//--------------------------------------------------------------
LivePatching::LivePatching() : PatchObject("live patching"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();  // texture
    _inletParams[1] = new float();  // alpha
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new float();  // alpha
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    posX = posY = drawW = drawH = 0.0f;

    this->setIsTextureObj(true);
    this->setIsHardwareObj(true);
    this->setIsResizable(true);

    alpha   = 127.0f;

    loaded  = false;

}

//--------------------------------------------------------------
void LivePatching::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"texture");
    this->addInlet(VP_LINK_NUMERIC,"alpha");

    this->setCustomVar(alpha,"ALPHA");
}

//--------------------------------------------------------------
void LivePatching::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);
}

//--------------------------------------------------------------
void LivePatching::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[1]){
        alpha = ofClamp(*(float *)&_inletParams[1],0.0f,255.0f);
    }

    *(float *)&_outletParams[0] = alpha;

    if(!loaded){
        loaded = true;
        alpha = this->getCustomVar("ALPHA");
    }
}

//--------------------------------------------------------------
void LivePatching::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);
}

//--------------------------------------------------------------
void LivePatching::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
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

        if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
            calcTextureDims(*static_cast<ofTexture *>(_inletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
            _nodeCanvas.getNodeDrawList()->AddRectFilled(window_pos,window_pos+ImVec2(scaledObjW*this->scaleFactor*_nodeCanvas.GetCanvasScale(), scaledObjH*this->scaleFactor*_nodeCanvas.GetCanvasScale()),ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)));
            ImGui::SetCursorPos(ImVec2(posX+(IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor), posY+(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor)));
            ImGui::Image((ImTextureID)(uintptr_t)static_cast<ofTexture *>(_inletParams[0])->getTextureData().textureID, ImVec2(drawW, drawH));
        }else{
            _nodeCanvas.getNodeDrawList()->AddRectFilled(window_pos,window_pos+ImVec2(scaledObjW*this->scaleFactor*_nodeCanvas.GetCanvasScale(), scaledObjH*this->scaleFactor*_nodeCanvas.GetCanvasScale()),ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)));
        }

        // get imgui node translated/scaled position/dimension for drawing textures in OF
        //objOriginX = (ImGui::GetWindowPos().x + ((IMGUI_EX_NODE_PINS_WIDTH_NORMAL - 1)*this->scaleFactor) - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale();
        //objOriginY = (ImGui::GetWindowPos().y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale();
        scaledObjW = this->width - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL+IMGUI_EX_NODE_PINS_WIDTH_SMALL)*this->scaleFactor/_nodeCanvas.GetCanvasScale();
        scaledObjH = this->height - (IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor/_nodeCanvas.GetCanvasScale();

        _nodeCanvas.EndNodeContent();
    }

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();
}

//--------------------------------------------------------------
void LivePatching::drawObjectNodeConfig(){
    ImGui::Spacing();

    if(ImGui::SliderFloat("Alpha",&alpha,0.0f,255.0f)){
        this->setCustomVar(alpha,"ALPHA");
    }

    ImGuiEx::ObjectInfo(
                "Displays real-time texture in the background patch, with alpha so you can work with the objects, for live sessions.",
                "https://mosaic.d3cod3.org/reference.php?r=live-patching", scaleFactor);
}

//--------------------------------------------------------------
void LivePatching::removeObjectContent(bool removeFileFromData){
    
}

OBJECT_REGISTER( LivePatching, "live patching", OFXVP_OBJECT_CAT_WINDOWING)

#endif
