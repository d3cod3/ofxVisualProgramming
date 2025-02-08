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

#include "VideoCrop.h"

//--------------------------------------------------------------
VideoCrop::VideoCrop() : PatchObject("texture crop"){

    this->numInlets  = 5;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();  // input
    _inletParams[1] = new float();      // x
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]) = 0.0f;
    _inletParams[2] = new float();      // y
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[2]) = 0.0f;
    _inletParams[3] = new float();      // w
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[3]) = 0.0f;
    _inletParams[4] = new float();      // h
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[4]) = 0.0f;

    _outletParams[0] = new ofTexture(); // output

    this->initInletsState();

    croppedFbo  = new ofFbo();
    needToGrab  = false;

    posX = posY = drawW = drawH = 0.0f;

    _x = 0.0f;
    _y = 0.0f;

    _w = 100.0f;
    _h = 100.0f;

    _maxW = STANDARD_TEXTURE_WIDTH;
    _maxH = STANDARD_TEXTURE_HEIGHT;

    loaded      = false;

    this->setIsTextureObj(true);
    this->setIsResizable(true);

    prevW                   = this->width;
    prevH                   = this->height;

}

//--------------------------------------------------------------
void VideoCrop::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"input");
    this->addInlet(VP_LINK_NUMERIC,"x");
    this->addInlet(VP_LINK_NUMERIC,"y");
    this->addInlet(VP_LINK_NUMERIC,"width");
    this->addInlet(VP_LINK_NUMERIC,"height");

    this->addOutlet(VP_LINK_TEXTURE,"croppedOutput");

    this->setCustomVar(_x,"POSX");
    this->setCustomVar(_y,"POSY");
    this->setCustomVar(_w,"WIDTH");
    this->setCustomVar(_h,"HEIGHT");

    this->setCustomVar(prevW,"OBJ_WIDTH");
    this->setCustomVar(prevH,"OBJ_HEIGHT");
}

//--------------------------------------------------------------
void VideoCrop::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);
}

//--------------------------------------------------------------
void VideoCrop::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(this->inletsConnected[1] && ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated()){
        _x = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]),0.0f,100.0f);
    }

    if(this->inletsConnected[2] && ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated()){
        _y = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[2]),0.0f,100.0f);
    }

    if(this->inletsConnected[3] && ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated()){
        _w = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[3]),0.0f,100.0f);
    }

    if(this->inletsConnected[4] && ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated()){
        _h = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[4]),0.0f,100.0f);
    }

    // UPDATE
    if(this->inletsConnected[0]){
        if(ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->isAllocated()){
            if(!needToGrab){
                needToGrab = true;
                ofDisableArbTex();
                croppedFbo->allocate(ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->getWidth(), ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->getHeight(), GL_RGBA );
                ofEnableArbTex();
                _maxW = ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->getWidth();
                _maxH = ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->getHeight();
            }

            croppedFbo->begin();
            ofClear(0,0,0,255);
            bounds.set((_x/100.0f)*_maxW,(_y/100.0f)*_maxH,(_w/100.0f)*_maxW,(_h/100)*_maxH);
            ofSetColor(255);
            drawTextureCropInsideRect(ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0]),0,0,ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->getWidth(),ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0])->getHeight(),bounds);
            croppedFbo->end();

            *ofxVP_CAST_PIN_PTR<ofTexture>(_outletParams[0]) = croppedFbo->getTexture();
        }
    }else{
        needToGrab = false;
    }

    if(!loaded){
        loaded = true;
        _x = this->getCustomVar("XPOS");
        _y = this->getCustomVar("YPOS");
        _w = this->getCustomVar("WIDTH");
        _h = this->getCustomVar("HEIGHT");
        prevW = this->getCustomVar("OBJ_WIDTH");
        prevH = this->getCustomVar("OBJ_HEIGHT");
        this->width             = prevW;
        this->height            = prevH;
    }
    
}

//--------------------------------------------------------------
void VideoCrop::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

}

//--------------------------------------------------------------
void VideoCrop::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
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
        _nodeCanvas.getNodeDrawList()->AddRectFilled(window_pos,window_pos+ImVec2(scaledObjW*this->scaleFactor*_nodeCanvas.GetCanvasScale(), scaledObjH*this->scaleFactor*_nodeCanvas.GetCanvasScale()),ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)));
        if(ofxVP_CAST_PIN_PTR<ofTexture>(_outletParams[0])->isAllocated()){
            calcTextureDims(*ofxVP_CAST_PIN_PTR<ofTexture>(_outletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
            ImGui::SetCursorPos(ImVec2(posX+(IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor), posY+(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor)));
            ImGui::Image((ImTextureID)(uintptr_t)ofxVP_CAST_PIN_PTR<ofTexture>(_outletParams[0])->getTextureData().textureID, ImVec2(drawW, drawH));
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
void VideoCrop::drawObjectNodeConfig(){
    ImGui::Spacing();
    ImGui::PushItemWidth(130*this->scaleFactor);
    if(ImGui::SliderFloat("START X %",&_x, 0.0f, 100.0f)){
        this->setCustomVar(_x,"XPOS");
    }
    if(ImGui::SliderFloat("START Y %",&_y, 0.0f, 100.0f)){
        this->setCustomVar(_y,"YPOS");
    }
    if(ImGui::SliderFloat("WIDTH %",&_w, 0, 100.0f)){
        this->setCustomVar(_w,"WIDTH");
    }
    if(ImGui::SliderFloat("HEIGHT %",&_h, 0, 100.0f)){
        this->setCustomVar(_h,"HEIGHT");
    }
    ImGui::PopItemWidth();

    ImGuiEx::ObjectInfo(
                "Basic texture crop.",
                "https://mosaic.d3cod3.org/reference.php?r=video-crop", scaleFactor);
}

//--------------------------------------------------------------
void VideoCrop::removeObjectContent(bool removeFileFromData){
    
}

//--------------------------------------------------------------
void VideoCrop::drawTextureCropInsideRect(ofTexture *texture,float x, float y, float w, float h,ofRectangle &bounds){

    ofRectangle tex = ofRectangle(x,y,w,h);
    ofRectangle intersection = getIntersection(bounds,tex);

    if (intersection.width == 0.0f && intersection.height == 0.0f){
        return;
    }

    ofRectangle texCoordsCrop;

    float signW = texture->texData.width / w; // w and h already include negative values, so it handles the mirroring "automatically"
    float signH = texture->texData.height / h;
    texCoordsCrop.width = (intersection.width);
    texCoordsCrop.height = (intersection.height);

    if(ofGetRectMode() == OF_RECTMODE_CORNER){
        texCoordsCrop.x = intersection.x - tex.x;
        texCoordsCrop.y = intersection.y - tex.y;
    }else{
        texCoordsCrop.x = intersection.x - tex.x + tex.width/2 - intersection.width/2;
        texCoordsCrop.y = intersection.y - tex.y + tex.height/2 - intersection.height/2;
    }

    texture->drawSubsection(intersection.x, intersection.y,intersection.width, intersection.height,signW * texCoordsCrop.x ,signH * texCoordsCrop.y,signW * texCoordsCrop.width,signH * texCoordsCrop.height);

}

//--------------------------------------------------------------
ofRectangle VideoCrop::getIntersection(ofRectangle & r1,ofRectangle & r2){

    if(ofGetRectMode() == OF_RECTMODE_CORNER){
        return r1.getIntersection(r2);
    }else{
        ofRectangle rect0 = r1;
        ofRectangle rect1 = r2;

        rect0.x -= rect0.width * 0.5;
        rect0.y -= rect0.height * 0.5;
        rect1.x -= rect1.width * 0.5;
        rect1.y -= rect1.height * 0.5;

        ofRectangle result = rect0.getIntersection(rect1);

        result.x += result.width * 0.5;
        result.y += result.height * 0.5;

        return result;
    }
}

OBJECT_REGISTER( VideoCrop, "texture crop", OFXVP_OBJECT_CAT_TEXTURE)

#endif
