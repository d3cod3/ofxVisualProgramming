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
        PatchObject("number"),

        // define default values
        inputValueNew(0.f, "")
{

    this->numInlets  = 2;
    this->numOutlets = 2;

    // Bind Inlets to values
    _inletParams[0] = new float();  // bang
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]) = 0.0f;

    _inletParams[1] = new float();
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]) = inputValueNew.get(); // value

    // Bind outlets to values
    _outletParams[0] = new float();
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = inputValueNew.get(); // float output

     _outletParams[1] = new string(); // string output
     *ofxVP_CAST_PIN_PTR<string>(this->_outletParams[1]) = "";

    this->initInletsState();

    bang                = false;
    nextFrame           = true;
    isON                = false;

    loaded              = false;
    varName             = "";

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
    this->setCustomVar(static_cast<float>(isON),"ON");
}

//--------------------------------------------------------------
void Constant::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);
}

//--------------------------------------------------------------
void Constant::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(this->inletsConnected[0] && !isON){
        if(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]) < 1.0){
            bang = false;
        }else{
            bang = true;
        }
    }

    if(this->inletsConnected[1]){
      inputValueNew = *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]);
    }

    if(!nextFrame){
        nextFrame = true;
        *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = inputValueNew;
    }

    if(bang){
        nextFrame = false;
        *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = inputValueNew+0.00001f;
        *ofxVP_CAST_PIN_PTR<string>(this->_outletParams[1]) = ofToString( inputValueNew.get() );
    }

    if(!loaded){
        loaded = true;
        inputValueNew = this->getCustomVar("NUMBER");
        isON = static_cast<bool>(this->getCustomVar("ON"));
        bang = isON;
        if(filepath != "" && filepath.find("|") != filepath.npos){
            varName = filepath.substr(filepath.find("|")+2,filepath.size());
            this->setSpecialName("| "+varName);
        }
    }

}

//--------------------------------------------------------------
void Constant::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);
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

        ImGui::SetCursorPos(ImVec2(IMGUI_EX_NODE_PINS_WIDTH_NORMAL+(4*scaleFactor), (this->height/2 *_nodeCanvas.GetCanvasScale()) - (6*scaleFactor)));
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

    ImGui::Spacing();
    ImGui::PushItemWidth(80*scaleFactor);
    ImGui::InputText("Variable name",&varName);
    ImGui::Spacing();
    if(ImGui::Button("APPLY",ImVec2(224*scaleFactor,26*scaleFactor))){
        this->setSpecialName("| "+varName);
        filepath = this->getSpecialName();
        this->saveConfig(false);
    }
    ImGui::Spacing();
    ImGui::Spacing();
    if(ImGui::Checkbox("ON",&isON)){
        this->setCustomVar(static_cast<float>(isON),"ON");
        bang = isON;
    }



    ImGuiEx::ObjectInfo(
                "Single numeric value controller.",
                "https://mosaic.d3cod3.org/reference.php?r=constant", scaleFactor);
}

//--------------------------------------------------------------
void Constant::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
}

OBJECT_REGISTER( Constant, "number", OFXVP_OBJECT_CAT_MATH)
