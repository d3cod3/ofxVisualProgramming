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

#include "moComment.h"

//--------------------------------------------------------------
moComment::moComment() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;
    _inletParams[1] = new string();  // comment
    *static_cast<string *>(_inletParams[1]) = "";

    _outletParams[0] = new string(); // output string
    *static_cast<string *>(_outletParams[0]) = "";

    this->initInletsState();

    paragraph = new ofxParagraph();
    textBuffer = new moTextBuffer();
    label = ofxSmartFont::add(MAIN_FONT, this->fontSize,"verdana");

    actualComment   = "This project deals with the idea of integrate/amplify human-machine communication, offering a real-time flowchart based visual interface for high level creative coding. As live-coding scripting languages offer a high level coding environment, ofxVisualProgramming and the Mosaic Project as his parent layer container, aim at a high level visual-programming environment, with embedded multi scripting languages availability (Lua, Python, GLSL and BASH).";
    bang            = false;

    this->isBigGuiComment   = true;
    this->width             *= 2;
}

//--------------------------------------------------------------
void moComment::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_STRING,"comment");
    this->addOutlet(VP_LINK_STRING,"text");
}

//--------------------------------------------------------------
void moComment::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    paragraph->setColor(ofColor::white);
    paragraph->setAlignment(ofxParagraph::ALIGN_CENTER);
    paragraph->setFont(label);
    paragraph->setWidth(this->width-20);
    paragraph->setSpacing(this->fontSize*.7f);
    paragraph->setLeading(this->fontSize*.5f);
    paragraph->setBorderPadding(10);
    paragraph->setPosition(10,this->headerHeight+20);

    loadCommentSetting();
}

//--------------------------------------------------------------
void moComment::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] < 1.0){
            bang = false;
        }else{
            bang = true;
        }
    }

    if(this->inletsConnected[1]){
        actualComment = "";
        actualComment.append(*static_cast<string *>(_inletParams[1]));
    }else{
        actualComment = textBuffer->getText();
    }

    paragraph->setText(actualComment);

    if(bang){
        *static_cast<string *>(_outletParams[0]) = actualComment;
    }else{
        *static_cast<string *>(_outletParams[0]) = "";
    }

    if(this->isRetina){
        this->box->height = paragraph->getHeight() + this->headerHeight+120;
    }else{
        this->box->height = paragraph->getHeight() + this->headerHeight+60;
    }


}

//--------------------------------------------------------------
void moComment::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    paragraph->draw();
    if(this->isMouseOver && !this->inletsConnected[1]){
        ofSetColor(255,255,255,100);
        if(this->isRetina){
           ofDrawRectangle(paragraph->getLastLetterPosition().x,paragraph->getLastLetterPosition().y,this->fontSize*.5f,font->getLineHeight()*20);
        }else{
            ofDrawRectangle(paragraph->getLastLetterPosition().x,paragraph->getLastLetterPosition().y,this->fontSize*.5f,font->getLineHeight()*10);
        }
    }
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void moComment::removeObjectContent(bool removeFileFromData){
    
}

//--------------------------------------------------------------
void moComment::mousePressedObjectContent(ofVec3f _m){
    bang = true;
}

//--------------------------------------------------------------
void moComment::mouseReleasedObjectContent(ofVec3f _m){
    bang = false;
}

//--------------------------------------------------------------
void moComment::keyPressedObjectContent(int key){
    //ofLog(OF_LOG_NOTICE,"%i",key);
    if(!this->inletsConnected[1]){
        if (key < 127 && key > 31) {
            textBuffer->insert(key);
        }else if (key == 8) { // BACKSPACE
            if(textBuffer->getText().size() > 0){
                textBuffer->backspace();
            }
        }else if(key == 13){ // ENTER

        }
        saveCommentSetting();
    }
}

//--------------------------------------------------------------
void moComment::keyReleasedObjectContent(int key){

}

//--------------------------------------------------------------
void moComment::loadCommentSetting(){
    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){
        int totalObjects = XML.getNumTags("object");
        for(int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                if(XML.getValue("id", -1) == this->nId){
                    textBuffer->setText(XML.getValue("text","none"));
                }
                XML.popTag();
            }
        }
    }
}

//--------------------------------------------------------------
void moComment::saveCommentSetting(){
    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){
        int totalObjects = XML.getNumTags("object");
        for(int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                if(XML.getValue("id", -1) == this->nId){
                    XML.setValue("text",textBuffer->getText());
                }
                XML.popTag();
            }
        }
        XML.saveFile();
    }
}

OBJECT_REGISTER( moComment, "comment", OFXVP_OBJECT_CAT_GUI)
