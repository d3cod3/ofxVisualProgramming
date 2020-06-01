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

#include "ChromaKey.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
ChromaKey::ChromaKey() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();  // input
    _inletParams[1] = new ofTexture();  // mask
    _outletParams[0] = new ofTexture(); // output

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    posX = posY = drawW = drawH = 0.0f;

    isInputConnected    = false;

}

//--------------------------------------------------------------
void ChromaKey::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_TEXTURE,"input");
    this->addInlet(VP_LINK_TEXTURE,"mask");
    this->addOutlet(VP_LINK_TEXTURE,"output");

    this->setCustomVar(static_cast<float>(0.0),"RED");
    this->setCustomVar(static_cast<float>(0.0),"GREEN");
    this->setCustomVar(static_cast<float>(0.0),"BLUE");
    this->setCustomVar(static_cast<float>(0.37),"STRENGTH");
    this->setCustomVar(static_cast<float>(0.5),"MSTRENGTH");
    this->setCustomVar(static_cast<float>(0.39),"SPILL");
    this->setCustomVar(static_cast<float>(0.244),"BLUR");
    this->setCustomVar(static_cast<float>(0.28),"OFFSET");
}

//--------------------------------------------------------------
void ChromaKey::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    bgColor = gui->addTextInput("BG","000000");
    bgColor->setUseCustomMouse(true);
    bgColor->setInputType(ofxDatGuiInputType::COLORPICKER);

    redValue = gui->addSlider("RED",0.0,1.0,0);
    redValue->setUseCustomMouse(true);
    redValue->setValue(this->getCustomVar("RED"));
    greenValue = gui->addSlider("GREEN",0.0,1.0,0);
    greenValue->setUseCustomMouse(true);
    greenValue->setValue(this->getCustomVar("GREEN"));
    blueValue = gui->addSlider("BLUE",0.0,1.0,0);
    blueValue->setUseCustomMouse(true);
    blueValue->setValue(this->getCustomVar("BLUE"));
    gui->addBreak();

    updateBGColor();

    thresholdValue = gui->addSlider("STRENGTH",0.0,1.0);
    thresholdValue->setUseCustomMouse(true);
    thresholdValue->setValue(this->getCustomVar("STRENGTH"));
    maskStrengthValue = gui->addSlider("MSTRENGTH",0.0,1.0);
    maskStrengthValue->setUseCustomMouse(true);
    maskStrengthValue->setValue(this->getCustomVar("MSTRENGTH"));
    spillStrengthValue = gui->addSlider("SPILL",0.0,1.0);
    spillStrengthValue->setUseCustomMouse(true);
    spillStrengthValue->setValue(this->getCustomVar("SPILL"));
    blurValue = gui->addSlider("BLUR",0.0,1.0);
    blurValue->setUseCustomMouse(true);
    blurValue->setValue(this->getCustomVar("BLUR"));
    offsetValue = gui->addSlider("OFFSET",0.0,1.0);
    offsetValue->setUseCustomMouse(true);
    offsetValue->setValue(this->getCustomVar("OFFSET"));

    gui->onButtonEvent(this, &ChromaKey::onButtonEvent);
    gui->onSliderEvent(this, &ChromaKey::onSliderEvent);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

}

//--------------------------------------------------------------
void ChromaKey::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    gui->update();
    header->update();
    if(!header->getIsCollapsed()){
        bgColor->update();
        redValue->update();
        greenValue->update();
        blueValue->update();
        thresholdValue->update();
        maskStrengthValue->update();
        spillStrengthValue->update();
        blurValue->update();
        offsetValue->update();
    }

    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(!isInputConnected){
            isInputConnected = true;
            chromakey = new ofxChromaKeyShader(static_cast<ofTexture *>(_inletParams[0])->getWidth(),static_cast<ofTexture *>(_inletParams[0])->getHeight());
            updateBGColor();
            updateChromaVars();
        }

        if(this->inletsConnected[1] && static_cast<ofTexture *>(_inletParams[1])->isAllocated()){
            chromakey->updateChromakeyMask(*static_cast<ofTexture *>(_inletParams[0]));
            *static_cast<ofTexture *>(_outletParams[0]) = chromakey->getFinalMask(*static_cast<ofTexture *>(_inletParams[1]));
        }else{
            *static_cast<ofTexture *>(_outletParams[0]) = *static_cast<ofTexture *>(_inletParams[0]);
        }
    }else{
        isInputConnected = false;
    }
    
}

//--------------------------------------------------------------
void ChromaKey::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
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
void ChromaKey::removeObjectContent(bool removeFileFromData){
    
}

//--------------------------------------------------------------
void ChromaKey::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    bgColor->setCustomMousePos(static_cast<float>(_m.x - this->getPos().x),static_cast<float>(_m.y - this->getPos().y));
    thresholdValue->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    redValue->setCustomMousePos(static_cast<float>(_m.x - this->getPos().x),static_cast<float>(_m.y - this->getPos().y));
    greenValue->setCustomMousePos(static_cast<float>(_m.x - this->getPos().x),static_cast<float>(_m.y - this->getPos().y));
    blueValue->setCustomMousePos(static_cast<float>(_m.x - this->getPos().x),static_cast<float>(_m.y - this->getPos().y));
    maskStrengthValue->setCustomMousePos(static_cast<float>(_m.x - this->getPos().x),static_cast<float>(_m.y - this->getPos().y));
    spillStrengthValue->setCustomMousePos(static_cast<float>(_m.x - this->getPos().x),static_cast<float>(_m.y - this->getPos().y));
    blurValue->setCustomMousePos(static_cast<float>(_m.x - this->getPos().x),static_cast<float>(_m.y - this->getPos().y));
    offsetValue->setCustomMousePos(static_cast<float>(_m.x - this->getPos().x),static_cast<float>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || bgColor->hitTest(_m-this->getPos())
                            || maskStrengthValue->hitTest(_m-this->getPos()) || spillStrengthValue->hitTest(_m-this->getPos())
                            || blurValue->hitTest(_m-this->getPos()) || offsetValue->hitTest(_m-this->getPos())
                            || thresholdValue->hitTest(_m-this->getPos()) || redValue->hitTest(_m-this->getPos())
                            || greenValue->hitTest(_m-this->getPos()) || blueValue->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void ChromaKey::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        bgColor->setCustomMousePos(static_cast<float>(_m.x - this->getPos().x),static_cast<float>(_m.y - this->getPos().y));
        thresholdValue->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        redValue->setCustomMousePos(static_cast<float>(_m.x - this->getPos().x),static_cast<float>(_m.y - this->getPos().y));
        greenValue->setCustomMousePos(static_cast<float>(_m.x - this->getPos().x),static_cast<float>(_m.y - this->getPos().y));
        blueValue->setCustomMousePos(static_cast<float>(_m.x - this->getPos().x),static_cast<float>(_m.y - this->getPos().y));
        maskStrengthValue->setCustomMousePos(static_cast<float>(_m.x - this->getPos().x),static_cast<float>(_m.y - this->getPos().y));
        spillStrengthValue->setCustomMousePos(static_cast<float>(_m.x - this->getPos().x),static_cast<float>(_m.y - this->getPos().y));
        blurValue->setCustomMousePos(static_cast<float>(_m.x - this->getPos().x),static_cast<float>(_m.y - this->getPos().y));
        offsetValue->setCustomMousePos(static_cast<float>(_m.x - this->getPos().x),static_cast<float>(_m.y - this->getPos().y));

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
void ChromaKey::updateBGColor(){
    std::stringstream ss;
    ofColor temp = ofColor(redValue->getValue()*255,greenValue->getValue()*255,blueValue->getValue()*255);
    ss << std::hex << temp.getHex();
    std::string res ( ss.str() );
    while(res.size() < 6) res+="0";
    bgColor->setText(ofToUpper(res));
    bgColor->setBackgroundColor(temp);
    double a = 1 - ( 0.299 * temp.r + 0.587 * temp.g + 0.114 * temp.b)/255;
    bgColor->setTextInactiveColor(a < 0.5 ? ofColor::black : ofColor::white);

    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        chromakey->setbgColor(temp);
    }
}

//--------------------------------------------------------------
void ChromaKey::updateChromaVars(){
    chromakey->setbaseMaskStrength(static_cast<float>(thresholdValue->getValue()));
    chromakey->setchromaMaskStrength(static_cast<float>(maskStrengthValue->getValue()));
    chromakey->setgreenSpillStrength(static_cast<float>(spillStrengthValue->getValue()));
    chromakey->setblurValue(static_cast<float>(blurValue->getValue())*4096.0f);
    chromakey->setmultiplyFilterHueOffset(static_cast<float>(offsetValue->getValue()));
}

//--------------------------------------------------------------
void ChromaKey::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){

    }

}

//--------------------------------------------------------------
void ChromaKey::onSliderEvent(ofxDatGuiSliderEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == redValue || e.target == greenValue || e.target == blueValue){
            updateBGColor();
            if(e.target == redValue){
                this->setCustomVar(static_cast<float>(e.value),"RED");
            }else if(e.target == greenValue){
                this->setCustomVar(static_cast<float>(e.value),"GREEN");
            }else if(e.target == blueValue){
                this->setCustomVar(static_cast<float>(e.value),"BLUE");
            }
        }else if(e.target == thresholdValue){
            if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated() && this->inletsConnected[1] && static_cast<ofTexture *>(_inletParams[1])->isAllocated()){
                chromakey->setbaseMaskStrength(e.value);
                this->setCustomVar(static_cast<float>(e.value),"STRENGTH");
            }
        }else if(e.target == maskStrengthValue){
            if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated() && this->inletsConnected[1] && static_cast<ofTexture *>(_inletParams[1])->isAllocated()){
                chromakey->setchromaMaskStrength(e.value);
                this->setCustomVar(static_cast<float>(e.value),"MSTRENGTH");
            }
        }else if(e.target == spillStrengthValue){
            if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated() && this->inletsConnected[1] && static_cast<ofTexture *>(_inletParams[1])->isAllocated()){
                chromakey->setgreenSpillStrength(e.value);
                this->setCustomVar(static_cast<float>(e.value),"SPILL");
            }
        }else if(e.target == blurValue){
            if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated() && this->inletsConnected[1] && static_cast<ofTexture *>(_inletParams[1])->isAllocated()){
                chromakey->setblurValue(e.value*4096.0f);
                this->setCustomVar(static_cast<float>(e.value),"BLUR");
            }
        }else if(e.target == offsetValue){
            if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated() && this->inletsConnected[1] && static_cast<ofTexture *>(_inletParams[1])->isAllocated()){
                chromakey->setmultiplyFilterHueOffset(e.value);
                this->setCustomVar(static_cast<float>(e.value),"OFFSET");
            }
        }
    }

}

OBJECT_REGISTER( ChromaKey, "chroma key", OFXVP_OBJECT_CAT_CV)

#endif