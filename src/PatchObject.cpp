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

#include "PatchObject.h"

//--------------------------------------------------------------
PatchObject::PatchObject(const std::string& _customUID ) : ofxVPHasUID(_customUID) {
    nId                 = -1;
    name                = "none";
    specialName         = "";
    filepath            = "none";
    patchFile           = "";
    patchFolderPath     = "";

    specialLinkTypeName = "";

    subpatchName        = "root";

    numInlets   = 0;
    numOutlets  = 0;

    isSystemObject          = false;
    bActive                 = false;
    isObjectSelected        = false;
    isOverGUI               = false;
    isRetina                = false;
    isGUIObject             = false;
    isAudioINObject         = false;
    isAudioOUTObject        = false;
    isPDSPPatchableObject   = false;
    isTextureObject         = false;
    isSharedContextObject   = false;
    isHardwareObject        = false;
    isResizable             = false;
    willErase               = false;

    initWirelessLink        = false;
    resetWirelessLink       = false;
    resetWirelessPin        = -1;

    wirelessName            = "";
    wirelessType            = -1;

    width       = OBJECT_WIDTH;
    height      = OBJECT_HEIGHT;
    headerHeight= HEADER_HEIGHT;
    x           = 0.0f;
    y           = 0.0f;
    fontSize    = 12;

    configMenuWidth     = 360.0f;

    canvasScale         = 1;
    scaleFactor         = 1.0f;

    output_width        = 320;
    output_height       = 240;

}

//--------------------------------------------------------------
PatchObject::~PatchObject(){

}

//--------------------------------------------------------------
void PatchObject::setup(std::shared_ptr<ofAppGLFWWindow> &mainWindow){

    // init vars
    for(int i=0;i<static_cast<int>(inletsType.size());i++){
        inletsPositions.push_back( ImVec2(this->x, this->y + this->height*.5f) );
    }
    for(int i=0;i<static_cast<int>(outletsType.size());i++){
        outletsPositions.push_back( ImVec2(this->x, this->y + this->height*.5f) );
    }

    setupObjectContent(mainWindow);

}

//--------------------------------------------------------------
void PatchObject::setIsRetina(bool ir, float sf){
    isRetina = ir;
    scaleFactor = sf;
    width               *= scaleFactor;
    height              *= scaleFactor;
    headerHeight        *= scaleFactor;
    configMenuWidth     *= scaleFactor;
    fontSize = static_cast<int>(floor(ofMap(scaleFactor,1,6,MIN_OF_GUI_FONT_SIZE,MAX_OF_GUI_FONT_SIZE)));
}

//--------------------------------------------------------------
void PatchObject::setupDSP(pdsp::Engine &engine){
    if(this->isPDSPPatchableObject){
        for(int i=0;i<static_cast<int>(inletsType.size());i++){
            int it = getInletType(i);
            if(it == 4){ // VP_LINK_AUDIO
                pdsp::PatchNode *temp = new pdsp::PatchNode();
                this->pdspIn[i] = *temp;
            }
        }
        for(int i=0;i<static_cast<int>(outletsType.size());i++){
            int ot = getOutletType(i);
            if(ot == 4){ // VP_LINK_AUDIO
                pdsp::PatchNode *temp = new pdsp::PatchNode();
                this->pdspOut[i] = *temp;
            }
        }
    }
    if(isAudioOUTObject){
        setupAudioOutObjectContent(engine);
    }
}

//--------------------------------------------------------------
void PatchObject::update(std::map<int,std::shared_ptr<PatchObject>> &patchObjects, pdsp::Engine &engine){

    if(willErase) return;

    // update links
    for(int out=0;out<getNumOutlets();out++){
        for(int i=0;i<static_cast<int>(outPut.size());i++){
            if(!outPut[i]->isDisabled && outPut[i]->fromOutletID == out && patchObjects[outPut[i]->toObjectID]!=nullptr && !patchObjects[outPut[i]->toObjectID]->getWillErase()){
                outPut[i]->posFrom = getOutletPosition(out);
                outPut[i]->posTo = patchObjects[outPut[i]->toObjectID]->getInletPosition(outPut[i]->toInletID);

                // check first if link is deactivated by shift click
                outPut[i]->isDeactivated = false;
                for(size_t di=0;di<linksDeactivated.size();di++){
                    if(outPut[i]->id == linksDeactivated.at(di)){
                        outPut[i]->isDeactivated = true;
                        break;
                    }
                }
                // send data through links
                if(!outPut[i]->isDeactivated){
                    if(!patchObjects[outPut[i]->toObjectID]->inletsConnected[outPut[i]->toInletID]){
                        patchObjects[outPut[i]->toObjectID]->inletsConnected[outPut[i]->toInletID] = true;
                        if(outPut[i]->type == VP_LINK_AUDIO && patchObjects[outPut[i]->toObjectID]->getIsPDSPPatchableObject()){
                            if(this->getIsPDSPPatchableObject()){ //  || this->getName() == "audio device"
                                this->pdspOut[outPut[i]->fromOutletID] >> patchObjects[outPut[i]->toObjectID]->pdspIn[outPut[i]->toInletID];
                            }
                        }
                        patchObjects[outPut[i]->toObjectID]->_inletParams[outPut[i]->toInletID] = _outletParams[out];
                    }else{
                        patchObjects[outPut[i]->toObjectID]->_inletParams[outPut[i]->toInletID] = _outletParams[out];
                    }

                }else{
                    patchObjects[outPut[i]->toObjectID]->inletsConnected[outPut[i]->toInletID] = false;
                    if(outPut[i]->type == VP_LINK_AUDIO){
                        if(patchObjects[outPut[i]->toObjectID]->getIsPDSPPatchableObject() && patchObjects[outPut[i]->toObjectID]->pdspIn[outPut[i]->toInletID].getInputsList().size() > 0){
                            patchObjects[outPut[i]->toObjectID]->pdspIn[outPut[i]->toInletID].disconnectIn();
                        }
                    }
                }

            }
        }
    }

    updateObjectContent(patchObjects);

    if(this->isPDSPPatchableObject){
        updateAudioObjectContent(engine);
    }

}

//--------------------------------------------------------------
void PatchObject::updateWirelessLinks(std::map<int,std::shared_ptr<PatchObject>> &patchObjects){

    if(willErase) return;

    // Continuosly update float type ONLY wireless links
    for(std::map<int,std::shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->second != nullptr){
            for(int in=0;in<it->second->getNumInlets();in++){
                for(int out=0;out<this->getNumOutlets();out++){
                    if(outletsIDs.size() > out){
                        if(it->second->getInletWirelessReceive(in) && this->getOutletWirelessSend(out) && this->getOutletType(out) == it->second->getInletType(in) && this->getOutletType(out) == VP_LINK_NUMERIC && this->getOutletID(out) == it->second->getInletID(in)){
                            if(it->second->inletsConnected[in]){
                                it->second->_inletParams[in] = this->_outletParams[out];
                            }
                        }
                    }
                }
            }
        }

    }

    // manually send data through wireless links ( if var ID, transport data )
    if(initWirelessLink && resetWirelessPin != -1){
        initWirelessLink = false;
        if(this->getOutletWirelessSend(resetWirelessPin)){
            for(std::map<int,std::shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
                if(it->second != nullptr){
                    for(int in=0;in<it->second->getNumInlets();in++){
                        if(this->getOutletType(resetWirelessPin) == it->second->getInletType(in) && this->getOutletID(resetWirelessPin) == it->second->getInletID(in) && it->second->getInletWirelessReceive(in)){
                            if(!it->second->inletsConnected[in]){ // open wireless transport
                                it->second->inletsConnected[in] = true;
                                if(this->getOutletType(resetWirelessPin) == VP_LINK_AUDIO && this->getIsPDSPPatchableObject() && it->second->getIsPDSPPatchableObject()){
                                    this->pdspOut[resetWirelessPin] >> it->second->pdspIn[in];
                                }
                                it->second->_inletParams[in] = this->_outletParams[resetWirelessPin];
                                //std::cout << "Wireless connection ON between " << this->getName() << " and " << it->second->getName() << std::endl;
                            }
                        }
                    }
                }
            }
        }
        resetWirelessPin = -1;
    }


    // Manually close wireless link from internal object code ( GUI )
    if(resetWirelessLink && resetWirelessPin != -1){
        resetWirelessLink = false;
        for(std::map<int,std::shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if(it->second != nullptr){
                if(this->getId() != it->first){
                    for(int in=0;in<it->second->getNumInlets();in++){
                        if(this->getOutletType(resetWirelessPin) == it->second->getInletType(in) && this->getOutletID(resetWirelessPin) == it->second->getInletID(in)){
                            if(it->second->inletsConnected[in]){ // close wireless transport
                                it->second->inletsConnected[in] = false;
                                if(this->getOutletType(resetWirelessPin) == VP_LINK_AUDIO && this->getIsPDSPPatchableObject() && it->second->getIsPDSPPatchableObject() && it->second->pdspIn[in].getInputsList().size() > 0){
                                    it->second->pdspIn[in].disconnectIn();
                                }
                                //std::cout << "Wireless connection OFF between " << this->getName() << " and " << it->second->getName() << std::endl;
                            }
                        }
                    }
                }
            }
        }
        resetWirelessPin = -1;
    }
}

//--------------------------------------------------------------
void PatchObject::draw(ofTrueTypeFont *font){

    if(willErase) return;

    // Draw the specific object content ()
    drawObjectContent(font,(std::shared_ptr<ofBaseGLRenderer>&)ofGetCurrentRenderer());

}

//--------------------------------------------------------------
void PatchObject::drawImGuiNode(ImGuiEx::NodeCanvas& _nodeCanvas, std::map<int,std::shared_ptr<PatchObject>> &patchObjects){

    if(willErase) return;

    // check min width and height
    if(this->isResizable){
        if(this->width < OBJECT_WIDTH*scaleFactor){
            this->width = OBJECT_WIDTH*scaleFactor;
        }

        if(this->height < OBJECT_HEIGHT*scaleFactor){
            this->height = OBJECT_HEIGHT*scaleFactor;
        }
    }

    ImVec2 imPos( this->getPos() );
    ImVec2 imSize( this->width, this->height );

    // Begin Node
    std::string displayName = "";
    if(this->getSpecialName() != ""){
        displayName = PatchObject::getDisplayName()+" "+this->getSpecialName();
    }else{
        displayName = PatchObject::getDisplayName();
    }

    static bool isNodeVisible;
    isNodeVisible = _nodeCanvas.BeginNode( nId, PatchObject::getUID().c_str(), displayName, imPos, imSize, this->getNumInlets(), this->getNumOutlets(), this->getIsResizable(), this->getIsTextureObject() );

    // Always draw [in/out]lets (so wires render correctly)
    // Updates pin positions

    {

        // Inlets
        for(int i=0;i<static_cast<int>(inletsType.size());i++){
            auto pinCol = getInletColor(i);
            std::vector<ImGuiEx::ofxVPLinkData> tempLinkData;

            // if connected, get link origin (outlet origin position and link id)
            if(inletsConnected[i]){
                for(std::map<int,std::shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
                    if(it->second != nullptr){
                        for(int j=0;j<static_cast<int>(it->second->outPut.size());j++){
                            if(it->second->outPut[j]->toObjectID == nId && it->second->outPut[j]->toInletID == i){
                                ImGuiEx::ofxVPLinkData tvpld;
                                tvpld._toPinPosition = it->second->outPut[j]->posFrom;
                                tvpld._linkID = it->second->outPut[j]->id;
                                tvpld._linkLabel = it->second->getOutletName(it->second->outPut[j]->fromOutletID);
                                tvpld._fromObjectID = it->second->getId();
                                tvpld._fromPinID = it->second->outPut[j]->fromOutletID;

                                tempLinkData.push_back(tvpld);
                                break;
                            }
                        }
                    }
                }
            }

            std::ostringstream inletInfo;
            inletInfo << inletsNames.at(i); // << " " << _inletParams[i];
            ImGuiEx::NodeConnectData connectData = _nodeCanvas.AddNodePin( nId, i, inletInfo.str().c_str(), tempLinkData, getInletTypeName(i), getInletWirelessReceive(i), inletsConnected[i], IM_COL32(pinCol.r,pinCol.g,pinCol.b,pinCol.a), ImGuiExNodePinsFlags_Left );

            inletsPositions[i] = _nodeCanvas.getInletPosition(nId,i);

            // check for inbound connections
            if(connectData.connectType == 1){ // connect new
                //cout << "Connect object " << nId << " from outlet " << i << " to object " << this->getId() << " at inlet " << connectData.toInletPinID << endl;
                // if previously connected, disconnect and refresh connection
                if(this->inletsConnected.at(connectData.toInletPinID)){
                    // Disconnect from --> inlet link
                    disconnectFrom(patchObjects,connectData.toInletPinID);
                }
                // if compatible type, connect
                if(getInletType(connectData.toInletPinID) == patchObjects[connectData.fromObjectID]->getOutletType(connectData.fromOutletPinID)){
                    connectTo(patchObjects,connectData.fromObjectID,connectData.fromOutletPinID,connectData.toInletPinID,getInletType(connectData.toInletPinID));
                    patchObjects[connectData.fromObjectID]->saveConfig(true);
                }
            }else if(connectData.connectType == 2){ // re-connect
                // disconnect from elsewhere ( if another object have this connection )
                if(this->inletsConnected.at(connectData.toInletPinID)){
                    disconnectFrom(patchObjects,connectData.toInletPinID);
                }
                // disconnect previous link
                disconnectLink(patchObjects,connectData.linkID);
                // if compatible type, connect
                if(getInletType(connectData.toInletPinID) == patchObjects[connectData.fromObjectID]->getOutletType(connectData.fromOutletPinID)){
                    connectTo(patchObjects,connectData.fromObjectID,connectData.fromOutletPinID,connectData.toInletPinID,getInletType(connectData.toInletPinID));
                    patchObjects[connectData.fromObjectID]->saveConfig(true);
                }

            }else if(connectData.connectType == 3){ // disconnect
                // disconnect link
                disconnectLink(patchObjects,connectData.linkID);
            }

        }

        // Outlets
        for(int i=0;i<static_cast<int>(outletsType.size());i++){
            auto pinCol = getOutletColor(i);

            // links
            std::vector<ImGuiEx::ofxVPLinkData> tempLinkData;

            for(int j=0;j<static_cast<int>(outPut.size());j++){
                if(!outPut[j]->isDisabled && outPut[j]->fromOutletID == i){
                    ImGuiEx::ofxVPLinkData tvpld;
                    tvpld._toPinPosition = outPut[j]->posTo;
                    tvpld._linkID = outPut[j]->id;
                    tvpld._linkLabel = getOutletName(outPut[j]->fromOutletID);
                    tvpld._fromObjectID = nId;
                    tvpld._fromPinID = outPut[j]->fromOutletID;

                    tempLinkData.push_back(tvpld);

                }
            }

            std::ostringstream outletInfo;
            outletInfo << getOutletName(i); // << " " << _outletParams[i];

            _nodeCanvas.AddNodePin( nId, i, outletInfo.str().c_str(), tempLinkData, getOutletTypeName(i), getOutletWirelessSend(i), getIsOutletConnected(i), IM_COL32(pinCol.r,pinCol.g,pinCol.b,pinCol.a), ImGuiExNodePinsFlags_Right );

            outletsPositions[i] = _nodeCanvas.getOutletPosition(nId,i);
        }

    }

    // Draw Node content and handle
    if(isNodeVisible){

        // save node state on click
        if(ImGui::IsWindowHovered() && ImGui::IsMouseReleased(0)){
            //ofLog(OF_LOG_NOTICE, "Clicked object with id %i", this->nId);
            saveConfig(false);
        }

        // Check menu state
        if( _nodeCanvas.doNodeMenuAction(ImGuiExNodeMenuActionFlags_DeleteNode) ){
            ofNotifyEvent(removeEvent, nId);
            this->setWillErase(true);
        }
        //else if( _nodeCanvas.doNodeMenuAction(ImGuiExNodeMenuActionFlags_CopyNode) ){
        //          ofGetWindowPtr()->setClipboardString( this->serialize() );
            // ofNotifyEvent(copyEvent, nId); ?
        //}
        else if( _nodeCanvas.doNodeMenuAction(ImGuiExNodeMenuActionFlags_DuplicateNode) ){
            ofNotifyEvent(duplicateEvent, nId);
        }


        // Refresh links to eventually disconnect ( backspace key )
        linksToDisconnect   = _nodeCanvas.getSelectedLinks();

        // Refresh links deactivated
        linksDeactivated    = _nodeCanvas.getDeactivatedLinks();

        // Refresh objects selected to eventually duplicate or delete ( cmd-d or backsapce )
        objectsSelected = _nodeCanvas.getSelectedNodesId();

        // Let objects draw their own Gui
        this->drawObjectNodeGui( _nodeCanvas );
    }

    // Close Node
    _nodeCanvas.EndNode();

    // Update pos & size
    if( imPos.x != this->x )
        this->x = imPos.x;
    if( imPos.y != this->y )
        this->y = imPos.y;
    if( imSize.x != this->width )
        this->width = imSize.x;
    if( imSize.y != this->height )
        this->height = imSize.y;

    canvasTranslation   = _nodeCanvas.GetCanvasTranslation();
    canvasScale         = _nodeCanvas.GetCanvasScale();

}

//--------------------------------------------------------------
void PatchObject::drawImGuiNodeConfig(){
    drawObjectNodeConfig();
}

//--------------------------------------------------------------
void PatchObject::move(int _x, int _y){
    int px = _x;
    int py = _y;
    if(isRetina){
        px *= 2;
        py *= 2;
    }

    this->x = px;
    this->y = py;
}

//--------------------------------------------------------------
ImVec2 PatchObject::getInletPosition(int iid){
    return ImVec2((inletsPositions[iid].x - canvasTranslation.x)/canvasScale, (inletsPositions[iid].y - canvasTranslation.y)/canvasScale);
}

//--------------------------------------------------------------
ImVec2 PatchObject::getOutletPosition(int oid){
    return ImVec2((outletsPositions[oid].x - canvasTranslation.x)/canvasScale, (outletsPositions[oid].y - canvasTranslation.y)/canvasScale);
}

//--------------------------------------------------------------
bool PatchObject::getIsOutletConnected(int oid){
    for(int j=0;j<static_cast<int>(outPut.size());j++){
        if(!outPut[j]->isDisabled){
            if(outPut[j]->fromOutletID == oid){
                return true;
            }
        }
    }

    return false;
}

//---------------------------------------------------------------------------------- PatchLinks utils
//--------------------------------------------------------------
bool PatchObject::connectTo(std::map<int,std::shared_ptr<PatchObject>> &patchObjects, int fromObjectID, int fromOutlet, int toInlet, int linkType){
    bool connected = false;

    if( (fromObjectID != -1) && (patchObjects[fromObjectID] != nullptr) && (fromObjectID!=this->getId()) && (this->getId() != -1) && (patchObjects[fromObjectID]->getOutletType(fromOutlet) == getInletType(toInlet)) && !inletsConnected[toInlet]){

        //cout << "Mosaic :: "<< "Connect object " << getName().c_str() << ":" << ofToString(getId()) << " to object " << getName().c_str() << ":" << ofToString(this->getId()) << endl;

        std::shared_ptr<PatchLink> tempLink = std::shared_ptr<PatchLink>(new PatchLink());

        std::string tmpID = ofToString(fromObjectID)+ofToString(fromOutlet)+ofToString(this->getId())+ofToString(toInlet);

        tempLink->id            = stoi(tmpID);
        tempLink->posFrom       = patchObjects[fromObjectID]->getOutletPosition(fromOutlet);
        tempLink->posTo         = getInletPosition(toInlet);
        tempLink->type          = getInletType(toInlet);
        tempLink->fromOutletID  = fromOutlet;
        tempLink->toObjectID    = this->getId();
        tempLink->toInletID     = toInlet;
        tempLink->isDisabled    = false;
        tempLink->isDeactivated = false;

        patchObjects[fromObjectID]->outPut.push_back(tempLink);

        inletsConnected[toInlet] = true;

        if(tempLink->type == VP_LINK_NUMERIC){
            _inletParams[toInlet] = new float();
        }else if(tempLink->type == VP_LINK_STRING){
            _inletParams[toInlet] = new string();
        }else if(tempLink->type == VP_LINK_ARRAY){
            _inletParams[toInlet] = new vector<float>();
        }else if(tempLink->type == VP_LINK_PIXELS){
            _inletParams[toInlet] = new ofPixels();
        }else if(tempLink->type == VP_LINK_TEXTURE){
            _inletParams[toInlet] = new ofTexture();
        }else if(tempLink->type == VP_LINK_FBO){
            _inletParams[toInlet] = new ofxPingPong();
        }else if(tempLink->type == VP_LINK_AUDIO){
            _inletParams[toInlet] = new ofSoundBuffer();
            if(patchObjects[fromObjectID]->getIsPDSPPatchableObject() && getIsPDSPPatchableObject()){
                patchObjects[fromObjectID]->pdspOut[fromOutlet] >> pdspIn[toInlet];
            }
        }

        // check special connections
        if(patchObjects[fromObjectID]->getName() == "lua script"){
            if((this->getName() == "glsl shader" || this->getName() == "output window") && linkType == VP_LINK_TEXTURE){
                patchObjects[fromObjectID]->resetResolution(this->getId(),this->getOutputWidth(),this->getOutputHeight());
            }
        }
        if(patchObjects[fromObjectID]->getName() == "glsl shader"){
            if(this->getName() == "output window" && linkType == VP_LINK_TEXTURE){
                patchObjects[fromObjectID]->resetResolution(this->getId(),this->getOutputWidth(),this->getOutputHeight());
            }
        }

        connected = true;
    }

    return connected;
}

//--------------------------------------------------------------
void PatchObject::disconnectFrom(std::map<int,std::shared_ptr<PatchObject>> &patchObjects, int objectInlet){

    for(std::map<int,std::shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->second != nullptr){
            for(int j=0;j<static_cast<int>(it->second->outPut.size());j++){
                if(it->second->outPut[j]->toObjectID == this->getId() && it->second->outPut[j]->toInletID == objectInlet){
                    // remove link
                    vector<bool> tempEraseLinks;
                    for(int s=0;s<static_cast<int>(it->second->outPut.size());s++){
                        if(it->second->outPut[s]->toObjectID == this->getId() && it->second->outPut[s]->toInletID == objectInlet){
                            tempEraseLinks.push_back(true);
                        }else{
                            tempEraseLinks.push_back(false);
                        }
                    }

                    std::vector<std::shared_ptr<PatchLink>> tempBuffer;
                    tempBuffer.reserve(it->second->outPut.size()-tempEraseLinks.size());

                    for(int s=0;s<static_cast<int>(it->second->outPut.size());s++){
                        if(!tempEraseLinks[s]){
                            tempBuffer.push_back(it->second->outPut[s]);
                        }else{
                            it->second->removeLinkFromConfig(it->second->outPut[s]->fromOutletID,it->second->outPut[s]->toObjectID,it->second->outPut[s]->toInletID);
                            this->inletsConnected[objectInlet] = false;
                            if(this->getIsPDSPPatchableObject()){
                                this->pdspIn[objectInlet].disconnectIn();
                            }
                        }
                    }

                    it->second->outPut = tempBuffer;

                    break;
                }

            }
        }

    }
}

//--------------------------------------------------------------
void PatchObject::disconnectLink(std::map<int,std::shared_ptr<PatchObject>> &patchObjects, int linkID){

    for(std::map<int,std::shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->second != nullptr){
            for(int j=0;j<static_cast<int>(it->second->outPut.size());j++){
                if(it->second->outPut[j]->id == linkID){
                    // remove link
                    vector<bool> tempEraseLinks;
                    for(int s=0;s<static_cast<int>(it->second->outPut.size());s++){
                        if(it->second->outPut[s]->id == linkID){
                            tempEraseLinks.push_back(true);
                        }else{
                            tempEraseLinks.push_back(false);
                        }
                    }

                    std::vector<std::shared_ptr<PatchLink>> tempBuffer;
                    tempBuffer.reserve(it->second->outPut.size()-tempEraseLinks.size());

                    for(int s=0;s<static_cast<int>(it->second->outPut.size());s++){
                        if(!tempEraseLinks[s]){
                            tempBuffer.push_back(it->second->outPut[s]);
                        }else{
                            it->second->removeLinkFromConfig(it->second->outPut[s]->fromOutletID,it->second->outPut[s]->toObjectID,it->second->outPut[s]->toInletID);
                            if(patchObjects[it->second->outPut[j]->toObjectID] != nullptr){
                                patchObjects[it->second->outPut[j]->toObjectID]->inletsConnected[it->second->outPut[j]->toInletID] = false;
                                if(patchObjects[it->second->outPut[j]->toObjectID]->getIsPDSPPatchableObject()){
                                    patchObjects[it->second->outPut[j]->toObjectID]->pdspIn[it->second->outPut[j]->toInletID].disconnectIn();
                                }
                            }
                        }
                    }

                    it->second->outPut = tempBuffer;

                    break;
                }
            }
        }
    }

}

//---------------------------------------------------------------------------------- LOAD/SAVE
//--------------------------------------------------------------
bool PatchObject::loadConfig(std::shared_ptr<ofAppGLFWWindow> &mainWindow, pdsp::Engine &engine,int oTag, string &configFile){

    patchFile = configFile;
    ofxVPXml.loadMosaicPatch(patchFile);

    pugi::xml_node objNode = ofxVPXml.getObjectAtPos(oTag);

    // exit with false if object at oTag do not exists
    if(ofxVPXml.isEmptyNode(objNode)) return false;

    nId = ofxVPXml.getPatchChildInt(objNode,"id");
    name = ofxVPXml.getPatchChildString(objNode,"name");
    filepath = ofxVPXml.getPatchChildString(objNode,"filepath");
    subpatchName = ofxVPXml.getPatchChildString(objNode,"subpatch");
    if(subpatchName == ""){
        subpatchName = "root";
    }

    ofVec2f p = ofxVPXml.getObjectPosition(nId);
    move(p.x,p.y);

    pugi::xpath_node_set objVars = ofxVPXml.getObjectVars(nId);
    if(!objVars.empty()){
        for(auto & var: objVars){
            auto v = var.node();
            customVars[ofxVPXml.getPatchChildString(v,"name")] = ofxVPXml.getPatchChildFloat(v,"value");
        }
    }

    pugi::xpath_node_set objInlets = ofxVPXml.getObjectInlets(nId);
    if(!objInlets.empty()){
        inletsPositions.clear();
        inletsIDs.clear();
        inletsWirelessReceive.clear();
        for(auto & inlet: objInlets){
            auto i = inlet.node();
            inletsType.push_back(ofxVPXml.getPatchChildInt(i,"type"));
            inletsNames.push_back(ofxVPXml.getPatchChildString(i,"name"));
            inletsIDs.push_back("");
            inletsWirelessReceive.push_back(false);
            inletsPositions.push_back( ImVec2(this->x, this->y + this->height*.5f) );
        }
    }

    setup(mainWindow);
    setupDSP(engine);

    pugi::xpath_node_set objOutlets = ofxVPXml.getObjectOutlets(nId);
    if(!objOutlets.empty()){
        outletsPositions.clear();
        outletsIDs.clear();
        outletsWirelessSend.clear();
        for(auto & outlet: objOutlets){
            auto o = outlet.node();
            outletsType.push_back(ofxVPXml.getPatchChildInt(o,"type"));
            outletsNames.push_back(ofxVPXml.getPatchChildString(o,"name"));
            outletsIDs.push_back("");
            outletsWirelessSend.push_back(false);
            outletsPositions.push_back( ImVec2( this->x + this->width, this->y + this->height*.5f) );
        }
    }

    return true;

}

//--------------------------------------------------------------
bool PatchObject::saveConfig(bool newConnection){

    if(patchFile != ""){

        ofxVPXml.loadMosaicPatch(patchFile);

        // first save of the object
        if(nId == -1){

            nId = ofxVPXml.addNewObject(name,filepath,subpatchName,ofVec2f(static_cast<double>(x),static_cast<double>(y)));

            // Save Custom Vars (GUI, vars, etc...)
            for(map<string,float>::iterator it = customVars.begin(); it != customVars.end(); it++ ){
                ofxVPXml.addObjectVar(nId, it->first, it->second);
            }

            // Save inlets
            for(int i=0;i<static_cast<int>(inletsType.size());i++){
                ofxVPXml.addObjectInlet(nId,inletsType.at(i),inletsNames.at(i));
            }

            // Save oulets & links
            for(int i=0;i<static_cast<int>(outletsType.size());i++){
                ofxVPXml.addObjectOutlet(nId,outletsType.at(i),outletsNames.at(i));
            }



        }else{ // previously saved object
            ofxVPXml.setObjectFilepath(nId, filepath);
            ofxVPXml.setObjectSubpatch(nId, subpatchName);
            ofxVPXml.setObjectPos(nId, ofVec2f(static_cast<double>(x),static_cast<double>(y)));


            // Dynamic reloading custom vars (reconfig capabilities objects, as ShaderObject, etc...)
            ofxVPXml.removeObjectVars(nId);
            ofxVPXml.appendObjectVarsBlock(nId);
            for(map<string,float>::iterator it = customVars.begin(); it != customVars.end(); it++ ){
                ofxVPXml.addObjectVar(nId, it->first, it->second);
            }

            // Dynamic reloading inlets (reconfig capabilities objects, as ShaderObject, etc...)
            ofxVPXml.removeObjectInlets(nId);
            ofxVPXml.appendObjectInletsBlock(nId);
            for(int i=0;i<static_cast<int>(inletsType.size());i++){
                ofxVPXml.addObjectInlet(nId,inletsType.at(i),inletsNames.at(i));
            }

            // Fixed static outlets
            for(int j=0;j<static_cast<int>(outletsType.size());j++){
                if(static_cast<int>(outPut.size()) > 0 && newConnection){
                    int totalTo = ofxVPXml.getObjectLinks(nId,j).size();
                    if(outPut.at(static_cast<int>(outPut.size())-1)->fromOutletID == j){
                        ofxVPXml.addObjectLink(nId, j, outPut.at(static_cast<int>(outPut.size())-1)->toObjectID, outPut.at(static_cast<int>(outPut.size())-1)->toInletID);
                    }
                    if(static_cast<int>(outPut.size())<totalTo){
                        for(int z=totalTo;z>static_cast<int>(outPut.size());z--){
                            ofxVPXml.removeObjectLink(nId, j-1, z);
                        }
                    }
                }
            }

        }

    }

    return true;

}

//--------------------------------------------------------------
bool PatchObject::removeLinkFromConfig(int outlet, int toObjectID, int toInletID){

    if(patchFile != ""){

        ofxVPXml.loadMosaicPatch(patchFile);

        pugi::xpath_node_set links = ofxVPXml.getObjectLinks(nId, outlet);
        int linkToRemove = -1;
        if(!links.empty()){
            int lindex = 0;
            for(auto & link: links){
                auto l = link.node();
                if(ofxVPXml.getPatchChildInt(l,"id") == toObjectID && ofxVPXml.getPatchChildInt(l,"inlet") == toInletID){
                    linkToRemove = lindex;
                }
                lindex++;
            }
        }

        if(linkToRemove != -1){
            ofxVPXml.removeObjectLink(nId,outlet,linkToRemove);
            return true;
        }

    }

    return false;
}

//--------------------------------------------------------------
bool PatchObject::clearCustomVars(){

    if(patchFile != ""){

        ofxVPXml.loadMosaicPatch(patchFile);

        pugi::xpath_node_set vars = ofxVPXml.getObjectVars(nId);
        if(!vars.empty()){
            for(auto & var: vars){
                auto v = var.node();
                std::string vn = ofxVPXml.getPatchChildString(v,"name");
                if(ofIsStringInString(vn,"GUI_")){
                    customVars.erase(vn);
                    ofxVPXml.removeObjectVar(nId, vn);
                }
            }
        }

        return true;

    }

    return false;
}

//--------------------------------------------------------------
map<string,float> PatchObject::loadCustomVars(){
    std::map<std::string,float> tempVars;

    if(patchFile != ""){

        ofxVPXml.loadMosaicPatch(patchFile);

        pugi::xpath_node_set objVars = ofxVPXml.getObjectVars(nId);
        if(!objVars.empty()){
            for(auto & var: objVars){
                auto v = var.node();
                tempVars[ofxVPXml.getPatchChildString(v,"name")] = ofxVPXml.getPatchChildFloat(v,"value");
            }
        }

    }

    return tempVars;
}

//---------------------------------------------------------------------------------- GETTERS
//--------------------------------------------------------------
ofColor PatchObject::getInletColor(const int& iid) const {
    switch( getInletType(iid) ) {
        case 0: return COLOR_NUMERIC;
            break;
        case 1: return COLOR_STRING;
            break;
        case 2: return COLOR_ARRAY;
            break;
        case 3: return COLOR_TEXTURE;
            break;
        case 4: return COLOR_AUDIO;
            break;
        case 5: return COLOR_SCRIPT;
            break;
        case 6: return COLOR_PIXELS;
            break;
        case 7: return COLOR_FBO;
            break;
        default:
            break;
    }
    // Default color
    return COLOR_UNKNOWN;
}

//--------------------------------------------------------------
ofColor PatchObject::getOutletColor(const int& oid) const {
    switch( getOutletType(oid) ) {
        case 0: return COLOR_NUMERIC;
            break;
        case 1: return COLOR_STRING;
            break;
        case 2: return COLOR_ARRAY;
            break;
        case 3: return COLOR_TEXTURE;
            break;
        case 4: return COLOR_AUDIO;
            break;
        case 5: return COLOR_SCRIPT;
            break;
        case 6: return COLOR_PIXELS;
            break;
        case 7: return COLOR_FBO;
            break;
        default:
            break;
    }
    // Default color
    return COLOR_UNKNOWN;
}

//--------------------------------------------------------------
std::string PatchObject::getInletTypeName(const int& iid) const{
    switch( getInletType(iid) ) {
        case 0: return "float";
            break;
        case 1: return "string";
            break;
        case 2: return "vector<float>";
            break;
        case 3: return "ofTexture";
            break;
        case 4: return "ofSoundBuffer";
            break;
        case 5: return specialLinkTypeName;
            break;
        case 6: return "ofPixels";
            break;
        case 7: return "ofFbo";
            break;
        default:
            break;
    }
    // Default type name
    return "";
}

//--------------------------------------------------------------
std::string PatchObject::getOutletTypeName(const int& oid) const{
    switch( getOutletType(oid) ) {
        case 0: return "float";
            break;
        case 1: return "string";
            break;
        case 2: return "vector<float>";
            break;
        case 3: return "ofTexture";
            break;
        case 4: return "ofSoundBuffer";
            break;
        case 5: return specialLinkTypeName;
            break;
        case 6: return "ofPixels";
            break;
        case 7: return "ofFbo";
            break;
        default:
            break;
    }
    // Default type name
    return "";
}

//---------------------------------------------------------------------------------- SETTERS
//--------------------------------------------------------------
void PatchObject::setPatchfile(std::string pf) {
    patchFile = pf;
    ofxVPXml.loadMosaicPatch(patchFile);
    ofFile temp(patchFile);
    patchFolderPath = temp.getEnclosingDirectory()+"data/";
    if(filepath != "none"){
        ofFile t2(filepath);
        if(t2.isDirectory()){
            std::string tst = filepath.substr(0, filepath.size()-1);
            size_t needle = tst.find_last_of("/");
            std::string folderName = filepath.substr(needle+1);
            filepath = patchFolderPath+folderName;
        }else{
            filepath = patchFolderPath+t2.getFileName();
        }

        if(this->getName() == "timeline"){
            this->customReset();
        }

        saveConfig(false);
    }
}

//--------------------------------------------------------------
void PatchObject::keyPressed(ofKeyEventArgs &e,std::map<int,std::shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);
    if(!willErase){

    }
}

//--------------------------------------------------------------
void PatchObject::keyReleased(ofKeyEventArgs &e,std::map<int,std::shared_ptr<PatchObject>> &patchObjects){
    if(!willErase){
        // DELETE SELECTED OBJECTS
        if(e.key == OF_KEY_BACKSPACE){
            for (int j=0;j<static_cast<int>(linksToDisconnect.size());j++){
                disconnectLink(patchObjects,linksToDisconnect.at(j));
            }
            linksToDisconnect.clear();

            for(int j=0;j<static_cast<int>(objectsSelected.size());j++){
                if(objectsSelected.at(j) == this->nId){
                    if(this->getName() != "audio device"){
                        ofNotifyEvent(removeEvent, objectsSelected.at(j));
                        this->setWillErase(true);
                    }
                }
            }
            //objectsSelected.clear();
        // OSX: CMD-D, WIN/LINUX: CTRL-D    (DUPLICATE SELECTED OBJECTS)
        }else if(e.hasModifier(MOD_KEY) && e.keycode == 68){
            for(int j=0;j<static_cast<int>(objectsSelected.size());j++){
                if(objectsSelected.at(j) == this->nId){
                    if(!this->getIsHardwareObject()){
                        ofNotifyEvent(duplicateEvent, objectsSelected.at(j));
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------
void PatchObject::audioIn(ofSoundBuffer &inputBuffer){
    if(isAudioINObject && !willErase){
        audioInObject(inputBuffer);
    }
}

//--------------------------------------------------------------
void PatchObject::audioOut(ofSoundBuffer &outputBuffer){
    if(isAudioOUTObject && !willErase){
        audioOutObject(outputBuffer);
    }
}
