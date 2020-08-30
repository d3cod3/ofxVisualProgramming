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

#ifndef OFXVP_BUILD_WITH_MINIMAL_OBJECTS

#include "MotionDetection.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
MotionDetection::MotionDetection() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();  // input

    _outletParams[0] = new float(); // MOTION QUANTITY
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    newConnection       = false;

    _totPixels          = 320*240;
    frameCounter        = 0;
    numPixelsChanged    = 0;

}

//--------------------------------------------------------------
void MotionDetection::newObject(){
    PatchObject::setName( this->objectName );
    this->addInlet(VP_LINK_TEXTURE,"input");
    this->addOutlet(VP_LINK_NUMERIC,"motionQuantity");

    this->setCustomVar(static_cast<float>(100.0),"THRESHOLD");
    this->setCustomVar(static_cast<float>(10.0),"NOISE_COMP");
}

//--------------------------------------------------------------
void MotionDetection::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    resetTextures(320,240);

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    
    gui->onSliderEvent(this, &MotionDetection::onSliderEvent);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    thresholdValue = gui->addSlider("THRESH",0.0,255.0);
    thresholdValue->setUseCustomMouse(true);
    thresholdValue->setValue(static_cast<double>(this->getCustomVar("THRESHOLD")));
    noiseValue = gui->addSlider("NOISE COMP",0.0,1000.0);
    noiseValue->setUseCustomMouse(true);
    noiseValue->setValue(static_cast<double>(this->getCustomVar("NOISE_COMP")));

    gui2 = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui2->setAutoDraw(false);
    gui2->setWidth(this->width);

    rPlotter = gui2->addValuePlotter("",0.0f,1.0f);
    rPlotter->setDrawMode(ofxDatGuiGraph::LINES);
    rPlotter->setSpeed(1);

    gui2->setPosition(0,this->height-rPlotter->getHeight());

}

//--------------------------------------------------------------
void MotionDetection::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    gui->update();
    header->update();
    if(!header->getIsCollapsed()){
        thresholdValue->update();
        noiseValue->update();
    }
    gui2->update();
    rPlotter->setValue(*(float *)&_outletParams[0]);

    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(!newConnection){
            newConnection = true;
            resetTextures(static_cast<ofTexture *>(_inletParams[0])->getWidth(),static_cast<ofTexture *>(_inletParams[0])->getHeight());
        }

        static_cast<ofTexture *>(_inletParams[0])->readToPixels(*pix);

        colorImg->setFromPixels(*pix);
        colorImg->updateTexture();

        if(frameCounter > 5){// dont do anything until we have enough in history
            *grayNow = *colorImg;
            grayNow->updateTexture();

            motionImg->absDiff(*grayPrev, *grayNow);   // motionImg is the difference between current and previous frame
            motionImg->updateTexture();
            cvThreshold(motionImg->getCvImage(), motionImg->getCvImage(), static_cast<int>(thresholdValue->getValue()), 255, CV_THRESH_TOZERO); // anything below threshold, drop to zero (compensate for noise)
            numPixelsChanged = motionImg->countNonZeroInRegion(0, 0, static_cast<ofTexture *>(_inletParams[0])->getWidth(), static_cast<ofTexture *>(_inletParams[0])->getHeight());

            if(numPixelsChanged >= static_cast<int>(noiseValue->getValue())){ // noise compensation
                *grayPrev = *grayNow; // save current frame for next loop
                cvThreshold(motionImg->getCvImage(), motionImg->getCvImage(), static_cast<int>(thresholdValue->getValue()), 255, CV_THRESH_TOZERO);// chop dark areas
                motionImg->updateTexture();
            }else{
                motionImg->setFromPixels(blackPixels, static_cast<ofTexture *>(_inletParams[0])->getWidth(), static_cast<ofTexture *>(_inletParams[0])->getHeight());
                motionImg->updateTexture();
            }


            *(float *)&_outletParams[0] = static_cast<float>(numPixelsChanged)/static_cast<float>(_totPixels);

        }

    }else{
        newConnection = false;
    }

    frameCounter++;

}

//--------------------------------------------------------------
void MotionDetection::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    gui2->draw();
    font->draw(ofToString(*(float *)&_outletParams[0]),this->fontSize,this->width/2,this->headerHeight*2.3);
}

//--------------------------------------------------------------
void MotionDetection::removeObjectContent(bool removeFileFromData){
    
}

//--------------------------------------------------------------
void MotionDetection::resetTextures(int w, int h){

    pix         = new ofPixels();
    colorImg    = new ofxCvColorImage();
    grayPrev    = new ofxCvGrayscaleImage();
    grayNow     = new ofxCvGrayscaleImage();
    motionImg   = new ofxCvGrayscaleImage();

    _totPixels          = w*h;

    pix->allocate(static_cast<size_t>(w),static_cast<size_t>(h),1);

    colorImg->allocate(w,h);
    grayPrev->allocate(w,h);
    grayNow->allocate(w,h);
    motionImg->allocate(w,h);

    blackPixels = new unsigned char[_totPixels];
    for(unsigned int b=0;b<_totPixels;b++){
        blackPixels[b] = 0;
    }
}

//--------------------------------------------------------------
void MotionDetection::onSliderEvent(ofxDatGuiSliderEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == thresholdValue){
            this->setCustomVar(static_cast<float>(e.value),"THRESHOLD");
        }else if(e.target == noiseValue){
            this->setCustomVar(static_cast<float>(e.value),"NOISE_COMP");
        }
    }

}

OBJECT_REGISTER( MotionDetection, "motion detection", OFXVP_OBJECT_CAT_CV)

#endif
