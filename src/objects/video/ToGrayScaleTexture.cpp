/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2020 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "ToGrayScaleTexture.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
ToGrayScaleTexture::ToGrayScaleTexture() : PatchObject("to grayscale texture"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();  // input

    _outletParams[0] = new ofTexture(); // output

    this->initInletsState();

    resetTextures(320,240);

    posX = posY = drawW = drawH = 0.0f;

    newConnection       = false;

    prevW               = this->width;
    prevH               = this->height;
    loaded              = false;

    this->setIsTextureObj(true);
    this->setIsResizable(true);

}

//--------------------------------------------------------------
void ToGrayScaleTexture::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"rgbTexture");

    this->addOutlet(VP_LINK_TEXTURE,"grayscaleTexture");

    this->setCustomVar(static_cast<float>(prevW),"WIDTH");
    this->setCustomVar(static_cast<float>(prevH),"HEIGHT");

}

//--------------------------------------------------------------
void ToGrayScaleTexture::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);
}

//--------------------------------------------------------------
void ToGrayScaleTexture::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    // UPDATE STUFF
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(!newConnection){
            newConnection = true;
            resetTextures(static_cast<int>(floor(static_cast<ofTexture *>(_inletParams[0])->getWidth())),static_cast<int>(floor(static_cast<ofTexture *>(_inletParams[0])->getHeight())));
        }

        static_cast<ofTexture *>(_inletParams[0])->readToPixels(*pix);

        colorImg->setFromPixels(*pix);
        colorImg->updateTexture();

        *grayImg = *colorImg;
        grayImg->updateTexture();

        *static_cast<ofTexture *>(_outletParams[0]) = grayImg->getTexture();

    }else if(!this->inletsConnected[0]){
        newConnection = false;
    }

    if(!loaded){
        loaded = true;
        prevW = this->getCustomVar("WIDTH");
        prevH = this->getCustomVar("HEIGHT");
        this->width             = prevW;
        this->height            = prevH;
    }
}

//--------------------------------------------------------------
void ToGrayScaleTexture::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

}

//--------------------------------------------------------------
void ToGrayScaleTexture::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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
void ToGrayScaleTexture::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "RGB to Grayscale texture conversion.",
                "https://mosaic.d3cod3.org/reference.php?r=to-grayscale-texture", scaleFactor);
}

//--------------------------------------------------------------
void ToGrayScaleTexture::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void ToGrayScaleTexture::resetTextures(int w, int h){

    pix             = new ofPixels();
    pix->allocate(static_cast<size_t>(w),static_cast<size_t>(h),1);

    colorImg        = new ofxCvColorImage();
    grayImg         = new ofxCvGrayscaleImage();

    ofDisableArbTex();

    colorImg->allocate(w,h);
    grayImg->allocate(w,h);

    ofEnableArbTex();

}


OBJECT_REGISTER( ToGrayScaleTexture, "to grayscale texture", OFXVP_OBJECT_CAT_TEXTURE)

#endif
