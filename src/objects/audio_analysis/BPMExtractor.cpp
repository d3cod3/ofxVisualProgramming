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

#include "BPMExtractor.h"

//--------------------------------------------------------------
BPMExtractor::BPMExtractor() : PatchObject("bpm extractor"){

    this->numInlets  = 1;
    this->numOutlets = 3;

    _inletParams[0] = new vector<float>();  // RAW Data

    _outletParams[0] = new float(); // beat
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0.0f;
    _outletParams[1] = new float(); // BPM
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[1]) = 0.0f;
    _outletParams[2] = new float(); // MS
    *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[3]) = 0.0f;

    this->initInletsState();

    isNewConnection     = false;
    isConnectionRight   = false;

    this->height        *= 0.7;

}

//--------------------------------------------------------------
void BPMExtractor::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"data");

    this->addOutlet(VP_LINK_NUMERIC,"beat");
    this->addOutlet(VP_LINK_NUMERIC,"bpm");
    this->addOutlet(VP_LINK_NUMERIC,"millis");
}

//--------------------------------------------------------------
void BPMExtractor::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    ofxXmlSettings XML;

#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
    if (XML.loadFile(patchFile)){
#else
    if (XML.load(patchFile)){
#endif
        if (XML.pushTag("settings")){
            bufferSize = XML.getValue("buffer_size",0);
            spectrumSize = (bufferSize/2) + 1;
            arrayPosition = bufferSize + spectrumSize + MEL_SCALE_CRITICAL_BANDS + 1;
            XML.popTag();
        }
    }

}

//--------------------------------------------------------------
void BPMExtractor::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[0]){
        if(!isNewConnection){
            isNewConnection = true;
            for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
                if(it->second != nullptr){
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
        }
    }else{
        isNewConnection = false;
        isConnectionRight = false;
    }

    if(this->inletsConnected[0] && !ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->empty() && isConnectionRight){
        *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->back(); // beat
        *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[1]) = ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0])->at(arrayPosition); // bpm
        *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[2]) = 60000.0f / *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[1]); // millis
    }else if(this->inletsConnected[0] && !isConnectionRight){
        ofLog(OF_LOG_ERROR,"%s --> This object can receive data from audio analyzer object ONLY! Just reconnect it right!",this->getName().c_str());
    }

}

//--------------------------------------------------------------
void BPMExtractor::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void BPMExtractor::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){

        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            drawObjectNodeConfig(); this->configMenuWidth = ImGui::GetWindowWidth();



            ImGui::EndMenu();
        }

        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 window_size = ImVec2(this->width*_nodeCanvas.GetCanvasScale(),this->height*_nodeCanvas.GetCanvasScale());
        ImVec2 pos = ImVec2(window_pos.x + window_size.x - (50*scaleFactor), window_pos.y + window_size.y/2);

        char temp[32];
        sprintf_s(temp,"%i",static_cast<int>(floor(*ofxVP_CAST_PIN_PTR<float>(this->_outletParams[1]))));
        _nodeCanvas.getNodeDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), pos, IM_COL32_WHITE,temp, NULL, 0.0f);

        if(*ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) > 0){
            // draw beat
            _nodeCanvas.getNodeDrawList()->AddCircleFilled(ImVec2(pos.x - (10*scaleFactor),pos.y + (8*scaleFactor)), 6*scaleFactor, IM_COL32(255, 255, 120, 255), 40);
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void BPMExtractor::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Get the beat, the average bmp over a period of time, and the beat time period in milliseconds",
                "https://mosaic.d3cod3.org/reference.php?r=bpm-extractor", scaleFactor);
}

//--------------------------------------------------------------
void BPMExtractor::removeObjectContent(bool removeFileFromData){

}

OBJECT_REGISTER( BPMExtractor , "bpm extractor", OFXVP_OBJECT_CAT_AUDIOANALYSIS)

#endif
