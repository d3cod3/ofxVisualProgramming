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

#include "BPMExtractor.h"

//--------------------------------------------------------------
BPMExtractor::BPMExtractor() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 2;

    _inletParams[0] = new vector<float>();  // RAW Data

    _outletParams[0] = new float(); // BPM
    *(float *)&_outletParams[0] = 0.0f;
    _outletParams[1] = new float(); // MS
    *(float *)&_outletParams[1] = 0.0f;

    this->initInletsState();

    bufferSize = 1024;
    spectrumSize = (bufferSize/2) + 1;

    arrayPosition = bufferSize + spectrumSize + MELBANDS_BANDS_NUM + DCT_COEFF_NUM + HPCP_SIZE + TRISTIMULUS_BANDS_NUM + 9;
}

//--------------------------------------------------------------
void BPMExtractor::newObject(){
    this->setName("bpm extractor");
    this->addInlet(VP_LINK_ARRAY,"data");
    this->addOutlet(VP_LINK_NUMERIC,"bpm");
    this->addOutlet(VP_LINK_NUMERIC,"millis");
}

//--------------------------------------------------------------
void BPMExtractor::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){
        if (XML.pushTag("settings")){
            bufferSize = XML.getValue("buffer_size",0);
            spectrumSize = (bufferSize/2) + 1;
            arrayPosition = bufferSize + spectrumSize + MELBANDS_BANDS_NUM + DCT_COEFF_NUM + HPCP_SIZE + TRISTIMULUS_BANDS_NUM + 9;
            XML.popTag();
        }
    }

    bpmPlot = new ofxHistoryPlot(NULL, "BPM", this->width, false);
    bpmPlot->setRange(0,200);
    bpmPlot->setColor(ofColor(255,255,255));
    bpmPlot->setRespectBorders(true);
    bpmPlot->setShowNumericalInfo(false);
    bpmPlot->setDrawTitle(false);
    bpmPlot->setLineWidth(1);
    bpmPlot->setBackgroundColor(ofColor(50,50,50,220));

}

//--------------------------------------------------------------
void BPMExtractor::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){
  if(this->inletsConnected[0] && !static_cast<vector<float> *>(_inletParams[0])->empty()){
      *(float *)&_outletParams[0] = static_cast<vector<float> *>(_inletParams[0])->at(arrayPosition);
      bpmPlot->update(*(float *)&_outletParams[0]);
      *(float *)&_outletParams[1] = 60000.0f / *(float *)&_outletParams[0];
  }


}

//--------------------------------------------------------------
void BPMExtractor::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    bpmPlot->draw(0,0,this->width,this->height);
    font->draw(ofToString(*(float *)&_outletParams[0]),this->fontSize,this->width/2,this->headerHeight*2.3);
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void BPMExtractor::removeObjectContent(){

}
