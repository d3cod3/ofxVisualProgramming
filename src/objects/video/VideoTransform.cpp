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

#include "VideoTransform.h"

//--------------------------------------------------------------
VideoTransform::VideoTransform() : PatchObject("texture transform"){

    this->numInlets  = 8;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();  // input
    _inletParams[1] = new float();      // x
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();      // y
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();      // w
    *(float *)&_inletParams[3] = 0.0f;
    _inletParams[4] = new float();      // h
    *(float *)&_inletParams[4] = 0.0f;
    _inletParams[5] = new float();      // angleX
    *(float *)&_inletParams[5] = 0.0f;
    _inletParams[6] = new float();      // angleY
    *(float *)&_inletParams[6] = 0.0f;
    _inletParams[7] = new float();      // angleZ
    *(float *)&_inletParams[7] = 0.0f;

    _outletParams[0] = new ofTexture(); // output

    this->initInletsState();

    this->width             *= 2;
    this->height            *= 2;

    scaledFbo  = new ofFbo();
    needToGrab  = false;

    posX = posY = drawW = drawH = 0.0f;

    _x = 0.0f;
    _y = 0.0f;

    _w = STANDARD_TEXTURE_WIDTH;
    _h = STANDARD_TEXTURE_HEIGHT;

    _maxW = STANDARD_TEXTURE_WIDTH;
    _maxH = STANDARD_TEXTURE_HEIGHT;

    angleX = 0.0f;
    angleY = 0.0f;
    angleZ = 0.0f;

    loaded      = false;

    this->setIsTextureObj(true);
    this->setIsResizable(true);

    prevW                   = this->width;
    prevH                   = this->height;

}

//--------------------------------------------------------------
void VideoTransform::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"input");
    this->addInlet(VP_LINK_NUMERIC,"x");
    this->addInlet(VP_LINK_NUMERIC,"y");
    this->addInlet(VP_LINK_NUMERIC,"width");
    this->addInlet(VP_LINK_NUMERIC,"height");
    this->addInlet(VP_LINK_NUMERIC,"angleX");
    this->addInlet(VP_LINK_NUMERIC,"angleY");
    this->addInlet(VP_LINK_NUMERIC,"angleZ");

    this->addOutlet(VP_LINK_TEXTURE,"transformedOutput");

    this->setCustomVar(_x,"XPOS");
    this->setCustomVar(_y,"YPOS");
    this->setCustomVar(STANDARD_TEXTURE_WIDTH,"WIDTH");
    this->setCustomVar(STANDARD_TEXTURE_HEIGHT,"HEIGHT");
    this->setCustomVar(angleX,"ANGLEX");
    this->setCustomVar(angleY,"ANGLEY");
    this->setCustomVar(angleZ,"ANGLEZ");

    this->setCustomVar(prevW,"OBJ_WIDTH");
    this->setCustomVar(prevH,"OBJ_HEIGHT");
}

//--------------------------------------------------------------
void VideoTransform::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void VideoTransform::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[0]){
        if(static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
            if(!needToGrab){
                needToGrab = true;
                scaledFbo->allocate(static_cast<ofTexture *>(_inletParams[0])->getWidth(), static_cast<ofTexture *>(_inletParams[0])->getHeight(), GL_RGBA );
                _maxW = static_cast<ofTexture *>(_inletParams[0])->getWidth();
                _maxH = static_cast<ofTexture *>(_inletParams[0])->getHeight();
            }

            scaledFbo->begin();
            ofClear(0,0,0,255);
            bounds.set(_x,_y,_w,_h);

            ofPushMatrix();
            ofTranslate(bounds.x+(bounds.width/2),bounds.y+(bounds.height/2));
            ofRotateXDeg(angleX);
            ofRotateYDeg(angleY);
            ofRotateZDeg(angleZ);
            ofSetColor(255);
            static_cast<ofTexture *>(_inletParams[0])->draw(-bounds.width/2,0-bounds.height/2,bounds.width,bounds.height);
            ofPopMatrix();
            scaledFbo->end();

            *static_cast<ofTexture *>(_outletParams[0]) = scaledFbo->getTexture();
        }
    }else{
        needToGrab = false;
    }

    if(this->inletsConnected[1] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        _x = ofClamp(*(float *)&_inletParams[1],0.0f,static_cast<ofTexture *>(_inletParams[0])->getWidth());
    }

    if(this->inletsConnected[2] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        _y = ofClamp(*(float *)&_inletParams[2],0.0f,static_cast<ofTexture *>(_inletParams[0])->getHeight());
    }

    if(this->inletsConnected[3] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        _w = ofClamp(*(float *)&_inletParams[3],0.0f,static_cast<ofTexture *>(_inletParams[0])->getWidth());
    }

    if(this->inletsConnected[4] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        _h = ofClamp(*(float *)&_inletParams[4],0.0f,static_cast<ofTexture *>(_inletParams[0])->getHeight());
    }

    if(this->inletsConnected[5] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        angleX = ofClamp(*(float *)&_inletParams[5],0.0f,360.0f);
    }

    if(this->inletsConnected[6] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        angleY = ofClamp(*(float *)&_inletParams[6],0.0f,360.0f);
    }

    if(this->inletsConnected[7] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        angleZ = ofClamp(*(float *)&_inletParams[7],0.0f,360.0f);
    }

    if(!loaded){
        loaded = true;
        _x = this->getCustomVar("XPOS");
        _y = this->getCustomVar("YPOS");
        _w = this->getCustomVar("WIDTH");
        _h = this->getCustomVar("HEIGHT");
        angleX = this->getCustomVar("ANGLEX");
        angleY = this->getCustomVar("ANGLEY");
        angleZ = this->getCustomVar("ANGLEZ");
        prevW = this->getCustomVar("OBJ_WIDTH");
        prevH = this->getCustomVar("OBJ_HEIGHT");
        this->width             = prevW;
        this->height            = prevH;
    }

}

//--------------------------------------------------------------
void VideoTransform::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
        // draw node texture preview with OF
        if(scaledObjW*canvasZoom > 90.0f){
            drawNodeOFTexture(*static_cast<ofTexture *>(_outletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
        }
    }else{
        // background
        if(scaledObjW*canvasZoom > 90.0f){
            ofSetColor(34,34,34);
            ofDrawRectangle(objOriginX - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/canvasZoom), objOriginY-(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor/canvasZoom),scaledObjW + (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/canvasZoom),scaledObjH + (((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor)/canvasZoom) );
        }
    }
}

//--------------------------------------------------------------
void VideoTransform::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
    if(_nodeCanvas.BeginNodeMenu()){
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            ImGui::Spacing();
            ImGui::PushItemWidth(130*this->scaleFactor);
            if(ImGui::SliderFloat("START X",&_x, 0.0f, _maxW)){
                this->setCustomVar(_x,"XPOS");
            }
            if(ImGui::SliderFloat("START Y",&_y, 0.0f, _maxH)){
                this->setCustomVar(_y,"YPOS");
            }
            if(ImGui::SliderFloat("WIDTH",&_w, 0, _maxW)){
                this->setCustomVar(_w,"WIDTH");
            }
            if(ImGui::SliderFloat("HEIGHT",&_h, 0, _maxH)){
                this->setCustomVar(_h,"HEIGHT");
            }
            ImGui::Spacing();
            if(ImGui::SliderFloat("ANGLE X",&angleX, 0.0f, 360.0f)){
                this->setCustomVar(angleX,"ANGLEX");
            }
            if(ImGui::SliderFloat("ANGLE Y",&angleY, 0.0f, 360.0f)){
                this->setCustomVar(angleY,"ANGLEY");
            }
            if(ImGui::SliderFloat("ANGLE Z",&angleZ, 0.0f, 360.0f)){
                this->setCustomVar(angleZ,"ANGLEZ");
            }

            ImGui::PopItemWidth();

            ImGuiEx::ObjectInfo(
                        "This object allows you to scale and rotate a texture.",
                        "https://mosaic.d3cod3.org/reference.php?r=video-transform", scaleFactor);

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

        // save object dimensions (for resizable ones)
        if(this->width != prevW){
            prevW = this->width;
            this->setCustomVar(prevW,"OBJ_WIDTH");
        }
        if(this->width != prevH){
            prevH = this->height;
            this->setCustomVar(prevH,"OBJ_HEIGHT");
        }

        _nodeCanvas.EndNodeContent();
    }

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();
}

//--------------------------------------------------------------
void VideoTransform::removeObjectContent(bool removeFileFromData){

}


OBJECT_REGISTER( VideoTransform, "texture transform", OFXVP_OBJECT_CAT_TEXTURE)

#endif
