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

#include "OscSender.h"

//--------------------------------------------------------------
OscSender::OscSender() : PatchObject(){

    this->numInlets  = 0;
    this->numOutlets = 0;

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    osc_port            = 12345;
    osc_host            = "localhost";

    _tempPixels         = new ofPixels();
    _tempImage          = new ofImage();
    _tempBuffer         = new ofBuffer();
}

//--------------------------------------------------------------
void OscSender::newObject(){
    this->setName(this->objectName);

    this->setCustomVar(static_cast<float>(osc_port),"PORT");
}

//--------------------------------------------------------------
void OscSender::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    if(filepath == "none"){
        filepath = osc_host;
    }

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onButtonEvent(this, &OscSender::onButtonEvent);
    gui->onTextInputEvent(this, &OscSender::onTextInputEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    host = gui->addTextInput("HOST","localhost");
    host->setUseCustomMouse(true);
    host->setText(filepath);

    port = gui->addTextInput("PORT","12345");
    port->setUseCustomMouse(true);
    port->setText(ofToString(static_cast<int>(floor(this->getCustomVar("PORT")))));

    gui->addBreak();

    addOSCNumber = gui->addButton("ADD OSC NUMBER");
    addOSCNumber->setUseCustomMouse(true);
    addOSCNumber->setLabelAlignment(ofxDatGuiAlignment::CENTER);
    addOSCText = gui->addButton("ADD OSC TEXT");
    addOSCText->setUseCustomMouse(true);
    addOSCText->setLabelAlignment(ofxDatGuiAlignment::CENTER);
    addOSCVector = gui->addButton("ADD OSC VECTOR");
    addOSCVector->setUseCustomMouse(true);
    addOSCVector->setLabelAlignment(ofxDatGuiAlignment::CENTER);
    addOSCTexture = gui->addButton("ADD OSC IMAGE");
    addOSCTexture->setUseCustomMouse(true);
    addOSCTexture->setLabelAlignment(ofxDatGuiAlignment::CENTER);

    gui->addBreak();

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    initInlets();

    osc_host = filepath;
    osc_port = static_cast<int>(floor(this->getCustomVar("PORT")));
    osc_sender.setup(osc_host.c_str(),osc_port);

}

//--------------------------------------------------------------
void OscSender::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    host->update();
    port->update();
    addOSCNumber->update();
    addOSCText->update();
    addOSCVector->update();
    addOSCTexture->update();

    for(size_t l=0;l<labels.size();l++){
        labels.at(l)->update();
    }

    for(int i=0;i<this->getNumInlets();i++){
        if(this->inletsConnected[i]){
            ofxOscMessage m;
            bool messageOK = false;
            m.setAddress(osc_labels.at(i));
            if(this->getInletType(i) == VP_LINK_NUMERIC){
                m.addFloatArg(*(float *)&_inletParams[i]);
                messageOK = true;
            }else if(this->getInletType(i) == VP_LINK_STRING){
                m.addStringArg(*static_cast<string *>(_inletParams[i]));
                messageOK = true;
            }else if(this->getInletType(i) == VP_LINK_ARRAY){
                for(size_t s=0;s<static_cast<size_t>(static_cast<vector<float> *>(_inletParams[i])->size());s++){
                    m.addFloatArg(static_cast<vector<float> *>(_inletParams[i])->at(s));
                }
                messageOK = true;
            }else if(this->getInletType(i) == VP_LINK_TEXTURE && static_cast<ofTexture *>(_inletParams[i])->isAllocated()){
                // note: the size of the image depends greatly on your network buffer sizes,
                // if an image is too big the message won't come through
                int depth = 1;
                if(static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_LUMINANCE || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_LUMINANCE8 || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_LUMINANCE16 || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_LUMINANCE32F_ARB){
                    depth = 1;
                }else if(static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGB || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGB8 || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGB16 || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGB32F || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGB32F_ARB){
                    depth = 3;
                }else if(static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGBA ||static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGBA16 || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGBA32F_ARB){
                    depth = 4;
                }
                if(static_cast<ofTexture *>(_inletParams[i])->getWidth()*static_cast<ofTexture *>(_inletParams[i])->getHeight()*depth < 327680){
                    static_cast<ofTexture *>(_inletParams[i])->readToPixels(*_tempPixels);
                    m.addFloatArg(static_cast<ofTexture *>(_inletParams[i])->getWidth());
                    m.addFloatArg(static_cast<ofTexture *>(_inletParams[i])->getHeight());
                    if(static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_LUMINANCE || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_LUMINANCE8 || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_LUMINANCE16 || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_LUMINANCE32F_ARB){
                        _tempImage->setFromPixels(_tempPixels->getData(),static_cast<ofTexture *>(_inletParams[i])->getWidth(),static_cast<ofTexture *>(_inletParams[i])->getHeight(),OF_IMAGE_GRAYSCALE);
                        m.addInt32Arg(OF_IMAGE_GRAYSCALE);
                    }else if(static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGB || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGB8 || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGB16 || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGB32F || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGB32F_ARB){
                        _tempImage->setFromPixels(_tempPixels->getData(),static_cast<ofTexture *>(_inletParams[i])->getWidth(),static_cast<ofTexture *>(_inletParams[i])->getHeight(),OF_IMAGE_COLOR);
                        m.addInt32Arg(OF_IMAGE_COLOR);
                    }else if(static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGBA ||static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGBA16 || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGBA32F_ARB){
                        _tempImage->setFromPixels(_tempPixels->getData(),static_cast<ofTexture *>(_inletParams[i])->getWidth(),static_cast<ofTexture *>(_inletParams[i])->getHeight(),OF_IMAGE_COLOR_ALPHA);
                        m.addInt32Arg(OF_IMAGE_COLOR_ALPHA);
                    }
                    _tempImage->save(*_tempBuffer);
                    m.addBlobArg(*_tempBuffer);
                    _tempBuffer->clear();
                    messageOK = true;
                }else{
                    ofLog(OF_LOG_ERROR,"The image you're trying to send via OSC is too big! Please choose an image below 640x480 GRAYSCALE, or 320x240 RGB, RGBA");
                }
            }
            if(messageOK){
                osc_sender.sendMessage(m,false);
            }
        }
    }

}

//--------------------------------------------------------------
void OscSender::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void OscSender::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void OscSender::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    host->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    port->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    addOSCNumber->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    addOSCText->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    addOSCVector->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    addOSCTexture->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    for(size_t l=0;l<labels.size();l++){
        labels.at(l)->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || host->hitTest(_m-this->getPos()) || port->hitTest(_m-this->getPos())
                          || addOSCNumber->hitTest(_m-this->getPos()) || addOSCText->hitTest(_m-this->getPos()) || addOSCVector->hitTest(_m-this->getPos()) || addOSCTexture->hitTest(_m-this->getPos());

        for(size_t l=0;l<labels.size();l++){
            this->isOverGUI = labels.at(l)->hitTest(_m-this->getPos());
        }
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }
}

//--------------------------------------------------------------
void OscSender::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        host->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        port->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        addOSCNumber->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        addOSCText->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        addOSCVector->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        addOSCTexture->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

        for(size_t l=0;l<labels.size();l++){
            labels.at(l)->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        }
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
void OscSender::initInlets(){
    ofxXmlSettings XML;
    if(XML.loadFile(this->patchFile)){
        int totalObjects = XML.getNumTags("object");

        // Get object inlets config
        for(int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                if(XML.getValue("id", -1) == this->nId){
                    vector<int> tempTypes;
                    if(XML.pushTag("inlets")){
                        for (int t=0;t<XML.getNumTags("link");t++){
                            if(XML.pushTag("link",t)){
                                int type = XML.getValue("type", 0);
                                tempTypes.push_back(type);
                                XML.popTag();
                            }
                        }
                        XML.popTag();
                    }
                    if (XML.pushTag("vars")){
                        int totalOutlets = XML.getNumTags("var");
                        this->numInlets = totalOutlets-1;

                        int tempCounter = 0;
                        for (int t=0;t<totalOutlets;t++){
                            if(XML.pushTag("var",t)){
                                if(XML.getValue("name","") != "PORT"){
                                    if(tempTypes.at(tempCounter) == 0){ // float
                                        _inletParams[tempCounter] = new float();
                                        *(float *)&_inletParams[tempCounter] = 0.0f;
                                        ofxDatGuiTextInput* temp;
                                        temp = gui->addTextInput("N",XML.getValue("name",""));
                                        temp->setUseCustomMouse(true);
                                        temp->setTextUpperCase(false);
                                        labels.push_back(temp);
                                        osc_labels.push_back(XML.getValue("name",""));
                                        gui->setWidth(this->width);
                                        tempCounter++;
                                    }else if(tempTypes.at(tempCounter) == 1){ // string
                                        _inletParams[tempCounter] = new string();  // control
                                        *static_cast<string *>(_inletParams[tempCounter]) = "";
                                        ofxDatGuiTextInput* temp;
                                        temp = gui->addTextInput("T",XML.getValue("name",""));
                                        temp->setUseCustomMouse(true);
                                        temp->setTextUpperCase(false);
                                        labels.push_back(temp);
                                        osc_labels.push_back(XML.getValue("name",""));
                                        gui->setWidth(this->width);
                                        tempCounter++;
                                    }else if(tempTypes.at(tempCounter) == 2){ // vector<float>
                                        _inletParams[tempCounter] = new vector<float>();
                                        ofxDatGuiTextInput* temp;
                                        temp = gui->addTextInput("V",XML.getValue("name",""));
                                        temp->setUseCustomMouse(true);
                                        temp->setTextUpperCase(false);
                                        labels.push_back(temp);
                                        osc_labels.push_back(XML.getValue("name",""));
                                        gui->setWidth(this->width);
                                        tempCounter++;
                                    }else if(tempTypes.at(tempCounter) == 3){ // ofTexture
                                        _inletParams[tempCounter] = new ofTexture();
                                        ofxDatGuiTextInput* temp;
                                        temp = gui->addTextInput("I",XML.getValue("name",""));
                                        temp->setUseCustomMouse(true);
                                        temp->setTextUpperCase(false);
                                        labels.push_back(temp);
                                        osc_labels.push_back(XML.getValue("name",""));
                                        gui->setWidth(this->width);
                                        tempCounter++;
                                    }
                                }
                                XML.popTag();
                            }
                        }
                        XML.popTag();
                    }
                }
                XML.popTag();
            }
        }
    }

    this->initInletsState();
}

//--------------------------------------------------------------
void OscSender::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == addOSCNumber){
            _inletParams[this->numInlets] = new float();
            *(float *)&_inletParams[this->numInlets] = 0.0f;
            this->addInlet(VP_LINK_NUMERIC,"number");
            this->inletsConnected.push_back(false);

            ofxDatGuiTextInput* temp;
            temp = gui->addTextInput("N","/labelnumber");
            temp->setUseCustomMouse(true);
            temp->setTextUpperCase(false);
            labels.push_back(temp);
            osc_labels.push_back("/labelnumber");
            gui->setWidth(this->width);

            this->setCustomVar(0.0f,"/labelnumber");

            this->numInlets++;

            this->saveConfig(false,this->nId);
        }else if(e.target == addOSCText){
            _inletParams[this->numInlets] = new string();  // control
            *static_cast<string *>(_inletParams[this->numInlets]) = "";
            this->addInlet(VP_LINK_STRING,"text");
            this->inletsConnected.push_back(false);

            ofxDatGuiTextInput* temp;
            temp = gui->addTextInput("T","/labeltext");
            temp->setUseCustomMouse(true);
            temp->setTextUpperCase(false);
            labels.push_back(temp);
            osc_labels.push_back("/labeltext");
            gui->setWidth(this->width);

            this->setCustomVar(0.0f,"/labeltext");

            this->numInlets++;

            this->saveConfig(false,this->nId);
        }else if(e.target == addOSCVector){
            _inletParams[this->numInlets] = new vector<float>();
            this->addInlet(VP_LINK_ARRAY,"vector");
            this->inletsConnected.push_back(false);

            ofxDatGuiTextInput* temp;
            temp = gui->addTextInput("V","/labelvector");
            temp->setUseCustomMouse(true);
            temp->setTextUpperCase(false);
            labels.push_back(temp);
            osc_labels.push_back("/labelvector");
            gui->setWidth(this->width);

            this->setCustomVar(0.0f,"/labelvector");

            this->numInlets++;

            this->saveConfig(false,this->nId);
        }else if(e.target == addOSCTexture){
            _inletParams[this->numInlets] = new ofTexture();
            this->addInlet(VP_LINK_TEXTURE,"image");
            this->inletsConnected.push_back(false);

            ofxDatGuiTextInput* temp;
            temp = gui->addTextInput("I","/labelimage");
            temp->setUseCustomMouse(true);
            temp->setTextUpperCase(false);
            labels.push_back(temp);
            osc_labels.push_back("/labelimage");
            gui->setWidth(this->width);

            this->setCustomVar(0.0f,"/labelimage");

            this->numInlets++;

            this->saveConfig(false,this->nId);
        }
    }
}

//--------------------------------------------------------------
void OscSender::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == host){
            osc_host = e.text;
            filepath = osc_host;
            osc_sender.setup(osc_host.c_str(),osc_port);
        }else if(e.target == port){
            if(isInteger(e.text)){
                this->setCustomVar(static_cast<float>(ofToInt(e.text)),"PORT");
                osc_port = ofToInt(e.text);
                osc_sender.setup(osc_host.c_str(),osc_port);
            }else{
                port->setText(ofToString(osc_port));
            }
        }

        for(int i=0;i<labels.size();i++){
            if(e.target == labels.at(i)){
                this->substituteCustomVar(osc_labels.at(i),e.text);
                osc_labels.at(i) = e.text;

                this->saveConfig(false,this->nId);
            }
        }
    }
}

OBJECT_REGISTER( OscSender, "osc sender", OFXVP_OBJECT_CAT_COMMUNICATIONS)
