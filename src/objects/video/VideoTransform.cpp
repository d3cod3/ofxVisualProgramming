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

#include "VideoTransform.h"

//--------------------------------------------------------------
VideoTransform::VideoTransform() : PatchObject(){

    this->numInlets  = 8;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();  // input
    _inletParams[1] = new float();      // x
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();      // y
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();      // w
    *(float *)&_inletParams[3] = 0.0f;
    _inletParams[4] = new float();      // h
    *(float *)&_inletParams[4] = 0.0f;
    _inletParams[5] = new float();      // angleX
    *(float *)&_inletParams[5] = 0.0f;
    _inletParams[6] = new float();      // angleY
    *(float *)&_inletParams[6] = 0.0f;
    _inletParams[7] = new float();      // angleZ
    *(float *)&_inletParams[7] = 0.0f;

    _outletParams[0] = new ofTexture(); // output

    this->initInletsState();

    isGUIObject             = true;
    this->isOverGUI         = true;

    this->width             *= 2;
    this->height            *= 2;

    scaledFbo  = new ofFbo();
    needToGrab  = false;

    loaded      = false;

}

//--------------------------------------------------------------
void VideoTransform::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_TEXTURE,"input");
    this->addInlet(VP_LINK_NUMERIC,"x");
    this->addInlet(VP_LINK_NUMERIC,"y");
    this->addInlet(VP_LINK_NUMERIC,"width");
    this->addInlet(VP_LINK_NUMERIC,"height");
    this->addInlet(VP_LINK_NUMERIC,"angleX");
    this->addInlet(VP_LINK_NUMERIC,"angleY");
    this->addInlet(VP_LINK_NUMERIC,"angleZ");
    this->addOutlet(VP_LINK_TEXTURE,"transformedOutput");

    this->setCustomVar(0.001f,"POSX");
    this->setCustomVar(0.001f,"POSY");
    this->setCustomVar(STANDARD_TEXTURE_WIDTH,"WIDTH");
    this->setCustomVar(STANDARD_TEXTURE_HEIGHT,"HEIGHT");
    this->setCustomVar(360.0f,"ANGLEX");
    this->setCustomVar(360.0f,"ANGLEY");
    this->setCustomVar(360.0f,"ANGLEZ");
}

//--------------------------------------------------------------
void VideoTransform::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->on2dPadEvent(this, &VideoTransform::on2dPadEvent);
    gui->onSliderEvent(this, &VideoTransform::onSliderEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    gui->addBreak();
    gui->addBreak();
    pad = gui->add2dPad("POS");
    pad->setUseCustomMouse(true);
    pad->setPoint(ofPoint(this->getCustomVar("POSX"),this->getCustomVar("POSY"),0));
    sliderW = gui->addSlider("Width", 0.0,1280.0,this->getCustomVar("WIDTH"));
    sliderW->setUseCustomMouse(true);
    sliderH = gui->addSlider("Height", 0.0,720.0,this->getCustomVar("HEIGHT"));
    sliderH->setUseCustomMouse(true);
    angleX = gui->addSlider("AngleX", 0.0,360.0,this->getCustomVar("ANGLEX"));
    angleX->setUseCustomMouse(true);
    angleY = gui->addSlider("AngleY", 0.0,360.0,this->getCustomVar("ANGLEY"));
    angleY->setUseCustomMouse(true);
    angleZ = gui->addSlider("AngleZ", 0.0,360.0,this->getCustomVar("ANGLEZ"));
    angleZ->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);
}

//--------------------------------------------------------------
void VideoTransform::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    gui->update();
    header->update();
    sliderW->update();
    sliderH->update();
    angleX->update();
    angleY->update();
    angleZ->update();
    pad->update();

    if(this->inletsConnected[0]){
        if(static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
            if(!needToGrab){
                needToGrab = true;
                scaledFbo->allocate(static_cast<ofTexture *>(_inletParams[0])->getWidth(), static_cast<ofTexture *>(_inletParams[0])->getHeight(), GL_RGBA );
                sliderW->setMax(static_cast<ofTexture *>(_inletParams[0])->getWidth());
                sliderH->setMax(static_cast<ofTexture *>(_inletParams[0])->getHeight());
            }

            scaledFbo->begin();
            ofClear(0,0,0,255);
            bounds.set((pad->getPoint().x/pad->getBounds().width)*static_cast<ofTexture *>(_inletParams[0])->getWidth(),(pad->getPoint().y/pad->getBounds().height)*static_cast<ofTexture *>(_inletParams[0])->getHeight(),sliderW->getValue(),sliderH->getValue());

            ofPushMatrix();
            ofTranslate(bounds.x+(bounds.width/2),bounds.y+(bounds.height/2));
            ofRotateXDeg(angleX->getValue());
            ofRotateYDeg(angleY->getValue());
            ofRotateZDeg(angleZ->getValue());
            static_cast<ofTexture *>(_inletParams[0])->draw(-bounds.width/2,0-bounds.height/2,bounds.width,bounds.height);
            ofPopMatrix();
            scaledFbo->end();

            *static_cast<ofTexture *>(_outletParams[0]) = scaledFbo->getTexture();
        }
    }else{
        needToGrab = false;
    }

    if(this->inletsConnected[1] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(*(float *)&_inletParams[1] == 0.0){
            pad->setPoint(ofPoint(0.000001,pad->getPoint().y,pad->getPoint().z));
        }else if(*(float *)&_inletParams[1] == static_cast<ofTexture *>(_inletParams[0])->getWidth()){
            pad->setPoint(ofPoint(pad->getBounds().width*0.999999,pad->getPoint().y,pad->getPoint().z));
        }else{
            pad->setPoint(ofPoint(ofClamp(*(float *)&_inletParams[1],0,static_cast<ofTexture *>(_inletParams[0])->getWidth())/static_cast<ofTexture *>(_inletParams[0])->getWidth()*pad->getBounds().width,pad->getPoint().y,pad->getPoint().z));
        }
    }

    if(this->inletsConnected[2] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(*(float *)&_inletParams[2] == 0.0){
            pad->setPoint(ofPoint(pad->getPoint().x,0.000001,pad->getPoint().z));
        }else if(*(float *)&_inletParams[2] == static_cast<ofTexture *>(_inletParams[0])->getHeight()){
            pad->setPoint(ofPoint(pad->getPoint().x,pad->getBounds().height*0.999999,pad->getPoint().z));
        }else{
            pad->setPoint(ofPoint(pad->getPoint().x,ofClamp(*(float *)&_inletParams[2],0,static_cast<ofTexture *>(_inletParams[0])->getHeight())/static_cast<ofTexture *>(_inletParams[0])->getHeight()*pad->getBounds().height,pad->getPoint().z));
        }
    }

    if(this->inletsConnected[3] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        sliderW->setValue(ofClamp(*(float *)&_inletParams[3],0.0f,static_cast<ofTexture *>(_inletParams[0])->getWidth()));
    }

    if(this->inletsConnected[4] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        sliderH->setValue(ofClamp(*(float *)&_inletParams[4],0.0f,static_cast<ofTexture *>(_inletParams[0])->getHeight()));
    }

    if(this->inletsConnected[5] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        angleX->setValue(ofClamp(*(float *)&_inletParams[5],0.0f,360.0f));
    }

    if(this->inletsConnected[6] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        angleY->setValue(ofClamp(*(float *)&_inletParams[6],0.0f,360.0f));
    }

    if(this->inletsConnected[7] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        angleZ->setValue(ofClamp(*(float *)&_inletParams[7],0.0f,360.0f));
    }

    if(!loaded){
        loaded = true;
        pad->setPoint(ofPoint(this->getCustomVar("POSX"),this->getCustomVar("POSY"),0));
        sliderW->setValue(this->getCustomVar("WIDTH"));
        sliderH->setValue(this->getCustomVar("HEIGHT"));
        angleX->setValue(this->getCustomVar("ANGLEX"));
        angleY->setValue(this->getCustomVar("ANGLEY"));
        angleZ->setValue(this->getCustomVar("ANGLEZ"));
    }

}

//--------------------------------------------------------------
void VideoTransform::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
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
void VideoTransform::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void VideoTransform::on2dPadEvent(ofxDatGui2dPadEvent e){
    this->setCustomVar(static_cast<float>(e.x),"POSX");
    this->setCustomVar(static_cast<float>(e.y),"POSY");
}

//--------------------------------------------------------------
void VideoTransform::onSliderEvent(ofxDatGuiSliderEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == sliderW){
            this->setCustomVar(static_cast<float>(e.value),"WIDTH");
        }else if(e.target == sliderH){
            this->setCustomVar(static_cast<float>(e.value),"HEIGHT");
        }else if(e.target == angleX){
            this->setCustomVar(static_cast<float>(e.value),"ANGLEX");
        }else if(e.target == angleY){
            this->setCustomVar(static_cast<float>(e.value),"ANGLEY");
        }else if(e.target == angleZ){
            this->setCustomVar(static_cast<float>(e.value),"ANGLEZ");
        }
    }
}

OBJECT_REGISTER( VideoTransform, "video transform", OFXVP_OBJECT_CAT_VIDEO)

#endif
