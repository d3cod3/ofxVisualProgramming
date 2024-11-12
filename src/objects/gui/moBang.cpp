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

#include "moBang.h"

//--------------------------------------------------------------
moBang::moBang() :
        PatchObject("bang")

{

    this->numInlets  = 1;
    this->numOutlets = 2;

    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;

    _outletParams[0] = new float(); // output numeric
    *(float *)&_outletParams[0] = 0.0f;

    _outletParams[1] = new string(); // output string
    *static_cast<string *>(_outletParams[1]) = "";

    isAudioOUTObject        = true;

    this->initInletsState();

    bang            = false;
    isBangFinished  = true;

}

//--------------------------------------------------------------
void moBang::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"bang");

    this->addOutlet(VP_LINK_NUMERIC,"bang");
    this->addOutlet(VP_LINK_STRING,"bang");
}

//--------------------------------------------------------------
void moBang::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    pressColor = { 250/255.0f, 250/255.0f, 5/255.0f, 1.0f };
    releaseColor = { 0.f, 0.f, 0.f, 0.f };

    currentColor = releaseColor;
}

//--------------------------------------------------------------
void moBang::setupAudioOutObjectContent(pdsp::Engine &engine){
    unusedArgs(engine);

}

//--------------------------------------------------------------
void moBang::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

}

//--------------------------------------------------------------
void moBang::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

}

//--------------------------------------------------------------
void moBang::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        // BANG (PD Style) button
        auto state = ImGuiEx::BangButton("", currentColor, ImVec2(ImGui::GetWindowSize().x,ImGui::GetWindowSize().y));

        if (state == SmartButtonState_Pressed){
            currentColor = pressColor;
            if(!bang && !this->inletsConnected[0]){
                bang = true;
            }
        }

        if(state == SmartButtonState_Released){
            currentColor = releaseColor;
            if(bang && !this->inletsConnected[0]){
                bang = false;
                isBangFinished = true;
            }
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void moBang::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Triggers a bang. You can control it manually with the mouse or automate it by its inlet.",
                "https://mosaic.d3cod3.org/reference.php?r=bang", scaleFactor);
}

//--------------------------------------------------------------
void moBang::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
}

//--------------------------------------------------------------
void moBang::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);

    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] < 1.0){
            bang = false;
            isBangFinished = true;
        }else{
            bang = true;
        }
    }

    if(bang && isBangFinished){
        isBangFinished = false;

        currentColor = pressColor;

        *(float *)&_outletParams[0] = static_cast<float>(bang);
        *static_cast<string *>(_outletParams[1]) = "bang";

    }else{
        currentColor = releaseColor;

        *(float *)&_outletParams[0] = 0;
        *static_cast<string *>(_outletParams[1]) = "";
    }

}

OBJECT_REGISTER( moBang, "bang", OFXVP_OBJECT_CAT_GUI)

#endif
