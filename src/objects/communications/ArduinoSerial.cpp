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

#include "ArduinoSerial.h"

//--------------------------------------------------------------
ArduinoSerial::ArduinoSerial() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets  = 1;

    _inletParams[0] = new vector<float>();

    _outletParams[0] = new vector<float>();

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    arduinoIcon         = new ofImage();

    serialDeviceID      = 0;
    baudRateID          = 0;

    resetTime           = ofGetElapsedTimeMillis();
}

//--------------------------------------------------------------
void ArduinoSerial::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_ARRAY,"data");
    this->addOutlet(VP_LINK_ARRAY,"dataFromArduino");

    this->setCustomVar(static_cast<float>(serialDeviceID),"DEVICE_ID");
    this->setCustomVar(static_cast<float>(baudRateID),"BAUDRATE_ID");
}

//--------------------------------------------------------------
void ArduinoSerial::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    arduinoIcon->load("images/arduino.png");

    ofLog(OF_LOG_NOTICE," ");
    ofLog(OF_LOG_NOTICE,"------------------- SERIAL DEVICES");


    deviceList = serial.getDeviceList();
    for(int i=0;i<deviceList.size();i++){
        ofLog(OF_LOG_NOTICE,"[%i] - %s",i,deviceList.at(i).getDeviceName().c_str());
        deviceNameList.push_back(deviceList.at(i).getDeviceName());
    }
    baudrateList = {"9600","19200","38400","57600","74880","115200","230400","250000"};

    for(int i=0;i<MAX_ARDUINO_RECEIVING_VECTOR_LENGTH;i++){
        static_cast<vector<float> *>(_outletParams[0])->push_back(0);
    }

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onMatrixEvent(this, &ArduinoSerial::onMatrixEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    serialDeviceName = gui->addLabel("");
    if(static_cast<int>(floor(this->getCustomVar("DEVICE_ID"))) >= 0 && static_cast<int>(floor(this->getCustomVar("DEVICE_ID"))) < static_cast<int>(deviceNameList.size())){
        serialDeviceID = static_cast<int>(floor(this->getCustomVar("DEVICE_ID")));
    }else{
        serialDeviceID = 0;
        this->setCustomVar(static_cast<float>(serialDeviceID),"DEVICE_ID");
    }
    if(static_cast<int>(floor(this->getCustomVar("BAUDRATE_ID"))) >= 0 && static_cast<int>(floor(this->getCustomVar("BAUDRATE_ID"))) < static_cast<int>(baudrateList.size())){
        baudRateID = ofClamp(static_cast<int>(floor(this->getCustomVar("BAUDRATE_ID"))),0,7);
    }else{
        baudRateID = 0;
        this->setCustomVar(static_cast<float>(baudRateID),"BAUDRATE_ID");
    }

    if(deviceNameList.size() > 0){
        serialDeviceName->setLabel(deviceNameList.at(serialDeviceID));

        deviceSelector = gui->addMatrix("DEVICE",deviceNameList.size(),true);
        deviceSelector->setUseCustomMouse(true);
        deviceSelector->setRadioMode(true);
        deviceSelector->getChildAt(serialDeviceID)->setSelected(true);
        deviceSelector->onMatrixEvent(this, &ArduinoSerial::onMatrixEvent);
        gui->addBreak();
        baudRates = gui->addDropdown("Baudrate",baudrateList);
        baudRates->onDropdownEvent(this,&ArduinoSerial::onDropdownEvent);
        baudRates->setUseCustomMouse(true);
        baudRates->select(baudRateID);
        for(int i=0;i<baudRates->children.size();i++){
            baudRates->getChildAt(i)->setUseCustomMouse(true);
        }

        serial.setup(serialDeviceID, ofToInt(baudrateList.at(baudRateID)));

    }else{
        ofLog(OF_LOG_WARNING,"You have no SERIAL devices available, please enable one in order to use the arduino sender object!");
    }

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

}

//--------------------------------------------------------------
void ArduinoSerial::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){
    gui->update();
    header->update();
    if(!header->getIsCollapsed()){
        if(deviceNameList.size() > 0){
            deviceSelector->update();
        }
        baudRates->update();
        for(int i=0;i<baudRates->children.size();i++){
            baudRates->getChildAt(i)->update();
        }
    }

    if(deviceNameList.size() > 0){
        if(ofGetElapsedTimeMillis()-resetTime > 40){
            resetTime = ofGetElapsedTimeMillis();

            // SENDING DATA
            if(this->inletsConnected[0]){
                temp = "";
                if(static_cast<int>(static_cast<vector<float> *>(_inletParams[0])->size()) <= MAX_ARDUINO_SENDING_VECTOR_LENGTH){
                    temp += ofToString(static_cast<int>(static_cast<vector<float> *>(_inletParams[0])->size()));
                    temp += ",";
                    for(size_t s=0;s<static_cast<size_t>(static_cast<vector<float> *>(_inletParams[0])->size());s++){
                        temp += ofToString(ofClamp(static_cast<int>(static_cast<vector<float> *>(_inletParams[0])->at(s)),0,127));
                        if(s<static_cast<size_t>(static_cast<vector<float> *>(_inletParams[0])->size())-1){
                            temp += ",";
                        }
                    }
                    serial.writeBytes((unsigned char*)temp.c_str(),temp.length());
                    serial.writeByte('\n');
                }else{
                    ofLog(OF_LOG_ERROR,"Arduino Serial --> The data vector MAX SIZE for sending data to Arduino is %i, your data vector is too big, so just reduce it to MAX %i!",MAX_ARDUINO_SENDING_VECTOR_LENGTH,MAX_ARDUINO_SENDING_VECTOR_LENGTH);
                }
            }


            // RECEIVING DATA
            if(serial.available() > 0){
                memset(bytesReturned, 0, MAX_ARDUINO_RECEIVING_VECTOR_LENGTH);
                memset(bytesReadString, 0, MAX_ARDUINO_RECEIVING_VECTOR_LENGTH+1);

                if(serial.readBytes(bytesReturned, MAX_ARDUINO_RECEIVING_VECTOR_LENGTH) > 0){
                    memcpy(bytesReadString, bytesReturned, MAX_ARDUINO_RECEIVING_VECTOR_LENGTH);

                    for(int i=0;i<MAX_ARDUINO_RECEIVING_VECTOR_LENGTH;i++){
                        static_cast<vector<float> *>(_outletParams[0])->at(i) = static_cast<int>(bytesReadString[i]);
                    }
                }
            }

        }

    }
}

//--------------------------------------------------------------
void ArduinoSerial::drawObjectContent(ofxFontStash *font){
    ofSetColor(0,151,157);
    ofDrawRectangle(0,0,this->width,this->height);
    ofSetColor(255);
    ofEnableAlphaBlending();
    arduinoIcon->draw(this->width/2,this->headerHeight*1.8f,((this->height/2.2f)/arduinoIcon->getHeight())*arduinoIcon->getWidth(),this->height/2.2f);
    // GUI
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void ArduinoSerial::removeObjectContent(bool removeFileFromData){
    if(serial.isInitialized() && deviceNameList.size() > 0){
        serial.close();
    }
}

//--------------------------------------------------------------
void ArduinoSerial::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    baudRates->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    if(deviceNameList.size() > 0){
        deviceSelector->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }
    for(int i=0;i<baudRates->children.size();i++){
        baudRates->getChildAt(i)->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || deviceSelector->hitTest(_m-this->getPos()) || baudRates->hitTest(_m-this->getPos());

        for(int i=0;i<baudRates->children.size();i++){
            this->isOverGUI = baudRates->getChildAt(i)->hitTest(_m-this->getPos());
        }

    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void ArduinoSerial::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        baudRates->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        if(deviceNameList.size() > 0){
            deviceSelector->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        }
        for(int i=0;i<baudRates->children.size();i++){
            baudRates->getChildAt(i)->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void ArduinoSerial::resetSERIALSettings(int devID, int br){

    if(devID!=serialDeviceID || br != baudRateID){
        ofLog(OF_LOG_NOTICE,"Changing SERIAL Device to: %s", deviceNameList.at(devID).c_str());

        serialDeviceID = devID;
        if(deviceNameList.at(devID).size() > 22){
            serialDeviceName->setLabel(deviceNameList.at(devID).substr(0,21)+"...");
        }else{
            serialDeviceName->setLabel(deviceNameList.at(devID));
        }
        this->setCustomVar(static_cast<float>(serialDeviceID),"DEVICE_ID");

        baudRateID = ofClamp(br,0,7);
        this->setCustomVar(static_cast<float>(baudRateID),"BAUDRATE_ID");

        if(serial.isInitialized()){
            serial.close();
        }

        serial.setup(serialDeviceID,ofToInt(baudrateList.at(baudRateID)));

        if(serial.isInitialized() && serial.available()){
            ofLog(OF_LOG_NOTICE,"SERIAL device %s connected!", deviceNameList.at(devID).c_str());
            this->saveConfig(false,this->nId);
        }
    }
}

//--------------------------------------------------------------
void ArduinoSerial::onMatrixEvent(ofxDatGuiMatrixEvent e){
    if(!header->getIsCollapsed()){
        if(deviceNameList.size() > 0){
            if(e.target == deviceSelector){
                resetSERIALSettings(e.child,baudRateID);
            }
        }
    }
}

//--------------------------------------------------------------
void ArduinoSerial::onDropdownEvent(ofxDatGuiDropdownEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == baudRates){
            if(deviceNameList.size() > 0){
                resetSERIALSettings(serialDeviceID,e.child);
                e.target->expand();
            }
        }
    }
}

OBJECT_REGISTER( ArduinoSerial, "arduino serial", OFXVP_OBJECT_CAT_COMMUNICATIONS);

#endif
