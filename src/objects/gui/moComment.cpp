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

    this->numInlets  = 1;
    this->numOutlets = 0;

    _inletParams[0] = new string();  // comment

    for(int i=0;i<this->numInlets;i++){
        this->inletsConnected.push_back(false);
    }

    paragraph = new ofxParagraph();
    textBuffer = new moTextBuffer();
    label = ofxSmartFont::add(MAIN_FONT, this->fontSize,"verdana");

    actualComment = "This project deals with the idea of integrate/amplify man-machine communication, offering a real-time flowchart based visual interface for high level creative coding. As live-coding scripting languages offer a high level coding environment, ofxVisualProgramming and the Mosaic Project as his parent layer container, aim at a high level visual-programming environment, with embedded multi scripting languages availability (Lua, Python, GLSL and BASH).";

    this->isBigGuiComment   = true;
    this->width             *= 2;
    this->height            *= 2;
}

//--------------------------------------------------------------
void moComment::newObject(){
    this->setName("comment");
    this->addInlet(VP_LINK_STRING,"comment");
}

//--------------------------------------------------------------
void moComment::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    paragraph->setColor(ofColor::fromHex(0x555555));
    paragraph->setAlignment(ofxParagraph::ALIGN_CENTER);
    paragraph->setFont(label);
    paragraph->setWidth(this->width-20);
    paragraph->setSpacing(this->fontSize*.7f);
    paragraph->setLeading(this->fontSize*.5f);
    paragraph->setBorderPadding(10);
    paragraph->setPosition(10,this->headerHeight+20);
}

//--------------------------------------------------------------
void moComment::updateObjectContent(map<int,PatchObject*> &patchObjects){

    if(this->inletsConnected[0]){
        actualComment = "";
        actualComment.append(*(string *)&_inletParams[0]);
    }else{
        actualComment = textBuffer->getText();
    }

    paragraph->setText(actualComment);

    this->box->height = paragraph->getHeight() + this->headerHeight+40;

}

//--------------------------------------------------------------
void moComment::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    paragraph->draw();
    if(this->isMouseOver && !this->inletsConnected[0]){
        ofSetColor(255,255,255,100);
        ofDrawRectangle(paragraph->getLastLetterPosition().x,paragraph->getLastLetterPosition().y,this->fontSize*.5f,font->getLineHeight()*10);
    }
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void moComment::removeObjectContent(){
    
}

//--------------------------------------------------------------
void moComment::keyPressedObjectContent(int key){
    // Add printable ASCII characters to text buffer
    if(!this->inletsConnected[0]){
        if (key < 127 && key > 31) {
            textBuffer->insert(key);
        }else if (key == 8) {
            textBuffer->backspace();
        }
    }
}
