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

#include "moSlider.h"

//--------------------------------------------------------------
moSlider::moSlider() : PatchObject(){

    this->numInlets  = 3;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // min
    _inletParams[1] = new float();  // max
    _inletParams[2] = new float();  // value
    *(float *)&_inletParams[0] = 0.0f;
    *(float *)&_inletParams[1] = 1.0f;
    *(float *)&_inletParams[2] = 0.0f;

    _outletParams[0] = new float(); // output
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    loaded              = false;

}

//--------------------------------------------------------------
void moSlider::newObject(){
    this->setName("slider");
    this->addInlet(VP_LINK_NUMERIC,"min");
    this->addInlet(VP_LINK_NUMERIC,"max");
    this->addInlet(VP_LINK_NUMERIC,"value");
    this->addOutlet(VP_LINK_NUMERIC,"value");

    this->setCustomVar(static_cast<float>(0),"MIN");
    this->setCustomVar(static_cast<float>(1),"MAX");
    this->setCustomVar(static_cast<float>(0),"VALUE");
}

//--------------------------------------------------------------
void moSlider::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onSliderEvent(this, &moSlider::onSliderEvent);

    slider = gui->addSlider("", 0,1,0);
    slider->setUseCustomMouse(true);

    gui->setPosition(0,this->height/3 + slider->getHeight()/2);
}

//--------------------------------------------------------------
void moSlider::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){
    gui->update();
    slider->update();

    if(this->inletsConnected[0]){
        slider->setMin(*(float *)&_inletParams[0]);
    }
    if(this->inletsConnected[1]){
        slider->setMax(*(float *)&_inletParams[1]);
    }

    if(this->inletsConnected[2]){
        slider->setValue(*(float *)&_inletParams[2]);
    }

    *(float *)&_outletParams[0] = static_cast<float>(slider->getValue());

    if(!loaded){
        loaded = true;
        slider->setMin(this->getCustomVar("MIN"));
        slider->setMax(this->getCustomVar("MAX"));
        slider->setValue(this->getCustomVar("VALUE"));
    }
}

//--------------------------------------------------------------
void moSlider::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void moSlider::removeObjectContent(){
    
}

//--------------------------------------------------------------
void moSlider::mouseMovedObjectContent(ofVec3f _m){
    if(!this->inletsConnected[2]){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        slider->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }

    this->isOverGUI = slider->hitTest(_m-this->getPos());
}

//--------------------------------------------------------------
void moSlider::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI && !this->inletsConnected[2]){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        slider->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void moSlider::onSliderEvent(ofxDatGuiSliderEvent e){
    *(float *)&_outletParams[0] = static_cast<float>(e.value);
    this->setCustomVar(static_cast<float>(e.value),"VALUE");
    if(this->inletsConnected[0]){
        this->setCustomVar(static_cast<float>(*(float *)&_inletParams[0]),"MIN");
    }
    if(this->inletsConnected[1]){
        this->setCustomVar(static_cast<float>(*(float *)&_inletParams[1]),"MAX");
    }
}
