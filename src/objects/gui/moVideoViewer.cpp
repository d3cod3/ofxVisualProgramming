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

#include "moVideoViewer.h"

//--------------------------------------------------------------
moVideoViewer::moVideoViewer() : PatchObject("video viewer"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();  // texture

    _outletParams[0] = new ofTexture();  // texture

    posX = posY = drawW = drawH = 0.0f;

    this->width             *= 2;
    this->height            *= 2;

    this->initInletsState();

    this->setIsResizable(true);
}

//--------------------------------------------------------------
void moVideoViewer::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"texture");
    this->addOutlet(VP_LINK_TEXTURE,"texture");

    this->setCustomVar(static_cast<float>(this->width),"WIDTH");
    this->setCustomVar(static_cast<float>(this->height),"HEIGHT");
}

//--------------------------------------------------------------
void moVideoViewer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    this->width = static_cast<int>(floor(this->getCustomVar("WIDTH")));
    this->height = static_cast<int>(floor(this->getCustomVar("HEIGHT")));
}

//--------------------------------------------------------------
void moVideoViewer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        *static_cast<ofTexture *>(_outletParams[0]) = *static_cast<ofTexture *>(_inletParams[0]);
    }
}

//--------------------------------------------------------------
void moVideoViewer::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void moVideoViewer::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            ImGuiEx::ObjectInfo(
                        "A basic video viewer, the object visualizes it, and bypasses it by its outlet.",
                        "https://mosaic.d3cod3.org/reference.php?r=video-viewer");

            ImGui::EndMenu();
        }
        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
            float _tw = this->width*_nodeCanvas.GetCanvasScale();
            float _th = (this->height*_nodeCanvas.GetCanvasScale()) - (IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT);

            ImGuiEx::drawOFTexture(static_cast<ofTexture *>(_inletParams[0]),_tw,_th,posX,posY,drawW,drawH);
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void moVideoViewer::removeObjectContent(bool removeFileFromData){
    
}

OBJECT_REGISTER( moVideoViewer, "video viewer", OFXVP_OBJECT_CAT_GUI)

#endif
