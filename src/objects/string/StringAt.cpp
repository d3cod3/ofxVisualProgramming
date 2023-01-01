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

#include "StringAt.h"

//--------------------------------------------------------------
StringAt::StringAt() : PatchObject("string at"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new string();             // input string
    *static_cast<string *>(_inletParams[0]) = "";
    _inletParams[1] = new float();              // at
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new string();            // char
    *static_cast<string *>(_outletParams[0]) = "";

    this->initInletsState();

    stringAt            = 0;
    loaded              = false;

}

//--------------------------------------------------------------
void StringAt::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_STRING,"string");
    this->addInlet(VP_LINK_NUMERIC,"at");

    this->addOutlet(VP_LINK_STRING,"char");

    this->setCustomVar(static_cast<float>(stringAt),"AT");
}

//--------------------------------------------------------------
void StringAt::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);
}

//--------------------------------------------------------------
void StringAt::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){


    if(this->inletsConnected[1]){
        stringAt = static_cast<int>(floor(*(float *)&_inletParams[1]));
    }

    if(this->inletsConnected[0]){
        if(stringAt >= 0 && stringAt < static_cast<string *>(_inletParams[0])->size()){
            string s(1,static_cast<string *>(_inletParams[0])->at(stringAt));
            *static_cast<string *>(_outletParams[0]) = s;
        }else{
            *static_cast<string *>(_outletParams[0]) = "";
        }
    }else{
        *static_cast<string *>(_outletParams[0]) = "";
    }

    if(!loaded){
        loaded = true;
        stringAt = static_cast<int>(floor(this->getCustomVar("AT")));
    }

}

//--------------------------------------------------------------
void StringAt::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void StringAt::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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
        if(ImGui::DragInt("", &stringAt)){
            if(stringAt < 0){
                stringAt = 0;
            }
            this->setCustomVar(static_cast<float>(stringAt),"AT");
        }
        ImGui::Spacing();
        ImGui::Text("string[%i] = %s", stringAt,static_cast<string *>(_outletParams[0])->c_str());
        ImGui::PopItemWidth();

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void StringAt::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Extracts the char at a particular position from a string",
                "https://mosaic.d3cod3.org/reference.php?r=vector-at", scaleFactor);
}

//--------------------------------------------------------------
void StringAt::removeObjectContent(bool removeFileFromData){

}


OBJECT_REGISTER( StringAt, "string at", OFXVP_OBJECT_CAT_STRING)

#endif
