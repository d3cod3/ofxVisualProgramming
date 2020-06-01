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

#include "KeyReleased.h"

//--------------------------------------------------------------
KeyReleased::KeyReleased() : PatchObject(){

    this->numInlets  = 0;
    this->numOutlets = 1;

    _outletParams[0] = new float(); // output
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    lastKey             = -1;
    persistentKey       = -1;

}

//--------------------------------------------------------------
void KeyReleased::newObject(){
    this->setName(this->objectName);
    this->addOutlet(VP_LINK_NUMERIC,"bang");

    this->setCustomVar(static_cast<float>(lastKey),"KEY");
}

//--------------------------------------------------------------
void KeyReleased::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setWidth(this->width);
    gui->addBreak();
    gui->onTextInputEvent(this, &KeyReleased::onTextInputEvent);

    inputNumber = gui->addTextInput("KEY","-1");
    inputNumber->setUseCustomMouse(true);
    inputNumber->setText(ofToString(this->getCustomVar("KEY")));

    gui->setPosition(0,this->headerHeight);

    ofAddListener(mainWindow->events().keyReleased,this,&KeyReleased::objectKeyReleased);
}

//--------------------------------------------------------------
void KeyReleased::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    gui->update();
    inputNumber->update();

    if(lastKey == static_cast<int>(floor(this->getCustomVar("KEY"))) && lastKey != -1){
        lastKey = -1;
        *(float *)&_outletParams[0] = 1.0f;
    }else{
        *(float *)&_outletParams[0] = 0.0f;
    }

}

//--------------------------------------------------------------
void KeyReleased::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    font->draw("Last Key: "+ofToString(persistentKey),this->fontSize,this->width/3,this->height/2);
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void KeyReleased::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void KeyReleased::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    inputNumber->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    this->isOverGUI = inputNumber->hitTest(_m-this->getPos());
}

//--------------------------------------------------------------
void KeyReleased::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        inputNumber->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void KeyReleased::objectKeyReleased(ofKeyEventArgs &e){
    lastKey         = e.key;
    persistentKey   = e.key;
}

//--------------------------------------------------------------
void KeyReleased::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(e.target == inputNumber){
        if(isInteger(e.text) || isFloat(e.text)){
            this->setCustomVar(static_cast<float>(ofToFloat(e.text)),"KEY");
        }

    }
}

OBJECT_REGISTER( KeyReleased, "key released", OFXVP_OBJECT_CAT_COMMUNICATIONS)

#endif