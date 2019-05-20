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

#include "VideoDelay.h"

//--------------------------------------------------------------
VideoDelay::VideoDelay() : PatchObject(){

    this->numInlets  = 5;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();  // input
    _inletParams[1] = new float();      // x
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();      // y
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();      // scale
    *(float *)&_inletParams[3] = 0.0f;
    _inletParams[4] = new float();      // alpha
    *(float *)&_inletParams[4] = 0.0f;

    _outletParams[0] = new ofTexture(); // output

    this->initInletsState();

    isGUIObject             = true;
    this->isOverGUI         = true;

    backBufferTex   = new ofTexture();
    delayFbo        = new ofFbo();

    alpha           = 0.0f;
    alphaTo         = 1.0f;
    scale           = 1;
    scaleTo         = 1;
    needToGrab      = false;

    loaded          = false;

}

//--------------------------------------------------------------
void VideoDelay::newObject(){
    this->setName("video feedback");
    this->addInlet(VP_LINK_TEXTURE,"input");
    this->addInlet(VP_LINK_NUMERIC,"x");
    this->addInlet(VP_LINK_NUMERIC,"y");
    this->addInlet(VP_LINK_NUMERIC,"scale");
    this->addInlet(VP_LINK_NUMERIC,"alpha");
    this->addOutlet(VP_LINK_TEXTURE,"feedbackOutput");

    this->setCustomVar(0.0f,"POSX");
    this->setCustomVar(0.0f,"POSY");
    this->setCustomVar(static_cast<float>(scaleTo),"SCALE");
    this->setCustomVar(static_cast<float>(alphaTo),"ALPHA");
}

//--------------------------------------------------------------
void VideoDelay::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->on2dPadEvent(this, &VideoDelay::on2dPadEvent);
    gui->onSliderEvent(this, &VideoDelay::onSliderEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    gui->addBreak();
    gui->addBreak();
    pad = gui->add2dPad("POS");
    pad->setUseCustomMouse(true);
    pad->setPoint(ofPoint(this->getCustomVar("POSX"),this->getCustomVar("POSY"),0));
    slider = gui->addSlider("Scale", 0.0,1.0,1.0);
    slider->setUseCustomMouse(true);
    sliderA = gui->addSlider("Alpha", 0.0,1.0,1.0);
    sliderA->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);
}

//--------------------------------------------------------------
void VideoDelay::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    pad->update();
    slider->update();
    sliderA->update();

    alpha       = .995 * alpha + .005 * alphaTo;
    scale       = .95 * scale + .05 * scaleTo;
    halfscale   = (1.000000f - scale) / 2.000000f;
    
    if(this->inletsConnected[0]){
        if(!needToGrab){
            needToGrab = true;
            backBufferTex->allocate(static_cast<ofTexture *>(_inletParams[0])->getWidth(), static_cast<ofTexture *>(_inletParams[0])->getHeight(), GL_RGB);
            delayFbo->allocate(static_cast<ofTexture *>(_inletParams[0])->getWidth(), static_cast<ofTexture *>(_inletParams[0])->getHeight(), GL_RGBA);
            backBufferTex = static_cast<ofTexture *>(_inletParams[0]);
        }

        delayFbo->begin();
        ofEnableAlphaBlending();
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f,1.0f,1.0f,alpha);
        glPushMatrix();

        bounds.set((pad->getPoint().x/pad->getBounds().width)*static_cast<ofTexture *>(_inletParams[0])->getWidth(),(pad->getPoint().y/pad->getBounds().height)*static_cast<ofTexture *>(_inletParams[0])->getHeight(),static_cast<ofTexture *>(_inletParams[0])->getWidth(), static_cast<ofTexture *>(_inletParams[0])->getHeight());
        backBufferTex->draw(bounds.x, bounds.y, static_cast<ofTexture *>(_inletParams[0])->getWidth() * scale, static_cast<ofTexture *>(_inletParams[0])->getHeight() * scale );
        backBufferTex = static_cast<ofTexture *>(_inletParams[0]);

        glPopMatrix();
        ofDisableAlphaBlending();
        delayFbo->end();

        *static_cast<ofTexture *>(_outletParams[0]) = delayFbo->getTexture();
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

    if(this->inletsConnected[3]){
        scaleTo = ofClamp(*(float *)&_inletParams[1],0.0f,1.0f);
        slider->setValue(scaleTo);
    }

    if(this->inletsConnected[4]){
        alphaTo = ofClamp(*(float *)&_inletParams[2],0.0f,1.0f);
        sliderA->setValue(alphaTo);
    }

    if(!loaded){
        loaded = true;
        pad->setPoint(ofPoint(this->getCustomVar("POSX"),this->getCustomVar("POSY"),0));
        slider->setValue(this->getCustomVar("SCALE"));
        sliderA->setValue(this->getCustomVar("ALPHA"));
    }
    
}

//--------------------------------------------------------------
void VideoDelay::drawObjectContent(ofxFontStash *font){
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
void VideoDelay::removeObjectContent(){
    
}

//--------------------------------------------------------------
void VideoDelay::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    pad->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    slider->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    sliderA->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || pad->hitTest(_m-this->getPos()) || slider->hitTest(_m-this->getPos()) || sliderA->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void VideoDelay::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        pad->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        slider->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        sliderA->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void VideoDelay::on2dPadEvent(ofxDatGui2dPadEvent e){
    this->setCustomVar(static_cast<float>(e.x),"POSX");
    this->setCustomVar(static_cast<float>(e.y),"POSY");
}

//--------------------------------------------------------------
void VideoDelay::onSliderEvent(ofxDatGuiSliderEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == slider){
            this->setCustomVar(static_cast<float>(e.value),"SCALE");
            scaleTo = ofClamp(static_cast<float>(e.value),0.0f,1.0f);
        }else if(e.target == sliderA){
            this->setCustomVar(static_cast<float>(e.value),"ALPHA");
            alphaTo = ofClamp(static_cast<float>(e.value),0.0f,1.0f);
        }
    }

}
