/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2020 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "moDataViewer.h"

//--------------------------------------------------------------
moDataViewer::moDataViewer() : PatchObject("data viewer"){

    this->numInlets  = 1;
    this->numOutlets = 0;

    _inletParams[0] = new vector<float>();  // RAW Data

    this->initInletsState();

    max   = 1.0f;
    color = ImVec4(1.0f,1.0f,1.0f,1.0f);

    this->setIsResizable(true);

    prevW                   = this->width;
    prevH                   = this->height;

    loaded                  = false;

}

//--------------------------------------------------------------
void moDataViewer::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"data");

    this->setCustomVar(max,"MAX");
    this->setCustomVar(1.0f,"RED");
    this->setCustomVar(1.0f,"GREEN");
    this->setCustomVar(1.0f,"BLUE");
    this->setCustomVar(1.0f,"ALPHA");

    this->setCustomVar(static_cast<float>(prevW),"WIDTH");
    this->setCustomVar(static_cast<float>(prevH),"HEIGHT");

}

//--------------------------------------------------------------
void moDataViewer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void moDataViewer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(!loaded){
        loaded = true;
        max = this->getCustomVar("MAX");
        prevW = this->getCustomVar("WIDTH");
        prevH = this->getCustomVar("HEIGHT");
        this->width             = prevW;
        this->height            = prevH;
        color = ImVec4(this->getCustomVar("RED"),this->getCustomVar("GREEN"),this->getCustomVar("BLUE"),this->getCustomVar("ALPHA"));
    }

}

//--------------------------------------------------------------
void moDataViewer::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void moDataViewer::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        // draw data
        if(this->inletsConnected[0] && !static_cast<vector<float> *>(_inletParams[0])->empty()){
            ImGuiEx::PlotBands(_nodeCanvas.getNodeDrawList(), 0, ImGui::GetWindowSize().y - 26, static_cast<vector<float> *>(_inletParams[0]), max, IM_COL32(color.x*255,color.y*255,color.z*255,color.w*255));
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void moDataViewer::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(ImGui::InputFloat("Max",&max)){
        this->setCustomVar(max,"MAX");
    }
    ImGui::Spacing();
    if(ImGui::ColorEdit4( "Color", (float*)&color )){
        this->setCustomVar(color.x,"RED");
        this->setCustomVar(color.y,"GREEN");
        this->setCustomVar(color.z,"BLUE");
        this->setCustomVar(color.w,"ALPHA");
    }

    ImGuiEx::ObjectInfo(
                "A basic data vector visualizer",
                "https://mosaic.d3cod3.org/reference.php?r=fft-extractor", scaleFactor);
}

//--------------------------------------------------------------
void moDataViewer::removeObjectContent(bool removeFileFromData){

}

OBJECT_REGISTER( moDataViewer , "data viewer", OFXVP_OBJECT_CAT_GUI)

#endif
