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

#include "VideoTimelapse.h"

//--------------------------------------------------------------
VideoTimelapse::VideoTimelapse() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();  // input

    _inletParams[1] = new float();  // delay frames
    *(float *)&_inletParams[1] = 25.0f;

    _outletParams[0] = new ofTexture(); // output

    this->initInletsState();

    videoBuffer = new circularTextureBuffer();
    pix         = new ofPixels();
    kuro        = new ofImage();

    nDelayFrames    = 25;
    capturedFrame   = 0;
    delayFrame      = 0;

    resetTime       = ofGetElapsedTimeMillis();
    wait            = 1000/static_cast<int>(ofGetFrameRate());

}

//--------------------------------------------------------------
void VideoTimelapse::newObject(){
    this->setName("video timedelay");
    this->addInlet(VP_LINK_TEXTURE,"input");
    this->addInlet(VP_LINK_NUMERIC,"delay");
    this->addOutlet(VP_LINK_TEXTURE,"timeDelayedOutput");

    this->setCustomVar(static_cast<float>(nDelayFrames),"DELAY_FRAMES");
}

//--------------------------------------------------------------
void VideoTimelapse::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setWidth(this->width);
    gui->onTextInputEvent(this, &VideoTimelapse::onTextInputEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    guiDelayMS = gui->addTextInput("Frames","25");
    guiDelayMS->setUseCustomMouse(true);
    guiDelayMS->setText(ofToString(this->getCustomVar("DELAY_FRAMES")));

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    nDelayFrames = this->getCustomVar("DELAY_FRAMES");
    videoBuffer->setup(nDelayFrames);

    // load kuro
    kuro->load("images/kuro.jpg");

}

//--------------------------------------------------------------
void VideoTimelapse::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    guiDelayMS->update();
    
    if(this->inletsConnected[0]){
        if(ofGetElapsedTimeMillis()-resetTime > wait){
            resetTime       = ofGetElapsedTimeMillis();

            ofImage rgbaImage;
            rgbaImage.allocate(static_cast<ofTexture *>(_inletParams[0])->getWidth(), static_cast<ofTexture *>(_inletParams[0])->getHeight(),OF_IMAGE_COLOR_ALPHA);
            static_cast<ofTexture *>(_inletParams[0])->readToPixels(*pix);
            rgbaImage.setFromPixels(*pix);
            videoBuffer->pushTexture(rgbaImage.getTexture());

            if(capturedFrame >= nDelayFrames){
                if(delayFrame < nDelayFrames-1){
                    delayFrame++;
                }else{
                    delayFrame = 0;
                }
            }else{
                capturedFrame++;
            }
        }
        if(capturedFrame >= nDelayFrames){
            *static_cast<ofTexture *>(_outletParams[0]) = videoBuffer->getDelayedtexture(delayFrame);
        }else{
            *static_cast<ofTexture *>(_outletParams[0]) = kuro->getTexture();
        }
    }else{
        *static_cast<ofTexture *>(_outletParams[0]) = kuro->getTexture();
    }

    if(this->inletsConnected[1]){
        if(nDelayFrames != static_cast<int>(floor(*(float *)&_inletParams[1]))){
            nDelayFrames = static_cast<int>(floor(*(float *)&_inletParams[1]));
            guiDelayMS->setText(ofToString(nDelayFrames));

            capturedFrame   = 0;
            delayFrame      = 0;

            resetTime       = ofGetElapsedTimeMillis();
            wait            = 1000/static_cast<int>(ofGetFrameRate());

            videoBuffer->setup(nDelayFrames);
        }
    }
    
}

//--------------------------------------------------------------
void VideoTimelapse::drawObjectContent(ofxFontStash *font){
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
        static_cast<ofTexture *>(_outletParams[0])->draw(posX,posY,drawW,drawH);
    }
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void VideoTimelapse::removeObjectContent(bool removeFileFromData){
    
}

//--------------------------------------------------------------
void VideoTimelapse::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    guiDelayMS->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || guiDelayMS->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }
}

//--------------------------------------------------------------
void VideoTimelapse::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        guiDelayMS->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void VideoTimelapse::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == guiDelayMS){
            if(isInteger(e.text) || isFloat(e.text)){
                this->setCustomVar(static_cast<float>(ofToInt(e.text)),"DELAY_FRAMES");
                nDelayFrames    = ofToInt(e.text);
                capturedFrame   = 0;
                delayFrame      = 0;

                resetTime       = ofGetElapsedTimeMillis();
                wait            = 1000/static_cast<int>(ofGetFrameRate());

                videoBuffer->setup(nDelayFrames);
            }else{
                guiDelayMS->setText(ofToString(nDelayFrames));
            }

        }
    }
}
