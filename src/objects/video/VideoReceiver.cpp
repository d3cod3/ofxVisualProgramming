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

#if defined(TARGET_WIN32)
    // Unavailable on windows.
#elif !defined(OFXVP_BUILD_WITH_MINIMAL_OBJECTS)

#include "VideoReceiver.h"

//--------------------------------------------------------------
VideoReceiver::VideoReceiver() : PatchObject(){

    this->numInlets  = 0;
    this->numOutlets = 1;

    _outletParams[0] = new ofTexture(); // input

    this->initInletsState();

    posX = posY = drawW = drawH = 0.0f;

    needToGrab = true;

}

//--------------------------------------------------------------
void VideoReceiver::newObject(){
    PatchObject::setName( this->objectName );
    this->addOutlet(VP_LINK_TEXTURE,"textureReceived");
}

//--------------------------------------------------------------
void VideoReceiver::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){


}

//--------------------------------------------------------------
void VideoReceiver::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    ndiGrabber.update();

    if(needToGrab && ndiGrabber.getTexture().isAllocated()){
        needToGrab = false;
        static_cast<ofTexture *>(_outletParams[0])->allocate(ndiGrabber.getTexture().getWidth(), ndiGrabber.getTexture().getHeight(), GL_RGB );
    }

    if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
        *static_cast<ofTexture *>(_outletParams[0]) = ndiGrabber.getTexture();
    }
}

//--------------------------------------------------------------
void VideoReceiver::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofSetCircleResolution(50);
    ofEnableAlphaBlending();

    if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
        if(static_cast<ofTexture *>(_outletParams[0])->getWidth() >= static_cast<ofTexture *>(_outletParams[0])->getHeight()){   // horizontal texture
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
        static_cast<ofTexture *>(_outletParams[0])->draw(posX,posY,drawW,drawH);
    }

}

//--------------------------------------------------------------
void VideoReceiver::removeObjectContent(bool removeFileFromData){

}

OBJECT_REGISTER( VideoReceiver, "video receiver", OFXVP_OBJECT_CAT_VIDEO)

#endif
