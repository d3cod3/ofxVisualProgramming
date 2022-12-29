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

#include "BangToFloat.h"

//--------------------------------------------------------------
BangToFloat::BangToFloat() : PatchObject("bang to float"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;

    _inletParams[1] = new float();  // number
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new float(); // output
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    number              = 0.0f;
    bang                = false;
    loaded              = false;

    this->height        *= 0.5f;

}

//--------------------------------------------------------------
void BangToFloat::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"number");

    this->addOutlet(VP_LINK_NUMERIC,"number");

    this->setCustomVar(number,"NUMBER");
}

//--------------------------------------------------------------
void BangToFloat::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){    
    this->unusedArgs(mainWindow);
}

//--------------------------------------------------------------
void BangToFloat::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] < 1.0){
            bang = false;
        }else{
            bang = true;
        }
    }

    if(this->inletsConnected[1]){
      number = *(float *)&_inletParams[1];
    }

    if(bang && this->inletsConnected[1]){
        *(float *)&_outletParams[0] = *(float *)&_inletParams[1];
    }else if(bang && !this->inletsConnected[1]){
        *(float *)&_outletParams[0] = number;
    }else{
        *(float *)&_outletParams[0] = 0.0f;
    }

    if(!loaded){
        loaded = true;
        number = this->getCustomVar("NUMBER");
    }

}

//--------------------------------------------------------------
void BangToFloat::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void BangToFloat::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGui::Dummy(ImVec2(-1,ImGui::GetWindowSize().y/2 - (26*scaleFactor))); // Padding top
        ImGui::PushItemWidth(-1);
        if(ImGui::DragFloat("", &number,0.1f)){
            this->setCustomVar(number,"NUMBER");
        }
        ImGui::PopItemWidth();

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void BangToFloat::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "This object only sends the configured numerical value after receiving a bang",
                "https://mosaic.d3cod3.org/reference.php?r=bang-to-float", scaleFactor);
}

//--------------------------------------------------------------
void BangToFloat::removeObjectContent(bool removeFileFromData){

}


OBJECT_REGISTER( BangToFloat, "bang to float", OFXVP_OBJECT_CAT_DATA)

#endif
