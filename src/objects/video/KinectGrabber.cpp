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

#include "KinectGrabber.h"

//--------------------------------------------------------------
KinectGrabber::KinectGrabber() : PatchObject(){

    this->numInlets  = 0;
    this->numOutlets = 2;

    _outletParams[0] = new ofTexture(); // video (IR or RGB)
    _outletParams[1] = new ofTexture(); // depth
    _outletParams[2] = new ofxKinect(); // kinect reference

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    isNewObject         = false;

    posX = posY = drawW = drawH = 0.0f;

    kinectWidth         = 640;
    kinectHeight        = 480;

    deviceID            = 0;
    isIR                = false;

    needReset           = false;

    weHaveKinect        = false;
}

//--------------------------------------------------------------
void KinectGrabber::newObject(){
    this->setName("kinect grabber");
    this->addOutlet(VP_LINK_TEXTURE,"kinectImage");
    this->addOutlet(VP_LINK_TEXTURE,"kinectDepth");

    this->setCustomVar(static_cast<float>(deviceID),"DEVICE_ID");
    this->setCustomVar(static_cast<float>(isIR),"INFRARED");
    this->setCustomVar(static_cast<float>(230.0),"NEAR_THRESH");
    this->setCustomVar(static_cast<float>(70.0),"FAR_THRESH");
}

//--------------------------------------------------------------
void KinectGrabber::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    int numKinects = ofxKinect::numAvailableDevices();
    if(numKinects > 0){
        ofLog(OF_LOG_NOTICE,"KINECT devices available: %i",numKinects);
        for(int i=0;i<numKinects;i++){
            devicesVector.push_back(ofToString(i));
        }
        weHaveKinect = true;
    }

    if(weHaveKinect){
        deviceName = gui->addLabel("Kinect Device "+ofToString(deviceID));
        deviceSelector = gui->addMatrix("DEVICE",devicesVector.size(),true);
        deviceSelector->setUseCustomMouse(true);
        deviceSelector->setRadioMode(true);
        deviceSelector->getChildAt(deviceID)->setSelected(true);
        deviceSelector->onMatrixEvent(this, &KinectGrabber::onMatrixEvent);
        gui->addBreak();

        irButton = gui->addToggle("INFRARED",false);
        irButton->setUseCustomMouse(true);
        irButton->setChecked(isIR);

        nearThreshold = gui->addSlider("NEAR",0,255,0);
        nearThreshold->setUseCustomMouse(true);
        farThreshold = gui->addSlider("FAR",0,255,0);
        farThreshold->setUseCustomMouse(true);

        loadKinectSettings();

    }

    gui->onToggleEvent(this, &KinectGrabber::onToggleEvent);
    gui->onSliderEvent(this, &KinectGrabber::onSliderEvent);
    gui->onMatrixEvent(this, &KinectGrabber::onMatrixEvent);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    // SETUP KINECT
    if(weHaveKinect){
        static_cast<ofxKinect *>(_outletParams[2])->setRegistration(true);
        static_cast<ofxKinect *>(_outletParams[2])->init(isIR,true,true);
        static_cast<ofxKinect *>(_outletParams[2])->open(deviceID);
        static_cast<ofxKinect *>(_outletParams[2])->setCameraTiltAngle(0);

        colorCleanImage.allocate(kinectWidth, kinectHeight);
        cleanImage.allocate(kinectWidth, kinectHeight);
        grayThreshNear.allocate(kinectWidth, kinectHeight);
        grayThreshFar.allocate(kinectWidth, kinectHeight);
    }
    
}

//--------------------------------------------------------------
void KinectGrabber::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    if(!header->getIsCollapsed() && weHaveKinect){
        deviceSelector->update();
        irButton->update();
        nearThreshold->update();
        farThreshold->update();
    }

    if(needReset){
        needReset = false;
        if(weHaveKinect){
            resetKinectSettings(deviceID);
        }
    }

    // KINECT UPDATE
    if(weHaveKinect && static_cast<ofxKinect *>(_outletParams[2])->isInitialized() && static_cast<ofxKinect *>(_outletParams[2])->isConnected()){
        static_cast<ofxKinect *>(_outletParams[2])->update();
        if(static_cast<ofxKinect *>(_outletParams[2])->isFrameNew()){
            *static_cast<ofTexture *>(_outletParams[0]) = static_cast<ofxKinect *>(_outletParams[2])->getTexture();

            cleanImage.setFromPixels(static_cast<ofxKinect *>(_outletParams[2])->getDepthPixels());
            cleanImage.updateTexture();

            grayThreshNear = cleanImage;
            grayThreshFar = cleanImage;
            grayThreshNear.threshold(nearThreshold->getValue(), true);
            grayThreshFar.threshold(farThreshold->getValue());

            grayThreshNear.updateTexture();
            grayThreshFar.updateTexture();

            cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), cleanImage.getCvImage(), nullptr);
            cleanImage.flagImageChanged();
            cleanImage.updateTexture();

            colorCleanImage = cleanImage;
            colorCleanImage.updateTexture();

            *static_cast<ofTexture *>(_outletParams[1]) = colorCleanImage.getTexture();
        }
    }

}

//--------------------------------------------------------------
void KinectGrabber::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(weHaveKinect && static_cast<ofxKinect *>(_outletParams[2])->isInitialized() && static_cast<ofxKinect *>(_outletParams[2])->isConnected() && static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
        if(static_cast<ofTexture *>(_outletParams[0])->getWidth()/static_cast<ofTexture *>(_outletParams[0])->getHeight() >= this->width/this->height){
            if(static_cast<ofTexture *>(_outletParams[0])->getWidth() > static_cast<ofTexture *>(_outletParams[0])->getHeight()){   // horizontal texture
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
        }else{ // always considered vertical texture
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
void KinectGrabber::removeObjectContent(bool removeFileFromData){
    static_cast<ofxKinect *>(_outletParams[2])->setCameraTiltAngle(0);
    static_cast<ofxKinect *>(_outletParams[2])->close();
}

//--------------------------------------------------------------
void KinectGrabber::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    if(weHaveKinect){
        deviceSelector->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        irButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        nearThreshold->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        farThreshold->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }

    if(!header->getIsCollapsed() && weHaveKinect){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || deviceSelector->hitTest(_m-this->getPos()) || irButton->hitTest(_m-this->getPos()) || nearThreshold->hitTest(_m-this->getPos()) || farThreshold->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void KinectGrabber::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        if(weHaveKinect){
            deviceSelector->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
            irButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
            nearThreshold->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
            farThreshold->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void KinectGrabber::loadKinectSettings(){
    if(static_cast<int>(floor(this->getCustomVar("DEVICE_ID"))) >= 0){
        deviceID = static_cast<int>(floor(this->getCustomVar("DEVICE_ID")));
    }else{
        deviceID = 0;
        this->setCustomVar(static_cast<float>(deviceID),"DEVICE_ID");
    }

    if(static_cast<int>(floor(this->getCustomVar("INFRARED"))) >= 0){
        isIR = static_cast<bool>(floor(this->getCustomVar("INFRARED")));
    }else{
        isIR = false;
        this->setCustomVar(static_cast<float>(isIR),"INFRARED");
    }

    nearThreshold->setValue(static_cast<double>(this->getCustomVar("NEAR_THRESH")));
    farThreshold->setValue(static_cast<double>(this->getCustomVar("FAR_THRESH")));

}

//--------------------------------------------------------------
void KinectGrabber::resetKinectSettings(int devID){

    if(devID!=deviceID){

        ofLog(OF_LOG_NOTICE,"Changing Device to: %i",ofToInt(devicesVector[devID]));

        deviceID = devID;
        deviceName->setLabel("Kinect Device "+ofToString(deviceID));
        this->setCustomVar(static_cast<float>(deviceID),"DEVICE_ID");


        if(static_cast<ofxKinect *>(_outletParams[2])->isInitialized()){
            static_cast<ofxKinect *>(_outletParams[2])->setCameraTiltAngle(0);
            static_cast<ofxKinect *>(_outletParams[2])->close();

            _outletParams[2] = new ofxKinect();
            static_cast<ofxKinect *>(_outletParams[2])->setRegistration(true);
            static_cast<ofxKinect *>(_outletParams[2])->init(isIR,true,true);
            static_cast<ofxKinect *>(_outletParams[2])->open(deviceID);
        }
    }
}

//--------------------------------------------------------------
void KinectGrabber::resetKinectImage(bool ir){

    if(ir!=isIR){

        isIR = ir;

        this->setCustomVar(static_cast<float>(ir),"INFRARED");

        if(static_cast<ofxKinect *>(_outletParams[2])->isInitialized()){
            static_cast<ofxKinect *>(_outletParams[2])->setCameraTiltAngle(0);
            static_cast<ofxKinect *>(_outletParams[2])->close();

            _outletParams[2] = new ofxKinect();
            static_cast<ofxKinect *>(_outletParams[2])->setRegistration(true);
            static_cast<ofxKinect *>(_outletParams[2])->init(ir,true,true);
            static_cast<ofxKinect *>(_outletParams[2])->open(deviceID);
        }
    }

}

//--------------------------------------------------------------
void KinectGrabber::onToggleEvent(ofxDatGuiToggleEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == irButton){
            resetKinectImage(e.checked);
        }
    }

}

//--------------------------------------------------------------
void KinectGrabber::onSliderEvent(ofxDatGuiSliderEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == nearThreshold){
            this->setCustomVar(static_cast<float>(e.value),"NEAR_THRESH");

        }else if(e.target == farThreshold){
            this->setCustomVar(static_cast<float>(e.value),"FAR_THRESH");
        }
    }

}

//--------------------------------------------------------------
void KinectGrabber::onMatrixEvent(ofxDatGuiMatrixEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == deviceSelector){
            resetKinectSettings(e.child);
        }
    }
}
