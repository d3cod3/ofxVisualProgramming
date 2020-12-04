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

#include "moSonogram.h"

//--------------------------------------------------------------
moSonogram::moSonogram() : PatchObject("sonogram"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new vector<float>();  // fft

    _outletParams[0] = new ofTexture();  // texture

    this->initInletsState();

     // 16:9 proportion
    this->width             = 428;
    this->height            = 240;


    sonogram                = new ofFbo();

    posX = posY = drawW = drawH = 0.0f;

    timePosition            = 0;
    resetTime               = ofGetElapsedTimeMillis();
    wait                    = 40;

    prevW                   = this->width;
    prevH                   = this->height;

    loaded                  = false;

    this->setIsResizable(true);
    this->setIsTextureObj(true);

}

//--------------------------------------------------------------
void moSonogram::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"fft");
    this->addOutlet(VP_LINK_TEXTURE,"sonogramTexture");

    this->setCustomVar(static_cast<float>(prevW),"WIDTH");
    this->setCustomVar(static_cast<float>(prevH),"HEIGHT");

}

//--------------------------------------------------------------
void moSonogram::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    this->width     *= scaleFactor;
    this->height    *= scaleFactor;

    resetTextures();

    colors.push_back(ofColor(0,0,0,240));           // BLACK
    colors.push_back(ofColor(0,0,180,240));         // BLUE
    colors.push_back(ofColor(55,0,110,240));        // PURPLE
    colors.push_back(ofColor(255,0,0,240));         // RED
    colors.push_back(ofColor(255,255,0,240));       // YELLOW
    colors.push_back(ofColor(255,255,255,240));     // WHITE

}

//--------------------------------------------------------------
void moSonogram::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    if(this->inletsConnected[0]){
        if(ofGetElapsedTimeMillis()-resetTime > wait){
            resetTime = ofGetElapsedTimeMillis();
            if(timePosition >= static_cast<ofTexture *>(_outletParams[0])->getWidth()){
                timePosition = 0;
            }else{
                timePosition++;
            }
        }

        if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
            *static_cast<ofTexture *>(_outletParams[0]) = sonogram->getTexture();
        }
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
void moSonogram::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){

    // background
    if(scaledObjW*canvasZoom > 90.0f){
        ofSetColor(34,34,34);
        ofDrawRectangle(objOriginX - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/canvasZoom), objOriginY-(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor/canvasZoom),scaledObjW + (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/canvasZoom),scaledObjH + (((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor)/canvasZoom) );
    }

    if(this->inletsConnected[0]){
        sonogram->begin();
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
        ofPushView();
        ofPushStyle();
        ofPushMatrix();
        ofSetColor(0,0,0,0);
        ofDrawRectangle(0,0,static_cast<ofTexture *>(_outletParams[0])->getWidth(),static_cast<ofTexture *>(_outletParams[0])->getHeight());
        for(size_t s=0;s<static_cast<size_t>(static_cast<vector<float> *>(_inletParams[0])->size());s++){
            float valueDB = log10(ofMap(static_cast<vector<float> *>(_inletParams[0])->at(s),0.0f,1.0f,1.0f,10.0f,true));
            int colorIndex = static_cast<int>(floor(ofMap(valueDB,0.0f,1.0f,0,colors.size()-1,true)));
            ofSetColor(0);
            ofDrawRectangle(timePosition,ofMap(s,0,static_cast<int>(static_cast<vector<float> *>(_inletParams[0])->size()),static_cast<ofTexture *>(_outletParams[0])->getHeight(),0,true),1,1);
            ofSetColor(colors.at(colorIndex),255*valueDB);
            ofDrawRectangle(timePosition,ofMap(s,0,static_cast<int>(static_cast<vector<float> *>(_inletParams[0])->size()),static_cast<ofTexture *>(_outletParams[0])->getHeight(),0,true),1,1);
        }
        ofPopMatrix();
        ofPopStyle();
        ofPopView();
        glPopAttrib();
        sonogram->end();

        if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
            if(scaledObjW*canvasZoom > 90.0f){
                // draw node texture preview with OF
                drawNodeOFTexture(*static_cast<ofTexture *>(_outletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
            }
        }
    }
}

//--------------------------------------------------------------
void moSonogram::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
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
void moSonogram::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Basic sonogram. A visual representation display of spectrum frequencies of a sound signal (FFT), showing how it changes over time.",
                "https://mosaic.d3cod3.org/reference.php?r=sonogram", scaleFactor);
}

//--------------------------------------------------------------
void moSonogram::removeObjectContent(bool removeFileFromData){
    
}

//--------------------------------------------------------------
void moSonogram::resetTextures(){

    sonogram                = new ofFbo();
    _outletParams[0]        = new ofTexture();

    sonogram->allocate(this->width,this->height,GL_RGBA);

    sonogram->begin();
    ofClear(0,0,0,255);
    sonogram->end();

    ofTextureData texData;
    texData.width = sonogram->getWidth();
    texData.height = sonogram->getHeight();
    texData.textureTarget = GL_TEXTURE_2D;
    texData.bFlipTexture = true;
    static_cast<ofTexture *>(_outletParams[0])->allocate(texData);
    *static_cast<ofTexture *>(_outletParams[0]) = sonogram->getTexture();
}

OBJECT_REGISTER( moSonogram, "sonogram", OFXVP_OBJECT_CAT_GUI)

#endif
