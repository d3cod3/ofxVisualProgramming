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

#include "SimpleNoise.h"

//--------------------------------------------------------------
SimpleNoise::SimpleNoise() : PatchObject("1D noise"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // step
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]) = 0.001f;

    _outletParams[0] = new float(); // output
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0.0f;

    this->initInletsState();

    step         = 0.001f;

    loaded      = false;
}

//--------------------------------------------------------------
void SimpleNoise::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"step");

    this->addOutlet(VP_LINK_NUMERIC,"noise");

    this->setCustomVar(step,"STEP");
}

//--------------------------------------------------------------
void SimpleNoise::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    ofSetRandomSeed(ofGetElapsedTimeMillis());

    timePosition = ofRandom(10000);
}

//--------------------------------------------------------------
void SimpleNoise::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = ofNoise(timePosition);

    if(this->inletsConnected[0]){
        step = fabs(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]));
    }

    timePosition += step;

    if(!loaded){
        loaded = true;
        step = this->getCustomVar("STEP");
    }
}

//--------------------------------------------------------------
void SimpleNoise::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void SimpleNoise::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGuiEx::plotValue(*ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]), 0.0f, 1.0f, IM_COL32(255,255,255,255), this->height*_nodeCanvas.GetCanvasScale() - (IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT), this->scaleFactor);

        _nodeCanvas.EndNodeContent();
    }
}

//--------------------------------------------------------------
void SimpleNoise::drawObjectNodeConfig(){
    ImGui::Spacing();
    ImGui::PushItemWidth(130*scaleFactor);
    if(ImGui::DragFloat("step", &step,0.0001f,0.0f,100.0f,"%.4f")){
        this->setCustomVar(step,"STEP");
    }
    ImGui::PopItemWidth();

    ImGuiEx::ObjectInfo(
                "Standard 1D Perlin noise generator.",
                "https://mosaic.d3cod3.org/reference.php?r=simple-noise", scaleFactor);
}

//--------------------------------------------------------------
void SimpleNoise::removeObjectContent(bool removeFileFromData){
    
}

OBJECT_REGISTER( SimpleNoise, "1D noise", OFXVP_OBJECT_CAT_MATH)

#endif
