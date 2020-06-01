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

#include "OpticalFlow.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
OpticalFlow::OpticalFlow() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 2;

    _inletParams[0] = new ofTexture();  // input

    _outletParams[0] = new ofTexture(); // output texture
    _outletParams[1] = new vector<float>(); // optical flow data

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    posX = posY = drawW = drawH = 0.0f;

    pix                 = new ofPixels();
    scaledPix           = new ofPixels();
    outputFBO           = new ofFbo();

    isFBOAllocated      = false;

}

//--------------------------------------------------------------
void OpticalFlow::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_TEXTURE,"input");
    this->addOutlet(VP_LINK_TEXTURE,"output");
    this->addOutlet(VP_LINK_ARRAY,"opticalFlowData");

    this->setCustomVar(static_cast<float>(0.0),"FB_USE_GAUSSIAN");
    this->setCustomVar(static_cast<float>(0.25),"FB_PYR_SCALE");
    this->setCustomVar(static_cast<float>(1.5),"FB_POLY_SIGMA");
    this->setCustomVar(static_cast<float>(4.0),"FB_LEVELS");
    this->setCustomVar(static_cast<float>(2.0),"FB_ITERATIONS");
    this->setCustomVar(static_cast<float>(7.0),"FB_POLY_N");
    this->setCustomVar(static_cast<float>(32.0),"FB_WIN_SIZE");

}

//--------------------------------------------------------------
void OpticalFlow::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    fbUseGaussian = gui->addToggle("GAUSS.",static_cast<int>(floor(this->getCustomVar("FB_USE_GAUSSIAN"))));
    fbUseGaussian->setUseCustomMouse(true);
    fbPyrScale = gui->addSlider("PSCALE",0.0,0.5);
    fbPyrScale->setUseCustomMouse(true);
    fbPyrScale->setValue(static_cast<double>(this->getCustomVar("FB_PYR_SCALE")));
    fbLevels = gui->addSlider("LEVELS",1,8);
    fbLevels->setUseCustomMouse(true);
    fbLevels->setValue(static_cast<double>(this->getCustomVar("FB_LEVELS")));
    fbWinSize = gui->addSlider("SIZE",16,64);
    fbWinSize->setUseCustomMouse(true);
    fbWinSize->setValue(static_cast<double>(this->getCustomVar("FB_WIN_SIZE")));
    fbIterations = gui->addSlider("ITER.",1,3);
    fbIterations->setUseCustomMouse(true);
    fbIterations->setValue(static_cast<double>(this->getCustomVar("FB_ITERATIONS")));
    fbPolyN = gui->addSlider("POLYN",5,10);
    fbPolyN->setUseCustomMouse(true);
    fbPolyN->setValue(static_cast<double>(this->getCustomVar("FB_POLY_N")));
    fbPolySigma = gui->addSlider("POLYS",1.1,2);
    fbPolySigma->setUseCustomMouse(true);
    fbPolySigma->setValue(static_cast<double>(this->getCustomVar("FB_POLY_SIGMA")));

    gui->onToggleEvent(this, &OpticalFlow::onToggleEvent);
    gui->onSliderEvent(this, &OpticalFlow::onSliderEvent);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

}

//--------------------------------------------------------------
void OpticalFlow::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    gui->update();
    header->update();
    if(!header->getIsCollapsed()){
        fbUseGaussian->update();
        fbPyrScale->update();
        fbLevels->update();
        fbWinSize->update();
        fbIterations->update();
        fbPolyN->update();
        fbPolySigma->update();
        fbUseGaussian->update();
    }

    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        
        if(!isFBOAllocated){
            isFBOAllocated = true;
            pix             = new ofPixels();
            scaledPix       = new ofPixels();
            scaledPix->allocate(320,static_cast<ofTexture *>(_inletParams[0])->getHeight()/static_cast<ofTexture *>(_inletParams[0])->getWidth()*320,OF_PIXELS_RGB);
            outputFBO->allocate(static_cast<ofTexture *>(_inletParams[0])->getWidth(),static_cast<ofTexture *>(_inletParams[0])->getHeight(),GL_RGB,1);
        }

        fb.setPyramidScale(fbPyrScale->getValue());
        fb.setNumLevels(static_cast<int>(floor(fbLevels->getValue())));
        fb.setWindowSize(static_cast<int>(floor(fbWinSize->getValue())));
        fb.setNumIterations(static_cast<int>(floor(fbIterations->getValue())));
        fb.setPolyN(static_cast<int>(floor(fbPolyN->getValue())));
        fb.setPolySigma(fbPolySigma->getValue());
        fb.setUseGaussian(fbUseGaussian->getChecked());

        static_cast<ofTexture *>(_inletParams[0])->readToPixels(*pix);

        pix->resizeTo(*scaledPix);

        fb.calcOpticalFlow(*scaledPix);

        if(outputFBO->isAllocated()){
            *static_cast<ofTexture *>(_outletParams[0]) = outputFBO->getTexture();

            static_cast<vector<float> *>(_outletParams[1])->clear();

            static_cast<vector<float> *>(_outletParams[1])->push_back(fb.getFlow().rows);
            static_cast<vector<float> *>(_outletParams[1])->push_back(fb.getFlow().cols);

            for(int y = 0; y < fb.getFlow().rows; y += 10) {
                for(int x = 0; x < fb.getFlow().cols; x += 10) {
                    static_cast<vector<float> *>(_outletParams[1])->push_back(x);
                    static_cast<vector<float> *>(_outletParams[1])->push_back(y);
                    static_cast<vector<float> *>(_outletParams[1])->push_back(fb.getFlowPosition(x, y).x);
                    static_cast<vector<float> *>(_outletParams[1])->push_back(fb.getFlowPosition(x, y).y);
                }
            }
        }

    }else{
        isFBOAllocated = false;
    }

}

//--------------------------------------------------------------
void OpticalFlow::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(this->inletsConnected[0] && outputFBO->isAllocated() && static_cast<ofTexture *>(_outletParams[0])->isAllocated()){

        outputFBO->begin();

        ofClear(0,0,0,255);

        ofSetColor(255);
        static_cast<ofTexture *>(_inletParams[0])->draw(0,0);

        ofSetColor(yellowPrint);
        fb.draw(0,0,static_cast<ofTexture *>(_outletParams[0])->getWidth(),static_cast<ofTexture *>(_outletParams[0])->getHeight());

        outputFBO->end();

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
void OpticalFlow::removeObjectContent(bool removeFileFromData){
    
}

//--------------------------------------------------------------
void OpticalFlow::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    fbUseGaussian->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    fbPyrScale->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    fbLevels->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    fbWinSize->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    fbIterations->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    fbPolyN->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    fbPolySigma->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    fbUseGaussian->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || fbUseGaussian->hitTest(_m-this->getPos()) || fbPyrScale->hitTest(_m-this->getPos()) || fbLevels->hitTest(_m-this->getPos())
                            || fbWinSize->hitTest(_m-this->getPos()) || fbIterations->hitTest(_m-this->getPos()) || fbPolyN->hitTest(_m-this->getPos())
                            || fbPolySigma->hitTest(_m-this->getPos()) || fbUseGaussian->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void OpticalFlow::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        fbUseGaussian->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        fbPyrScale->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        fbLevels->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        fbWinSize->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        fbIterations->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        fbPolyN->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        fbPolySigma->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        fbUseGaussian->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void OpticalFlow::onToggleEvent(ofxDatGuiToggleEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == fbUseGaussian){
            this->setCustomVar(static_cast<float>(e.checked),"FB_USE_GAUSSIAN");
        }
    }
}

//--------------------------------------------------------------
void OpticalFlow::onSliderEvent(ofxDatGuiSliderEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == fbPyrScale){
            this->setCustomVar(static_cast<float>(e.value),"FB_PYR_SCALE");
        }else if(e.target == fbLevels){
            this->setCustomVar(static_cast<float>(e.value),"FB_LEVELS");
        }else if(e.target == fbWinSize){
            this->setCustomVar(static_cast<float>(e.value),"FB_WIN_SIZE");
        }else if(e.target == fbIterations){
            this->setCustomVar(static_cast<float>(e.value),"FB_ITERATIONS");
        }else if(e.target == fbPolyN){
            this->setCustomVar(static_cast<float>(e.value),"FB_POLY_N");
        }else if(e.target == fbPolySigma){
            this->setCustomVar(static_cast<float>(e.value),"FB_POLY_SIGMA");
        }
    }

}

OBJECT_REGISTER( OpticalFlow, "optical flow", OFXVP_OBJECT_CAT_CV)

#endif