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

#include "Spigot.h"

//--------------------------------------------------------------
Spigot::Spigot() : PatchObject("spigot"){

    this->numInlets  = 6;
    this->numOutlets = 5;

    _inletParams[0] = new vector<float>();  // state

    _inletParams[1] = new float();  // float
    *(float *)&_inletParams[1] = 0.0f;

    _inletParams[2] = new string();  // string
    *static_cast<string *>(_inletParams[2]) = "";

    _inletParams[3] = new vector<float>(); // vector

    _inletParams[4] = new ofTexture(); // texture

    _inletParams[5] = new ofSoundBuffer();  // signal

    _outletParams[0] = new float(); // output numeric
    *(float *)&_outletParams[0] = 0.0f;

    _outletParams[1] = new string();  // string
    *static_cast<string *>(_outletParams[1]) = "";

    _outletParams[2] = new vector<float>(); // vector

    _outletParams[3] = new ofTexture(); // texture

    _outletParams[4] = new ofSoundBuffer();  // signal

    this->initInletsState();

    isOpen = new bool[5];
    for(int i=0;i<5;i++){
        isOpen[i] = false;
    }

    labels.assign(5,"");

    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    empty   = new vector<float>();
    kuro    = new ofImage();

    loaded  = false;
}

//--------------------------------------------------------------
void Spigot::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"state");
    this->addInlet(VP_LINK_NUMERIC,"float");
    this->addInlet(VP_LINK_STRING,"string");
    this->addInlet(VP_LINK_ARRAY,"vector");
    this->addInlet(VP_LINK_TEXTURE,"texture");
    this->addInlet(VP_LINK_AUDIO,"signal");

    this->addOutlet(VP_LINK_NUMERIC,"float");
    this->addOutlet(VP_LINK_STRING,"string");
    this->addOutlet(VP_LINK_ARRAY,"vector");
    this->addOutlet(VP_LINK_TEXTURE,"texture");
    this->addOutlet(VP_LINK_AUDIO,"signal");

    for(int i=0;i<5;i++){
        this->setCustomVar(static_cast<float>(isOpen[i]),"IS_OPEN_"+ofToString(i));
    }
}

//--------------------------------------------------------------
void Spigot::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    empty->assign(1,0);

    // load kuro
    kuro->load("images/kuro.jpg");

    labels.at(0) = "float";
    labels.at(1) = "string";
    labels.at(2) = "vector<float>";
    labels.at(3) = "texture";
    labels.at(4) = "audio signal";

    static_cast<ofSoundBuffer *>(_inletParams[5])->set(0.0f);
    static_cast<ofSoundBuffer *>(_outletParams[4])->set(0.0f);
}

//--------------------------------------------------------------
void Spigot::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);
    
    if(this->inletsConnected[0] && !static_cast<vector<float> *>(_inletParams[0])->empty()){

        for(size_t s=0;s<static_cast<size_t>(static_cast<vector<float> *>(_inletParams[0])->size());s++){
            if(s<5){
                if(static_cast<vector<float> *>(_inletParams[0])->at(s) < 1.0f){
                    isOpen[s] = false;
                }else{
                    isOpen[s] = true;
                }
            }else{
                break;
            }
        }

        if(isOpen[0]){
            *(float *)&_outletParams[0] = *(float *)&_inletParams[1];
        }else{
            *(float *)&_outletParams[0] = -1.0f;
        }
        if(isOpen[1]){
            *static_cast<string *>(_outletParams[1]) = *static_cast<string *>(_inletParams[2]);
        }else{
            *static_cast<string *>(_outletParams[1]) = "empty";
        }
        if(isOpen[2] && !static_cast<vector<float> *>(_inletParams[3])->empty()){
            *static_cast<vector<float> *>(_outletParams[2]) = *static_cast<vector<float> *>(_inletParams[3]);
        }else{
            *static_cast<vector<float> *>(_outletParams[2]) = *empty;
        }
        if(isOpen[3]){
            *static_cast<ofTexture *>(_outletParams[3]) = *static_cast<ofTexture *>(_inletParams[4]);
        }else{
            *static_cast<ofTexture *>(_outletParams[3]) = kuro->getTexture();
        }

    }

    if(!loaded){
        loaded = true;
        for(int i=0;i<5;i++){
            if(this->getCustomVar("IS_OPEN_"+ofToString(i)) < 1.0f){
               isOpen[i] = false;
            }else{
                isOpen[i] = true;
            }
        }
    }
    
}

//--------------------------------------------------------------
void Spigot::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

    ofSetColor(255);
}

//--------------------------------------------------------------
void Spigot::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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
        ImVec2 window_size = ImGui::GetWindowSize();
        float inletPinDistance = (window_size.y-((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor))/this->numInlets;
        float outletPinDistance = (window_size.y-((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor))/this->numOutlets;

        ImU32 tempCol;

        for(int i=0;i<5;i++){
            if(isOpen[i]){
                if(i==0){
                    tempCol = IM_COL32(210,210,210,255);
                }else if(i==1){
                    tempCol = IM_COL32(200,180,255,255);
                }else if(i==2){
                    tempCol = IM_COL32(120,180,120,255);
                }else if(i==3){
                    tempCol = IM_COL32(120,255,255,255);
                }else if(i==4){
                    tempCol = IM_COL32(255,255,120,255);
                }
                _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(window_pos.x,window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (inletPinDistance/2) + inletPinDistance*(i+1)),ImVec2(window_pos.x + window_size.x,window_pos.y + (IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor) + (outletPinDistance/2) + outletPinDistance*i),tempCol,2.0f);
            }
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void Spigot::drawObjectNodeConfig(){
    ImGui::Spacing();
    for(int i=0;i<5;i++){
        if(ImGui::Checkbox(labels.at(i).c_str(),&isOpen[i])){
            this->setCustomVar(static_cast<float>(isOpen[i]),"IS_OPEN_"+ofToString(i));
        }
        ImGui::Spacing();
    }


    ImGuiEx::ObjectInfo(
                "Multiple switch of different cable types.",
                "https://mosaic.d3cod3.org/reference.php?r=spigot", scaleFactor);
}

//--------------------------------------------------------------
void Spigot::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
}

//--------------------------------------------------------------
void Spigot::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);

    if(isOpen[4]){
        if(this->inletsConnected[5] && !static_cast<ofSoundBuffer *>(_inletParams[5])->getBuffer().empty()){
            *static_cast<ofSoundBuffer *>(_outletParams[4]) = *static_cast<ofSoundBuffer *>(_inletParams[5]);
        }else{
            static_cast<ofSoundBuffer *>(_outletParams[4])->set(0.0f);
        }
    }else{
        static_cast<ofSoundBuffer *>(_outletParams[4])->set(0.0f);
    }
}

OBJECT_REGISTER( Spigot, "spigot", OFXVP_OBJECT_CAT_LOGIC)

#endif
