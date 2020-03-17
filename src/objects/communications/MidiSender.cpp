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

#include "MidiSender.h"

//--------------------------------------------------------------
MidiSender::MidiSender() : PatchObject(){

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

    isGUIObject         = true;
    this->isOverGUI     = true;

    midiDeviceID        = 0;

    trigger             = false;
    lastNote            = 0.0f;

}

//--------------------------------------------------------------
void MidiSender::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"trigger");
    this->addInlet(VP_LINK_NUMERIC,"channel");
    this->addInlet(VP_LINK_NUMERIC,"note");
    this->addInlet(VP_LINK_NUMERIC,"velocity");

    this->setCustomVar(static_cast<float>(midiDeviceID),"DEVICE_ID");
}

//--------------------------------------------------------------
void MidiSender::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setTheme(new ofxDatGuiThemeCharcoal());
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onMatrixEvent(this, &MidiSender::onMatrixEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    midiDeviceName = gui->addLabel("");

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    midiOut.listOutPorts();
    midiDevicesList = midiOut.getOutPortList();

    if(static_cast<int>(floor(this->getCustomVar("DEVICE_ID"))) >= 0 && static_cast<int>(floor(this->getCustomVar("DEVICE_ID"))) < static_cast<int>(midiDevicesList.size())){
        midiDeviceID = static_cast<int>(floor(this->getCustomVar("DEVICE_ID")));
    }else{
        midiDeviceID = 0;
        this->setCustomVar(static_cast<float>(midiDeviceID),"DEVICE_ID");
    }
    
    // open port by number
    if(midiDevicesList.size() > 0){
        midiOut.openPort(midiDeviceID);
        midiDeviceName->setLabel(midiOut.getOutPortName(midiDeviceID));

        deviceSelector = gui->addMatrix("DEVICE",midiDevicesList.size(),true);
        deviceSelector->setUseCustomMouse(true);
        deviceSelector->setRadioMode(true);
        deviceSelector->getChildAt(midiDeviceID)->setSelected(true);
        deviceSelector->onMatrixEvent(this, &MidiSender::onMatrixEvent);
    
    }else{
        ofLog(OF_LOG_WARNING,"You have no MIDI devices available, please connect one in order to use the midi sender object!");
    }

}

//--------------------------------------------------------------
void MidiSender::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    if(!header->getIsCollapsed()){
        if(midiDevicesList.size() > 0){
            deviceSelector->update();
        }
    }

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

}

//--------------------------------------------------------------
void MidiSender::drawObjectContent(ofxFontStash *font){
    ofSetColor(30,31,36);
    ofDrawRectangle(0,0,this->width,this->height);
    ofSetColor(255);
    ofEnableAlphaBlending();
    string temp = "";
    for(int i=1;i<this->numInlets;i++){
        if(i==1){
            temp = "Channel ";
        }else if(i==2){
            temp = "Note ";
        }else if(i==3){
            temp = "Velocity ";
        }
        font->draw(temp+ofToString(static_cast<int>(floor(*(float *)&_inletParams[i]))),this->fontSize,this->width/2,this->headerHeight*2.3 + (i*this->fontSize*1.15));
    }
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void MidiSender::removeObjectContent(bool removeFileFromData){
    if(midiDevicesList.size() > 0){
        if(midiOut.isOpen()){
            midiOut.closePort();
        }
    }
}

//--------------------------------------------------------------
void MidiSender::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    if(midiDevicesList.size() > 0){
        deviceSelector->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }

    if(!header->getIsCollapsed()){
        if(midiDevicesList.size() > 0){
            this->isOverGUI = header->hitTest(_m-this->getPos()) || deviceSelector->hitTest(_m-this->getPos());
        }else{
            this->isOverGUI = header->hitTest(_m-this->getPos());
        }
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }
}

//--------------------------------------------------------------
void MidiSender::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        if(midiDevicesList.size() > 0){
            deviceSelector->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        }
    }else{
        ofNotifyEvent(dragEvent, nId);

        box->setFromCenter(_m.x, _m.y,box->getWidth(),box->getHeight());
        headerBox->set(box->getPosition().x,box->getPosition().y,box->getWidth(),headerHeight);

        x = box->getPosition().x;
        y = box->getPosition().y;

        for(int j=0;j<static_cast<int>(outPut.size());j++){
            outPut[j]->linkVertices[0].move(outPut[j]->posFrom.x,outPut[j]->posFrom.y);
            outPut[j]->linkVertices[1].move(outPut[j]->posFrom.x+20,outPut[j]->posFrom.y);
        }
    }
}

//--------------------------------------------------------------
void MidiSender::resetMIDISettings(int devID){

    if(devID!=midiDeviceID){
        ofLog(OF_LOG_NOTICE,"Changing MIDI Device to: %s", midiOut.getOutPortName(devID).c_str());

        midiDeviceID = devID;
        if(midiOut.getOutPortName(devID).size() > 22){
            midiDeviceName->setLabel(midiOut.getOutPortName(devID).substr(0,21)+"...");
        }else{
            midiDeviceName->setLabel(midiOut.getOutPortName(devID));
        }
        this->setCustomVar(static_cast<float>(midiDeviceID),"DEVICE_ID");

        midiOut.closePort();
        midiOut.openPort(midiDeviceID);

        if(midiOut.isOpen()){
            ofLog(OF_LOG_NOTICE,"MIDI device %s connected!", midiOut.getOutPortName(devID).c_str());
        }

    }

}

//--------------------------------------------------------------
void MidiSender::onMatrixEvent(ofxDatGuiMatrixEvent e){
    if(!header->getIsCollapsed()){
        if(midiDevicesList.size() > 0){
            if(e.target == deviceSelector){
                resetMIDISettings(e.child);
            }
        }
    }
}

OBJECT_REGISTER( MidiSender, "midi sender", OFXVP_OBJECT_CAT_COMMUNICATIONS)
