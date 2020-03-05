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

#include "Smooth.h"

//--------------------------------------------------------------
Smooth::Smooth() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // input
    _inletParams[1] = new float();  // smoothing
    *(float *)&_inletParams[0] = 0.0f;
    *(float *)&_inletParams[1] = 1.0f;

    _outletParams[0] = new float(); // output
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

}

//--------------------------------------------------------------
void Smooth::newObject(){
    this->setName("smooth");
    this->addInlet(VP_LINK_NUMERIC,"number");
    this->addOutlet(VP_LINK_NUMERIC,"smoothedValue");

    this->setCustomVar(1.0f,"SMOOTHING");
}

//--------------------------------------------------------------
void Smooth::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setWidth(this->width);
    gui->addBreak();
    gui->onSliderEvent(this, &Smooth::onSliderEvent);
    slider = gui->addSlider("", 0.0f, 1.0f,this->getCustomVar("SMOOTHING"));
    slider->setUseCustomMouse(true);
    gui->addBreak();
    rPlotter = gui->addValuePlotter("",0.0,1.0);
    rPlotter->setDrawMode(ofxDatGuiGraph::LINES);
    rPlotter->setSpeed(1);


    gui->setPosition(0,this->headerHeight);
}

//--------------------------------------------------------------
void Smooth::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){
    if(this->inletsConnected[0]){
        *(float *)&_outletParams[0] = *(float *)&_outletParams[0]*(1-slider->getValue()) + *(float *)&_inletParams[0]*slider->getValue();
        if(*(float *)&_inletParams[0] > rPlotter->getMax()){
            rPlotter->setRange(0.0,*(float *)&_inletParams[0]);
        }else if(*(float *)&_inletParams[0] < rPlotter->getMin()){
            rPlotter->setRange(*(float *)&_inletParams[0],0.0);
        }
    }else{
        *(float *)&_outletParams[0] = 0.0f;
        rPlotter->setRange(0.0,1.0);
    }

    gui->update();
    slider->update();

    rPlotter->setValue(*(float *)&_outletParams[0]);

    if(!loaded){
        loaded = true;
        slider->setValue(this->getCustomVar("SMOOTHING"));
    }

}

//--------------------------------------------------------------
void Smooth::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void Smooth::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void Smooth::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    slider->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    this->isOverGUI = slider->hitTest(_m-this->getPos());
}

//--------------------------------------------------------------
void Smooth::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
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
void Smooth::onSliderEvent(ofxDatGuiSliderEvent e){
    this->setCustomVar(static_cast<float>(e.value),"SMOOTHING");
}
