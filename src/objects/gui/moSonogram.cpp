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

#include "moSonogram.h"

//--------------------------------------------------------------
moSonogram::moSonogram() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 0;

    _inletParams[0] = new vector<float>();  // fft

    this->initInletsState();

    this->width             *= 2;
    this->height            *= 2;

    sonogram                = new ofFbo();

    isGUIObject             = true;
    this->isOverGUI         = false;

    timePosition            = 0;
    resetTime               = ofGetElapsedTimeMillis();
    wait                    = 40;

    resizeQuad.set(this->width-20,this->height-20,20,20);

}

//--------------------------------------------------------------
void moSonogram::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_ARRAY,"fft");

    this->setCustomVar(static_cast<float>(this->width),"WIDTH");
    this->setCustomVar(static_cast<float>(this->height),"HEIGHT");
}

//--------------------------------------------------------------
void moSonogram::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    this->width = static_cast<int>(floor(this->getCustomVar("WIDTH")));
    this->height = static_cast<int>(floor(this->getCustomVar("HEIGHT")));

    box->setWidth(this->width);
    box->setHeight(this->height);

    headerBox->setWidth(this->width);

    resizeQuad.set(this->width-20,this->height-20,20,20);

    sonogram->allocate(this->width,this->height,GL_RGBA);

    sonogram->begin();
    ofClear(0,0,0,255);
    sonogram->end();

    colors.push_back(ofColor(0,0,0,240));           // BLACK
    colors.push_back(ofColor(0,0,180,240));         // BLUE
    colors.push_back(ofColor(55,0,110,240));        // PURPLE
    colors.push_back(ofColor(255,0,0,240));         // RED
    colors.push_back(ofColor(255,255,0,240));       // YELLOW
    colors.push_back(ofColor(255,255,255,240));     // WHITE

}

//--------------------------------------------------------------
void moSonogram::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){
    if(this->inletsConnected[0]){
        if(ofGetElapsedTimeMillis()-resetTime > wait){
            resetTime = ofGetElapsedTimeMillis();
            if(timePosition >= this->width){
                timePosition = 0;
            }else{
                timePosition++;
            }
        }
    }
}

//--------------------------------------------------------------
void moSonogram::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofEnableAlphaBlending();
    if(this->inletsConnected[0]){
        sonogram->begin();
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
        ofPushView();
        ofPushStyle();
        ofPushMatrix();
        ofSetColor(0,0,0,0);
        ofDrawRectangle(0,0,this->width,this->height);
        for(size_t s=0;s<static_cast<size_t>(static_cast<vector<float> *>(_inletParams[0])->size());s++){
            float valueDB = static_cast<vector<float> *>(_inletParams[0])->at(s);
            int colorIndex = static_cast<int>(floor(ofMap(valueDB,0,1,0,colors.size()-1,true)));
            ofSetColor(0);
            ofDrawRectangle(timePosition,ofMap(s,0,static_cast<int>(static_cast<vector<float> *>(_inletParams[0])->size()),this->height,0,true),1,1);
            //ofSetColor(ofMap(valueDB,0.4,0.8,0,255,true),ofMap(valueDB,0.8,1,0,255,true),ofMap(valueDB,0.0,0.4,0,255,true),255*static_cast<vector<float> *>(_inletParams[0])->at(s));
            ofSetColor(colors.at(colorIndex),255*valueDB);
            ofDrawRectangle(timePosition,ofMap(s,0,static_cast<int>(static_cast<vector<float> *>(_inletParams[0])->size()),this->height,0,true),1,1);
        }
        ofPopMatrix();
        ofPopStyle();
        ofPopView();
        glPopAttrib();
        sonogram->end();
    }
    ofSetColor(255,255,255);
    sonogram->draw(0,0);

    ofSetColor(255,255,255,70);
    if(this->isOverGUI){
        ofFill();
    }else{
        ofNoFill();
    }
    ofDrawRectangle(resizeQuad);
    ofFill();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void moSonogram::removeObjectContent(bool removeFileFromData){
    
}

//--------------------------------------------------------------
void moSonogram::mouseMovedObjectContent(ofVec3f _m){
    this->isOverGUI = resizeQuad.inside(_m-this->getPos());
}

//--------------------------------------------------------------
void moSonogram::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        this->width =  _m.x - this->getPos().x;
        this->height =  _m.y - this->getPos().y;

        box->setWidth(_m.x - this->getPos().x);
        box->setHeight(_m.y - this->getPos().y);

        headerBox->setWidth(_m.x - this->getPos().x);

        resizeQuad.set(this->width-20,this->height-20,20,20);

        this->setCustomVar(static_cast<float>(this->width),"WIDTH");
        this->setCustomVar(static_cast<float>(this->height),"HEIGHT");

        sonogram->allocate(this->width,this->height,GL_RGBA);
        sonogram->begin();
        ofClear(0,0,0,255);
        sonogram->end();
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

OBJECT_REGISTER( moSonogram, "sonogram", OFXVP_OBJECT_CAT_GUI)

#endif
