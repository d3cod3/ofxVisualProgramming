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
    this->numOutlets = 6;

    _outletParams[0] = new float();         // channel
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0.0f;
    _outletParams[1] = new float();         // control
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[1]) = 0.0f;
    _outletParams[2] = new float();         // value
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[2]) = 0.0f;
    _outletParams[3] = new float();         // pitch
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[3]) = 0.0f;
    _outletParams[4] = new float();         // velocity
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[4]) = 0.0f;
    _outletParams[5] = new vector<float>(); // midi notes array ( polyphony )

    this->initInletsState();

    midiDeviceID        = 0;
    isLogging           = false;

    this->width         *= 2;

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
    this->addOutlet(VP_LINK_ARRAY,"midi notes");

    this->setCustomVar(static_cast<float>(midiDeviceID),"DEVICE_ID");
}

//--------------------------------------------------------------
void MidiReceiver::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    midiIn.listInPorts();
    midiDevicesList = midiIn.getInPortList();

    emptyVec.assign(1,0.0f);

}

//--------------------------------------------------------------
void MidiReceiver::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(midiDevicesList.size() > 0){
        if(midiIn.isOpen()){
            /// queued message handling
            if(midiIn.hasWaitingMessages()) {
                ofxMidiMessage message;

                // add the latest message to the message queue
                while(midiIn.getNextMessage(message)) {
                    midiMessages.push_back(message);
                }

                // remove any old messages if we have too many
                while(midiMessages.size() > maxMessages) {
                    midiMessages.erase(midiMessages.begin());
                }
            }
            // last message data
            *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = lastMessage.channel;
            *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[1]) = lastMessage.control;
            *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[2]) = lastMessage.value;
            *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[3]) = lastMessage.pitch;
            *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[4]) = lastMessage.velocity;
            // message queue data
            ofxVP_CAST_PIN_PTR<vector<float>>(_outletParams[5])->clear();
            ofxVP_CAST_PIN_PTR<vector<float>>(_outletParams[5])->push_back(0); // vector first position save number of notes ON

            for(size_t i=0;i<midiMessages.size();i++){
                ofxMidiMessage &message = midiMessages[i];
                // store pitch, velocity in midinotes map
                if(message.status == MIDI_NOTE_ON){
                    midinotes[message.pitch] = message.velocity;
                }else if(message.status == MIDI_NOTE_OFF){
                    midinotes[message.pitch] = 0;
                }
            }

            for(map<int,int>::iterator it = midinotes.begin(); it != midinotes.end(); it++ ){
                if(it->second > 0){
                    ofxVP_CAST_PIN_PTR<vector<float>>(_outletParams[5])->at(0) += 1;
                    ofxVP_CAST_PIN_PTR<vector<float>>(_outletParams[5])->push_back(it->first);
                    ofxVP_CAST_PIN_PTR<vector<float>>(_outletParams[5])->push_back(it->second);
                }
            }


        }else{
            *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0.0f;
            *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[1]) = 0.0f;
            *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[2]) = 0.0f;
            *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[3]) = 0.0f;
            *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[4]) = 0.0f;

            *ofxVP_CAST_PIN_PTR<vector<float>>(_outletParams[5]) = emptyVec;
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
            ofLog(OF_LOG_WARNING,"%s","You have no MIDI devices available, please connect one in order to use the midi receiver object!");
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
            ImGui::Text("%i\n%i\n%i\n%i\n%i",static_cast<int>(floor(*ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]))),static_cast<int>(floor(*ofxVP_CAST_PIN_PTR<float>(this->_outletParams[1]))),static_cast<int>(floor(*ofxVP_CAST_PIN_PTR<float>(this->_outletParams[2]))),static_cast<int>(floor(*ofxVP_CAST_PIN_PTR<float>(this->_outletParams[3]))),static_cast<int>(floor(*ofxVP_CAST_PIN_PTR<float>(this->_outletParams[4]))));
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
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Checkbox("Log MIDI",&isLogging);
    }else{
        ImGui::Text("No MIDI devices found!");

        if(midiDevicesList.size() == 0){
            if(ImGui::Button("Rescan Devices")){
                rescanMIDI();
            }
        }
    }


    ImGuiEx::ObjectInfo(
                "Receive data from a physical/virtual midi interface",
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
        ofLog(OF_LOG_WARNING,"%s","You have no MIDI devices available, please connect one in order to use the midi receiver object!");
    }
}

//--------------------------------------------------------------
void MidiReceiver::newMidiMessage(ofxMidiMessage& msg){
    //ofLog(OF_LOG_NOTICE,"%s",msg.toString().c_str());
    lastMessage = msg;

    if(isLogging){
        if(msg.status < MIDI_SYSEX) {
            if(msg.status == MIDI_NOTE_ON || msg.status == MIDI_NOTE_OFF) {
                ofLog(OF_LOG_NOTICE,"MIDI Message STATUS: %s, PITCH: %i, VELOCITY: %i",ofxMidiMessage::getStatusString(msg.status).c_str(),msg.pitch,msg.velocity);
            }
        }
        if(msg.status == MIDI_CONTROL_CHANGE) {
            ofLog(OF_LOG_NOTICE,"MIDI Message STATUS: %s, CONTROL: %i, VALUE: %i",ofxMidiMessage::getStatusString(msg.status).c_str(),msg.control,msg.value);
        }
        else if(msg.status == MIDI_PROGRAM_CHANGE) {
            ofLog(OF_LOG_NOTICE,"MIDI Message STATUS: %s, VALUE: %i",ofxMidiMessage::getStatusString(msg.status).c_str(),msg.value);
        }
        else if(msg.status == MIDI_PITCH_BEND) {
            ofLog(OF_LOG_NOTICE,"MIDI Message STATUS: %s, VALUE: %i",ofxMidiMessage::getStatusString(msg.status).c_str(),msg.value);
        }
        else if(msg.status == MIDI_AFTERTOUCH) {
            ofLog(OF_LOG_NOTICE,"MIDI Message STATUS: %s, VALUE: %i",ofxMidiMessage::getStatusString(msg.status).c_str(),msg.value);
        }
        else if(msg.status == MIDI_POLY_AFTERTOUCH) {
            ofLog(OF_LOG_NOTICE,"MIDI Message STATUS: %s, PITCH: %i, VALUE: %i",ofxMidiMessage::getStatusString(msg.status).c_str(),msg.pitch,msg.value);
        }

    }

    // add the latest message to the message queue
    midiMessages.push_back(msg);

    // remove any old messages if we have too many
    while(midiMessages.size() > maxMessages) {
        midiMessages.erase(midiMessages.begin());
    }
}

OBJECT_REGISTER( MidiReceiver, "midi receiver", OFXVP_OBJECT_CAT_COMMUNICATIONS)

#endif
