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

#include "FftExtractor.h"

//--------------------------------------------------------------
FftExtractor::FftExtractor() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new vector<float>();  // RAW Data

    _outletParams[0] = new vector<float>();  // FFT Data

    this->initInletsState();
    
    bufferSize = MOSAIC_DEFAULT_BUFFER_SIZE;
    spectrumSize = (bufferSize/2) + 1;

    isNewConnection   = false;
    isConnectionRight = false;
}

//--------------------------------------------------------------
void FftExtractor::newObject(){
    this->setName("fft extractor");
    this->addInlet(VP_LINK_ARRAY,"data");
    this->addOutlet(VP_LINK_ARRAY,"fft");
}

//--------------------------------------------------------------
void FftExtractor::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){
        if (XML.pushTag("settings")){
            bufferSize = XML.getValue("buffer_size",0);
            spectrumSize = (bufferSize/2) + 1;
            XML.popTag();
        }
    }

    // INIT FFT BUFFER
    for(int i=0;i<(bufferSize/2)+1;i++){
        static_cast<vector<float> *>(_outletParams[0])->push_back(0.0f);
    }

}

//--------------------------------------------------------------
void FftExtractor::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    if(this->inletsConnected[0]){
        if(!isNewConnection){
            isNewConnection = true;
            for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
                if(patchObjects[it->first] != nullptr && it->first != this->getId() && !patchObjects[it->first]->getWillErase()){
                    for(int o=0;o<static_cast<int>(it->second->outPut.size());o++){
                        if(!it->second->outPut[o]->isDisabled && it->second->outPut[o]->toObjectID == this->getId()){
                            if(it->second->getName() == "audio analyzer" || it->second->getName() == "file to data"){
                                isConnectionRight = true;
                            }
                            break;
                        }
                    }
                }
            }
        }
    }else{
        isNewConnection = false;
        isConnectionRight = false;
    }

    if(this->inletsConnected[0] && !static_cast<vector<float> *>(_inletParams[0])->empty() && isConnectionRight){
        int index = 0;
        for(int i=bufferSize;i<bufferSize + spectrumSize;i++){
            static_cast<vector<float> *>(_outletParams[0])->at(index) = static_cast<vector<float> *>(_inletParams[0])->at(i);
            index++;
        }
    }else if(this->inletsConnected[0] && !isConnectionRight){
        ofLog(OF_LOG_ERROR,"%s --> This object can receive data from audio analyzer object ONLY! Just reconnect it right!",this->getName().c_str());
    }

}

//--------------------------------------------------------------
void FftExtractor::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    ofSetColor(255,220,110,120);
    ofNoFill();

    float bin_w = (float) this->width / spectrumSize;
    for (int i = 0; i < static_cast<int>(spectrumSize); i++){
        float bin_h = -1 * (static_cast<vector<float> *>(_outletParams[0])->at(i) * this->height);
        ofDrawLine(i*bin_w, this->height, i*bin_w, bin_h + this->height);
    }
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void FftExtractor::removeObjectContent(bool removeFileFromData){
    
}
