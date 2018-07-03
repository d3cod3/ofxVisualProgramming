#include "moMessage.h"

//--------------------------------------------------------------
moMessage::moMessage() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;

    _inletParams[1] = new string();  // message
    *(string *)&_inletParams[1] = "";

    _outletParams[0] = new string(); // output
    *(string *)&_outletParams[0] = "";

    for(int i=0;i<this->numInlets;i++){
        this->inletsConnected.push_back(false);
    }

    isGUIObject     = true;
    isOverGui       = true;

    actualMessage   = "";

}

//--------------------------------------------------------------
void moMessage::newObject(){
    this->setName("message");
    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_STRING,"message");
    this->addOutlet(VP_LINK_STRING);
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
void moMessage::updateObjectContent(map<int,PatchObject*> &patchObjects){
    gui->update();
    sendButton->update();
    message->update();
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
void moMessage::removeObjectContent(){
    
}

//--------------------------------------------------------------
void moMessage::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    sendButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    message->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    isOverGui = sendButton->hitTest(_m-this->getPos()) || message->hitTest(_m-this->getPos());
}

//--------------------------------------------------------------
void moMessage::dragGUIObject(ofVec3f _m){
    if(isOverGui){

    }else{
        ofNotifyEvent(dragEvent, nId);

        box->setFromCenter(_m.x, _m.y,box->getWidth(),box->getHeight());
        headerBox->set(box->getPosition().x,box->getPosition().y,box->getWidth(),headerHeight);

        x = box->getPosition().x;
        y = box->getPosition().y;

        for(int j=0;j<outPut.size();j++){
            outPut[j]->linkVertices[0].move(outPut[j]->posFrom.x,outPut[j]->posFrom.y);
            outPut[j]->linkVertices[1].move(outPut[j]->posFrom.x+20,outPut[j]->posFrom.y);
        }
    }
}

//--------------------------------------------------------------
void moMessage::onButtonEvent(ofxDatGuiButtonEvent e){
    if (e.target == sendButton){
        *(string *)&_outletParams[0] = "";
        *(string *)&_outletParams[0] = message->getText();
        actualMessage = ofToUpper(message->getText());
    }
}

//--------------------------------------------------------------
void moMessage::onTextInputEvent(ofxDatGuiTextInputEvent e){
    // cout << "From Event Object: " << e.text << endl;
}
