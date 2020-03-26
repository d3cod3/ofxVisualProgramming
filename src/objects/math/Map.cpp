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

#include "Map.h"

//--------------------------------------------------------------
Map::Map() : PatchObject(){

    this->numInlets  = 5;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // value
    *(float *)&_inletParams[0] = 0.0f;
    _inletParams[1] = new float();  // in min
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();  // in max
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();  // out min
    *(float *)&_inletParams[3] = 0.0f;
    _inletParams[4] = new float();  // out max
    *(float *)&_inletParams[4] = 0.0f;

    _outletParams[0] = new float(); // mapped value
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    inMin = 0;
    inMax = 1;
    outMin = 0;
    outMax = 1;

    loaded              = false;

}

//--------------------------------------------------------------
void Map::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"value");
    this->addInlet(VP_LINK_NUMERIC,"in min");
    this->addInlet(VP_LINK_NUMERIC,"in max");
    this->addInlet(VP_LINK_NUMERIC,"out min");
    this->addInlet(VP_LINK_NUMERIC,"out max");
    this->addOutlet(VP_LINK_NUMERIC,"mapped value");

    this->setCustomVar(static_cast<float>(inMin),"IN_MIN");
    this->setCustomVar(static_cast<float>(inMax),"IN_MAX");
    this->setCustomVar(static_cast<float>(outMin),"OUT_MIN");
    this->setCustomVar(static_cast<float>(outMax),"OUT_MAX");

}

//--------------------------------------------------------------
void Map::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setWidth(this->width);
    gui->addBreak();
    gui->onTextInputEvent(this, &Map::onTextInputEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    inputMin = gui->addTextInput("IN MIN","0");
    inputMin->setUseCustomMouse(true);
    inputMin->setText(ofToString(this->getCustomVar("IN_MIN")));
    inputMax = gui->addTextInput("IN MAX","1");
    inputMax->setUseCustomMouse(true);
    inputMax->setText(ofToString(this->getCustomVar("IN_MAX")));
    outputMin = gui->addTextInput("OUT MIN","0");
    outputMin->setUseCustomMouse(true);
    outputMin->setText(ofToString(this->getCustomVar("OUT_MIN")));
    outputMax = gui->addTextInput("OUT MAX","1");
    outputMax->setUseCustomMouse(true);
    outputMax->setText(ofToString(this->getCustomVar("OUT_MAX")));

    inMin = this->getCustomVar("IN_MIN");
    inMax = this->getCustomVar("IN_MAX");
    outMin = this->getCustomVar("OUT_MIN");
    outMax = this->getCustomVar("OUT_MAX");

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);
}

//--------------------------------------------------------------
void Map::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    inputMin->update();
    inputMax->update();
    outputMin->update();
    outputMax->update();

    if(this->inletsConnected[0]){
      if(this->inletsConnected[1]){
          inMin = *(float *)&_inletParams[1];
          inputMin->setText(ofToString(inMin));
      }
      if(this->inletsConnected[2]){
          inMax = *(float *)&_inletParams[2];
          inputMax->setText(ofToString(inMax));
      }
      if(this->inletsConnected[3]){
          outMin = *(float *)&_inletParams[3];
          outputMin->setText(ofToString(outMin));
      }
      if(this->inletsConnected[4]){
          outMax = *(float *)&_inletParams[4];
          outputMax->setText(ofToString(outMax));
      }
      *(float *)&_outletParams[0] = ofMap(*(float *)&_inletParams[0],inMin, inMax, outMin, outMax,true);
    }else{
      *(float *)&_outletParams[0] = 0.0f;
    }

    if(!loaded){
        loaded = true;
        inputMin->setText(ofToString(this->getCustomVar("IN_MIN")));
        inputMax->setText(ofToString(this->getCustomVar("IN_MAX")));
        outputMin->setText(ofToString(this->getCustomVar("OUT_MIN")));
        outputMax->setText(ofToString(this->getCustomVar("OUT_MAX")));
    }

}

//--------------------------------------------------------------
void Map::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    font->draw(ofToString(*(float *)&_outletParams[0]),this->fontSize,this->width/2,this->headerHeight*2.3);
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void Map::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void Map::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    inputMin->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    inputMax->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    outputMin->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    outputMax->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || inputMin->hitTest(_m-this->getPos()) || inputMax->hitTest(_m-this->getPos()) || outputMin->hitTest(_m-this->getPos()) || outputMax->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }


}

//--------------------------------------------------------------
void Map::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        inputMin->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        inputMax->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        outputMin->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        outputMax->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void Map::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == inputMin){
            if(isInteger(e.text) || isFloat(e.text)){
                this->setCustomVar(static_cast<float>(ofToFloat(e.text)),"IN_MIN");
                inMin = ofToFloat(e.text);
            }
        }else if(e.target == inputMax){
            if(isInteger(e.text) || isFloat(e.text)){
                this->setCustomVar(static_cast<float>(ofToFloat(e.text)),"IN_MAX");
                inMax = ofToFloat(e.text);
            }
        }else if(e.target == outputMin){
            if(isInteger(e.text) || isFloat(e.text)){
                this->setCustomVar(static_cast<float>(ofToFloat(e.text)),"OUT_MIN");
                outMin = ofToFloat(e.text);
            }
        }else if(e.target == outputMax){
            if(isInteger(e.text) || isFloat(e.text)){
                this->setCustomVar(static_cast<float>(ofToFloat(e.text)),"OUT_MAX");
                outMax = ofToFloat(e.text);
            }
        }
    }
}

OBJECT_REGISTER( Map, "map", OFXVP_OBJECT_CAT_MATH)
