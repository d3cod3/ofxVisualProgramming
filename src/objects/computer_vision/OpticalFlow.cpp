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

#include "OpticalFlow.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
OpticalFlow::OpticalFlow() : PatchObject("optical flow"){

    this->numInlets  = 1;
    this->numOutlets = 2;

    _inletParams[0] = new ofTexture();  // input

    _outletParams[0] = new ofTexture(); // output texture
    _outletParams[1] = new vector<float>(); // optical flow data

    this->initInletsState();

    posX = posY = drawW = drawH = 0.0f;

    pix                 = new ofPixels();
    scaledPix           = new ofPixels();
    outputFBO           = new ofFbo();

    isFBOAllocated      = false;

    fbUseGaussian       = false;
    fbPyrScale          = 0.25f;
    fbPolySigma         = 1.5f;
    fbLevels            = 4.0f;
    fbIterations        = 2.0f;
    fbPolyN             = 7.0f;
    fbWinSize           = 32.0f;

    loaded              = false;

    this->setIsTextureObj(true);

}

//--------------------------------------------------------------
void OpticalFlow::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"input");

    this->addOutlet(VP_LINK_TEXTURE,"output");
    this->addOutlet(VP_LINK_ARRAY,"opticalFlowData");

    this->setCustomVar(static_cast<float>(fbUseGaussian),"FB_USE_GAUSSIAN");
    this->setCustomVar(fbPyrScale,"FB_PYR_SCALE");
    this->setCustomVar(fbPolySigma,"FB_POLY_SIGMA");
    this->setCustomVar(fbLevels,"FB_LEVELS");
    this->setCustomVar(fbIterations,"FB_ITERATIONS");
    this->setCustomVar(fbPolyN,"FB_POLY_N");
    this->setCustomVar(fbWinSize,"FB_WIN_SIZE");

}

//--------------------------------------------------------------
void OpticalFlow::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void OpticalFlow::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(!loaded){
        loaded = true;

        fbUseGaussian = static_cast<bool>(floor(this->getCustomVar("FB_USE_GAUSSIAN")));
        fbPyrScale = this->getCustomVar("FB_PYR_SCALE");
        fbLevels = this->getCustomVar("FB_LEVELS");
        fbWinSize = this->getCustomVar("FB_WIN_SIZE");
        fbIterations = this->getCustomVar("FB_ITERATIONS");
        fbPolyN = this->getCustomVar("FB_POLY_N");
        fbPolySigma = this->getCustomVar("FB_POLY_SIGMA");
    }

}

//--------------------------------------------------------------
void OpticalFlow::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){

    // OPTICAL FLOW UPDATE
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){

        if(!isFBOAllocated){
            isFBOAllocated = true;
            pix             = new ofPixels();
            scaledPix       = new ofPixels();
            scaledPix->allocate(320,static_cast<ofTexture *>(_inletParams[0])->getHeight()/static_cast<ofTexture *>(_inletParams[0])->getWidth()*320,OF_PIXELS_RGB);
            outputFBO->allocate(static_cast<ofTexture *>(_inletParams[0])->getWidth(),static_cast<ofTexture *>(_inletParams[0])->getHeight(),GL_RGB,1);
        }

        fb.setPyramidScale(fbPyrScale);
        fb.setNumLevels(static_cast<int>(floor(fbLevels)));
        fb.setWindowSize(static_cast<int>(floor(fbWinSize)));
        fb.setNumIterations(static_cast<int>(floor(fbIterations)));
        fb.setPolyN(static_cast<int>(floor(fbPolyN)));
        fb.setPolySigma(fbPolySigma);
        fb.setUseGaussian(fbUseGaussian);

        static_cast<ofTexture *>(_inletParams[0])->readToPixels(*pix);

        pix->resizeTo(*scaledPix);

        fb.calcOpticalFlow(*scaledPix);

        if(outputFBO->isAllocated()){
            *static_cast<ofTexture *>(_outletParams[0]) = outputFBO->getTexture();

            static_cast<vector<float> *>(_outletParams[1])->clear();

            static_cast<vector<float> *>(_outletParams[1])->push_back(fb.getFlow().rows);
            static_cast<vector<float> *>(_outletParams[1])->push_back(fb.getFlow().cols);

            for(int y = 0; y < fb.getFlow().rows; y += 10) {
                for(int x = 0; x < fb.getFlow().cols; x += 10) {
                    static_cast<vector<float> *>(_outletParams[1])->push_back(x);
                    static_cast<vector<float> *>(_outletParams[1])->push_back(y);
                    static_cast<vector<float> *>(_outletParams[1])->push_back(fb.getFlowPosition(x, y).x);
                    static_cast<vector<float> *>(_outletParams[1])->push_back(fb.getFlowPosition(x, y).y);
                }
            }
        }

    }else{
        isFBOAllocated = false;
    }

    // OPTICAL FLOW DRAW
    if(this->inletsConnected[0] && outputFBO->isAllocated() && static_cast<ofTexture *>(_outletParams[0])->isAllocated()){

        outputFBO->begin();

        ofClear(0,0,0,255);

        ofSetColor(255);
        static_cast<ofTexture *>(_inletParams[0])->draw(0,0);

        ofSetColor(ofColor::yellowGreen);
        fb.draw(0,0,static_cast<ofTexture *>(_outletParams[0])->getWidth(),static_cast<ofTexture *>(_outletParams[0])->getHeight());

        outputFBO->end();
    }

    ofSetColor(255);
    // draw node texture preview with OF
    if(scaledObjW*canvasZoom > 90.0f){
        drawNodeOFTexture(*static_cast<ofTexture *>(_outletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
    }
}

//--------------------------------------------------------------
void OpticalFlow::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        _nodeCanvas.EndNodeContent();
    }

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();

}

//--------------------------------------------------------------
void OpticalFlow::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(ImGui::Checkbox("GAUSSIAN",&fbUseGaussian)){
        this->setCustomVar(static_cast<float>(fbUseGaussian),"FB_USE_GAUSSIAN");
    }
    ImGui::Spacing();
    if(ImGui::SliderFloat("pyramid scale",&fbPyrScale,0.0f,0.5f)){
        this->setCustomVar(fbPyrScale,"FB_PYR_SCALE");
    }
    ImGui::Spacing();
    if(ImGui::SliderFloat("num levels",&fbLevels,1.0f,8.0f)){
        this->setCustomVar(fbLevels,"FB_LEVELS");
    }
    ImGui::Spacing();
    if(ImGui::SliderFloat("window size",&fbWinSize,16.0f,64.0f)){
        this->setCustomVar(fbWinSize,"FB_WIN_SIZE");
    }
    ImGui::Spacing();
    if(ImGui::SliderFloat("num iterations",&fbIterations,1.0f,3.0f)){
        this->setCustomVar(fbIterations,"FB_ITERATIONS");
    }
    ImGui::Spacing();
    if(ImGui::SliderFloat("poly N",&fbPolyN,5.0f,10.0f)){
        this->setCustomVar(fbPolyN,"FB_POLY_N");
    }
    ImGui::Spacing();
    if(ImGui::SliderFloat("poly sigma",&fbPolySigma,1.1f,2.0f)){
        this->setCustomVar(fbPolySigma,"FB_POLY_SIGMA");
    }


    ImGuiEx::ObjectInfo(
                "Optical flow Farneback algorithm.",
                "https://mosaic.d3cod3.org/reference.php?r=optical-flow", scaleFactor);
}

//--------------------------------------------------------------
void OpticalFlow::removeObjectContent(bool removeFileFromData){
    
}



OBJECT_REGISTER( OpticalFlow, "optical flow", OFXVP_OBJECT_CAT_CV)

#endif
