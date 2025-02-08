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

#include "Receiver.h"

//--------------------------------------------------------------
vpReceiver::vpReceiver() : PatchObject("receiver") {

    this->numInlets  = 1;
    this->numOutlets = 1;

    this->initInletsState();

    this->height            *= 0.5;

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    dataTypes               = {"float","string","vector<float>","texture","audio signal"};
    isReceivingON           = false;
    varName                 = "";
    prevVarName             = "_unassigned";
    wirelessPin             = 0;
    receiveTypeIndex        = 0;
    signalSendEvent         = false;

    emptyVector             = new vector<float>();
    kuro                    = new ofImage();

    this->wirelessType = receiveTypeIndex;

    loaded                  = false;

}

//--------------------------------------------------------------
void vpReceiver::newObject(){
    PatchObject::setName( this->objectName );

    this->setCustomVar(static_cast<float>(receiveTypeIndex),"DATA_TYPE");
    this->setCustomVar(static_cast<float>(isReceivingON),"IS_RECEIVING");
    this->setCustomVar(0.0f,prevVarName);

}

//--------------------------------------------------------------
void vpReceiver::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    emptyVector->assign(1,0);

    // load kuro
    ofDisableArbTex();
    kuro->load("images/kuro.jpg");
    ofEnableArbTex();

    // init wireless receiver
    initWireless();

}

//--------------------------------------------------------------
void vpReceiver::setupAudioOutObjectContent(pdsp::Engine &engine){
    unusedArgs(engine);

}

//--------------------------------------------------------------
void vpReceiver::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(receiveTypeIndex == VP_LINK_NUMERIC){
        if(this->inletsConnected[0] && isReceivingON){
            *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]);
        }else{
            *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0.0f;
        }
    }else if(receiveTypeIndex == VP_LINK_STRING){
        if(this->inletsConnected[0] && isReceivingON){
            *ofxVP_CAST_PIN_PTR<string>(_outletParams[0]) = *ofxVP_CAST_PIN_PTR<string>(this->_inletParams[0]);
        }else{
            *ofxVP_CAST_PIN_PTR<string>(_outletParams[0]) = "";
        }
    }else if(receiveTypeIndex == VP_LINK_ARRAY){
        if(this->inletsConnected[0] && isReceivingON){
            *ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0]) = *ofxVP_CAST_PIN_PTR<vector<float>>(this->_inletParams[0]);
        }else{
            *ofxVP_CAST_PIN_PTR<vector<float>>(this->_outletParams[0]) = *emptyVector;
        }

    }else if(receiveTypeIndex == VP_LINK_TEXTURE){
        if(this->inletsConnected[0] && isReceivingON){
            *ofxVP_CAST_PIN_PTR<ofTexture>(_outletParams[0]) = *ofxVP_CAST_PIN_PTR<ofTexture>(_inletParams[0]);
        }else{
            *ofxVP_CAST_PIN_PTR<ofTexture>(_outletParams[0]) = kuro->getTexture();
        }
    }

    if(signalSendEvent){
        signalSendEvent = false;
        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if(it->second != nullptr){
                if(it->second->getName() == "sender"){
                    for(int wid=0;wid<it->second->getNumOutlets();wid++){
                        if(it->second->getOutletWirelessSend(wid) && it->second->getOutletID(wid) == this->varName && this->varName != ""){
                            it->second->closeWirelessLink(wid);
                            it->second->openWirelessLink(wid);
                            break;
                        }
                    }
                }
            }
        }
    }

    if(static_cast<int>(this->outletsType.size()) > this->numOutlets){
        // reset outlets to numOutlets
        this->outletsType.resize(this->numOutlets);
        this->outletsNames.resize(this->numOutlets);
        this->outletsIDs.resize(this->numOutlets);
        this->outletsWirelessSend.resize(this->numOutlets);
    }

    if(!loaded){
        loaded = true;
    }
}

//--------------------------------------------------------------
void vpReceiver::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);
}

//--------------------------------------------------------------
void vpReceiver::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        if(isReceivingON){
            ImGui::SetCursorPos(ImVec2(IMGUI_EX_NODE_PINS_WIDTH_NORMAL+(4*scaleFactor), (this->height/2 *_nodeCanvas.GetCanvasScale()) - (2*scaleFactor)));
            ImGui::Text("%s",varName.c_str());
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void vpReceiver::drawObjectNodeConfig(){
    ImGui::Spacing();
    ImGui::InputText("Variable name",&varName);
    ImGui::Spacing();
    int typeIndex = receiveTypeIndex;
    if(ofxImGui::VectorCombo("Data type", &typeIndex,dataTypes)){
        if(receiveTypeIndex != typeIndex){
            receiveTypeIndex = typeIndex;
            this->wirelessType = receiveTypeIndex;
            this->setCustomVar(static_cast<float>(receiveTypeIndex),"DATA_TYPE");
            isReceivingON = false;
            this->closeWirelessLink(wirelessPin);
            this->setCustomVar(static_cast<float>(isReceivingON),"IS_RECEIVING");
            if(prevVarName != "_unassigned"){
                this->substituteCustomVar(prevVarName,"_unassigned");
                prevVarName = "_unassigned";
            }
            this->saveConfig(false);

            changeDataType(receiveTypeIndex,false);
        }
    }
    ImGui::Spacing();
    if(ImGui::Button("RECEIVE",ImVec2(224*scaleFactor,26*scaleFactor))){
        if(varName != ""){
            isReceivingON = true;
            signalSendEvent = true;
            this->wirelessName = varName;
        }else{
            isReceivingON = false;
        }
        this->setInletID(wirelessPin,varName);
        this->setCustomVar(static_cast<float>(isReceivingON),"IS_RECEIVING");
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
        isReceivingON = false;
        this->setInletID(wirelessPin,varName);
        this->setCustomVar(static_cast<float>(isReceivingON),"IS_RECEIVING");
        this->wirelessName = "";
        if(prevVarName != "_unassigned"){
            this->substituteCustomVar(prevVarName,"_unassigned");
        }
        this->saveConfig(false);
        prevVarName = "_unassigned";
    }
    ImGui::PopStyleColor(3);

    ImGuiEx::ObjectInfo(
                "Wireless receiver.",
                "https://mosaic.d3cod3.org/reference.php?r=receiver", scaleFactor);
}

//--------------------------------------------------------------
void vpReceiver::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);

    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void vpReceiver::audioOutObject(ofSoundBuffer &outBuffer){
    unusedArgs(outBuffer);

    if(receiveTypeIndex == VP_LINK_AUDIO){
        if(this->inletsConnected[0] && isReceivingON && !ofxVP_CAST_PIN_PTR<ofSoundBuffer>(_inletParams[0])->getBuffer().empty()){
            *ofxVP_CAST_PIN_PTR<ofSoundBuffer>(_outletParams[0]) = *ofxVP_CAST_PIN_PTR<ofSoundBuffer>(_inletParams[0]);
        }/*else{
            if(ofxVP_CAST_PIN_PTR<ofSoundBuffer>(_outletParams[0]) != nullptr && ofxVP_CAST_PIN_PTR<ofSoundBuffer>(_outletParams[0])->getBuffer().empty()){
                ofxVP_CAST_PIN_PTR<ofSoundBuffer>(_outletParams[0])->set(0.0f);
            }

        }*/
    }

}

//--------------------------------------------------------------
void vpReceiver::initWireless(){

    receiveTypeIndex = static_cast<int>(this->getCustomVar("DATA_TYPE"));

    changeDataType(receiveTypeIndex);
    this->wirelessType = receiveTypeIndex;

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
                            if(XML.pushTag("var",t)){
                                if(XML.getValue("name","") != "DATA_TYPE" && XML.getValue("name","") != "IS_RECEIVING" && XML.getValue("name","") != "_unassigned"){
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

    isReceivingON = this->getCustomVar("IS_RECEIVING");

    if(isReceivingON && varName != ""){
        this->setInletID(wirelessPin,varName);
        signalSendEvent = true;
        prevVarName = varName;
        this->wirelessName = varName;
    }else if(varName == ""){
        prevVarName = "_unassigned";
    }
}

//--------------------------------------------------------------
void vpReceiver::changeDataType(int type, bool init){

    // disconnect everything if connected
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }

    this->pdspIn.clear();
    this->pdspOut.clear();

    // stop and reset receiving
    isReceivingON = false;

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
            *ofxVP_CAST_PIN_PTR<float>(this->_inletParams[0]) = 0.0f;

            _outletParams[0] = new float();
            *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[0]) = 0.0f;

            this->addInlet(VP_LINK_NUMERIC,"number");
            this->addOutlet(VP_LINK_NUMERIC,"number");

            break;
        case 1:
            _inletParams[0] = new string();
            *ofxVP_CAST_PIN_PTR<string>(this->_inletParams[0]) = "";

            _outletParams[0] = new string();
            *ofxVP_CAST_PIN_PTR<string>(_outletParams[0]) = "";

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

            ofxVP_CAST_PIN_PTR<ofSoundBuffer>(_inletParams[0])->set(0.0f);
            ofxVP_CAST_PIN_PTR<ofSoundBuffer>(_outletParams[0])->set(0.0f);

            break;
        default:
            break;
    }

    // changed inlets/outlet type
    if(!init){ // remove links
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
                            int newLink = XML.addTag("link");
                            if(XML.pushTag("link",newLink)){
                                XML.setValue("type",this->outletsType.at(0));
                                XML.setValue("name",this->outletsNames.at(0));
                                XML.popTag();
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

        this->outPut.clear();
    }


    this->saveConfig(false);

    this->setInletWirelessReceive(wirelessPin,true);

}

OBJECT_REGISTER( vpReceiver, "receiver", OFXVP_OBJECT_CAT_DATA)

#endif
