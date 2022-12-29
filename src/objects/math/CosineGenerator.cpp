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

#include "CosineGenerator.h"

//--------------------------------------------------------------
CosineGenerator::CosineGenerator() : PatchObject("cosine generator")
{

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;
    _inletParams[1] = new float();  // speed
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new float(); // cosine value
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    angle = 0.0f;
    increment = TWO_PI/360.0f;

    loaded  = false;

}

//--------------------------------------------------------------
void CosineGenerator::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"speed");

    this->addOutlet(VP_LINK_NUMERIC,"cosine");

    this->setCustomVar(increment,"SPEED");
}

//--------------------------------------------------------------
void CosineGenerator::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    this->unusedArgs(mainWindow);
}

//--------------------------------------------------------------
void CosineGenerator::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] < 1.0){
            bang = false;
        }else{
            bang = true;
        }
    }

    if(this->inletsConnected[1]){
        increment = ofClamp(*(float *)&_inletParams[1],0.0f,PI);
    }

    if(bang){
        angle += increment;
        if(angle >= TWO_PI || angle < 0.0f){
            angle = 0.0f;
        }
        *(float *)&_outletParams[0] = static_cast<float>(cos(angle));
    }

    if(!loaded){
        loaded = true;
        increment = this->getCustomVar("SPEED");
    }
}

//--------------------------------------------------------------
void CosineGenerator::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void CosineGenerator::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGuiEx::plotValue(*(float *)&_outletParams[0], -1.0f, 1.0f, IM_COL32(255,255,255,255), this->scaleFactor);

        _nodeCanvas.EndNodeContent();
    }
}

//--------------------------------------------------------------
void CosineGenerator::drawObjectNodeConfig(){
    if(ImGui::SliderFloat("speed",&increment,0.0f,PI)){
        this->setCustomVar(increment,"SPEED");
    }

    ImGuiEx::ObjectInfo(
                "Cosine function generator.",
                "https://mosaic.d3cod3.org/reference.php?r=cosine-generator", scaleFactor);
}

//--------------------------------------------------------------
void CosineGenerator::removeObjectContent(bool removeFileFromData){

}

OBJECT_REGISTER( CosineGenerator, "cosine generator", OFXVP_OBJECT_CAT_MATH)

#endif
