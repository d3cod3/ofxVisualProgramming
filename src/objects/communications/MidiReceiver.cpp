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

#include "MidiReceiver.h"

//--------------------------------------------------------------
MidiReceiver::MidiReceiver() : PatchObject("midi receiver"){

    this->numInlets  = 0;
    this->numOutlets = 5;

    _outletParams[0] = new float();         // channel
    *(float *)&_outletParams[0] = 0.0f;
    _outletParams[1] = new float();         // control
    *(float *)&_outletParams[1] = 0.0f;
    _outletParams[2] = new float();         // value
    *(float *)&_outletParams[2] = 0.0f;
    _outletParams[3] = new float();         // pitch
    *(float *)&_outletParams[3] = 0.0f;
    _outletParams[4] = new float();         // velocity
    *(float *)&_outletParams[4] = 0.0f;

    this->initInletsState();

    midiDeviceID        = 0;

    loaded              = false;

}

//--------------------------------------------------------------
void MidiReceiver::newObject(){
    PatchObject::setName( this->objectName );

    this->addOutlet(VP_LINK_NUMERIC,"channel");
    this->addOutlet(VP_LINK_NUMERIC,"control");
    this->addOutlet(VP_LINK_NUMERIC,"value");
    this->addOutlet(VP_LINK_NUMERIC,"pitch");
    this->addOutlet(VP_LINK_NUMERIC,"velocity");

    this->setCustomVar(static_cast<float>(midiDeviceID),"DEVICE_ID");
}

//--------------------------------------------------------------
void MidiReceiver::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    midiIn.listInPorts();
    midiDevicesList = midiIn.getInPortList();

}

//--------------------------------------------------------------
void MidiReceiver::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(midiDevicesList.size() > 0){
        if(midiIn.isOpen()){
            *(float *)&_outletParams[0] = lastMessage.channel;
            *(float *)&_outletParams[1] = lastMessage.control;
            *(float *)&_outletParams[2] = lastMessage.value;
            *(float *)&_outletParams[3] = lastMessage.pitch;
            *(float *)&_outletParams[4] = lastMessage.velocity;
        }else{
            *(float *)&_outletParams[0] = 0.0f;
            *(float *)&_outletParams[1] = 0.0f;
            *(float *)&_outletParams[2] = 0.0f;
            *(float *)&_outletParams[3] = 0.0f;
            *(float *)&_outletParams[4] = 0.0f;
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
            midiIn.openPort(midiDeviceID);

            // don't ignore sysex, timing, & active sense messages,
            midiIn.ignoreTypes(false, false, false);
            midiIn.addListener(this);
            midiIn.setVerbose(true);
        }else{
            ofLog(OF_LOG_WARNING,"You have no MIDI devices available, please connect one in order to use the midi receiver object!");
        }
    }

}

//--------------------------------------------------------------
void MidiReceiver::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){

}

//--------------------------------------------------------------
void MidiReceiver::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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
            ImGui::Text("Channel\nControl\nValue\nPitch\nVelocity"); ImGui::SameLine();
            ImGui::Text("%i\n%i\n%i\n%i\n%i",static_cast<int>(floor(*(float *)&_outletParams[0])),static_cast<int>(floor(*(float *)&_outletParams[1])),static_cast<int>(floor(*(float *)&_outletParams[2])),static_cast<int>(floor(*(float *)&_outletParams[3])),static_cast<int>(floor(*(float *)&_outletParams[4])));
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void MidiReceiver::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(midiDevicesList.size() > 0){
        if(ImGui::BeginCombo("Device", midiDevicesList.at(midiDeviceID).c_str() )){
            for(int i=0; i < midiDevicesList.size(); ++i){
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
                "Receive data from a physical midi interface",
                "https://mosaic.d3cod3.org/reference.php?r=midi-receiver", scaleFactor);
}

//--------------------------------------------------------------
void MidiReceiver::removeObjectContent(bool removeFileFromData){
    if(midiDevicesList.size() > 0){
        if(midiIn.isOpen()){
            midiIn.closePort();
        }
        midiIn.removeListener(this);
    }
}

//--------------------------------------------------------------
void MidiReceiver::resetMIDISettings(int devID){

    if(devID!=midiDeviceID){
        ofLog(OF_LOG_NOTICE,"Changing MIDI Device to: %s", midiIn.getInPortName(devID).c_str());

        midiDeviceID = devID;

        this->setCustomVar(static_cast<float>(midiDeviceID),"DEVICE_ID");

        midiIn.closePort();
        midiIn.openPort(midiDeviceID);

        if(midiIn.isOpen()){
            ofLog(OF_LOG_NOTICE,"MIDI device %s connected!", midiIn.getInPortName(devID).c_str());
        }

    }

}

//--------------------------------------------------------------
void MidiReceiver::rescanMIDI(){
    midiIn.listInPorts();
    midiDevicesList = midiIn.getInPortList();

    // open port by number
    if(midiDevicesList.size() > 0){
        midiIn.openPort(midiDeviceID);

        // don't ignore sysex, timing, & active sense messages,
        midiIn.ignoreTypes(false, false, false);
        midiIn.addListener(this);
        midiIn.setVerbose(true);
    }else{
        ofLog(OF_LOG_WARNING,"You have no MIDI devices available, please connect one in order to use the midi receiver object!");
    }
}

//--------------------------------------------------------------
void MidiReceiver::newMidiMessage(ofxMidiMessage& msg){
    //ofLog(OF_LOG_NOTICE,"%s",msg.toString().c_str());
    lastMessage = msg;
}

OBJECT_REGISTER( MidiReceiver, "midi receiver", OFXVP_OBJECT_CAT_COMMUNICATIONS)

#endif
