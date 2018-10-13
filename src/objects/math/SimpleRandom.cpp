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

#include "SimpleRandom.h"

//--------------------------------------------------------------
SimpleRandom::SimpleRandom() : PatchObject(){

    this->numInlets  = 3;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // bang
    _inletParams[1] = new float();  // min
    _inletParams[2] = new float();  // max
    *(float *)&_inletParams[0] = 0.0f;
    *(float *)&_inletParams[1] = 0.0f;
    *(float *)&_inletParams[2] = 1.0f;

    _outletParams[0] = new float(); // output
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    changeRange     = false;
    bang            = false;

    lastMinRange    = *(float *)&_inletParams[1];
    lastMaxRange    = *(float *)&_inletParams[2];
}

//--------------------------------------------------------------
void SimpleRandom::newObject(){
    this->setName("simple random");
    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"min");
    this->addInlet(VP_LINK_NUMERIC,"max");
    this->addOutlet(VP_LINK_NUMERIC);
}

//--------------------------------------------------------------
void SimpleRandom::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    ofSeedRandom(ofGetElapsedTimeMillis());

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setWidth(this->width);

    rPlotter = gui->addValuePlotter("",*(float *)&_inletParams[1],*(float *)&_inletParams[2]);
    rPlotter->setDrawMode(ofxDatGuiGraph::LINES);
    rPlotter->setSpeed(1);

    gui->setPosition(0,this->height-rPlotter->getHeight());
}

//--------------------------------------------------------------
void SimpleRandom::updateObjectContent(map<int,PatchObject*> &patchObjects){
    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] < 1.0){
            bang = false;
        }else{
            bang = true;
        }
    }
    if(bang){
        *(float *)&_outletParams[0] = ofRandom(*(float *)&_inletParams[1],*(float *)&_inletParams[2]);
    }

    gui->update();
    rPlotter->setValue(*(float *)&_outletParams[0]);
    if(this->inletsConnected[1] || this->inletsConnected[2]){
        if(lastMinRange != *(float *)&_inletParams[1] || lastMaxRange != *(float *)&_inletParams[2]){
            changeRange = true;
        }
        if(changeRange){
            changeRange = false;
            rPlotter->setRange(*(float *)&_inletParams[1],*(float *)&_inletParams[2]);
        }
    }
}

//--------------------------------------------------------------
void SimpleRandom::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    font->draw(ofToString(*(float *)&_outletParams[0]),this->fontSize,this->width/2,this->headerHeight*2.3);
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void SimpleRandom::removeObjectContent(){
    
}
