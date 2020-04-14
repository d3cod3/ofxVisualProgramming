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

#include "mo2DPad.h"

//--------------------------------------------------------------
mo2DPad::mo2DPad() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 2;

    _inletParams[0] = new float();  // X
    _inletParams[1] = new float();  // Y
    *(float *)&_inletParams[0] = 0.0f;
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new float(); // output X
    _outletParams[1] = new float(); // output Y
    *(float *)&_outletParams[0] = 0.0f;
    *(float *)&_outletParams[1] = 0.0f;

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    loaded              = false;

}

//--------------------------------------------------------------
void mo2DPad::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"x");
    this->addInlet(VP_LINK_NUMERIC,"y");
    this->addOutlet(VP_LINK_NUMERIC,"padX");
    this->addOutlet(VP_LINK_NUMERIC,"padY");

    this->setCustomVar(static_cast<float>(0),"XPOS");
    this->setCustomVar(static_cast<float>(0),"YPOS");
}

//--------------------------------------------------------------
void mo2DPad::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->on2dPadEvent(this, &mo2DPad::on2dPadEvent);

    pad = gui->add2dPad("");
    pad->setUseCustomMouse(true);
    pad->setPoint(ofPoint(this->getCustomVar("XPOS"),this->getCustomVar("YPOS"),0));

    gui->setPosition(0,this->box->getHeight()-pad->getHeight());
}

//--------------------------------------------------------------
void mo2DPad::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){
    gui->update();
    pad->update();

    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] == 0.0){
            pad->setPoint(ofPoint(0.000001,pad->getPoint().y,pad->getPoint().z));
        }else if(*(float *)&_inletParams[0] == 1.0){
            pad->setPoint(ofPoint(pad->getBounds().width*0.999999,pad->getPoint().y,pad->getPoint().z));
        }else{
            pad->setPoint(ofPoint(*(float *)&_inletParams[0]*pad->getBounds().width,pad->getPoint().y,pad->getPoint().z));
        }
    }

    if(this->inletsConnected[1]){
        if(*(float *)&_inletParams[1] == 0.0){
            pad->setPoint(ofPoint(pad->getPoint().x,0.000001,pad->getPoint().z));
        }else if(*(float *)&_inletParams[1] == 1.0){
            pad->setPoint(ofPoint(pad->getPoint().x,pad->getBounds().height*0.999999,pad->getPoint().z));
        }else{
            pad->setPoint(ofPoint(pad->getPoint().x,*(float *)&_inletParams[1]*pad->getBounds().height,pad->getPoint().z));
        }
    }

    if(!loaded){
        loaded = true;
        pad->setPoint(ofPoint(this->getCustomVar("XPOS"),this->getCustomVar("YPOS"),0));
    }

    *(float *)&_outletParams[0] = static_cast<float>(pad->getPoint().x/pad->getBounds().width);
    *(float *)&_outletParams[1] = static_cast<float>(pad->getPoint().y/pad->getBounds().height);

}

//--------------------------------------------------------------
void mo2DPad::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void mo2DPad::removeObjectContent(bool removeFileFromData){
    
}

//--------------------------------------------------------------
void mo2DPad::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    pad->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    this->isOverGUI = pad->hitTest(_m-this->getPos());
}

//--------------------------------------------------------------
void mo2DPad::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        pad->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }else{
        

        box->setFromCenter(_m.x, _m.y,box->getWidth(),box->getHeight());
        headerBox->set(box->getPosition().x,box->getPosition().y,box->getWidth(),headerHeight);

        x = box->getPosition().x;
        y = box->getPosition().y;

        for(int j=0;j<static_cast<int>(outPut.size());j++){
            // (outPut[j]->posFrom.x,outPut[j]->posFrom.y);
            // (outPut[j]->posFrom.x+20,outPut[j]->posFrom.y);
        }
    }
}

//--------------------------------------------------------------
void mo2DPad::on2dPadEvent(ofxDatGui2dPadEvent e){
    this->setCustomVar(static_cast<float>(e.x),"XPOS");
    this->setCustomVar(static_cast<float>(e.y),"YPOS");
}

OBJECT_REGISTER( mo2DPad, "2d pad", OFXVP_OBJECT_CAT_GUI)

#endif