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

#include "moPlayerControls.h"

//--------------------------------------------------------------
moPlayerControls::moPlayerControls() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;

    _inletParams[1] = new float();  // select
    *(float *)&_inletParams[1] = 0.0f;

    _outletParams[0] = new string(); // output
    *static_cast<string *>(_outletParams[0]) = "";

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    bang                = false;
    loaded              = false;

}

//--------------------------------------------------------------
void moPlayerControls::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"select");
    this->addOutlet(VP_LINK_STRING,"command");

    this->setCustomVar(static_cast<float>(0),"PAUSE");
    this->setCustomVar(static_cast<float>(0),"LOOP");
}

//--------------------------------------------------------------
void moPlayerControls::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width/3 * 2);
    gui->onButtonEvent(this, &moPlayerControls::onButtonEvent);
    gui->onToggleEvent(this, &moPlayerControls::onToggleEvent);

    playButton = gui->addButton("PLAY");
    playButton->setUseCustomMouse(true);
    stopButton = gui->addButton("STOP");
    stopButton->setUseCustomMouse(true);
    pauseButton = gui->addToggle("PAUSE",false);
    pauseButton->setUseCustomMouse(true);
    loopButton = gui->addToggle("LOOP",false);
    loopButton->setUseCustomMouse(true);

    gui->setPosition(this->width/3 + 1,this->height - (playButton->getHeight()*4) - 4);
}

//--------------------------------------------------------------
void moPlayerControls::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){
    gui->update();
    playButton->update();
    stopButton->update();
    pauseButton->update();
    loopButton->update();

    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] < 1.0){
            bang = false;
        }else{
            bang = true;
        }
    }

    if(bang && this->inletsConnected[1]){
        int temp = static_cast<int>(floor(*(float *)&_inletParams[1]));
        switch (temp) {
            case 1:
                *static_cast<string *>(_outletParams[0]) = "play";
                pauseButton->setChecked(false);
                break;
            case 2:
                *static_cast<string *>(_outletParams[0]) = "stop";
                pauseButton->setChecked(false);
                break;
            case 3:
                *static_cast<string *>(_outletParams[0]) = "pause";
                break;
            case 4:
                *static_cast<string *>(_outletParams[0]) = "unpause";
                break;
            case 5:
                *static_cast<string *>(_outletParams[0]) = "loop_normal";
                break;
            case 6:
                *static_cast<string *>(_outletParams[0]) = "loop_none";
                break;
            default:
                break;
        }
    }

    if(!loaded){
        loaded = true;
        pauseButton->setChecked(static_cast<int>(floor(this->getCustomVar("PAUSE"))));
        loopButton->setChecked(static_cast<int>(floor(this->getCustomVar("LOOP"))));
        if(pauseButton->getChecked()){
            *static_cast<string *>(_outletParams[0]) = "pause";
        }else{
            *static_cast<string *>(_outletParams[0]) = "unpause";
        }
        if(loopButton->getChecked()){
            *static_cast<string *>(_outletParams[0]) = "loop_normal";
        }else{
            *static_cast<string *>(_outletParams[0]) = "loop_none";
        }
    }
    
}

//--------------------------------------------------------------
void moPlayerControls::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void moPlayerControls::removeObjectContent(bool removeFileFromData){
    
}

//--------------------------------------------------------------
void moPlayerControls::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    playButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    stopButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    pauseButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    loopButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    this->isOverGUI = playButton->hitTest(_m-this->getPos()) || stopButton->hitTest(_m-this->getPos()) || pauseButton->hitTest(_m-this->getPos()) || loopButton->hitTest(_m-this->getPos());

    if(this->isOverGUI){
        // reset output message
        *static_cast<string *>(_outletParams[0]) = "";
    }
}

//--------------------------------------------------------------
void moPlayerControls::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        playButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        stopButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        pauseButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        loopButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));        
    }else{
        ofNotifyEvent(dragEvent, nId);

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
void moPlayerControls::onButtonEvent(ofxDatGuiButtonEvent e){
    if (e.target == playButton){
        *static_cast<string *>(_outletParams[0]) = "play";
        pauseButton->setChecked(false);
    }else if(e.target == stopButton){
        *static_cast<string *>(_outletParams[0]) = "stop";
        pauseButton->setChecked(false);
    }
}

//--------------------------------------------------------------
void moPlayerControls::onToggleEvent(ofxDatGuiToggleEvent e){
    if(e.target == pauseButton){
        if(e.checked){
            *static_cast<string *>(_outletParams[0]) = "pause";
        }else{
            *static_cast<string *>(_outletParams[0]) = "unpause";
        }
        this->setCustomVar(static_cast<float>(e.checked),"PAUSE");
        this->saveConfig(false,this->nId);
    }else if(e.target == loopButton){
        if(e.checked){
            *static_cast<string *>(_outletParams[0]) = "loop_normal";
        }else{
            *static_cast<string *>(_outletParams[0]) = "loop_none";
        }
        this->setCustomVar(static_cast<float>(e.checked),"LOOP");
        this->saveConfig(false,this->nId);
    }

}

OBJECT_REGISTER( moPlayerControls, "player controls", OFXVP_OBJECT_CAT_GUI)

#endif