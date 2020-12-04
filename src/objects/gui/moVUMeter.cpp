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

#include "moVUMeter.h"

//--------------------------------------------------------------
moVUMeter::moVUMeter() : PatchObject("vu meter"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();  // signal

    _outletParams[0] = new float(); // RMS
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    this->height            /= 2;

    RMS                     = 0.0f;

}

//--------------------------------------------------------------
void moVUMeter::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addOutlet(VP_LINK_NUMERIC,"RMS");
}

//--------------------------------------------------------------
void moVUMeter::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void moVUMeter::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    RMS = ofClamp(static_cast<ofSoundBuffer *>(_inletParams[0])->getRMSAmplitude(),0.0,1.0);

    if(this->inletsConnected[0]){
        *(float *)&_outletParams[0] = RMS;
    }else{
        *(float *)&_outletParams[0] = 0.0f;
    }
}

//--------------------------------------------------------------
void moVUMeter::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){

}

//--------------------------------------------------------------
void moVUMeter::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
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

        ImGuiEx::VUMeter(_nodeCanvas.getNodeDrawList(), 0, ImGui::GetWindowSize().y - (26*scaleFactor), RMS);

        _nodeCanvas.EndNodeContent();
    }
}

//--------------------------------------------------------------
void moVUMeter::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Simple VUMeter, shows the audio signal level.",
                "https://mosaic.d3cod3.org/reference.php?r=vu-meter", scaleFactor);
}

//--------------------------------------------------------------
void moVUMeter::removeObjectContent(bool removeFileFromData){
    
}

OBJECT_REGISTER( moVUMeter, "vu meter", OFXVP_OBJECT_CAT_GUI)

#endif
