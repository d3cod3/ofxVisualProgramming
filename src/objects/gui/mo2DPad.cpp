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

#include "mo2DPad.h"

//--------------------------------------------------------------
mo2DPad::mo2DPad() : PatchObject("2d pad"){

    this->numInlets  = 2;
    this->numOutlets = 2;

    _inletParams[0] = new float();  // X
    _inletParams[1] = new float();  // Y
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]) = 0.0f;
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]) = 0.0f;

    _outletParams[0] = new float(); // output X
    _outletParams[1] = new float(); // output Y
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0.0f;
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[1]) = 0.0f;

    this->initInletsState();

    loaded              = false;

    _x = 0.5f;
    _y = 0.5f;

}

//--------------------------------------------------------------
void mo2DPad::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"x");
    this->addInlet(VP_LINK_NUMERIC,"y");
    this->addOutlet(VP_LINK_NUMERIC,"padX");
    this->addOutlet(VP_LINK_NUMERIC,"padY");

    this->setCustomVar(static_cast<float>(_x),"XPOS");
    this->setCustomVar(static_cast<float>(_y),"YPOS");
}

//--------------------------------------------------------------
void mo2DPad::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);
}

//--------------------------------------------------------------
void mo2DPad::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){


    if(this->inletsConnected[0]){
        _x = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]),0.0f,1.0f);
    }

    if(this->inletsConnected[1]){
        _y = ofClamp(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]),0.0f,1.0f);
    }

    if(!loaded){
        loaded = true;
        _x = this->getCustomVar("XPOS");
        _y = this->getCustomVar("YPOS");
    }

    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = _x;
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[1]) = _y;

}

//--------------------------------------------------------------
void mo2DPad::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void mo2DPad::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        if(ImGuiEx::Pad2D(_nodeCanvas.getNodeDrawList(), 0, (this->height*_nodeCanvas.GetCanvasScale()) - (26*this->scaleFactor),&_x,&_y)){
            this->setCustomVar(static_cast<float>(_x),"XPOS");
            this->setCustomVar(static_cast<float>(_y),"YPOS");
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void mo2DPad::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Simultaneously adjust two float values XY (with a range from 0.0 to 1.0) by moving the pad point.",
                "https://mosaic.d3cod3.org/reference.php?r=2d-pad", scaleFactor);
}

//--------------------------------------------------------------
void mo2DPad::removeObjectContent(bool removeFileFromData){
    
}


OBJECT_REGISTER( mo2DPad, "2d pad", OFXVP_OBJECT_CAT_GUI)

#endif
