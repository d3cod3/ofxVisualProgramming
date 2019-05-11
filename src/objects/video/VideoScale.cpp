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

#include "VideoScale.h"

//--------------------------------------------------------------
VideoScale::VideoScale() : PatchObject(){

    this->numInlets  = 5;
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

    _outletParams[0] = new ofTexture(); // output

    this->initInletsState();

    isGUIObject             = true;
    this->isOverGUI         = true;

    scaledFbo  = new ofFbo();
    needToGrab  = false;

    loaded      = false;

}

//--------------------------------------------------------------
void VideoScale::newObject(){
    this->setName("video scale");
    this->addInlet(VP_LINK_TEXTURE,"input");
    this->addInlet(VP_LINK_NUMERIC,"x");
    this->addInlet(VP_LINK_NUMERIC,"y");
    this->addInlet(VP_LINK_NUMERIC,"width");
    this->addInlet(VP_LINK_NUMERIC,"height");
    this->addOutlet(VP_LINK_TEXTURE);

    this->setCustomVar(0.0f,"POSX");
    this->setCustomVar(0.0f,"POSY");
    this->setCustomVar(1280.0f,"WIDTH");
    this->setCustomVar(720.0f,"HEIGHT");
}

//--------------------------------------------------------------
void VideoScale::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->on2dPadEvent(this, &VideoScale::on2dPadEvent);
    gui->onSliderEvent(this, &VideoScale::onSliderEvent);

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

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);
}

//--------------------------------------------------------------
void VideoScale::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    sliderW->update();
    sliderH->update();
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
            static_cast<ofTexture *>(_inletParams[0])->draw(bounds.x,bounds.y,bounds.width,bounds.height);
            scaledFbo->end();

            *static_cast<ofTexture *>(_outletParams[0]) = scaledFbo->getTexture();
        }
    }else{
        needToGrab = false;
    }

    if(this->inletsConnected[1]){
        pad->setPoint(ofPoint(*(float *)&_inletParams[1]*pad->getBounds().width,pad->getPoint().y,pad->getPoint().z));
    }

    if(this->inletsConnected[2]){
        pad->setPoint(ofPoint(pad->getPoint().x,*(float *)&_inletParams[2]*pad->getBounds().height,pad->getPoint().z));
    }

    if(this->inletsConnected[3] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        sliderW->setValue(ofClamp(*(float *)&_inletParams[3],0.0f,static_cast<ofTexture *>(_inletParams[0])->getWidth()));
    }

    if(this->inletsConnected[4] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        sliderH->setValue(ofClamp(*(float *)&_inletParams[4],0.0f,static_cast<ofTexture *>(_inletParams[0])->getHeight()));
    }

    if(!loaded){
        loaded = true;
        pad->setPoint(ofPoint(this->getCustomVar("POSX"),this->getCustomVar("POSY"),0));
        sliderW->setValue(this->getCustomVar("WIDTH"));
        sliderH->setValue(this->getCustomVar("HEIGHT"));
    }
    
}

//--------------------------------------------------------------
void VideoScale::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
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
        static_cast<ofTexture *>(_outletParams[0])->draw(posX,posY,drawW,drawH);
    }
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void VideoScale::removeObjectContent(){
    
}

//--------------------------------------------------------------
void VideoScale::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    pad->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    sliderW->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    sliderH->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || pad->hitTest(_m-this->getPos()) || sliderW->hitTest(_m-this->getPos()) || sliderH->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void VideoScale::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        pad->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        sliderW->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        sliderH->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void VideoScale::on2dPadEvent(ofxDatGui2dPadEvent e){
    this->setCustomVar(static_cast<float>(e.x),"POSX");
    this->setCustomVar(static_cast<float>(e.y),"POSY");
}

//--------------------------------------------------------------
void VideoScale::onSliderEvent(ofxDatGuiSliderEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == sliderW){
            this->setCustomVar(static_cast<float>(e.value),"WIDTH");
        }else if(e.target == sliderH){
            this->setCustomVar(static_cast<float>(e.value),"HEIGHT");
        }
    }
}
