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

#ifndef OFXVP_BUILD_WITH_MINIMAL_OBJECTS

#include "VideoMixer.h"

//--------------------------------------------------------------
VideoMixer::VideoMixer() : PatchObject("texture mixer"){

    this->numInlets  = 6;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();  // tex1
    _inletParams[1] = new ofTexture();  // tex2
    _inletParams[2] = new ofTexture();  // tex3
    _inletParams[3] = new ofTexture();  // tex4
    _inletParams[4] = new ofTexture();  // tex5
    _inletParams[5] = new ofTexture();  // tex6

    _outletParams[0] = new ofTexture(); // texture output

    this->initInletsState();

    posX = posY = drawW = drawH = 0.0f;

    dataInlets      = 6;

    needReset       = false;
    loaded          = false;

    kuro            = new ofImage();
    mixFbo          = new ofFbo();

    alphas.assign(32,255);

    canvasWidth     = STANDARD_TEXTURE_WIDTH;
    canvasHeight    = STANDARD_TEXTURE_HEIGHT;
    temp_width      = STANDARD_TEXTURE_WIDTH;
    temp_height     = STANDARD_TEXTURE_HEIGHT;

    this->setIsResizable(true);
    this->setIsTextureObj(true);

    prevW                   = this->width;
    prevH                   = this->height;

}

//--------------------------------------------------------------
void VideoMixer::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"t0");
    this->addInlet(VP_LINK_TEXTURE,"t1");
    this->addInlet(VP_LINK_TEXTURE,"t2");
    this->addInlet(VP_LINK_TEXTURE,"t3");
    this->addInlet(VP_LINK_TEXTURE,"t4");
    this->addInlet(VP_LINK_TEXTURE,"t5");

    this->addOutlet(VP_LINK_TEXTURE,"output");

    this->setCustomVar(static_cast<float>(dataInlets),"NUM_INLETS");

    this->setCustomVar(static_cast<float>(prevW),"WIDTH");
    this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
    this->setCustomVar(static_cast<float>(canvasWidth),"CANVAS_WIDTH");
    this->setCustomVar(static_cast<float>(canvasHeight),"CANVAS_HEIGHT");

    for(int i=0;i<32;i++){
        this->setCustomVar(alphas.at(i),"T"+ofToString(i)+"_alpha");
    }
}

//--------------------------------------------------------------
void VideoMixer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    // load kuro
    kuro->load("images/kuro.jpg");

    texData.width = kuro->getWidth();
    texData.height = kuro->getHeight();
    texData.textureTarget = GL_TEXTURE_2D;
    texData.bFlipTexture = true;

    kuroTex = new ofTexture();
    kuroTex->clear();
    kuroTex->allocate(texData);
    kuroTex->loadData(kuro->getPixels());

    mixFbo->allocate(canvasWidth, canvasHeight, GL_RGBA);
    mixFbo->begin();
    glColor4f(0.0f,0.0f,0.0f,1.0f);
    ofDrawRectangle(0,0,canvasWidth, canvasHeight);
    mixFbo->end();

}

//--------------------------------------------------------------
void VideoMixer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    // mix texture inlets with alpha
    if(mixFbo->isAllocated()){
        *static_cast<ofTexture *>(_outletParams[0]) = mixFbo->getTexture();
    }else{
        *static_cast<ofTexture *>(_outletParams[0]) = *kuroTex;
    }


    if(needReset){
        needReset = false;
        resetInletsSettings();
    }

    if(!loaded){
        loaded  = true;
        canvasWidth = this->getCustomVar("CANVAS_WIDTH");
        canvasHeight = this->getCustomVar("CANVAS_HEIGHT");
        prevW = this->getCustomVar("WIDTH");
        prevH = this->getCustomVar("HEIGHT");
        this->width             = prevW;
        this->height            = prevH;

        for(int i=0;i<32;i++){
            alphas.at(i) = this->getCustomVar("T"+ofToString(i)+"_alpha");
        }

        initInlets();
    }
}

//--------------------------------------------------------------
void VideoMixer::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

    mixFbo->begin();

    ofSetColor(0,0,0);
    ofDrawRectangle(0,0,canvasWidth,canvasHeight);

    for(int i=0;i<dataInlets;i++){
        if(this->inletsConnected[i]){
            ofSetColor(255,255,255,alphas.at(i));
            static_cast<ofTexture *>(_inletParams[i])->draw(0,0,canvasWidth,canvasHeight);
        }
    }

    mixFbo->end();

    // DRAW
    ofSetColor(255);
    if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
        // draw node texture preview with OF
        if(scaledObjW*canvasZoom > 90.0f && this->width > 120 && this->height > 70){
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
void VideoMixer::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        // save object dimensions (for resizable ones)
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
void VideoMixer::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(ImGui::InputInt("Texture Inlets",&dataInlets)){
        if(dataInlets > MAX_INLETS){
            dataInlets = MAX_INLETS;
        }
    }
    ImGui::SameLine(); ImGuiEx::HelpMarker("You can set 32 inlets max.");
    ImGui::Spacing();
    if(ImGui::Button("APPLY",ImVec2(224*scaleFactor,26*scaleFactor))){
        this->setCustomVar(static_cast<float>(dataInlets),"NUM_INLETS");
        needReset = true;
    }
    ImGui::Spacing();
    if(ImGui::InputInt("Width",&temp_width)){
        if(temp_width > OUTPUT_TEX_MAX_WIDTH){
            temp_width = OUTPUT_TEX_MAX_WIDTH;
        }
    }
    ImGui::SameLine(); ImGuiEx::HelpMarker("You can set the texture resolution WxH (limited for now at max. 4800x4800)");

    if(ImGui::InputInt("Height",&temp_height)){
        if(temp_height > OUTPUT_TEX_MAX_HEIGHT){
            temp_height = OUTPUT_TEX_MAX_HEIGHT;
        }
    }
    ImGui::Spacing();
    if(ImGui::Button("APPLY",ImVec2(224*scaleFactor,26*scaleFactor))){
        needReset = true;
    }
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::PushItemWidth(130*this->scaleFactor);
    for(int i=0;i<dataInlets;i++){
        string tempstr = "T"+ofToString(i)+" alpha";
        if(ImGui::SliderFloat(tempstr.c_str(),&alphas.at(i), 0.0f, 255.0f)){
            this->setCustomVar(alphas.at(i),"T"+ofToString(i)+"_alpha");
        }
    }
    ImGui::PopItemWidth();

    ImGui::Spacing();

    ImGuiEx::ObjectInfo(
                "Receives up to 32 textures, and mix them using alpha.",
                "https://mosaic.d3cod3.org/reference.php?r=texture-mixer", scaleFactor);
}

//--------------------------------------------------------------
void VideoMixer::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void VideoMixer::initInlets(){
    dataInlets = this->getCustomVar("NUM_INLETS");

    this->numInlets = dataInlets;

    resetInletsSettings();
}

//--------------------------------------------------------------
void VideoMixer::resetInletsSettings(){

    vector<bool> tempInletsConn;
    for(int i=0;i<this->numInlets;i++){
        if(this->inletsConnected[i]){
            tempInletsConn.push_back(true);
        }else{
            tempInletsConn.push_back(false);
        }
    }

    this->numInlets = dataInlets;

    for(int i=0;i<this->numInlets;i++){
        _inletParams[i] = new ofTexture();
    }

    this->inletsType.clear();
    this->inletsNames.clear();

    for(int i=0;i<this->numInlets;i++){
        this->addInlet(VP_LINK_TEXTURE,"t"+ofToString(i));
    }

    this->inletsConnected.clear();
    for(int i=0;i<this->numInlets;i++){
        if(i<static_cast<int>(tempInletsConn.size())){
            if(tempInletsConn.at(i)){
                this->inletsConnected.push_back(true);
            }else{
                this->inletsConnected.push_back(false);
            }
        }else{
            this->inletsConnected.push_back(false);
        }
    }

    if(canvasWidth != temp_width || canvasHeight != temp_height){
        canvasWidth = temp_width;
        canvasHeight = temp_height;

        ofLog(OF_LOG_NOTICE,"Changing texture dimensions to: %ix%i",canvasWidth,canvasHeight);

        this->setCustomVar(static_cast<float>(canvasWidth),"CANVAS_WIDTH");
        this->setCustomVar(static_cast<float>(canvasHeight),"CANVAS_HEIGHT");

        mixFbo = new ofFbo();
        mixFbo->allocate(canvasWidth, canvasHeight, GL_RGBA);
        mixFbo->begin();
        glColor4f(0.0f,0.0f,0.0f,1.0f);
        ofDrawRectangle(0,0,canvasWidth, canvasHeight);
        mixFbo->end();

        _outletParams[0] = new ofTexture();
        static_cast<ofTexture *>(_outletParams[0])->allocate(canvasWidth,canvasHeight,GL_RGB);

    }

    ofNotifyEvent(this->resetEvent, this->nId);

    this->saveConfig(false);

}

OBJECT_REGISTER( VideoMixer, "texture mixer", OFXVP_OBJECT_CAT_TEXTURE)

#endif
