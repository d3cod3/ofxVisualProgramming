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

#include "VideoGrabber.h"

//--------------------------------------------------------------
VideoGrabber::VideoGrabber() : PatchObject(){

    this->numInlets  = 0;
    this->numOutlets = 1;

    _outletParams[0] = new ofTexture(); // output

    for(int i=0;i<this->numInlets;i++){
        this->inletsConnected.push_back(false);
    }

    vidGrabber = new ofVideoGrabber();

    isGUIObject         = true;
    this->isOverGUI     = true;

    isNewObject         = false;

    posX = posY = drawW = drawH = 0.0f;

    camWidth            = 320;
    camHeight           = 240;
    temp_width          = camWidth;
    temp_height         = camHeight;

    deviceID            = 0;

    needReset           = false;
}

//--------------------------------------------------------------
void VideoGrabber::newObject(){
    this->setName("video grabber");
    this->addOutlet(VP_LINK_TEXTURE);

    this->setCustomVar(static_cast<float>(camWidth),"CAM_WIDTH");
    this->setCustomVar(static_cast<float>(camHeight),"CAM_HEIGHT");
    this->setCustomVar(static_cast<float>(deviceID),"DEVICE_ID");
}

//--------------------------------------------------------------
void VideoGrabber::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    wdevices = vidGrabber->listDevices();
    for(int i=0;i<static_cast<int>(wdevices.size());i++){
        if(wdevices[i].bAvailable){
            devicesVector.push_back(wdevices[i].deviceName);
        }
    }

    loadCameraSettings();

    deviceName = gui->addLabel(devicesVector[deviceID]);
    if(devicesVector[deviceID].size() > 22){
        deviceName->setLabel(devicesVector[deviceID].substr(0,21)+"...");
    }else{
        deviceName->setLabel(devicesVector[deviceID]);
    }

    deviceSelector = gui->addMatrix("DEVICE",devicesVector.size(),true);
    deviceSelector->setUseCustomMouse(true);
    deviceSelector->setRadioMode(true);
    deviceSelector->getChildAt(deviceID)->setSelected(true);
    deviceSelector->onMatrixEvent(this, &VideoGrabber::onMatrixEvent);
    gui->addBreak();

    guiTexWidth = gui->addTextInput("WIDTH",ofToString(camWidth));
    guiTexWidth->setUseCustomMouse(true);
    guiTexHeight = gui->addTextInput("HEIGHT",ofToString(camHeight));
    guiTexHeight->setUseCustomMouse(true);
    applyButton = gui->addButton("APPLY");
    applyButton->setUseCustomMouse(true);
    applyButton->setLabelAlignment(ofxDatGuiAlignment::CENTER);

    gui->onButtonEvent(this, &VideoGrabber::onButtonEvent);
    gui->onTextInputEvent(this, &VideoGrabber::onTextInputEvent);
    gui->onMatrixEvent(this, &VideoGrabber::onMatrixEvent);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    if(wdevices.size() > 0){
        vidGrabber->setDeviceID(deviceID);
        vidGrabber->setup(camWidth, camHeight);
    }
    
}

//--------------------------------------------------------------
void VideoGrabber::updateObjectContent(map<int,PatchObject*> &patchObjects){

    gui->update();
    header->update();
    if(!header->getIsCollapsed()){
        guiTexWidth->update();
        guiTexHeight->update();
        applyButton->update();
        deviceSelector->update();
    }

    if(needReset){
        resetCameraSettings(deviceID);
    }

    if(vidGrabber->isInitialized()){
        vidGrabber->update();
        if(vidGrabber->isFrameNew()){
            *static_cast<ofTexture *>(_outletParams[0]) = vidGrabber->getTexture();
        }
    }

}

//--------------------------------------------------------------
void VideoGrabber::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(vidGrabber->isInitialized() && static_cast<ofTexture *>(_outletParams[0])->isAllocated() && !needReset){
        if(static_cast<ofTexture *>(_outletParams[0])->getWidth() >= static_cast<ofTexture *>(_outletParams[0])->getHeight()){   // horizontal texture
            drawW           = this->width;
            drawH           = (this->width/static_cast<ofTexture *>(_outletParams[0])->getWidth())*static_cast<ofTexture *>(_outletParams[0])->getHeight();
            posX            = 0;
            posY            = (this->height-drawH)/2.0f;
        }else{ // vertical texture
            drawW           = (static_cast<ofTexture *>(_outletParams[0])->getWidth()*this->height)/static_cast<ofTexture *>(_outletParams[0])->getHeight();
            drawH           = this->height;
            posX            = (this->width-drawW)/2.0f;
            posY            = 0;
        }
        static_cast<ofTexture *>(_outletParams[0])->draw(posX,posY,drawW,drawH);
    }
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void VideoGrabber::removeObjectContent(){
    vidGrabber->close();
}

//--------------------------------------------------------------
void VideoGrabber::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    guiTexWidth->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    guiTexHeight->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    applyButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    deviceSelector->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || guiTexWidth->hitTest(_m-this->getPos()) || guiTexHeight->hitTest(_m-this->getPos()) || applyButton->hitTest(_m-this->getPos()) || deviceSelector->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void VideoGrabber::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        guiTexWidth->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        guiTexHeight->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        applyButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void VideoGrabber::loadCameraSettings(){
    if(static_cast<int>(floor(this->getCustomVar("CAM_WIDTH"))) > 0){
        camWidth = static_cast<int>(floor(this->getCustomVar("CAM_WIDTH")));
        camHeight = static_cast<int>(floor(this->getCustomVar("CAM_HEIGHT")));

        temp_width      = camWidth;
        temp_height     = camHeight;
    }

    if(static_cast<int>(floor(this->getCustomVar("DEVICE_ID"))) >= 0 && static_cast<int>(floor(this->getCustomVar("DEVICE_ID"))) < static_cast<int>(devicesVector.size())){
        deviceID = static_cast<int>(floor(this->getCustomVar("DEVICE_ID")));
    }else{
        deviceID = 0;
        this->setCustomVar(static_cast<float>(deviceID),"DEVICE_ID");
    }

}

//--------------------------------------------------------------
void VideoGrabber::resetCameraSettings(int devID){

    if(camWidth != temp_width || camHeight != temp_height || devID!=deviceID){

        if(devID!=deviceID){
            ofLog(OF_LOG_NOTICE,"Changing Device to: %s",devicesVector[devID].c_str());

            deviceID = devID;
            if(devicesVector[deviceID].size() > 22){
                deviceName->setLabel(devicesVector[deviceID].substr(0,21)+"...");
            }else{
                deviceName->setLabel(devicesVector[deviceID]);
            }
            this->setCustomVar(static_cast<float>(deviceID),"DEVICE_ID");
        }


        if(camWidth != temp_width || camHeight != temp_height){
            camWidth = temp_width;
            camHeight = temp_height;

            ofLog(OF_LOG_NOTICE,"Changing %s Capture dimensions to: %ix%i",devicesVector[devID].c_str(),camWidth,camHeight);

            this->setCustomVar(static_cast<float>(camWidth),"CAM_WIDTH");
            this->setCustomVar(static_cast<float>(camHeight),"CAM_HEIGHT");
        }

        if(vidGrabber->isInitialized()){
            vidGrabber->close();

            vidGrabber = new ofVideoGrabber();
            vidGrabber->setDeviceID(deviceID);
            vidGrabber->setup(camWidth, camHeight);
        }
    }

    needReset = false;
}

//--------------------------------------------------------------
void VideoGrabber::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if (e.target == applyButton){
            needReset = true;
        }
    }

}

//--------------------------------------------------------------
void VideoGrabber::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(!header->getIsCollapsed()){
        int tempInValue = ofToInt(e.text);
        if(e.target == guiTexWidth){
            if(tempInValue <= CAM_MAX_WIDTH){
                temp_width = tempInValue;
            }else{
                temp_width = camWidth;
            }
        }else if(e.target == guiTexHeight){
            if(tempInValue <= CAM_MAX_HEIGHT){
                temp_height = tempInValue;
            }else{
                temp_height = camHeight;
            }
        }
    }
}

//--------------------------------------------------------------
void VideoGrabber::onMatrixEvent(ofxDatGuiMatrixEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == deviceSelector){
            resetCameraSettings(e.child);
        }
    }
}
