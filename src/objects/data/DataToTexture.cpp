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

#include "DataToTexture.h"

//--------------------------------------------------------------
DataToTexture::DataToTexture() : PatchObject(){

    this->numInlets  = 3;
    this->numOutlets = 1;

    _inletParams[0] = new vector<float>();  // red
    _inletParams[1] = new vector<float>();  // green
    _inletParams[2] = new vector<float>();  // blu

    _outletParams[0] = new ofTexture(); // texture output

    this->initInletsState();

    pix                 = new ofPixels();
    scaledPix           = new ofPixels();

    this->output_width  = STANDARD_TEXTURE_WIDTH;
    this->output_height = STANDARD_TEXTURE_HEIGHT;

    temp_width          = this->output_width;
    temp_height         = this->output_height;

    isGUIObject         = true;
    this->isOverGUI     = true;

    loaded              = false;
    needReset           = false;

    this->setCustomVar(static_cast<float>(this->output_width),"OUTPUT_WIDTH");
    this->setCustomVar(static_cast<float>(this->output_height),"OUTPUT_HEIGHT");

}

//--------------------------------------------------------------
void DataToTexture::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_ARRAY,"red");
    this->addInlet(VP_LINK_ARRAY,"green");
    this->addInlet(VP_LINK_ARRAY,"blue");
    this->addOutlet(VP_LINK_TEXTURE,"output");
}

//--------------------------------------------------------------
void DataToTexture::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    // GUI
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onButtonEvent(this, &DataToTexture::onButtonEvent);
    gui->onTextInputEvent(this, &DataToTexture::onTextInputEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    guiTexWidth = gui->addTextInput("WIDTH",ofToString(this->output_width));
    guiTexWidth->setUseCustomMouse(true);
    guiTexHeight = gui->addTextInput("HEIGHT",ofToString(this->output_height));
    guiTexHeight->setUseCustomMouse(true);
    applyButton = gui->addButton("APPLY");
    applyButton->setUseCustomMouse(true);
    applyButton->setLabelAlignment(ofxDatGuiAlignment::CENTER);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);


    pix->allocate(320,240,OF_PIXELS_RGB);
}

//--------------------------------------------------------------
void DataToTexture::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    guiTexWidth->update();
    guiTexHeight->update();
    applyButton->update();

    if(needReset){
        needReset = false;
        resetResolution();
    }

    if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
        if(this->inletsConnected[0] || this->inletsConnected[1] || this->inletsConnected[2]){
            for(int s=0;s<pix->size();s++){
                int posR = 0;
                int sampleR = 0;
                int posG = 0;
                int sampleG = 0;
                int posB = 0;
                int sampleB = 0;
                // RED
                if(this->inletsConnected[0] && static_cast<int>(static_cast<vector<float> *>(_inletParams[0])->size()) > 0){
                    posR = static_cast<int>(floor(ofMap(s,0,pix->size(),0,static_cast<int>(static_cast<vector<float> *>(_inletParams[0])->size()))));
                    sampleR = static_cast<int>(floor(ofMap(static_cast<vector<float> *>(_inletParams[0])->at(posR), -0.5f, 0.5f, 0, 255)));
                }
                // GREEN
                if(this->inletsConnected[1] && static_cast<int>(static_cast<vector<float> *>(_inletParams[1])->size()) > 0){
                    posG = static_cast<int>(floor(ofMap(s,0,pix->size(),0,static_cast<int>(static_cast<vector<float> *>(_inletParams[1])->size()))));
                    sampleG = static_cast<int>(floor(ofMap(static_cast<vector<float> *>(_inletParams[1])->at(posG), -0.5f, 0.5f, 0, 255)));
                }
                // BLUE
                if(this->inletsConnected[2] && static_cast<int>(static_cast<vector<float> *>(_inletParams[2])->size()) > 0){
                    posB = static_cast<int>(floor(ofMap(s,0,pix->size(),0,static_cast<int>(static_cast<vector<float> *>(_inletParams[2])->size()))));
                    sampleB = static_cast<int>(floor(ofMap(static_cast<vector<float> *>(_inletParams[2])->at(posB), -0.5f, 0.5f, 0, 255)));
                }
                ofColor c(sampleR,sampleG,sampleB);
                int x = s % pix->getWidth();
                int y = static_cast<int>(ceil(s / pix->getWidth()));
                if(x >= 0 && x <= pix->getWidth() && y >= 0 && y <= pix->getHeight()){
                    pix->setColor(x,y,c);
                }
            }
            pix->resizeTo(*scaledPix);
            static_cast<ofTexture *>(_outletParams[0])->loadData(*scaledPix);
        }
    }

    if(!loaded){
        loaded = true;
        this->output_width = static_cast<int>(floor(this->getCustomVar("OUTPUT_WIDTH")));
        this->output_height = static_cast<int>(floor(this->getCustomVar("OUTPUT_HEIGHT")));
        temp_width      = this->output_width;
        temp_height     = this->output_height;

        guiTexWidth->setText(ofToString(this->getCustomVar("OUTPUT_WIDTH")));
        guiTexHeight->setText(ofToString(this->getCustomVar("OUTPUT_HEIGHT")));
        scaledPix->allocate(this->output_width,this->output_height,OF_PIXELS_RGB);

        static_cast<ofTexture *>(_outletParams[0])->allocate(this->output_width,this->output_height,GL_RGB);
    }

}

//--------------------------------------------------------------
void DataToTexture::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
        if(static_cast<ofTexture *>(_outletParams[0])->getWidth()/static_cast<ofTexture *>(_outletParams[0])->getHeight() >= this->width/this->height){
            if(static_cast<ofTexture *>(_outletParams[0])->getWidth() > static_cast<ofTexture *>(_outletParams[0])->getHeight()){   // horizontal texture
                drawW           = this->width;
                drawH           = (this->width/static_cast<ofTexture *>(_outletParams[0])->getWidth())*static_cast<ofTexture *>(_outletParams[0])->getHeight();
                posX            = 0;
                posY            = (this->height-drawH)/2.0f;
            }else{ // vertical texture
                drawW           = (static_cast<ofTexture *>(_outletParams[0])->getWidth()*this->height)/static_cast<ofTexture *>(_outletParams[0])->getHeight();
                drawH           = this->height;
                posX            = (this->width-drawW)/2.0f;
                posY            = 0;
            }
        }else{ // always considered vertical texture
            drawW           = (static_cast<ofTexture *>(_outletParams[0])->getWidth()*this->height)/static_cast<ofTexture *>(_outletParams[0])->getHeight();
            drawH           = this->height;
            posX            = (this->width-drawW)/2.0f;
            posY            = 0;
        }
        if(this->inletsConnected[0] || this->inletsConnected[1] || this->inletsConnected[2]){
            static_cast<ofTexture *>(_outletParams[0])->draw(posX,posY,drawW,drawH);
        }

    }
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void DataToTexture::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void DataToTexture::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    guiTexWidth->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    guiTexHeight->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    applyButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || guiTexWidth->hitTest(_m-this->getPos()) || guiTexHeight->hitTest(_m-this->getPos()) || applyButton->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void DataToTexture::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        guiTexWidth->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        guiTexHeight->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        applyButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void DataToTexture::resetResolution(){

    if(this->output_width != temp_width || this->output_height != temp_height){
        this->output_width = temp_width;
        this->output_height = temp_height;

        scaledPix = new ofPixels();
        scaledPix->allocate(this->output_width,this->output_height,OF_PIXELS_RGB);
        _outletParams[0] = new ofTexture();
        static_cast<ofTexture *>(_outletParams[0])->allocate(this->output_width,this->output_height,GL_RGB);


        if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
            this->setCustomVar(static_cast<float>(this->output_width),"OUTPUT_WIDTH");
            this->setCustomVar(static_cast<float>(this->output_height),"OUTPUT_HEIGHT");
            this->saveConfig(false,this->nId);

            ofLog(OF_LOG_NOTICE,"%s: RESOLUTION CHANGED TO %ix%i",this->name.c_str(),this->output_width,this->output_height);
        }
    }

}

//--------------------------------------------------------------
void DataToTexture::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if (e.target == applyButton){
            needReset = true;
        }
    }
}

//--------------------------------------------------------------
void DataToTexture::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(!header->getIsCollapsed()){
        int tempInValue = ofToInt(e.text);
        if(e.target == guiTexWidth){
            if(tempInValue <= OUTPUT_TEX_MAX_WIDTH){
                temp_width = tempInValue;
            }else{
                temp_width = this->output_width;
            }
        }else if(e.target == guiTexHeight){
            if(tempInValue <= OUTPUT_TEX_MAX_HEIGHT){
                temp_height = tempInValue;
            }else{
                temp_height = this->output_height;
            }
        }
    }
}

OBJECT_REGISTER( DataToTexture, "data to texture", OFXVP_OBJECT_CAT_DATA)

#endif