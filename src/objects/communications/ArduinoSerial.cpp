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
ArduinoSerial::ArduinoSerial() : PatchObject("arduino serial"){

    this->numInlets  = 1;
    this->numOutlets  = 1;

    _inletParams[0] = new vector<float>();

    _outletParams[0] = new vector<float>();

    this->initInletsState();

    arduinoIcon         = new ofImage();
    posX = posY = drawW = drawH = 0.0f;

    serialDeviceID      = 0;
    baudRateID          = 0;

    baudrateList        = {"9600","19200","38400","57600","74880","115200","230400","250000"};

    resetTime           = ofGetElapsedTimeMillis();

    loaded              = false;

    this->setIsTextureObj(true);
}

//--------------------------------------------------------------
void ArduinoSerial::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"data");

    this->addOutlet(VP_LINK_ARRAY,"dataFromArduino");

    this->setCustomVar(static_cast<float>(serialDeviceID),"DEVICE_ID");
    this->setCustomVar(static_cast<float>(baudRateID),"BAUDRATE_ID");
}

//--------------------------------------------------------------
void ArduinoSerial::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    arduinoIcon->load("images/arduino.jpg");

    ofLog(OF_LOG_NOTICE," ");
    ofLog(OF_LOG_NOTICE,"------------------- SERIAL DEVICES");


    deviceList = serial.getDeviceList();
    for(int i=0;i<deviceList.size();i++){
        ofLog(OF_LOG_NOTICE,"[%i] - %s",i,deviceList.at(i).getDeviceName().c_str());
        deviceNameList.push_back(deviceList.at(i).getDeviceName());
    }


    for(int i=0;i<MAX_ARDUINO_RECEIVING_VECTOR_LENGTH;i++){
        static_cast<vector<float> *>(_outletParams[0])->push_back(0);
    }


}

//--------------------------------------------------------------
void ArduinoSerial::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

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

    if(!loaded){
        loaded = true;

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
            serial.setup(serialDeviceID, ofToInt(baudrateList.at(baudRateID)));

        }else{
            ofLog(OF_LOG_WARNING,"You have no SERIAL devices available, please enable one in order to use the arduino serial object!");
        }
    }

}

//--------------------------------------------------------------
void ArduinoSerial::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    // draw node texture preview with OF
    drawNodeOFTexture(arduinoIcon->getTexture(), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, IMGUI_EX_NODE_FOOTER_HEIGHT);
}

//--------------------------------------------------------------
void ArduinoSerial::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            ImGui::Spacing();
            ImGui::Text("%s",deviceNameList.at(serialDeviceID).c_str());

            ImGui::Spacing();
            if(ImGui::BeginCombo("Device", deviceNameList.at(serialDeviceID).c_str() )){
                for(int i=0; i < deviceNameList.size(); ++i){
                    bool is_selected = (serialDeviceID == i );
                    if (ImGui::Selectable(deviceNameList.at(i).c_str(), is_selected)){
                        resetSERIALSettings(i,baudRateID);
                    }
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::Spacing();
            if(ImGui::BeginCombo("Baudrate", baudrateList.at(baudRateID).c_str() )){
                for(int i=0; i < baudrateList.size(); ++i){
                    bool is_selected = (baudRateID == i );
                    if (ImGui::Selectable(baudrateList.at(i).c_str(), is_selected)){
                        resetSERIALSettings(serialDeviceID,i);
                    }
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }


            ImGuiEx::ObjectInfo(
                        "This object communicates with Arduino for both sending and receiving data. This template MosaicConnector.ino must be used as Arduino template file",
                        "https://mosaic.d3cod3.org/reference.php?r=arduino-serial");

            ImGui::EndMenu();
        }
        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        // get imgui node translated/scaled position/dimension for drawing textures in OF
        objOriginX = (ImGui::GetWindowPos().x + IMGUI_EX_NODE_PINS_WIDTH_NORMAL - 1 - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale();
        objOriginY = (ImGui::GetWindowPos().y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale();
        scaledObjW = this->width - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL/_nodeCanvas.GetCanvasScale());
        scaledObjH = this->height - ((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)/_nodeCanvas.GetCanvasScale());

        _nodeCanvas.EndNodeContent();
    }

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();

}

//--------------------------------------------------------------
void ArduinoSerial::removeObjectContent(bool removeFileFromData){
    if(serial.isInitialized() && deviceNameList.size() > 0){
        serial.close();
    }
}

//--------------------------------------------------------------
void ArduinoSerial::resetSERIALSettings(int devID, int br){

    if(devID!=serialDeviceID || br != baudRateID){
        ofLog(OF_LOG_NOTICE,"Changing SERIAL Device to: %s", deviceNameList.at(devID).c_str());

        serialDeviceID = devID;

        this->setCustomVar(static_cast<float>(serialDeviceID),"DEVICE_ID");

        baudRateID = ofClamp(br,0,7);
        this->setCustomVar(static_cast<float>(baudRateID),"BAUDRATE_ID");

        if(serial.isInitialized()){
            serial.close();
        }

        serial.setup(serialDeviceID,ofToInt(baudrateList.at(baudRateID)));

        if(serial.isInitialized() && serial.available()){
            ofLog(OF_LOG_NOTICE,"SERIAL device %s connected!", deviceNameList.at(devID).c_str());
            this->saveConfig(false);
        }
    }
}

OBJECT_REGISTER( ArduinoSerial, "arduino serial", OFXVP_OBJECT_CAT_COMMUNICATIONS)

#endif
