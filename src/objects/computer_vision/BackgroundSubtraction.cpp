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

#include "BackgroundSubtraction.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
BackgroundSubtraction::BackgroundSubtraction() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();  // input
    _inletParams[1] = new float();  // bang
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new ofTexture(); // output

    this->initInletsState();

    resetTextures(320,240);

    bgSubTech           = 0; // 0 abs, 1 lighter than, 2 darker than

    isGUIObject         = true;
    this->isOverGUI     = true;

    posX = posY = drawW = drawH = 0.0f;

    newConnection       = false;
    bLearnBackground    = false;

}

//--------------------------------------------------------------
void BackgroundSubtraction::newObject(){
    this->setName("background subtraction");
    this->addInlet(VP_LINK_TEXTURE,"input");
    this->addInlet(VP_LINK_NUMERIC,"reset");
    this->addOutlet(VP_LINK_TEXTURE);

    this->setCustomVar(static_cast<float>(80.0),"THRESHOLD");
    this->setCustomVar(static_cast<float>(bgSubTech),"SUBTRACTION_TECHNIQUE");
    this->setCustomVar(static_cast<float>(0.0),"BRIGHTNESS");
    this->setCustomVar(static_cast<float>(0.0),"CONTRAST");
    this->setCustomVar(static_cast<float>(1.0),"BLUR");
    this->setCustomVar(static_cast<float>(0.0),"ERODE");
    this->setCustomVar(static_cast<float>(0.0),"DILATE");
}

//--------------------------------------------------------------
void BackgroundSubtraction::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    resetButton = gui->addButton("RESET BACKGROUND");
    resetButton->setUseCustomMouse(true);

    thresholdValue = gui->addSlider("THRESH",0,255);
    thresholdValue->setUseCustomMouse(true);
    thresholdValue->setValue(static_cast<double>(this->getCustomVar("THRESHOLD")));
    vector<int> techs = {0,1,2};
    bgTechLabel = gui->addLabel("B&W ABS");
    bgTechLabel->setUseCustomMouse(true);
    bgSubTechSelector = gui->addMatrix("TECHNIQUE",techs.size(),true);
    bgSubTechSelector->setUseCustomMouse(true);
    bgSubTechSelector->setRadioMode(true);
    bgSubTech = static_cast<int>(floor(this->getCustomVar("SUBTRACTION_TECHNIQUE")));

    if(bgSubTech == 0){
        bgTechLabel->setLabel("B&W ABS");
    }else if(bgSubTech == 1){
        bgTechLabel->setLabel("LIGHTER THAN");
    }else if(bgSubTech == 2){
        bgTechLabel->setLabel("DARKER THAN");
    }

    bgSubTechSelector->getChildAt(bgSubTech)->setSelected(true);
    gui->addBreak();
    brightnessValue = gui->addSlider("BRIGTH",-1.0,3.0);
    brightnessValue->setUseCustomMouse(true);
    brightnessValue->setValue(static_cast<double>(this->getCustomVar("BRIGHTNESS")));
    contrastValue = gui->addSlider("CONTRAST",0.0,1.0);
    contrastValue->setUseCustomMouse(true);
    contrastValue->setValue(static_cast<double>(this->getCustomVar("CONTRAST")));
    blurValue = gui->addSlider("BLUR",0,33);
    blurValue->setUseCustomMouse(true);
    blurValue->setValue(static_cast<double>(this->getCustomVar("BLUR")));
    erodeButton = gui->addToggle("ERODE",static_cast<int>(floor(this->getCustomVar("ERODE"))));
    erodeButton->setUseCustomMouse(true);
    dilateButton = gui->addToggle("DILATE",static_cast<int>(floor(this->getCustomVar("DILATE"))));
    dilateButton->setUseCustomMouse(true);

    gui->onButtonEvent(this, &BackgroundSubtraction::onButtonEvent);
    gui->onToggleEvent(this, &BackgroundSubtraction::onToggleEvent);
    gui->onSliderEvent(this, &BackgroundSubtraction::onSliderEvent);
    gui->onMatrixEvent(this, &BackgroundSubtraction::onMatrixEvent);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);
}

//--------------------------------------------------------------
void BackgroundSubtraction::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){
    gui->update();
    header->update();
    if(!header->getIsCollapsed()){
        resetButton->update();
        thresholdValue->update();
        bgTechLabel->update();
        bgSubTechSelector->update();
        brightnessValue->update();
        contrastValue->update();
        blurValue->update();
        erodeButton->update();
        dilateButton->update();
    }

    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(!newConnection){
            newConnection = true;
            resetTextures(static_cast<int>(floor(static_cast<ofTexture *>(_inletParams[0])->getWidth())),static_cast<int>(floor(static_cast<ofTexture *>(_inletParams[0])->getHeight())));
        }

        static_cast<ofTexture *>(_inletParams[0])->readToPixels(*pix);

        colorImg->setFromPixels(*pix);
        colorImg->updateTexture();

        *grayImg = *colorImg;
        grayImg->updateTexture();
        grayImg->brightnessContrast(brightnessValue->getValue(),contrastValue->getValue());

        if(bgSubTech == 0){ // B&W ABS
            grayThresh->absDiff(*grayBg, *grayImg);
        }else if(bgSubTech == 1){ // LIGHTER THAN
            *grayThresh = *grayImg;
            *grayThresh -= *grayBg;
        }else if(bgSubTech == 2){ // DARKER THAN
            *grayThresh = *grayBg;
            *grayThresh -= *grayImg;
        }


        grayThresh->threshold(thresholdValue->getValue());
        if(erodeButton->getChecked()){
            grayThresh->erode();
        }
        if(dilateButton->getChecked()){
            grayThresh->dilate();
        }
        if(static_cast<int>(floor(blurValue->getValue()))%2 == 0){
            grayThresh->blur(static_cast<int>(floor(blurValue->getValue()))+1);
        }else{
            grayThresh->blur(static_cast<int>(floor(blurValue->getValue())));
        }
        grayThresh->updateTexture();

        *static_cast<ofTexture *>(_outletParams[0]) = grayThresh->getTexture();

    }else if(!this->inletsConnected[0]){
        newConnection = false;
    }

    // External background reset (BANG)
    if(this->inletsConnected[1] && *(float *)&_inletParams[1] == 1.0f){
        bLearnBackground = true;
    }

    //////////////////////////////////////////////
    // background learning
    if(bLearnBackground == true){
        bLearnBackground = false;
        *grayBg = *grayImg;
        grayBg->updateTexture();
    }
    //////////////////////////////////////////////
}

//--------------------------------------------------------------
void BackgroundSubtraction::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
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
void BackgroundSubtraction::removeObjectContent(){
    
}

//--------------------------------------------------------------
void BackgroundSubtraction::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    resetButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    thresholdValue->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    bgTechLabel->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    bgSubTechSelector->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    brightnessValue->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    contrastValue->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    blurValue->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    erodeButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    dilateButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || bgTechLabel->hitTest(_m-this->getPos()) || resetButton->hitTest(_m-this->getPos()) || thresholdValue->hitTest(_m-this->getPos()) || bgSubTechSelector->hitTest(_m-this->getPos()) || brightnessValue->hitTest(_m-this->getPos()) || contrastValue->hitTest(_m-this->getPos()) || blurValue->hitTest(_m-this->getPos()) || erodeButton->hitTest(_m-this->getPos()) || dilateButton->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void BackgroundSubtraction::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        resetButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        thresholdValue->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        bgTechLabel->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        bgSubTechSelector->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        brightnessValue->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        contrastValue->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        blurValue->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        erodeButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        dilateButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void BackgroundSubtraction::resetTextures(int w, int h){

    pix         = new ofPixels();
    colorImg    = new ofxCvColorImage();
    grayImg     = new ofxCvGrayscaleImage();
    grayBg      = new ofxCvGrayscaleImage();
    grayThresh  = new ofxCvGrayscaleImage();

    pix->allocate(static_cast<size_t>(w),static_cast<size_t>(h),1);
    colorImg->allocate(w,h);
    grayImg->allocate(w,h);
    grayBg->allocate(w,h);
    grayThresh->allocate(w,h);
}

//--------------------------------------------------------------
void BackgroundSubtraction::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == resetButton){
            bLearnBackground = true;
        }
    }
}

//--------------------------------------------------------------
void BackgroundSubtraction::onToggleEvent(ofxDatGuiToggleEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == erodeButton){
            this->setCustomVar(static_cast<float>(e.checked),"ERODE");
        }else if(e.target == dilateButton){
            this->setCustomVar(static_cast<float>(e.checked),"DILATE");
        }
    }
}

//--------------------------------------------------------------
void BackgroundSubtraction::onSliderEvent(ofxDatGuiSliderEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == thresholdValue){
            this->setCustomVar(static_cast<float>(e.value),"THRESHOLD");
        }else if(e.target == brightnessValue){
            this->setCustomVar(static_cast<float>(e.value),"BRIGHTNESS");
        }else if(e.target == contrastValue){
            this->setCustomVar(static_cast<float>(e.value),"CONTRAST");
        }else if(e.target == blurValue){
            this->setCustomVar(static_cast<float>(e.value),"BLUR");
        }
    }

}

//--------------------------------------------------------------
void BackgroundSubtraction::onMatrixEvent(ofxDatGuiMatrixEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == bgSubTechSelector){
            bgSubTech = e.child;
            this->setCustomVar(static_cast<float>(bgSubTech),"SUBTRACTION_TECHNIQUE");
            if(bgSubTech == 0){
                bgTechLabel->setLabel("B&W ABS");
            }else if(bgSubTech == 1){
                bgTechLabel->setLabel("LIGHTER THAN");
            }else if(bgSubTech == 2){
                bgTechLabel->setLabel("DARKER THAN");
            }
        }
    }
}
