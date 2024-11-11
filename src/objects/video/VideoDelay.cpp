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

#include "VideoDelay.h"

//--------------------------------------------------------------
VideoDelay::VideoDelay() : PatchObject("video feedback"){

    this->numInlets  = 5;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();  // input
    _inletParams[1] = new float();      // x
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();      // y
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();      // scale
    *(float *)&_inletParams[3] = 0.0f;
    _inletParams[4] = new float();      // alpha
    *(float *)&_inletParams[4] = 0.0f;

    _outletParams[0] = new ofTexture(); // output

    this->initInletsState();

    posX = posY = drawW = drawH = 0.0f;

    backBufferTex   = new ofTexture();
    delayFbo        = new ofFbo();

    _x              = 0.0f;
    _y              = 0.0f;

    alpha           = 0.0f;
    alphaTo         = 1.0f;
    scale           = 1.0f;
    scaleTo         = 1.0f;
    needToGrab      = false;

    prevW           = this->width;
    prevH           = this->height;
    loaded          = false;

    this->setIsTextureObj(true);
    this->setIsResizable(true);

}

//--------------------------------------------------------------
void VideoDelay::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"input");
    this->addInlet(VP_LINK_NUMERIC,"x");
    this->addInlet(VP_LINK_NUMERIC,"y");
    this->addInlet(VP_LINK_NUMERIC,"scale");
    this->addInlet(VP_LINK_NUMERIC,"alpha");

    this->addOutlet(VP_LINK_TEXTURE,"feedbackOutput");

    this->setCustomVar(_x,"POSX");
    this->setCustomVar(_y,"POSY");
    this->setCustomVar(static_cast<float>(scaleTo),"SCALE");
    this->setCustomVar(static_cast<float>(alphaTo),"ALPHA");

    this->setCustomVar(static_cast<float>(prevW),"WIDTH");
    this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
}

//--------------------------------------------------------------
void VideoDelay::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);
}

//--------------------------------------------------------------
void VideoDelay::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(this->inletsConnected[1] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        _x = ofClamp(*(float *)&_inletParams[1],-static_cast<ofTexture *>(_inletParams[0])->getWidth(),static_cast<ofTexture *>(_inletParams[0])->getWidth());
    }

    if(this->inletsConnected[2] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        _y = ofClamp(*(float *)&_inletParams[2],-static_cast<ofTexture *>(_inletParams[0])->getHeight(),static_cast<ofTexture *>(_inletParams[0])->getHeight());
    }

    if(this->inletsConnected[3]){
        scaleTo = ofClamp(*(float *)&_inletParams[3],0.0f,1.0f);
    }

    if(this->inletsConnected[4]){
        alphaTo = ofClamp(*(float *)&_inletParams[4],0.0f,1.0f);
    }

    // UPDATE
    alpha       = .995 * alpha + .005 * alphaTo;
    scale       = .95 * scale + .05 * scaleTo;
    halfscale   = (1.000000f - scale) / 2.000000f;

    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(!needToGrab){
            needToGrab = true;
            ofDisableArbTex();
            backBufferTex->allocate(static_cast<ofTexture *>(_inletParams[0])->getWidth(), static_cast<ofTexture *>(_inletParams[0])->getHeight(), GL_RGB);
            delayFbo->allocate(static_cast<ofTexture *>(_inletParams[0])->getWidth(), static_cast<ofTexture *>(_inletParams[0])->getHeight(), GL_RGBA);
            ofEnableArbTex();
            delayFbo->begin();
            glColor4f(0.0f,0.0f,0.0f,1.0f);
            ofDrawRectangle(0,0,static_cast<ofTexture *>(_inletParams[0])->getWidth(), static_cast<ofTexture *>(_inletParams[0])->getHeight());
            delayFbo->end();
            backBufferTex = static_cast<ofTexture *>(_inletParams[0]);
        }

        delayFbo->begin();
        ofEnableAlphaBlending();
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f,1.0f,1.0f,alpha);
        glPushMatrix();

        bounds.set(_x,_y,static_cast<ofTexture *>(_inletParams[0])->getWidth(), static_cast<ofTexture *>(_inletParams[0])->getHeight());
        backBufferTex->draw(bounds.x, bounds.y, static_cast<ofTexture *>(_inletParams[0])->getWidth() * scale, static_cast<ofTexture *>(_inletParams[0])->getHeight() * scale );
        backBufferTex = static_cast<ofTexture *>(_inletParams[0]);

        glPopMatrix();
        ofDisableAlphaBlending();
        delayFbo->end();

        *static_cast<ofTexture *>(_outletParams[0]) = delayFbo->getTexture();
    }else{
        needToGrab = false;
    }

    if(!loaded && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        loaded = true;
        _x = ofClamp(this->getCustomVar("POSX"),-static_cast<ofTexture *>(_inletParams[0])->getWidth(),static_cast<ofTexture *>(_inletParams[0])->getWidth());
        _y = ofClamp(this->getCustomVar("POSY"),-static_cast<ofTexture *>(_inletParams[0])->getHeight(),static_cast<ofTexture *>(_inletParams[0])->getHeight());
        scaleTo = this->getCustomVar("SCALE");
        alphaTo = this->getCustomVar("ALPHA");

        prevW = this->getCustomVar("WIDTH");
        prevH = this->getCustomVar("HEIGHT");
        this->width             = prevW;
        this->height            = prevH;
    }
    
}

//--------------------------------------------------------------
void VideoDelay::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

}

//--------------------------------------------------------------
void VideoDelay::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
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
void VideoDelay::drawObjectNodeConfig(){
    ImGui::Spacing();
    ImGui::PushItemWidth(130*this->scaleFactor);
    if(static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(ImGui::SliderFloat("POS X",&_x, -static_cast<ofTexture *>(_inletParams[0])->getWidth(),static_cast<ofTexture *>(_inletParams[0])->getWidth())){
            this->setCustomVar(_x,"XPOS");
        }
        if(ImGui::SliderFloat("POS Y",&_y, -static_cast<ofTexture *>(_inletParams[0])->getHeight(),static_cast<ofTexture *>(_inletParams[0])->getHeight())){
            this->setCustomVar(_y,"YPOS");
        }
    }
    if(ImGui::SliderFloat("SCALE",&scaleTo, 0.0f, 1.0f)){
        this->setCustomVar(scaleTo,"WIDTH");
    }
    if(ImGui::SliderFloat("ALPHA",&alphaTo, 0.0f, 1.0f)){
        this->setCustomVar(alphaTo,"ALPHA");
    }
    ImGui::PopItemWidth();

    ImGuiEx::ObjectInfo(
                "With this object you can make a texture feedback. The action is visualized by scaling and/or moving the texture, showing the trace of its edges in the path.",
                "https://mosaic.d3cod3.org/reference.php?r=video-feedback", scaleFactor);
}

//--------------------------------------------------------------
void VideoDelay::removeObjectContent(bool removeFileFromData){
    
}

OBJECT_REGISTER( VideoDelay, "video feedback", OFXVP_OBJECT_CAT_TEXTURE)

#endif
