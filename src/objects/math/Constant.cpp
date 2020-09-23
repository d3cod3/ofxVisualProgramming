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

#include "Constant.h"

//--------------------------------------------------------------
Constant::Constant() :
        // Extend the classes you need
        PatchObject("constant"),

        // define default values
        inputValueNew(0.f, "")
{

    this->numInlets  = 2;
    this->numOutlets = 2;

    // Bind Inlets to values
    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;

    *(float *)&_inletParams[1] = inputValueNew.get(); // value

    // Bind outlets to values
    *(float *)&_outletParams[0] = inputValueNew.get(); // float output

     _outletParams[1] = new string(); // string output
     *static_cast<string *>(_outletParams[1]) = "";

    this->initInletsState();

    bang                = false;
    nextFrame           = true;
    loaded              = false;

    this->height        /= 2;

}

//--------------------------------------------------------------
void Constant::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"number");
    this->addOutlet(VP_LINK_NUMERIC,"number");
    this->addOutlet(VP_LINK_STRING,"numberString");

    this->setCustomVar(static_cast<float>(inputValueNew.get()),"NUMBER");
}

//--------------------------------------------------------------
void Constant::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void Constant::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] < 1.0){
            bang = false;
        }else{
            bang = true;
        }
    }

    if(this->inletsConnected[1]){
      inputValueNew = *(float *)&_inletParams[1]; 
    }

    if(!nextFrame){
        nextFrame = true;
        *(float *)&_outletParams[0] = inputValueNew;
    }

    if(bang){
        nextFrame = false;
        *(float *)&_outletParams[0] = inputValueNew+0.00001f;
        *static_cast<string *>(_outletParams[1]) = ofToString( inputValueNew.get() );
    }

    if(!loaded){
        loaded = true;
        inputValueNew = this->getCustomVar("NUMBER");
    }

}

//--------------------------------------------------------------
void Constant::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void Constant::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGui::Dummy(ImVec2(-1,ImGui::GetWindowSize().y/2 - (28*scaleFactor))); // Padding top
        ImGui::PushItemWidth(-1);
        if(ImGui::DragFloat("", &inputValueNew.get(),0.01f)){
            this->setCustomVar(static_cast<float>(inputValueNew.get()),"NUMBER");
        }
        ImGui::PopItemWidth();

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void Constant::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Single numeric value controller.",
                "https://mosaic.d3cod3.org/reference.php?r=constant", scaleFactor);
}

//--------------------------------------------------------------
void Constant::removeObjectContent(bool removeFileFromData){

}

OBJECT_REGISTER( Constant, "constant", OFXVP_OBJECT_CAT_MATH)
