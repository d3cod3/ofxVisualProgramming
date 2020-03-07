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

#include "moMessage.h"

//--------------------------------------------------------------
moMessage::moMessage() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;

    _inletParams[1] = new string();  // message
    *static_cast<string *>(_inletParams[1]) = "";

    _outletParams[0] = new string(); // output
    *static_cast<string *>(_outletParams[0]) = "";

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    actualMessage   = "";

}

//--------------------------------------------------------------
void moMessage::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_STRING,"message");
    this->addOutlet(VP_LINK_STRING,"message");
}

//--------------------------------------------------------------
void moMessage::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width/3 * 2);
    gui->onButtonEvent(this, &moMessage::onButtonEvent);
    gui->onTextInputEvent(this, &moMessage::onTextInputEvent);

    sendButton = gui->addButton("SEND");
    sendButton->setUseCustomMouse(true);

    message = gui->addTextInput("","");
    message->setUseCustomMouse(true);

    gui->setPosition(this->width/3 + 1,this->height - (sendButton->getHeight()*2));
}

//--------------------------------------------------------------
void moMessage::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){
    gui->update();
    sendButton->update();
    message->update();

    if(this->inletsConnected[0] && *(float *)&_inletParams[0] >= 1.0){
        if(this->inletsConnected[1]){
            *static_cast<string *>(_outletParams[0]) = "";
            *static_cast<string *>(_outletParams[0]) = *static_cast<string *>(_inletParams[1]);
            actualMessage = ofToUpper(*static_cast<string *>(_inletParams[1]));
        }else{
            *static_cast<string *>(_outletParams[0]) = "";
            *static_cast<string *>(_outletParams[0]) = message->getText();
            actualMessage = ofToUpper(message->getText());
        }

    }
}

//--------------------------------------------------------------
void moMessage::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    font->draw(actualMessage,this->fontSize,this->width - (strlen(actualMessage.c_str())*this->fontSize),this->headerHeight*2.3);
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void moMessage::removeObjectContent(bool removeFileFromData){
    
}

//--------------------------------------------------------------
void moMessage::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    sendButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    message->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    this->isOverGUI = sendButton->hitTest(_m-this->getPos()) || message->hitTest(_m-this->getPos());
}

//--------------------------------------------------------------
void moMessage::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        sendButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        message->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void moMessage::onButtonEvent(ofxDatGuiButtonEvent e){
    if (e.target == sendButton){
        *static_cast<string *>(_outletParams[0]) = "";
        *static_cast<string *>(_outletParams[0]) = message->getText();
        actualMessage = ofToUpper(message->getText());
    }
}

//--------------------------------------------------------------
void moMessage::onTextInputEvent(ofxDatGuiTextInputEvent e){
    // cout << "From Event Object: " << e.text << endl;
}

OBJECT_REGISTER( moMessage, "message", OFXVP_OBJECT_CAT_GUI);
