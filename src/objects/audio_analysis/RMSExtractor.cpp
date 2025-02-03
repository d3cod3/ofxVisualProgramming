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

#include "RMSExtractor.h"

//--------------------------------------------------------------
RMSExtractor::RMSExtractor() : PatchObject("rms extractor"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new vector<float>();  // RAW Data

    _outletParams[0] = new float(); // RMS
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    isNewConnection   = false;
    isConnectionRight = false;
}

//--------------------------------------------------------------
void RMSExtractor::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"data");

    this->addOutlet(VP_LINK_NUMERIC,"RMS");
}

//--------------------------------------------------------------
void RMSExtractor::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
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
            arrayPosition = bufferSize + spectrumSize + MEL_SCALE_CRITICAL_BANDS - 1;
            XML.popTag();
        }
    }

}

//--------------------------------------------------------------
void RMSExtractor::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
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

    if(this->inletsConnected[0] && !static_cast<vector<float> *>(_inletParams[0])->empty() && isConnectionRight){
        *(float *)&_outletParams[0] = static_cast<vector<float> *>(_inletParams[0])->at(arrayPosition);
    }else if(this->inletsConnected[0] && !isConnectionRight){
        ofLog(OF_LOG_ERROR,"%s --> This object can receive data from audio analyzer object ONLY! Just reconnect it right!",this->getName().c_str());
    }

}

//--------------------------------------------------------------
void RMSExtractor::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

    ofSetColor(255);

}

//--------------------------------------------------------------
void RMSExtractor::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGuiEx::plotValue(*(float *)&_outletParams[0], 0.f, 1.f,IM_COL32(255,255,120,255), this->height*_nodeCanvas.GetCanvasScale() - (IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT), this->scaleFactor*_nodeCanvas.GetCanvasScale());

        _nodeCanvas.EndNodeContent();
    }
}

//--------------------------------------------------------------
void RMSExtractor::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Extracts the rms (Root Mean Square) power from the audio analysis data vector as a float value (between 0.0 and 1.0).",
                "https://mosaic.d3cod3.org/reference.php?r=rms-extractor", scaleFactor);
}

//--------------------------------------------------------------
void RMSExtractor::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
}

OBJECT_REGISTER( RMSExtractor , "rms extractor", OFXVP_OBJECT_CAT_AUDIOANALYSIS)

#endif
