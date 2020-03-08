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

#include "OscReceiver.h"

//--------------------------------------------------------------
OscReceiver::OscReceiver() : PatchObject(){

    this->numInlets  = 0;
    this->numOutlets = 0;

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    osc_port            = 12345;
    local_ip            = "0.0.0.0";

    _tempImage          = new ofImage();

}

//--------------------------------------------------------------
void OscReceiver::newObject(){
    this->setName(this->objectName);

    this->setCustomVar(static_cast<float>(osc_port),"PORT");
}

//--------------------------------------------------------------
void OscReceiver::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onButtonEvent(this, &OscReceiver::onButtonEvent);
    gui->onTextInputEvent(this, &OscReceiver::onTextInputEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    local_ip = getLocalIP();
    localIP = gui->addLabel(local_ip);

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

    initOutlets();

    osc_port = static_cast<int>(floor(this->getCustomVar("PORT")));
    osc_receiver.setup(osc_port);
}

//--------------------------------------------------------------
void OscReceiver::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    port->update();
    addOSCNumber->update();
    addOSCText->update();
    addOSCVector->update();
    addOSCTexture->update();

    for(size_t l=0;l<labels.size();l++){
        labels.at(l)->update();
    }

    if(osc_labels.size() > 0){
        while(osc_receiver.hasWaitingMessages()){
            ofxOscMessage m;
            osc_receiver.getNextMessage(m);

            for(int i=0;i<this->getNumOutlets();i++){
                if(m.getAddress() == osc_labels.at(i)){
                    if(this->getOutletType(i) == VP_LINK_NUMERIC){
                        if(m.getNumArgs() == 1){
                            if(m.getArgType(0) == OFXOSC_TYPE_INT32 || m.getArgType(0) == OFXOSC_TYPE_FLOAT){
                                *(float *)&_outletParams[i] = static_cast<float>(m.getArgAsFloat(0));
                            }
                        }
                    }else if(this->getOutletType(i) == VP_LINK_STRING){
                        if(m.getNumArgs() == 1 && m.getArgType(0) == OFXOSC_TYPE_STRING){
                            *static_cast<string *>(_outletParams[i]) = m.getArgAsString(0);
                        }
                    }else if(this->getOutletType(i) == VP_LINK_ARRAY){
                        static_cast<vector<float> *>(_outletParams[i])->clear();
                        for(size_t a = 0; a < m.getNumArgs(); a++){
                            if(m.getArgType(a) == OFXOSC_TYPE_INT32 || m.getArgType(a) == OFXOSC_TYPE_FLOAT){
                                static_cast<vector<float> *>(_outletParams[i])->push_back(m.getArgAsFloat(a));
                            }
                        }
                    }else if(this->getOutletType(i) == VP_LINK_TEXTURE){
                        // note: the size of the image depends greatly on your network buffer sizes,
                        // if an image is too big the message won't come through
                        if(m.getNumArgs() == 4 && m.getArgType(2) == OFXOSC_TYPE_INT32 && m.getArgType(3) == OFXOSC_TYPE_BLOB){
                            _tempImage->load(m.getArgAsBlob(3));
                            if(m.getArgAsInt32(2) == OF_IMAGE_GRAYSCALE){
                                static_cast<ofTexture *>(_outletParams[i])->allocate(m.getArgAsInt32(0),m.getArgAsInt32(1),GL_LUMINANCE);
                                static_cast<ofTexture *>(_outletParams[i])->loadData(_tempImage->getPixels(),GL_LUMINANCE);
                            }else if(m.getArgAsInt32(2) == OF_IMAGE_COLOR){
                                static_cast<ofTexture *>(_outletParams[i])->allocate(m.getArgAsInt32(0),m.getArgAsInt32(1),GL_RGB);
                                static_cast<ofTexture *>(_outletParams[i])->loadData(_tempImage->getPixels(),GL_RGB);
                            }else if(m.getArgAsInt32(2) == OF_IMAGE_COLOR_ALPHA){
                                static_cast<ofTexture *>(_outletParams[i])->allocate(m.getArgAsInt32(0),m.getArgAsInt32(1),GL_RGBA);
                                static_cast<ofTexture *>(_outletParams[i])->loadData(_tempImage->getPixels(),GL_RGB);
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
}

//--------------------------------------------------------------
void OscReceiver::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void OscReceiver::removeObjectContent(bool removeFileFromData){
    if(osc_receiver.isListening()){
        osc_receiver.stop();
    }
}

//--------------------------------------------------------------
void OscReceiver::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    port->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    addOSCNumber->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    addOSCText->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    addOSCVector->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    addOSCTexture->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    for(size_t l=0;l<labels.size();l++){
        labels.at(l)->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || port->hitTest(_m-this->getPos())
                          || addOSCNumber->hitTest(_m-this->getPos()) || addOSCText->hitTest(_m-this->getPos()) || addOSCVector->hitTest(_m-this->getPos()) || addOSCTexture->hitTest(_m-this->getPos());

        for(size_t l=0;l<labels.size();l++){
            this->isOverGUI = labels.at(l)->hitTest(_m-this->getPos());
        }
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }
}

//--------------------------------------------------------------
void OscReceiver::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        port->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        addOSCNumber->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        addOSCText->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        addOSCVector->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        addOSCTexture->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void OscReceiver::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == addOSCNumber){
            _outletParams[this->numOutlets] = new float();
            *(float *)&_outletParams[this->numOutlets] = 0.0f;
            this->addOutlet(VP_LINK_NUMERIC,"number");

            ofxDatGuiTextInput* temp;
            temp = gui->addTextInput("N","/labelnumber");
            temp->setUseCustomMouse(true);
            temp->setTextUpperCase(false);
            labels.push_back(temp);
            osc_labels.push_back("/labelnumber");
            gui->setWidth(this->width);

            this->setCustomVar(0.0f,"/labelnumber");

            this->numOutlets++;

            resetOutlets();
        }else if(e.target == addOSCText){
            _outletParams[this->numOutlets] = new string();  // control
            *static_cast<string *>(_outletParams[this->numOutlets]) = "";
            this->addOutlet(VP_LINK_STRING,"text");

            ofxDatGuiTextInput* temp;
            temp = gui->addTextInput("T","/labeltext");
            temp->setUseCustomMouse(true);
            temp->setTextUpperCase(false);
            labels.push_back(temp);
            osc_labels.push_back("/labeltext");
            gui->setWidth(this->width);

            this->setCustomVar(0.0f,"/labeltext");

            this->numOutlets++;

            resetOutlets();
        }else if(e.target == addOSCVector){
            _outletParams[this->numOutlets] = new vector<float>();
            this->addOutlet(VP_LINK_ARRAY,"numbersArray");

            ofxDatGuiTextInput* temp;
            temp = gui->addTextInput("V","/labelvector");
            temp->setUseCustomMouse(true);
            temp->setTextUpperCase(false);
            labels.push_back(temp);
            osc_labels.push_back("/labelvector");
            gui->setWidth(this->width);

            this->setCustomVar(0.0f,"/labelvector");

            this->numOutlets++;

            resetOutlets();
        }else if(e.target == addOSCTexture){
            _outletParams[this->numOutlets] = new ofTexture();
            this->addOutlet(VP_LINK_TEXTURE,"texture");

            ofxDatGuiTextInput* temp;
            temp = gui->addTextInput("I","/labelimage");
            temp->setUseCustomMouse(true);
            temp->setTextUpperCase(false);
            labels.push_back(temp);
            osc_labels.push_back("/labelimage");
            gui->setWidth(this->width);

            this->setCustomVar(0.0f,"/labelimage");

            this->numOutlets++;

            resetOutlets();
        }
    }
}

//--------------------------------------------------------------
void OscReceiver::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == port){
            if(isInteger(e.text)){
                this->setCustomVar(static_cast<float>(ofToInt(e.text)),"PORT");
                osc_port = ofToInt(e.text);
                osc_receiver.setup(osc_port);
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

//--------------------------------------------------------------
void OscReceiver::initOutlets(){
    ofxXmlSettings XML;
    if(XML.loadFile(this->patchFile)){
        int totalObjects = XML.getNumTags("object");

        // Load object outlet config
        for(int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                if(XML.getValue("id", -1) == this->nId){
                    vector<int> tempTypes;
                    if(XML.pushTag("outlets")){
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
                        this->numOutlets = totalOutlets-1;

                        int tempCounter = 0;
                        for (int t=0;t<totalOutlets;t++){
                            if(XML.pushTag("var",t)){
                                if(XML.getValue("name","") != "PORT"){
                                    if(tempTypes.at(tempCounter) == 0){
                                        _outletParams[tempCounter] = new float();
                                        *(float *)&_outletParams[tempCounter] = 0.0f;
                                        ofxDatGuiTextInput* temp;
                                        temp = gui->addTextInput("N",XML.getValue("name",""));
                                        temp->setUseCustomMouse(true);
                                        temp->setTextUpperCase(false);
                                        labels.push_back(temp);
                                        osc_labels.push_back(XML.getValue("name",""));
                                        gui->setWidth(this->width);
                                        tempCounter++;
                                    }else if(tempTypes.at(tempCounter) == 1){
                                        _outletParams[tempCounter] = new string();
                                        *static_cast<string *>(_outletParams[tempCounter]) = "";
                                        ofxDatGuiTextInput* temp;
                                        temp = gui->addTextInput("T",XML.getValue("name",""));
                                        temp->setUseCustomMouse(true);
                                        temp->setTextUpperCase(false);
                                        labels.push_back(temp);
                                        osc_labels.push_back(XML.getValue("name",""));
                                        gui->setWidth(this->width);
                                        tempCounter++;
                                    }else if(tempTypes.at(tempCounter) == 2){
                                        _outletParams[tempCounter] = new vector<float>();
                                        ofxDatGuiTextInput* temp;
                                        temp = gui->addTextInput("V",XML.getValue("name",""));
                                        temp->setUseCustomMouse(true);
                                        temp->setTextUpperCase(false);
                                        labels.push_back(temp);
                                        osc_labels.push_back(XML.getValue("name",""));
                                        gui->setWidth(this->width);
                                        tempCounter++;
                                    }else if(tempTypes.at(tempCounter) == 3){
                                        _outletParams[tempCounter] = new ofTexture();
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
}

//--------------------------------------------------------------
void OscReceiver::resetOutlets(){

    this->height      = OBJECT_HEIGHT;
    if(this->numOutlets > 6){
        this->height          *= 2;
    }
    if(this->numOutlets > 12){
        this->height          *= 2;
    }
    this->box->setHeight(this->height);
    gui->setPosition(0,this->height - header->getHeight());

    ofxXmlSettings XML;
    if(XML.loadFile(this->patchFile)){
        int totalObjects = XML.getNumTags("object");

        // Save new object outlet config
        for(int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                if(XML.getValue("id", -1) == this->nId){
                    // Dynamic reloading outlets
                    XML.removeTag("outlets");
                    int newOutlets = XML.addTag("outlets");
                    if(XML.pushTag("outlets",newOutlets)){
                        for(int j=0;j<static_cast<int>(this->outlets.size());j++){
                            int newLink = XML.addTag("link");
                            if(XML.pushTag("link",newLink)){
                                XML.setValue("type",this->outlets.at(j));
                                XML.popTag();
                            }
                        }
                        XML.popTag();
                    }
                }
                XML.popTag();
            }
        }

        XML.saveFile();
    }

    this->saveConfig(false,this->nId);
}

//--------------------------------------------------------------
string OscReceiver::getLocalIP(){
    string IP = "";

    string cmd = "";
    FILE *execFile;
#ifdef TARGET_LINUX
    cmd = "ifconfig | grep -w 'inet' | grep -v 127.0.0.1 | awk '{print $2}'";
    execFile = popen(cmd.c_str(), "r");
#elif defined(TARGET_OSX)
    cmd = "ifconfig | grep -w 'inet' | grep -v 127.0.0.1 | awk '{print $2}'";
    execFile = popen(cmd.c_str(), "r");
#elif defined(TARGET_WIN32)
    cmd = "ipconfig | findstr v4";
    execFile = _popen(cmd.c_str(), "r");
#endif

    if (execFile){
        char buffer[128];
        while(!feof(execFile)){
            if(fgets(buffer, sizeof(buffer), execFile) != nullptr){
                char *s = buffer;
                std::string tempstr(s);
                #ifdef TARGET_WIN32
                    size_t found = tempstr.find_last_of(":");
                    IP.append(tempstr.substr(found+2));
                #else
                    IP.append(tempstr);
                #endif
            }
        }

#ifdef TARGET_LINUX
        pclose(execFile);
#elif defined(TARGET_OSX)
        pclose(execFile);
#elif defined(TARGET_WIN32)
        _pclose(execFile);
#endif

    }

    return IP;
}

OBJECT_REGISTER( OscReceiver, "osc receiver", OFXVP_OBJECT_CAT_COMMUNICATIONS);
