/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2022 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "FboComposer.h"

//--------------------------------------------------------------
FboComposer::FboComposer() : PatchObject("fbo composer"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();
    _outletParams[0] = new ofxPingPong();

    kuro    = new ofImage();

    posX = posY = drawW = drawH = 0.0f;

    output_width    = STANDARD_TEXTURE_WIDTH;
    output_height   = STANDARD_TEXTURE_HEIGHT;

    prevW   = this->width;
    prevH   = this->height;

    loaded  = false;

    this->setIsResizable(true);
    this->setIsTextureObj(true);

    this->initInletsState();

}

//--------------------------------------------------------------
void FboComposer::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"texture");
    this->addOutlet(VP_LINK_FBO,"fbo");

    this->setCustomVar(static_cast<float>(output_width),"OUTPUT_WIDTH");
    this->setCustomVar(static_cast<float>(output_height),"OUTPUT_HEIGHT");

    this->setCustomVar(static_cast<float>(prevW),"WIDTH");
    this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
}

//--------------------------------------------------------------
void FboComposer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    initResolution();

    // load kuro
    kuro->load("images/kuro.jpg");
}

//--------------------------------------------------------------
void FboComposer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(!loaded){
        loaded = true;
        prevW = this->getCustomVar("WIDTH");
        prevH = this->getCustomVar("HEIGHT");
        this->width             = prevW;
        this->height            = prevH;
    }
}

//--------------------------------------------------------------
void FboComposer::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){

    static_cast<ofxPingPong *>(_outletParams[0])->src->begin();
    ofPushView();
    ofPushStyle();
    ofPushMatrix();
    ofEnableAlphaBlending();
    ofSetColor(255);
    if(this->inletsConnected[0] && this->getInletType(0) == VP_LINK_TEXTURE && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        static_cast<ofTexture *>(_inletParams[0])->draw(0,0,output_width, output_height);
    }else{
        kuro->draw(0,0,output_width, output_height);
    }
    ofPopMatrix();
    ofPopStyle();
    ofPopView();
    static_cast<ofxPingPong *>(_outletParams[0])->src->end();

    static_cast<ofxPingPong *>(_outletParams[0])->dst->begin();
    ofSetColor(255);
    static_cast<ofxPingPong *>(_outletParams[0])->src->draw(0,0);
    static_cast<ofxPingPong *>(_outletParams[0])->dst->end();

    static_cast<ofxPingPong *>(_outletParams[0])->swap();

    ofSetColor(255);
    if(static_cast<ofxPingPong *>(_outletParams[0])->src->getTexture().isAllocated()){
        // draw node texture preview with OF
        if(scaledObjW*canvasZoom > 90.0f){
            drawNodeOFTexture(static_cast<ofxPingPong *>(_outletParams[0])->dst->getTexture(), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
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
void FboComposer::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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
void FboComposer::drawObjectNodeConfig(){

    ImGui::Spacing();
    ImGui::Text("Rendering at: %.0fx%.0f",static_cast<ofxPingPong *>(_outletParams[0])->dst->getWidth(),static_cast<ofxPingPong *>(_outletParams[0])->dst->getHeight());
    ImGui::Spacing();

    ImGuiEx::ObjectInfo(
                "TODO",
                "https://mosaic.d3cod3.org/reference.php?r=fbo-composer", scaleFactor);
}

//--------------------------------------------------------------
void FboComposer::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void FboComposer::initResolution(){
    output_width = static_cast<int>(floor(this->getCustomVar("OUTPUT_WIDTH")));
    output_height = static_cast<int>(floor(this->getCustomVar("OUTPUT_HEIGHT")));

    // init pingpong
    _outletParams[0] = new ofxPingPong();
    static_cast<ofxPingPong *>(_outletParams[0])->allocate(output_width,output_height);

}

OBJECT_REGISTER( FboComposer, "fbo composer", OFXVP_OBJECT_CAT_SURFACE)

#endif
