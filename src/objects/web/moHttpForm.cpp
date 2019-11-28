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

#include "moHttpForm.h"

//--------------------------------------------------------------
moHttpForm::moHttpForm() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 0;

    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;
    _inletParams[1] = new string();  // url
    *static_cast<string *>(_inletParams[1]) = "";

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    fm      = new HttpFormManager();
    img     = new ofImage();

    form_url        = "localhost";

    bang            = false;
    isBangFinished  = true;

}

//--------------------------------------------------------------
void moHttpForm::newObject(){
    this->setName("http form");
    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_STRING,"url");
}

//--------------------------------------------------------------
void moHttpForm::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    if(filepath == "none"){
        filepath = form_url;
    }

    // GUI
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onButtonEvent(this, &moHttpForm::onButtonEvent);
    gui->onTextInputEvent(this, &moHttpForm::onTextInputEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    gui->addBreak();

    addFormField = gui->addButton("ADD FIELD");
    addFormField->setUseCustomMouse(true);
    addFormField->setLabelAlignment(ofxDatGuiAlignment::CENTER);
    addFormImage = gui->addButton("ADD FILE");
    addFormImage->setUseCustomMouse(true);
    addFormImage->setLabelAlignment(ofxDatGuiAlignment::CENTER);

    gui->addBreak();

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    initInlets();

    form_url = filepath;
    fm->setVerbose(true);
    ofAddListener(fm->formResponseEvent, this, &moHttpForm::newResponse);
}

//--------------------------------------------------------------
void moHttpForm::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    addFormField->update();
    addFormImage->update();

    for(size_t l=0;l<labels.size();l++){
        labels.at(l)->update();
    }

    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] < 1.0){
            bang = false;
            isBangFinished = true;
        }else{
            bang = true;
        }
    }

    if(this->inletsConnected[1]){
        form_url = *static_cast<string *>(_inletParams[1]);
        filepath = form_url;
    }

    if(bang && isBangFinished){
        isBangFinished = false;

        // Send http form
        HttpForm f = HttpForm(form_url);

        for(int i=2;i<this->getNumInlets();i++){
            if(this->inletsConnected[i]){
                if(this->getInletType(i) == VP_LINK_STRING){
                    f.addFormField(form_labels.at(i-2),*static_cast<string *>(_inletParams[i]));
                }else if(this->getInletType(i) == VP_LINK_TEXTURE && static_cast<ofTexture *>(_inletParams[i])->isAllocated()){
                    //saveTempImageFile(i);
                    f.addFile(form_labels.at(i-2).c_str(), ofToDataPath("temp/tempimage.jpg",true), "image/jpg");
                }
            }
        }

        fm->submitForm(f,false);
    }

}

//--------------------------------------------------------------
void moHttpForm::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void moHttpForm::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void moHttpForm::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    addFormField->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    addFormImage->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    for(size_t l=0;l<labels.size();l++){
        labels.at(l)->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) ||
                          addFormField->hitTest(_m-this->getPos()) || addFormImage->hitTest(_m-this->getPos());

        for(size_t l=0;l<labels.size();l++){
            this->isOverGUI = labels.at(l)->hitTest(_m-this->getPos());
        }
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }
}

//--------------------------------------------------------------
void moHttpForm::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        addFormField->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        addFormImage->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

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
void moHttpForm::initInlets(){
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
                        this->numInlets = totalOutlets+2;

                        int tempCounter = 2;
                        for (int t=0;t<totalOutlets;t++){
                            if(XML.pushTag("var",t)){
                                if(tempTypes.at(tempCounter) == 1){ // string
                                    _inletParams[tempCounter] = new string();
                                    *static_cast<string *>(_inletParams[tempCounter]) = "";
                                    ofxDatGuiTextInput* temp;
                                    temp = gui->addTextInput("FIELD",XML.getValue("name",""));
                                    temp->setUseCustomMouse(true);
                                    labels.push_back(temp);
                                    form_labels.push_back(XML.getValue("name",""));
                                    gui->setWidth(this->width);
                                    tempCounter++;
                                }else if(tempTypes.at(tempCounter) == 3){ // ofTexture
                                    _inletParams[tempCounter] = new ofTexture();
                                    ofxDatGuiTextInput* temp;
                                    temp = gui->addTextInput("IMAGE",XML.getValue("name",""));
                                    temp->setUseCustomMouse(true);
                                    labels.push_back(temp);
                                    form_labels.push_back(XML.getValue("name",""));
                                    gui->setWidth(this->width);
                                    tempCounter++;
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
void moHttpForm::saveTempImageFile(int id){
    static_cast<ofTexture *>(_inletParams[id])->readToPixels(capturePix);

    img = new ofImage();
    // OF_IMAGE_GRAYSCALE, OF_IMAGE_COLOR, or OF_IMAGE_COLOR_ALPHA
    // GL_LUMINANCE, GL_RGB, GL_RGBA
    if(static_cast<ofTexture *>(_inletParams[id])->getTextureData().glInternalFormat == GL_LUMINANCE){
        img->allocate(static_cast<ofTexture *>(_inletParams[id])->getWidth(),static_cast<ofTexture *>(_inletParams[id])->getHeight(),OF_IMAGE_GRAYSCALE);
    }else if(static_cast<ofTexture *>(_inletParams[id])->getTextureData().glInternalFormat == GL_RGB){
        img->allocate(static_cast<ofTexture *>(_inletParams[id])->getWidth(),static_cast<ofTexture *>(_inletParams[id])->getHeight(),OF_IMAGE_COLOR);
    }else if(static_cast<ofTexture *>(_inletParams[id])->getTextureData().glInternalFormat == GL_RGBA){
        img->allocate(static_cast<ofTexture *>(_inletParams[id])->getWidth(),static_cast<ofTexture *>(_inletParams[id])->getHeight(),OF_IMAGE_COLOR_ALPHA);
    }

    img->setFromPixels(capturePix);
    img->save(ofToDataPath("temp/tempimage.png",true));
}

//--------------------------------------------------------------
void moHttpForm::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == addFormField){
            _inletParams[this->numInlets] = new string();
            *static_cast<string *>(_inletParams[this->numInlets]) = "";
            this->addInlet(VP_LINK_STRING,"field");
            this->inletsConnected.push_back(false);

            ofxDatGuiTextInput* temp;
            temp = gui->addTextInput("FIELD","fieldname"+ofToString(this->numInlets));
            temp->setUseCustomMouse(true);
            labels.push_back(temp);
            form_labels.push_back("fieldname"+ofToString(this->numInlets));
            gui->setWidth(this->width);

            this->setCustomVar(0.0f,"fieldname"+ofToString(this->numInlets));

            this->numInlets++;

            this->saveConfig(false,this->nId);
        }else if(e.target == addFormImage){
            _inletParams[this->numInlets] = new ofTexture();
            this->addInlet(VP_LINK_TEXTURE,"image");
            this->inletsConnected.push_back(false);

            ofxDatGuiTextInput* temp;
            temp = gui->addTextInput("IMAGE","imagename"+ofToString(this->numInlets));
            temp->setUseCustomMouse(true);
            labels.push_back(temp);
            form_labels.push_back("imagename"+ofToString(this->numInlets));
            gui->setWidth(this->width);

            this->setCustomVar(0.0f,"imagename"+ofToString(this->numInlets));

            this->numInlets++;

            this->saveConfig(false,this->nId);
        }
    }
}

//--------------------------------------------------------------
void moHttpForm::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(!header->getIsCollapsed()){
        for(int i=0;i<labels.size();i++){
            if(e.target == labels.at(i)){
                this->substituteCustomVar(form_labels.at(i),e.text);
                form_labels.at(i) = e.text;

                this->saveConfig(false,this->nId);
            }
        }
    }
}

//--------------------------------------------------------------
void moHttpForm::newResponse(HttpFormResponse &response){
    ofLog(OF_LOG_NOTICE,"form '%s' returned : %s\n", response.url.c_str(), response.ok ? "OK" : "KO" );
}
