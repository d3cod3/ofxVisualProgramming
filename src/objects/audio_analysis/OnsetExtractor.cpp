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

#include "OnsetExtractor.h"

//--------------------------------------------------------------
OnsetExtractor::OnsetExtractor() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new vector<float>();  // RAW Data

    _outletParams[0] = new float(); // Onset
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    bufferSize = 256;
    spectrumSize = (bufferSize/2) + 1;

    arrayPosition = bufferSize + spectrumSize + MELBANDS_BANDS_NUM + DCT_COEFF_NUM + HPCP_SIZE + TRISTIMULUS_BANDS_NUM + 8;
}

//--------------------------------------------------------------
void OnsetExtractor::newObject(){
    this->setName("onset extractor");
    this->addInlet(VP_LINK_ARRAY,"data");
    this->addOutlet(VP_LINK_NUMERIC);
}

//--------------------------------------------------------------
void OnsetExtractor::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){
        if (XML.pushTag("settings")){
            bufferSize = XML.getValue("buffer_size",0);
            spectrumSize = (bufferSize/2) + 1;
            XML.popTag();
        }
    }

}

//--------------------------------------------------------------
void OnsetExtractor::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){
  if(this->inletsConnected[0] && !static_cast<vector<float> *>(_inletParams[0])->empty()){
      *(float *)&_outletParams[0] = static_cast<vector<float> *>(_inletParams[0])->at(arrayPosition);
  }
}

//--------------------------------------------------------------
void OnsetExtractor::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(*(float *)&_outletParams[0] > 0){
        ofSetColor(250,250,5);
        ofDrawRectangle(0,0,this->width,this->height);
    }
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void OnsetExtractor::removeObjectContent(){

}
