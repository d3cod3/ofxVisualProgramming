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

#include "MidiSender.h"

//--------------------------------------------------------------
MidiSender::MidiSender() : PatchObject("midi sender"){

    this->numInlets  = 4;
    this->numOutlets = 0;

    _inletParams[0] = new float();         // trigger
    *(float *)&_inletParams[0] = 0.0f;
    _inletParams[1] = new float();         // channel
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();         // note
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();         // velocity
    *(float *)&_inletParams[3] = 0.0f;

    this->initInletsState();

    midiDeviceID        = 0;

    trigger             = false;
    lastNote            = 0.0f;

    this->width         *= 2;

    loaded              = false;

}

//--------------------------------------------------------------
void MidiSender::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"trigger");
    this->addInlet(VP_LINK_NUMERIC,"channel");
    this->addInlet(VP_LINK_NUMERIC,"note");
    this->addInlet(VP_LINK_NUMERIC,"velocity");

    this->setCustomVar(static_cast<float>(midiDeviceID),"DEVICE_ID");
}

//--------------------------------------------------------------
void MidiSender::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    midiOut.listOutPorts();
    midiDevicesList = midiOut.getOutPortList();

}

//--------------------------------------------------------------
void MidiSender::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(midiDevicesList.size() > 0){
        if(midiOut.isOpen()){
            if(this->inletsConnected[0] && this->inletsConnected[1] && this->inletsConnected[2] && this->inletsConnected[3]){
                if(lastNote != *(float *)&_inletParams[2] && trigger){
                    trigger = false;
                }

                if(*(float *)&_inletParams[0] != 0.0f && !trigger){
                    trigger = true;
                    midiOut.sendNoteOn(static_cast<int>(floor(*(float *)&_inletParams[1])),static_cast<int>(floor(*(float *)&_inletParams[2])),static_cast<int>(floor(*(float *)&_inletParams[3])));
                }

                if(*(float *)&_inletParams[0] == 0.0f && trigger){
                    trigger = false;
                    for(int i=0;i<128;i++){
                        midiOut.sendNoteOff(static_cast<int>(floor(*(float *)&_inletParams[1])),i,0);
                    }
                }

                lastNote = *(float *)&_inletParams[2];
            }
        }
    }

    if(!loaded){
        loaded = true;

        if(static_cast<int>(floor(this->getCustomVar("DEVICE_ID"))) >= 0 && static_cast<int>(floor(this->getCustomVar("DEVICE_ID"))) < static_cast<int>(midiDevicesList.size())){
            midiDeviceID = static_cast<int>(floor(this->getCustomVar("DEVICE_ID")));
        }else{
            midiDeviceID = 0;
            this->setCustomVar(static_cast<float>(midiDeviceID),"DEVICE_ID");
        }

        // open port by number
        if(midiDevicesList.size() > 0){
            midiOut.openPort(midiDeviceID);
        }else{
            ofLog(OF_LOG_WARNING,"%s","You have no MIDI devices available, please connect one in order to use the midi sender object!");
        }
    }

}

//--------------------------------------------------------------
void MidiSender::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);
}

//--------------------------------------------------------------
void MidiSender::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        if(midiDevicesList.size() > 0){
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, VHS_GRAY);
            ImGui::Text("%s",midiDevicesList.at(midiDeviceID).c_str());
            ImGui::PopStyleColor(1);
            ImGui::Spacing();
            ImGui::Text("Channel\nNote\nVelocity"); ImGui::SameLine();
            ImGui::Text("%i\n%i\n%i",static_cast<int>(floor(*(float *)&_inletParams[1])),static_cast<int>(floor(*(float *)&_inletParams[2])),static_cast<int>(floor(*(float *)&_inletParams[3])));
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void MidiSender::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(midiDevicesList.size() > 0){
        if(ImGui::BeginCombo("Device", midiDevicesList.at(midiDeviceID).c_str() )){
            for(int i=0; i < static_cast<int>(midiDevicesList.size()); ++i){
                bool is_selected = (midiDeviceID == i );
                if (ImGui::Selectable(midiDevicesList.at(i).c_str(), is_selected)){
                    resetMIDISettings(i);
                }
                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }else{
        ImGui::Text("No MIDI devices found!");

        if(midiDevicesList.size() == 0){
            if(ImGui::Button("Rescan Devices")){
                rescanMIDI();
            }
        }
    }



    ImGuiEx::ObjectInfo(
                "Send data to a physical ( or virtual ) midi interface",
                "https://mosaic.d3cod3.org/reference.php?r=midi-sender", scaleFactor);
}

//--------------------------------------------------------------
void MidiSender::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);

    if(midiDevicesList.size() > 0){
        if(midiOut.isOpen()){
            midiOut.closePort();
        }
    }
}

//--------------------------------------------------------------
void MidiSender::resetMIDISettings(int devID){

    if(devID!=midiDeviceID){
        ofLog(OF_LOG_NOTICE,"Changing MIDI Device to: %s", midiOut.getOutPortName(devID).c_str());

        midiDeviceID = devID;

        this->setCustomVar(static_cast<float>(midiDeviceID),"DEVICE_ID");

        midiOut.closePort();
        midiOut.openPort(midiDeviceID);

        if(midiOut.isOpen()){
            ofLog(OF_LOG_NOTICE,"MIDI device %s connected!", midiOut.getOutPortName(devID).c_str());
        }

    }

}

//--------------------------------------------------------------
void MidiSender::rescanMIDI(){
    midiOut.listOutPorts();
    midiDevicesList = midiOut.getOutPortList();

    // open port by number
    if(midiDevicesList.size() > 0){
        midiOut.openPort(midiDeviceID);
    }else{
        ofLog(OF_LOG_WARNING,"%s","You have no MIDI devices available, please connect one in order to use the midi sender object!");
    }
}

OBJECT_REGISTER( MidiSender, "midi sender", OFXVP_OBJECT_CAT_COMMUNICATIONS)

#endif
