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

#include "Smooth.h"

//--------------------------------------------------------------
Smooth::Smooth() : PatchObject("smooth"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // input
    _inletParams[1] = new float();  // smoothing
    *(float *)&_inletParams[0] = 0.0f;
    *(float *)&_inletParams[1] = 1.0f;

    _outletParams[0] = new float(); // output
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    minRange    = 0.0f;
    maxRange    = 1.0f;
    smoothing   = 1.0f;

    loaded = false;

}

//--------------------------------------------------------------
void Smooth::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"number");
    this->addInlet(VP_LINK_NUMERIC,"smoothing");

    this->addOutlet(VP_LINK_NUMERIC,"smoothedValue");

    this->setCustomVar(smoothing,"SMOOTHING");
}

//--------------------------------------------------------------
void Smooth::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    this->unusedArgs(mainWindow);
}

//--------------------------------------------------------------
void Smooth::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    if(this->inletsConnected[0]){
        *(float *)&_outletParams[0] = *(float *)&_outletParams[0]*(1.0f-smoothing) + *(float *)&_inletParams[0]*smoothing;
        if(*(float *)&_inletParams[0] > maxRange){
            maxRange = *(float *)&_inletParams[0];
        }else if(*(float *)&_inletParams[0] < minRange){
            minRange = *(float *)&_inletParams[0];
        }
    }else{
        *(float *)&_outletParams[0] = 0.0f;
        minRange    = 0.0f;
        maxRange    = 1.0f;
    }

    if(this->inletsConnected[1]){
        smoothing = ofClamp(*(float *)&_inletParams[1],0.0f,1.0f);
    }

    if(!loaded){
        loaded = true;
        smoothing = this->getCustomVar("SMOOTHING");
    }

}

//--------------------------------------------------------------
void Smooth::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void Smooth::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGuiEx::plotValue(*(float *)&_outletParams[0], minRange, maxRange, IM_COL32(255,255,255,255), this->scaleFactor);


        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void Smooth::drawObjectNodeConfig(){
    ImGui::Spacing();
    ImGui::PushItemWidth(224*scaleFactor);
    if(ImGui::SliderFloat("Smoothing",&smoothing,0.0f,1.0f)){
        this->setCustomVar(smoothing,"SMOOTHING");
    }

    ImGuiEx::ObjectInfo(
                "Apply interpolation smoothing filter to received numerical data flow.",
                "https://mosaic.d3cod3.org/reference.php?r=smooth", scaleFactor);
}

//--------------------------------------------------------------
void Smooth::removeObjectContent(bool removeFileFromData){

}


OBJECT_REGISTER( Smooth, "smooth", OFXVP_OBJECT_CAT_MATH)

#endif
