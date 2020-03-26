/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2020 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "PixelsToTexture.h"

//--------------------------------------------------------------
PixelsToTexture::PixelsToTexture() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new ofPixels();  // pixels

    _outletParams[0] = new ofTexture(); // texture

    this->initInletsState();


}

//--------------------------------------------------------------
void PixelsToTexture::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_PIXELS,"pixels");
    this->addOutlet(VP_LINK_TEXTURE,"texture");
}

//--------------------------------------------------------------
void PixelsToTexture::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void PixelsToTexture::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    if(this->inletsConnected[0]){
        static_cast<ofTexture *>(_outletParams[0])->loadData(*static_cast<ofPixels *>(_inletParams[0]));
    }

}

//--------------------------------------------------------------
void PixelsToTexture::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();

    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void PixelsToTexture::removeObjectContent(bool removeFileFromData){

}

OBJECT_REGISTER( PixelsToTexture, "pixels to texture", OFXVP_OBJECT_CAT_VIDEO)
