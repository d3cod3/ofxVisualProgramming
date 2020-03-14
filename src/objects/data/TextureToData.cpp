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

#include "TextureToData.h"

//--------------------------------------------------------------
TextureToData::TextureToData() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();  // texture

    _outletParams[0] = new vector<float>(); // data

    this->initInletsState();

    newConnection       = false;
    col                 = 0;

}

//--------------------------------------------------------------
void TextureToData::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_TEXTURE,"texture");
    this->addOutlet(VP_LINK_ARRAY,"data");
}

//--------------------------------------------------------------
void TextureToData::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void TextureToData::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){
    static_cast<vector<float> *>(_outletParams[0])->clear();
    if(this->inletsConnected[0]){
        if(!newConnection){
            newConnection = true;
            pix = new ofPixels();
            pix->allocate(static_cast<ofTexture *>(_inletParams[0])->getWidth(),static_cast<ofTexture *>(_inletParams[0])->getHeight(),OF_PIXELS_RGB);
            col = static_cast<int>(static_cast<ofTexture *>(_inletParams[0])->getWidth()/2);
        }
        static_cast<ofTexture *>(_inletParams[0])->readToPixels(*pix);
        for(int n=0; n<static_cast<ofTexture *>(_inletParams[0])->getHeight(); ++n){
            float sampleR = ofMap(pix->getColor(col, n).r, 0, 255, -0.5f, 0.5f);        // RED CHANNEL
            float sampleG = ofMap(pix->getColor(col, n).g, 0, 255, -0.5f, 0.5f);        // GREEN CHANNEL
            float sampleB = ofMap(pix->getColor(col, n).b, 0, 255, -0.5f, 0.5f);        // BLUE CHANNEL
            static_cast<vector<float> *>(_outletParams[0])->push_back((sampleR+sampleG+sampleB)/3.0f);
        }
    }else{
        newConnection       = false;
    }

}

//--------------------------------------------------------------
void TextureToData::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(static_cast<ofTexture *>(_inletParams[0])->getWidth()/static_cast<ofTexture *>(_inletParams[0])->getHeight() >= this->width/this->height){
            if(static_cast<ofTexture *>(_inletParams[0])->getWidth() > static_cast<ofTexture *>(_inletParams[0])->getHeight()){   // horizontal texture
                drawW           = this->width;
                drawH           = (this->width/static_cast<ofTexture *>(_inletParams[0])->getWidth())*static_cast<ofTexture *>(_inletParams[0])->getHeight();
                posX            = 0;
                posY            = (this->height-drawH)/2.0f;
            }else{ // vertical texture
                drawW           = (static_cast<ofTexture *>(_inletParams[0])->getWidth()*this->height)/static_cast<ofTexture *>(_inletParams[0])->getHeight();
                drawH           = this->height;
                posX            = (this->width-drawW)/2.0f;
                posY            = 0;
            }
        }else{ // always considered vertical texture
            drawW           = (static_cast<ofTexture *>(_inletParams[0])->getWidth()*this->height)/static_cast<ofTexture *>(_inletParams[0])->getHeight();
            drawH           = this->height;
            posX            = (this->width-drawW)/2.0f;
            posY            = 0;
        }
        static_cast<ofTexture *>(_inletParams[0])->draw(posX,posY,drawW,drawH);
    }
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void TextureToData::removeObjectContent(bool removeFileFromData){

}

OBJECT_REGISTER( TextureToData, "texture to data", OFXVP_OBJECT_CAT_DATA)
