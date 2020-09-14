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

#include "MFCCExtractor.h"

//--------------------------------------------------------------
MFCCExtractor::MFCCExtractor() : PatchObject("mfcc extractor"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new vector<float>();  // RAW Data

    _outletParams[0] = new vector<float>();  // MFCC Data

    this->initInletsState();

    bufferSize = MOSAIC_DEFAULT_BUFFER_SIZE;
    spectrumSize = (bufferSize/2) + 1;

    startPosition = bufferSize + spectrumSize + MELBANDS_BANDS_NUM;
    endPosition = bufferSize + spectrumSize + MELBANDS_BANDS_NUM + DCT_COEFF_NUM;

    isNewConnection   = false;
    isConnectionRight = false;
}

//--------------------------------------------------------------
void MFCCExtractor::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"data");

    this->addOutlet(VP_LINK_ARRAY,"melFrequencyCepstrumCoefficents");
}

//--------------------------------------------------------------
void MFCCExtractor::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){
        if (XML.pushTag("settings")){
            bufferSize = XML.getValue("buffer_size",0);
            spectrumSize = (bufferSize/2) + 1;
            startPosition = bufferSize + spectrumSize + MELBANDS_BANDS_NUM;
            endPosition = bufferSize + spectrumSize + MELBANDS_BANDS_NUM + DCT_COEFF_NUM;
            XML.popTag();
        }
    }

    // INIT FFT BUFFER
    for(int i=0;i<DCT_COEFF_NUM;i++){
        static_cast<vector<float> *>(_outletParams[0])->push_back(0.0f);
    }

}

//--------------------------------------------------------------
void MFCCExtractor::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

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
        for(int i=startPosition;i<endPosition;i++){
            static_cast<vector<float> *>(_outletParams[0])->at(index) = static_cast<vector<float> *>(_inletParams[0])->at(i);
            index++;
        }
    }else if(this->inletsConnected[0] && !isConnectionRight){
        ofLog(OF_LOG_ERROR,"%s --> This object can receive data from audio analyzer object ONLY! Just reconnect it right!",this->getName().c_str());
    }

}

//--------------------------------------------------------------
void MFCCExtractor::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void MFCCExtractor::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){

        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            drawObjectNodeConfig();



            ImGui::EndMenu();
        }

        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        // draw FFT
        ImGuiEx::PlotBands(_nodeCanvas.getNodeDrawList(), 0, ImGui::GetWindowSize().y - 26, static_cast<vector<float> *>(_outletParams[0]));

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void MFCCExtractor::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Extracts 13 data in a dynamic vector of MFCC (Mel Frequency Cepstral Coeﬃcients) from the audio analysis data vector.",
                "https://mosaic.d3cod3.org/reference.php?r=mfcc-extractor", scaleFactor);
}

//--------------------------------------------------------------
void MFCCExtractor::removeObjectContent(bool removeFileFromData){

}

OBJECT_REGISTER( MFCCExtractor , "mfcc extractor", OFXVP_OBJECT_CAT_AUDIOANALYSIS)

#endif
