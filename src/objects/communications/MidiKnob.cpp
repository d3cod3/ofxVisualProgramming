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

#include "MidiKnob.h"

//--------------------------------------------------------------
MidiKnob::MidiKnob() : PatchObject("midi knob"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // control
    *(float *)&_inletParams[0] = 0.0f;
    _inletParams[1] = new float();  // value
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new float(); // output
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    lastControl     = 0;
    savedControl    = 0;
    actualValue     = 0.0f;
    loaded          = false;

    this->width     *= 1.5f;
    this->height    *= 1.5f;

}

//--------------------------------------------------------------
void MidiKnob::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"control");
    this->addInlet(VP_LINK_NUMERIC,"value");

    this->addOutlet(VP_LINK_NUMERIC,"value");

    this->setCustomVar(static_cast<float>(savedControl),"INDEX");
}

//--------------------------------------------------------------
void MidiKnob::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);
}

//--------------------------------------------------------------
void MidiKnob::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[0] && this->inletsConnected[1]){
        if(static_cast<int>(floor(*(float *)&_inletParams[0])) == savedControl){
            *(float *)&_outletParams[0] = *(float *)&_inletParams[1];
        }
    }else{
        *(float *)&_outletParams[0] = 0.0f;
    }

    actualValue = *(float *)&_outletParams[0];

    if(!loaded){
        loaded = true;
        lastControl  = static_cast<int>(this->getCustomVar("INDEX"));
        savedControl = lastControl;
    }

}

//--------------------------------------------------------------
void MidiKnob::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){

}

//--------------------------------------------------------------
void MidiKnob::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGui::SetCursorPos(ImVec2(IMGUI_EX_NODE_PINS_WIDTH_NORMAL+(4*scaleFactor), (this->height/4 *_nodeCanvas.GetCanvasScale()) - (6*scaleFactor)));

        if(ImGui::InputInt("CONTROL",&lastControl)){
            savedControl = lastControl;
            this->setCustomVar(static_cast<float>(savedControl),"INDEX");

        }

        ImGui::SetCursorPos(ImVec2((this->width/2 - 26*this->scaleFactor) *_nodeCanvas.GetCanvasScale(), (this->height - 110*this->scaleFactor) * _nodeCanvas.GetCanvasScale()));

        ImGuiKnobs::Knob("value", &actualValue, 0.0f, 127.0f, 0.1f, "%.0f", ImGuiKnobVariant_Stepped,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor);

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void MidiKnob::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "This object is used linked to the midi receiver object to map a knob on a midi device",
                "https://mosaic.d3cod3.org/reference.php?r=midi-knob", scaleFactor);
}

//--------------------------------------------------------------
void MidiKnob::removeObjectContent(bool removeFileFromData){

}



OBJECT_REGISTER( MidiKnob, "midi knob", OFXVP_OBJECT_CAT_COMMUNICATIONS)

#endif
