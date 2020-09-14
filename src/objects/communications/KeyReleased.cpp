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

#include "KeyReleased.h"

//--------------------------------------------------------------
KeyReleased::KeyReleased() : PatchObject("key released"){

    this->numInlets  = 0;
    this->numOutlets = 1;

    _outletParams[0] = new float(); // output
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    requiredKey         = -1;
    lastKey             = -1;
    persistentKey       = -1;

    loaded              = false;

}

//--------------------------------------------------------------
void KeyReleased::newObject(){
    PatchObject::setName( this->objectName );

    this->addOutlet(VP_LINK_NUMERIC,"bang");

    this->setCustomVar(static_cast<float>(requiredKey),"KEY");
}

//--------------------------------------------------------------
void KeyReleased::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    ofAddListener(mainWindow->events().keyReleased,this,&KeyReleased::objectKeyReleased);
}

//--------------------------------------------------------------
void KeyReleased::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(lastKey == static_cast<int>(floor(this->getCustomVar("KEY"))) && lastKey != -1){
        lastKey = -1;
        *(float *)&_outletParams[0] = 1.0f;
    }else{
        *(float *)&_outletParams[0] = 0.0f;
    }

    if(!loaded){
        loaded = true;
        requiredKey = static_cast<int>(this->getCustomVar("KEY"));
    }

}

//--------------------------------------------------------------
void KeyReleased::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void KeyReleased::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            drawObjectNodeConfig();


            ImGui::EndMenu();
        }
        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        ImGui::Dummy(ImVec2(-1,ImGui::GetWindowSize().y/2 - 40)); // Padding top

        if(ImGui::InputInt("KEY",&requiredKey)){
            this->setCustomVar(static_cast<float>(requiredKey),"KEY");
        }
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Text("Last Key: %i",persistentKey);

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void KeyReleased::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Sends a bang when the selected key has been released. To know the key ascii code, press it and its number appears next to Last Key:",
                "https://mosaic.d3cod3.org/reference.php?r=key-released", scaleFactor);
}

//--------------------------------------------------------------
void KeyReleased::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void KeyReleased::objectKeyReleased(ofKeyEventArgs &e){
    lastKey         = e.key;
    persistentKey   = e.key;
}


OBJECT_REGISTER( KeyReleased, "key released", OFXVP_OBJECT_CAT_COMMUNICATIONS)

#endif
