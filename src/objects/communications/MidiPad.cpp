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

#include "MidiPad.h"

//--------------------------------------------------------------
MidiPad::MidiPad() : PatchObject("midi pad"){

    this->numInlets  = 3;
    this->numOutlets = 3;

    _inletParams[0] = new float();  // pitch (index)
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]) = 0.0f;
    _inletParams[1] = new float();  // value
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]) = 0.0f;
    _inletParams[2] = new float();  // velocity
    *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[2]) = 0.0f;

    _outletParams[0] = new float(); // bang
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0.0f;
    _outletParams[1] = new float(); // value
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[1]) = 0.0f;
    _outletParams[2] = new float(); // velocity
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[2]) = 0.0f;

    this->initInletsState();

    lastPitch       = 0;
    savedPitch      = 0;
    onebang         = false;
    lockReadings    = false;

    loaded          = false;

    this->width *= 1.2f;

}

//--------------------------------------------------------------
void MidiPad::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"pitch");
    this->addInlet(VP_LINK_NUMERIC,"value");
    this->addInlet(VP_LINK_NUMERIC,"velocity");

    this->addOutlet(VP_LINK_NUMERIC,"bang");
    this->addOutlet(VP_LINK_NUMERIC,"pressure");
    this->addOutlet(VP_LINK_NUMERIC,"velocity");

    this->setCustomVar(static_cast<float>(savedPitch),"INDEX");
}

//--------------------------------------------------------------
void MidiPad::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);
}

//--------------------------------------------------------------
void MidiPad::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[0]){
        if(static_cast<int>(floor(*ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]))) == savedPitch){
            if(!lockReadings){
                lockReadings = true;
            }
            if(!onebang){
                onebang = true;
                *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 1.0f;
            }else{
                *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0.0f;
            }
        }else{
            *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0.0f;
            onebang = false;
        }

        if(lockReadings){
            if(this->inletsConnected[1]){
                *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[1]) = *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[1]);
            }
            if(this->inletsConnected[2]){
                *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[2]) = *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[2]);
            }
        }
        // velocity
        if(*ofxVP_CAST_PIN_PTR<float>(this->_outletParams[1]) == 0.0f && *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[2]) == 0.0f){
            *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0.0f;
            onebang = false;
            lockReadings = false;
        }
    }else{
        *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0.0f;
        *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[1]) = 0.0f;
        *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[2]) = 0.0f;
        onebang = false;
        lockReadings = false;
    }

    if(!loaded){
        loaded = true;
        lastPitch  = static_cast<int>(this->getCustomVar("INDEX"));
        savedPitch = lastPitch;
    }

}

//--------------------------------------------------------------
void MidiPad::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){

}

//--------------------------------------------------------------
void MidiPad::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        if(ImGui::InputInt("PITCH",&lastPitch)){
            savedPitch = lastPitch;
            this->setCustomVar(static_cast<float>(savedPitch),"INDEX");

        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void MidiPad::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "This object is used linked to the midi receiver object to map a pad on a midi device",
                "https://mosaic.d3cod3.org/reference.php?r=midi-pad", scaleFactor);
}

//--------------------------------------------------------------
void MidiPad::removeObjectContent(bool removeFileFromData){

}


OBJECT_REGISTER( MidiPad, "midi pad", OFXVP_OBJECT_CAT_COMMUNICATIONS)

#endif
