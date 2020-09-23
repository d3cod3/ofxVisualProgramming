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

#include "RandomString.h"

//--------------------------------------------------------------
RandomString::RandomString() : PatchObject("random string")
{

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;

    _inletParams[1] = new float();  // length
    *(float *)&_inletParams[1] = 1.0f;

    _outletParams[0] = new string();  // random string
    *static_cast<string *>(_outletParams[0]) = "";

    this->initInletsState();

    bang            = false;

    length          = 1;

    loaded          = false;

    this->height    /= 2.0f;

}

//--------------------------------------------------------------
void RandomString::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"length");
    this->addOutlet(VP_LINK_STRING,"randomString");

    this->setCustomVar(length,"LENGTH");

}

//--------------------------------------------------------------
void RandomString::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void RandomString::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] < 1.0){
            bang = false;
        }else{
            bang = true;
        }
    }
    if(bang){
        *static_cast<string *>(_outletParams[0]) = random_string(static_cast<size_t>(floor(length)));
    }

    if(this->inletsConnected[1]){
        length = abs(static_cast<int>(floor(*(float *)&_inletParams[1])));
    }

    if(!loaded){
        loaded = true;
        length = this->getCustomVar("LENGTH");
    }
}

//--------------------------------------------------------------
void RandomString::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void RandomString::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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



        _nodeCanvas.EndNodeContent();
    }
}

//--------------------------------------------------------------
void RandomString::drawObjectNodeConfig(){
    ImGui::Spacing();
    ImGui::PushItemWidth(130*scaleFactor);
    if(ImGui::DragInt("string length", &length)){
        if(length < 1){
            length = 1;
        }
        this->setCustomVar(length,"LENGTH");
    }

    ImGuiEx::ObjectInfo(
                "Random string generator.",
                "https://mosaic.d3cod3.org/reference.php?r=random-string", scaleFactor);
}

//--------------------------------------------------------------
void RandomString::removeObjectContent(bool removeFileFromData){

}



OBJECT_REGISTER( RandomString, "random string", OFXVP_OBJECT_CAT_STRING)

#endif
