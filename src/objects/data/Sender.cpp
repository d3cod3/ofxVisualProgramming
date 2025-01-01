/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2023 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "Sender.h"

//--------------------------------------------------------------
vpSender::vpSender() : PatchObject("sender") {

    this->numInlets  = 1;
    this->numOutlets = 1;

    this->initInletsState();

    this->height            *= 0.5;

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    dataTypes               = {"float","string","vector<float>","texture","audio signal"};
    isSendingON             = false;
    varName                 = "";
    prevVarName             = "_unassigned";
    wirelessPin             = 0;
    sendTypeIndex           = 0;
    resetLinks              = false;

    emptyVector             = new vector<float>();
    kuro                    = new ofImage();

    this->wirelessType = sendTypeIndex;

    loaded                  = false;
}

//--------------------------------------------------------------
void vpSender::newObject(){
    PatchObject::setName( this->objectName );

    this->setCustomVar(static_cast<float>(sendTypeIndex),"DATA_TYPE");
    this->setCustomVar(static_cast<float>(isSendingON),"IS_SENDING");
    this->setCustomVar(0.0f,prevVarName);

}

//--------------------------------------------------------------
void vpSender::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    emptyVector->assign(1,0);

    // load kuro
    ofDisableArbTex();
    kuro->load("images/kuro.jpg");
    ofEnableArbTex();

}

//--------------------------------------------------------------
void vpSender::setupAudioOutObjectContent(pdsp::Engine &engine){
    unusedArgs(engine);
}

//--------------------------------------------------------------
void vpSender::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(sendTypeIndex == VP_LINK_NUMERIC){
        if(this->inletsConnected[0] && isSendingON){
            *(float *)&_outletParams[0] = *(float *)&_inletParams[0];
        }else{
            *(float *)&_outletParams[0] = 0.0f;
        }
    }else if(sendTypeIndex == VP_LINK_STRING){
        if(this->inletsConnected[0] && isSendingON){
            *static_cast<string *>(_outletParams[0]) = *static_cast<string *>(_inletParams[0]);
        }else{
            *static_cast<string *>(_outletParams[0]) = "";
        }
    }else if(sendTypeIndex == VP_LINK_ARRAY){
        if(this->inletsConnected[0] && !static_cast<vector<float> *>(_inletParams[0])->empty() && isSendingON){
            *static_cast<vector<float> *>(_outletParams[0]) = *static_cast<vector<float> *>(_inletParams[0]);
        }else{
            *static_cast<vector<float> *>(_outletParams[0]) = *emptyVector;
        }

    }else if(sendTypeIndex == VP_LINK_TEXTURE){
        if(this->inletsConnected[0] && isSendingON){
            *static_cast<ofTexture *>(_outletParams[0]) = *static_cast<ofTexture *>(_inletParams[0]);
        }else{
            *static_cast<ofTexture *>(_outletParams[0]) = kuro->getTexture();
        }
    }

    if(resetLinks){
        resetLinks = false;
        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if(it->second != nullptr){
                vector<shared_ptr<PatchLink>> tempBuffer;
                for(int j=0;j<static_cast<int>(it->second->outPut.size());j++){
                    if(it->second->outPut[j]->toObjectID != this->getId()){
                        tempBuffer.push_back(it->second->outPut[j]);
                    }else{
                        it->second->outPut[j]->isDisabled = true;
                        patchObjects[it->second->outPut[j]->toObjectID]->inletsConnected[it->second->outPut[j]->toInletID] = false;
                    }
                }
                it->second->outPut = tempBuffer;
            }
        }
    }

    if(!loaded){
        loaded = true;

        initWireless();

    }
}

//--------------------------------------------------------------
void vpSender::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);
}

//--------------------------------------------------------------
void vpSender::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        if(isSendingON){
            ImGui::SetCursorPos(ImVec2(IMGUI_EX_NODE_PINS_WIDTH_NORMAL+(4*scaleFactor), (this->height/2 *_nodeCanvas.GetCanvasScale()) - (2*scaleFactor)));
            ImGui::Text("%s",varName.c_str());
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void vpSender::drawObjectNodeConfig(){
    ImGui::Spacing();
    ImGui::InputText("Variable name",&varName);
    ImGui::Spacing();
    int typeIndex = sendTypeIndex;
    if(ofxImGui::VectorCombo("Data type", &typeIndex,dataTypes)){
        if(sendTypeIndex != typeIndex){
            sendTypeIndex = typeIndex;
            this->setCustomVar(static_cast<float>(sendTypeIndex),"DATA_TYPE");
            isSendingON = false;
            this->wirelessType = sendTypeIndex;
            this->closeWirelessLink(wirelessPin);
            this->setCustomVar(static_cast<float>(isSendingON),"IS_SENDING");
            if(prevVarName != "_unassigned"){
                this->substituteCustomVar(prevVarName,"_unassigned");
                prevVarName = "_unassigned";
            }
            this->saveConfig(false);

            changeDataType(sendTypeIndex,false);
        }
    }
    ImGui::Spacing();
    if(ImGui::Button("SEND",ImVec2(224*scaleFactor,26*scaleFactor))){
        // reset previous wireless link
        isSendingON = true;
        this->closeWirelessLink(wirelessPin);

        // create new one
        this->setOutletID(wirelessPin,varName);
        if(varName != ""){
            this->openWirelessLink(wirelessPin);
            this->wirelessName = varName;
        }

        // save it
        this->setCustomVar(static_cast<float>(isSendingON),"IS_SENDING");
        if(prevVarName != varName){
            this->substituteCustomVar(prevVarName,varName);
        }
        this->saveConfig(false);
        prevVarName = varName;
    }
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button, VHS_RED);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, VHS_RED_OVER);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, VHS_RED_OVER);
    if(ImGui::Button("STOP",ImVec2(224*scaleFactor,26*scaleFactor))){
        isSendingON = false;
        this->closeWirelessLink(wirelessPin);
        this->setCustomVar(static_cast<float>(isSendingON),"IS_SENDING");
        this->wirelessName = "";
        if(prevVarName != "_unassigned"){
            this->substituteCustomVar(prevVarName,"_unassigned");
        }
        this->saveConfig(false);
        prevVarName = "_unassigned";
    }
    ImGui::PopStyleColor(3);

    ImGuiEx::ObjectInfo(
                "Wireless sender.",
                "https://mosaic.d3cod3.org/reference.php?r=sender", scaleFactor);
}

//--------------------------------------------------------------
void vpSender::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);

    this->closeWirelessLink(wirelessPin);

    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void vpSender::audioOutObject(ofSoundBuffer &outBuffer){
    unusedArgs(outBuffer);

    if(sendTypeIndex == VP_LINK_AUDIO){
        if(this->inletsConnected[0] && isSendingON && !static_cast<ofSoundBuffer *>(_inletParams[0])->getBuffer().empty()){
            *static_cast<ofSoundBuffer *>(_outletParams[0]) = *static_cast<ofSoundBuffer *>(_inletParams[0]);
        }/*else{
            if(static_cast<ofSoundBuffer *>(_outletParams[0]) != nullptr && static_cast<ofSoundBuffer *>(_outletParams[0])->getBuffer().empty()){
                static_cast<ofSoundBuffer *>(_outletParams[0])->set(0.0f);
            }
        }*/
    }

}

//--------------------------------------------------------------
void vpSender::initWireless(){

    sendTypeIndex = static_cast<int>(this->getCustomVar("DATA_TYPE"));

    changeDataType(sendTypeIndex);
    this->wirelessType = sendTypeIndex;

    ofxXmlSettings XML;
#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
    if (XML.loadFile(this->patchFile)){
#else
    if (XML.load(this->patchFile)){
#endif
        int totalObjects = XML.getNumTags("object");

        // Get object inlets config
        for(int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                if(XML.getValue("id", -1) == this->nId){
                    if (XML.pushTag("vars")){
                        int totalVars = XML.getNumTags("var");
                        for (int t=0;t<totalVars;t++){
                            //ofLog(OF_LOG_NOTICE,"%s",XML.getValue("name","").c_str());
                            if(XML.pushTag("var",t)){
                                if(XML.getValue("name","") != "DATA_TYPE" && XML.getValue("name","") != "IS_SENDING" && XML.getValue("name","") != "_unassigned"){
                                    varName = XML.getValue("name","");
                                }
                                XML.popTag();
                            }
                        }
                        XML.popTag();
                    }
                }
                XML.popTag();
            }
        }
    }

    isSendingON = this->getCustomVar("IS_SENDING");

    if(isSendingON && varName != ""){
        this->setOutletID(wirelessPin,varName);
        this->openWirelessLink(wirelessPin);
        prevVarName = varName;
        this->wirelessName = varName;
    }else if(varName == ""){
        prevVarName = "_unassigned";
    }
}

//--------------------------------------------------------------
void vpSender::changeDataType(int type, bool init){
    // disconnect everything if connected
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }

    this->pdspIn.clear();
    this->pdspOut.clear();

    // stop and reset sending
    isSendingON = false;
    this->closeWirelessLink(wirelessPin);

    // change inlet and outlet data type
    this->inletsType.clear();
    this->inletsNames.clear();
    this->inletsIDs.clear();
    this->inletsWirelessReceive.clear();

    this->outletsNames.clear();
    this->outletsType.clear();
    this->outletsIDs.clear();
    this->outletsWirelessSend.clear();

    switch( type ) {
        case 0:
            _inletParams[0] = new float();
            *(float *)&_inletParams[0] = 0.0f;

            _outletParams[0] = new float();
            *(float *)&_outletParams[0] = 0.0f;

            this->addInlet(VP_LINK_NUMERIC,"number");
            this->addOutlet(VP_LINK_NUMERIC,"number");

            break;
        case 1:
            _inletParams[0] = new string();
            *static_cast<string *>(_inletParams[0]) = "";

            _outletParams[0] = new string();
            *static_cast<string *>(_outletParams[0]) = "";

            this->addInlet(VP_LINK_STRING,"string");
            this->addOutlet(VP_LINK_STRING,"string");

            break;
        case 2:
            _inletParams[0] = new vector<float>();
            _outletParams[0] = new vector<float>();

            this->addInlet(VP_LINK_ARRAY,"array");
            this->addOutlet(VP_LINK_ARRAY,"array");

            break;
        case 3:
            _inletParams[0] = new ofTexture();
            _outletParams[0] = new ofTexture();

            this->addInlet(VP_LINK_TEXTURE,"texture");
            this->addOutlet(VP_LINK_TEXTURE,"texture");
            break;
        case 4:

            _inletParams[0] = new ofSoundBuffer();
            _outletParams[0] = new ofSoundBuffer();

            this->addInlet(VP_LINK_AUDIO,"signal");
            this->addOutlet(VP_LINK_AUDIO,"signal");

            this->pdspIn[0] >> this->pdspOut[0];

            static_cast<ofSoundBuffer *>(_inletParams[0])->set(0.0f);
            static_cast<ofSoundBuffer *>(_outletParams[0])->set(0.0f);

            break;
        default:
            break;
    }

    this->inletsConnected.clear();
    this->initInletsState();

    this->setOutletWirelessSend(wirelessPin,true);

    ofxXmlSettings XML;

#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
    if (XML.loadFile(this->patchFile)){
#else
    if (XML.load(this->patchFile)){
#endif
        int totalObjects = XML.getNumTags("object");

        // Save new object config
        for(int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                if(XML.getValue("id", -1) == this->nId){
                    // Dynamic reloading outlets
                    XML.removeTag("outlets");
                    int newOutlets = XML.addTag("outlets");
                    if(XML.pushTag("outlets",newOutlets)){
                        for(int j=0;j<static_cast<int>(this->outletsType.size());j++){
                            int newLink = XML.addTag("link");
                            if(XML.pushTag("link",newLink)){
                                XML.setValue("type",this->outletsType.at(j));
                                XML.setValue("name",this->outletsNames.at(j));
                                XML.popTag();
                            }
                        }
                        XML.popTag();
                    }
                }else{
                    // remove links to this object if are not of the same type as before
                    if(XML.pushTag("outlets")){
                        int totalLinks = XML.getNumTags("link");
                        for(int l=0;l<totalLinks;l++){
                            if(XML.pushTag("link",l)){
                                int totalTo = XML.getNumTags("to");
                                int type = XML.getValue("type",-1);
                                vector<bool> delLinks;
                                for(int t=0;t<totalTo;t++){
                                    if(XML.pushTag("to",t)){
                                        if(XML.getValue("id", -1) == this->nId && type != this->outletsType.at(wirelessPin)){
                                            delLinks.push_back(true);
                                        }else{
                                            delLinks.push_back(false);
                                        }
                                        XML.popTag();
                                    }
                                }
                                for(int d=delLinks.size()-1;d>=0;d--){
                                    if(delLinks.at(d)){
                                        XML.removeTag("to",d);
                                    }
                                }
                                XML.popTag();
                            }
                        }
                        XML.popTag();
                    }
                }
                XML.popTag();
            }
        }

#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
            XML.saveFile();
#else
            XML.save();
#endif

    }

    if(!init){
        resetLinks = true;
    }

    this->saveConfig(false);

}

OBJECT_REGISTER( vpSender, "sender", OFXVP_OBJECT_CAT_DATA)

#endif
