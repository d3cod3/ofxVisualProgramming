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

#include "VideoTimelapse.h"

//--------------------------------------------------------------
VideoTimelapse::VideoTimelapse() : PatchObject("video timedelay"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();  // input

    _inletParams[1] = new float();  // delay frames
    *(float *)&_inletParams[1] = 25.0f;

    _outletParams[0] = new ofTexture(); // output

    this->initInletsState();

    videoBuffer = new circularTextureBuffer();
    pix         = new ofPixels();
    kuro        = new ofImage();

    posX = posY = drawW = drawH = 0.0f;

    nDelayFrames    = 25;
    capturedFrame   = 0;
    delayFrame      = 0;

    resetTime       = ofGetElapsedTimeMillis();
    wait            = 1000/static_cast<int>(ofGetFrameRate());

    loaded = false;

    this->setIsTextureObj(true);

}

//--------------------------------------------------------------
void VideoTimelapse::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"input");
    this->addInlet(VP_LINK_NUMERIC,"delay");

    this->addOutlet(VP_LINK_TEXTURE,"timeDelayedOutput");

    this->setCustomVar(static_cast<float>(nDelayFrames),"DELAY_FRAMES");
}

//--------------------------------------------------------------
void VideoTimelapse::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    // load kuro
    kuro->load("images/kuro.jpg");
}

//--------------------------------------------------------------
void VideoTimelapse::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    
    if(this->inletsConnected[0]){
        if(ofGetElapsedTimeMillis()-resetTime > wait){
            resetTime       = ofGetElapsedTimeMillis();

            ofImage rgbaImage;
            rgbaImage.allocate(static_cast<ofTexture *>(_inletParams[0])->getWidth(), static_cast<ofTexture *>(_inletParams[0])->getHeight(),OF_IMAGE_COLOR_ALPHA);
            static_cast<ofTexture *>(_inletParams[0])->readToPixels(*pix);
            rgbaImage.setFromPixels(*pix);
            videoBuffer->pushTexture(rgbaImage.getTexture());

            if(capturedFrame >= nDelayFrames){
                if(delayFrame < nDelayFrames-1){
                    delayFrame++;
                }else{
                    delayFrame = 0;
                }
            }else{
                capturedFrame++;
            }
        }
        if(capturedFrame >= nDelayFrames){
            *static_cast<ofTexture *>(_outletParams[0]) = videoBuffer->getDelayedtexture(delayFrame);
        }else{
            *static_cast<ofTexture *>(_outletParams[0]) = kuro->getTexture();
        }
    }else{
        *static_cast<ofTexture *>(_outletParams[0]) = kuro->getTexture();
    }

    if(this->inletsConnected[1]){
        if(nDelayFrames != static_cast<int>(floor(*(float *)&_inletParams[1]))){
            nDelayFrames = static_cast<int>(floor(*(float *)&_inletParams[1]));

            capturedFrame   = 0;
            delayFrame      = 0;

            resetTime       = ofGetElapsedTimeMillis();
            wait            = 1000/static_cast<int>(ofGetFrameRate());

            videoBuffer->setup(nDelayFrames);
        }
    }

    if(!loaded){
        loaded = true;
        nDelayFrames = this->getCustomVar("DELAY_FRAMES");
        videoBuffer->setup(nDelayFrames);
    }
    
}

//--------------------------------------------------------------
void VideoTimelapse::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
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
void VideoTimelapse::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
    if(_nodeCanvas.BeginNodeMenu()){
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            drawObjectNodeConfig();

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
void VideoTimelapse::drawObjectNodeConfig(){
    ImGui::Spacing();

    if(ImGui::InputInt("Frames",&nDelayFrames)){
        capturedFrame   = 0;
        delayFrame      = 0;

        resetTime       = ofGetElapsedTimeMillis();
        wait            = 1000/static_cast<int>(ofGetFrameRate());

        videoBuffer->setup(nDelayFrames);
        this->setCustomVar(static_cast<float>(nDelayFrames),"DELAY_FRAMES");
    }

    ImGuiEx::ObjectInfo(
                "Delay the playback of a video file or live video.",
                "https://mosaic.d3cod3.org/reference.php?r=video-timedelay", scaleFactor);
}

//--------------------------------------------------------------
void VideoTimelapse::removeObjectContent(bool removeFileFromData){
    
}


OBJECT_REGISTER( VideoTimelapse, "video timedelay", OFXVP_OBJECT_CAT_TEXTURE)

#endif
