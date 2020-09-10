/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2020 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "SignalOperator.h"

//--------------------------------------------------------------
SignalOperator::SignalOperator() : PatchObject("signal operator"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();
    _inletParams[1] = new ofSoundBuffer();

    _outletParams[0] = new ofSoundBuffer();

    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    this->initInletsState();

    _operator           = Sig_Operator_ADD;

    loaded              = false;

    this->height        /= 2;
}

//--------------------------------------------------------------
void SignalOperator::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"s1");
    this->addInlet(VP_LINK_AUDIO,"s2");

    this->addOutlet(VP_LINK_AUDIO,"result");

    this->setCustomVar(_operator,"OPERATOR");

}

//--------------------------------------------------------------
void SignalOperator::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    loadSettings();

    operators_string.push_back("+~");
    operators_string.push_back("-~");
    operators_string.push_back("*~");
    operators_string.push_back("/~");
}

//--------------------------------------------------------------
void SignalOperator::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(!loaded){
        loaded = true;
        _operator = this->getCustomVar("OPERATOR");
    }

}

//--------------------------------------------------------------
void SignalOperator::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void SignalOperator::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){

        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            ImGuiEx::ObjectInfo(
                        "Signal operator",
                        "https://mosaic.d3cod3.org/reference.php?r=signal-operator", scaleFactor);

            ImGui::EndMenu();
        }

        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        ImGui::Dummy(ImVec2(-1,ImGui::GetWindowSize().y/2 - (28*scaleFactor))); // Padding top

        ImGui::PushItemWidth(-50*scaleFactor);
        if(ImGui::BeginCombo("operator", operators_string.at(_operator).c_str() )){
            for(int i=0; i < operators_string.size(); ++i){
                bool is_selected = (_operator == i );
                if (ImGui::Selectable(operators_string.at(i).c_str(), is_selected)){
                    _operator = i;
                    this->setCustomVar(_operator,"OPERATOR");
                }
                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void SignalOperator::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void SignalOperator::audioOutObject(ofSoundBuffer &outputBuffer){
    if(this->inletsConnected[0] && this->inletsConnected[1] && monoBuffer.getNumFrames() == static_cast<ofSoundBuffer *>(_inletParams[0])->getNumFrames() && monoBuffer.getNumFrames() == static_cast<ofSoundBuffer *>(_inletParams[1])->getNumFrames()){
        for(size_t i = 0; i < monoBuffer.getNumFrames(); i++) {
            if(_operator == Sig_Operator_ADD){
                monoBuffer.getSample(i,0) = static_cast<ofSoundBuffer *>(_inletParams[0])->getSample(i, 0) + static_cast<ofSoundBuffer *>(_inletParams[1])->getSample(i, 0);
            }else if(_operator == Sig_Operator_SUBTRACT){
                monoBuffer.getSample(i,0) = static_cast<ofSoundBuffer *>(_inletParams[0])->getSample(i, 0) - static_cast<ofSoundBuffer *>(_inletParams[1])->getSample(i, 0);
            }else if(_operator == Sig_Operator_MULTIPLY){
                monoBuffer.getSample(i,0) = static_cast<ofSoundBuffer *>(_inletParams[0])->getSample(i, 0) * static_cast<ofSoundBuffer *>(_inletParams[1])->getSample(i, 0);
            }else if(_operator == Sig_Operator_DIVIDE){
                // avoid division by zero
                if(static_cast<ofSoundBuffer *>(_inletParams[1])->getSample(i, 0) == 0.0f){
                    monoBuffer.getSample(i,0) = static_cast<ofSoundBuffer *>(_inletParams[0])->getSample(i, 0) / ( 0.000001f + static_cast<ofSoundBuffer *>(_inletParams[1])->getSample(i, 0));
                }else{
                    monoBuffer.getSample(i,0) = static_cast<ofSoundBuffer *>(_inletParams[0])->getSample(i, 0) / static_cast<ofSoundBuffer *>(_inletParams[1])->getSample(i, 0);
                }
            }else{
                monoBuffer.getSample(i,0) = 0.0f;
            }

            lastBuffer = monoBuffer;
        }
    }else if(this->inletsConnected[0] && !this->inletsConnected[1]){
        lastBuffer= *static_cast<ofSoundBuffer *>(_inletParams[0]);
    }else{
        lastBuffer = monoBuffer * 0.0f;
    }

    *static_cast<ofSoundBuffer *>(_outletParams[0]) = lastBuffer;

}

//--------------------------------------------------------------
void SignalOperator::loadSettings(){
    ofxXmlSettings XML;

    if(XML.loadFile(patchFile)){
        if(XML.pushTag("settings")){
            sampleRate = static_cast<double>(XML.getValue("sample_rate_out",0));
            bufferSize = XML.getValue("buffer_size",0);
            XML.popTag();
        }
    }

    shortBuffer = new short[bufferSize];
    for (int i = 0; i < bufferSize; i++){
        shortBuffer[i] = 0;
    }

    ofSoundBuffer tmpBuffer(shortBuffer,static_cast<size_t>(bufferSize),1,static_cast<unsigned int>(sampleRate));
    monoBuffer = tmpBuffer;
}

OBJECT_REGISTER( SignalOperator, "signal operator", OFXVP_OBJECT_CAT_SOUND)

#endif
