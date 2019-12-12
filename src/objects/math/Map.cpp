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

    inMin = 0;
    inMax = 1;
    outMin = 0;
    outMax = 1;

    this->initInletsState();

}

//--------------------------------------------------------------
void Map::newObject(){
    this->setName("map");
    this->addInlet(VP_LINK_NUMERIC,"value");
    this->addInlet(VP_LINK_NUMERIC,"in min");
    this->addInlet(VP_LINK_NUMERIC,"in max");
    this->addInlet(VP_LINK_NUMERIC,"out min");
    this->addInlet(VP_LINK_NUMERIC,"out max");
    this->addOutlet(VP_LINK_NUMERIC,"mapped value");

}

//--------------------------------------------------------------
void Map::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void Map::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){
    if(this->inletsConnected[0]){
      if(this->inletsConnected[1]){
          inMin = *(float *)&_inletParams[1];
      }
      if(this->inletsConnected[2]){
          inMax = *(float *)&_inletParams[2];
      }
      if(this->inletsConnected[3]){
          outMin = *(float *)&_inletParams[3];
      }
      if(this->inletsConnected[4]){
          outMax = *(float *)&_inletParams[4];
      }
      *(float *)&_outletParams[0] = ofMap(*(float *)&_inletParams[0],inMin, inMax, outMin, outMax,true);
    }else{
      *(float *)&_outletParams[0] = 0.0f;
    }
}

//--------------------------------------------------------------
void Map::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    font->draw(ofToString(*(float *)&_outletParams[0]),this->fontSize,this->width/2,this->headerHeight*2.3);
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void Map::removeObjectContent(bool removeFileFromData){

}
