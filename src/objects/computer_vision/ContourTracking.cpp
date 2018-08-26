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

#include "ContourTracking.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
ContourTracking::ContourTracking() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();  // input
    _outletParams[0] = new ofTexture(); // output

    this->initInletsState();

    contourFinder   = new ofxCv::ContourFinder();
    pix             = new ofPixels();
    outputFBO       = new ofFbo();

    isGUIObject         = true;
    this->isOverGUI     = true;

    posX = posY = drawW = drawH = 0.0f;

    isFBOAllocated      = false;

}

//--------------------------------------------------------------
void ContourTracking::newObject(){
    this->setName("contour tracking");
    this->addInlet(VP_LINK_TEXTURE,"input");
    this->addOutlet(VP_LINK_TEXTURE);
}

//--------------------------------------------------------------
void ContourTracking::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    thresholdValue = gui->addSlider("THRESH",0,255);
    thresholdValue->setUseCustomMouse(true);
    thresholdValue->setValue(128);

    gui->onButtonEvent(this, &ContourTracking::onButtonEvent);
    gui->onSliderEvent(this, &ContourTracking::onSliderEvent);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    contourFinder->setMinAreaRadius(10);
    contourFinder->setMaxAreaRadius(200);
    contourFinder->setThreshold(thresholdValue->getValue());
    contourFinder->setFindHoles(false);
    // wait for half a second before forgetting something
    contourFinder->getTracker().setPersistence(15);
    // an object can move up to 32 pixels per frame
    contourFinder->getTracker().setMaximumDistance(32);

}

//--------------------------------------------------------------
void ContourTracking::updateObjectContent(map<int,PatchObject*> &patchObjects){
    gui->update();
    header->update();
    if(!header->getIsCollapsed()){
        thresholdValue->update();
    }

    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){

        if(!isFBOAllocated){
            isFBOAllocated = true;
            outputFBO->allocate(static_cast<ofTexture *>(_inletParams[0])->getWidth(),static_cast<ofTexture *>(_inletParams[0])->getHeight(),GL_RGB,1);
        }

        static_cast<ofTexture *>(_inletParams[0])->readToPixels(*pix);

        blur(*pix, 10);
        contourFinder->findContours(*pix);

        if(outputFBO->isAllocated()){
            *static_cast<ofTexture *>(_outletParams[0]) = outputFBO->getTexture();
        }

    }
    
}

//--------------------------------------------------------------
void ContourTracking::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(this->inletsConnected[0] && outputFBO->isAllocated() && static_cast<ofTexture *>(_outletParams[0])->isAllocated()){

        outputFBO->begin();

        ofClear(0,0,0,255);

        ofSetColor(255);
        static_cast<ofTexture *>(_inletParams[0])->draw(0,0);
        contourFinder->draw();
        for(int i = 0; i < contourFinder->size(); i++) {
            ofPoint center = toOf(contourFinder->getCenter(i));
            ofPushMatrix();
            ofTranslate(center.x, center.y);
            int label = contourFinder->getLabel(i);
            string msg = ofToString(label) + ":" + ofToString(contourFinder->getTracker().getAge(label));
            font->draw(msg,fontSize,0,0);
        }

        outputFBO->end();

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
void ContourTracking::removeObjectContent(){
    
}

//--------------------------------------------------------------
void ContourTracking::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    thresholdValue->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || thresholdValue->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void ContourTracking::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        thresholdValue->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void ContourTracking::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){

    }

}

//--------------------------------------------------------------
void ContourTracking::onSliderEvent(ofxDatGuiSliderEvent e){
    if(!header->getIsCollapsed()){

    }

}
