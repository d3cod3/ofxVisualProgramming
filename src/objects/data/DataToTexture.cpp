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

    pix             = new ofPixels();
    scaledPix       = new ofPixels();

}

//--------------------------------------------------------------
void DataToTexture::newObject(){
    this->setName("data to texture");
    this->addInlet(VP_LINK_ARRAY,"red");
    this->addInlet(VP_LINK_ARRAY,"green");
    this->addInlet(VP_LINK_ARRAY,"blue");
    this->addOutlet(VP_LINK_TEXTURE,"output");
}

//--------------------------------------------------------------
void DataToTexture::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    pix->allocate(320,240,OF_PIXELS_RGB);
    scaledPix->allocate(1920,1080,OF_PIXELS_RGB);
    static_cast<ofTexture *>(_outletParams[0])->allocate(1920,1080,GL_RGB);
}

//--------------------------------------------------------------
void DataToTexture::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){

    if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
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

//--------------------------------------------------------------
void DataToTexture::drawObjectContent(ofxFontStash *font){
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
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void DataToTexture::removeObjectContent(){

}
