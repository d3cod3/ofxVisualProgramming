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

#include "DataToTexture.h"

//--------------------------------------------------------------
DataToTexture::DataToTexture() : PatchObject("data to texture"){

    this->numInlets  = 3;
    this->numOutlets = 1;

    _inletParams[0] = new vector<float>();  // red
    _inletParams[1] = new vector<float>();  // green
    _inletParams[2] = new vector<float>();  // blu

    _outletParams[0] = new ofTexture(); // texture output

    this->initInletsState();

    pix                 = new ofPixels();
    scaledPix           = new ofPixels();

    this->output_width  = STANDARD_TEXTURE_WIDTH;
    this->output_height = STANDARD_TEXTURE_HEIGHT;

    temp_width          = this->output_width;
    temp_height         = this->output_height;

    prevW               = this->width;
    prevH               = this->height;
    loaded              = false;
    needReset           = false;

    this->setIsTextureObj(true);
    this->setIsResizable(true);

}

//--------------------------------------------------------------
void DataToTexture::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"red");
    this->addInlet(VP_LINK_ARRAY,"green");
    this->addInlet(VP_LINK_ARRAY,"blue");

    this->addOutlet(VP_LINK_TEXTURE,"output");

    this->setCustomVar(static_cast<float>(this->output_width),"OUTPUT_WIDTH");
    this->setCustomVar(static_cast<float>(this->output_height),"OUTPUT_HEIGHT");

    this->setCustomVar(static_cast<float>(prevW),"WIDTH");
    this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
}

//--------------------------------------------------------------
void DataToTexture::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    pix->allocate(320,240,OF_PIXELS_RGB);
}

//--------------------------------------------------------------
void DataToTexture::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(needReset){
        needReset = false;
        resetResolution();
    }

    if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
        if((this->inletsConnected[0] || this->inletsConnected[1] || this->inletsConnected[2]) && (!static_cast<vector<float> *>(_inletParams[0])->empty() || !static_cast<vector<float> *>(_inletParams[1])->empty() || !static_cast<vector<float> *>(_inletParams[2])->empty())){
            for(size_t s=0;s<pix->size();s++){
                int posR = 0;
                int sampleR = 0;
                int posG = 0;
                int sampleG = 0;
                int posB = 0;
                int sampleB = 0;
                // RED
                if(this->inletsConnected[0] && !static_cast<vector<float> *>(_inletParams[0])->empty()){
                    posR = static_cast<int>(floor(ofMap(s,0,pix->size(),0,static_cast<int>(static_cast<vector<float> *>(_inletParams[0])->size()))));
                    sampleR = static_cast<int>(floor(ofMap(static_cast<vector<float> *>(_inletParams[0])->at(posR), -0.5f, 0.5f, 0, 255)));
                }
                // GREEN
                if(this->inletsConnected[1] && !static_cast<vector<float> *>(_inletParams[1])->empty()){
                    posG = static_cast<int>(floor(ofMap(s,0,pix->size(),0,static_cast<int>(static_cast<vector<float> *>(_inletParams[1])->size()))));
                    sampleG = static_cast<int>(floor(ofMap(static_cast<vector<float> *>(_inletParams[1])->at(posG), -0.5f, 0.5f, 0, 255)));
                }
                // BLUE
                if(this->inletsConnected[2] && !static_cast<vector<float> *>(_inletParams[2])->empty()){
                    posB = static_cast<int>(floor(ofMap(s,0,pix->size(),0,static_cast<int>(static_cast<vector<float> *>(_inletParams[2])->size()))));
                    sampleB = static_cast<int>(floor(ofMap(static_cast<vector<float> *>(_inletParams[2])->at(posB), -0.5f, 0.5f, 0, 255)));
                }
                ofColor c(sampleR,sampleG,sampleB);
                size_t x = s % pix->getWidth();
                size_t y = static_cast<size_t>(ceil(s / pix->getWidth()));
                if(x >= 0 && x <= pix->getWidth() && y >= 0 && y <= pix->getHeight()){
                    pix->setColor(x,y,c);
                }
            }
            pix->resizeTo(*scaledPix);
            static_cast<ofTexture *>(_outletParams[0])->loadData(*scaledPix);
        }
    }

    if(!loaded){
        loaded = true;
        this->output_width = static_cast<int>(floor(this->getCustomVar("OUTPUT_WIDTH")));
        this->output_height = static_cast<int>(floor(this->getCustomVar("OUTPUT_HEIGHT")));
        temp_width      = this->output_width;
        temp_height     = this->output_height;

        prevW = this->getCustomVar("WIDTH");
        prevH = this->getCustomVar("HEIGHT");
        this->width             = prevW;
        this->height            = prevH;

        scaledPix->allocate(this->output_width,this->output_height,OF_PIXELS_RGB);

        ofDisableArbTex();
        _outletParams[0] = new ofTexture();
        static_cast<ofTexture *>(_outletParams[0])->allocate(this->output_width,this->output_height,GL_RGB);
        ofEnableArbTex();
    }

}

//--------------------------------------------------------------
void DataToTexture::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);
}

//--------------------------------------------------------------
void DataToTexture::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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
void DataToTexture::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(ImGui::InputInt("Width",&temp_width)){
        if(temp_width > OUTPUT_TEX_MAX_WIDTH){
            temp_width = this->output_width;
        }
    }
    ImGui::SameLine(); ImGuiEx::HelpMarker("You can set a supported resolution WxH (limited for now at max. 4800x4800)");

    if(ImGui::InputInt("Height",&temp_height)){
        if(temp_height > OUTPUT_TEX_MAX_HEIGHT){
            temp_height = this->output_height;
        }
    }
    ImGui::Spacing();
    if(ImGui::Button("APPLY",ImVec2(224*scaleFactor,26*scaleFactor))){
        needReset = true;
    }

    ImGuiEx::ObjectInfo(
                "This object performs “analog style” video synthesis, with a separate control over RGB channels",
                "https://mosaic.d3cod3.org/reference.php?r=data-to-texture", scaleFactor);
}

//--------------------------------------------------------------
void DataToTexture::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
}

//--------------------------------------------------------------
void DataToTexture::resetResolution(){

    if(this->output_width != temp_width || this->output_height != temp_height){
        this->output_width = temp_width;
        this->output_height = temp_height;

        scaledPix = new ofPixels();
        scaledPix->allocate(this->output_width,this->output_height,OF_PIXELS_RGB);

        ofDisableArbTex();
        _outletParams[0] = new ofTexture();
        static_cast<ofTexture *>(_outletParams[0])->allocate(this->output_width,this->output_height,GL_RGB);
        ofEnableArbTex();


        if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
            this->setCustomVar(static_cast<float>(this->output_width),"OUTPUT_WIDTH");
            this->setCustomVar(static_cast<float>(this->output_height),"OUTPUT_HEIGHT");
            this->saveConfig(false);

            ofLog(OF_LOG_NOTICE,"%s: RESOLUTION CHANGED TO %ix%i",this->name.c_str(),this->output_width,this->output_height);
        }
    }

}

OBJECT_REGISTER( DataToTexture, "data to texture", OFXVP_OBJECT_CAT_DATA)

#endif
