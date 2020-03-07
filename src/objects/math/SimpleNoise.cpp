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

#include "SimpleNoise.h"

//--------------------------------------------------------------
SimpleNoise::SimpleNoise() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // step
    *(float *)&_inletParams[0] = 0.001f;

    _outletParams[0] = new float(); // output
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    timePosition = ofRandom(10000);
}

//--------------------------------------------------------------
void SimpleNoise::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"step");
    this->addOutlet(VP_LINK_NUMERIC,"noise");
}

//--------------------------------------------------------------
void SimpleNoise::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setWidth(this->width);

    noisePlotter = gui->addValuePlotter("",0.0f,1.0f);
    noisePlotter->setDrawMode(ofxDatGuiGraph::LINES);
    noisePlotter->setSpeed(0.6);

    gui->setPosition(0,this->height-noisePlotter->getHeight());
}

//--------------------------------------------------------------
void SimpleNoise::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){
    *(float *)&_outletParams[0] = ofNoise(timePosition);

    gui->update();
    noisePlotter->setValue(*(float *)&_outletParams[0]);

    timePosition += *(float *)&_inletParams[0];
}

//--------------------------------------------------------------
void SimpleNoise::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    font->draw(ofToString(*(float *)&_outletParams[0]),this->fontSize,this->width/2,this->headerHeight*2.3);
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void SimpleNoise::removeObjectContent(bool removeFileFromData){
    
}

OBJECT_REGISTER( SimpleNoise, "simple noise", OFXVP_OBJECT_CAT_MATH);
