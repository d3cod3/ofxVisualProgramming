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
void PatchObject::setup(shared_ptr<ofAppGLFWWindow> &mainWindow){

    // init vars
    for(int i=0;i<static_cast<int>(inletsType.size());i++){
        inletsPositions.push_back( ImVec2(this->x, this->y + this->height*.5f) );
    }
    for(int i=0;i<static_cast<int>(outletsType.size());i++){
        outletsPositions.push_back( ImVec2(this->x, this->y + this->height*.5f) );
    }

    // previously set with setIsRetina()
    if(isRetina){
        width           *= 2;
        height          *= 2;
        headerHeight    *= 2;
        fontSize         = 16;
        scaleFactor      = 2.0f;
        configMenuWidth  *= 2;
    }

    setupObjectContent(mainWindow);

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
void PatchObject::update(map<int,shared_ptr<PatchObject>> &patchObjects, pdsp::Engine &engine){

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
                            if(this->getIsPDSPPatchableObject() || this->getName() == "audio device"){
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
void PatchObject::updateWirelessLinks(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(willErase) return;

    // Continuosly update float type ONLY wireless links
    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        for(int in=0;in<it->second->getNumInlets();in++){
            for(int out=0;out<this->getNumOutlets();out++){
                if(it->second->getInletWirelessReceive(in) && this->getOutletWirelessSend(out) && this->getOutletType(out) == it->second->getInletType(in) && this->getOutletType(out) == VP_LINK_NUMERIC && this->getOutletID(out) == it->second->getInletID(in)){
                    if(it->second->inletsConnected[in]){
                        it->second->_inletParams[in] = this->_outletParams[out];
                    }
                }
            }
        }
    }

    // manually send data through wireless links ( if var ID, transport data )
    if(initWirelessLink && resetWirelessPin != -1){
        initWirelessLink = false;
        if(this->getOutletWirelessSend(resetWirelessPin)){
            for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
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
        resetWirelessPin = -1;
    }


    // Manually close wireless link from internal object code ( GUI )
    if(resetWirelessLink && resetWirelessPin != -1){
        resetWirelessLink = false;
        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
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
        resetWirelessPin = -1;
    }
}

//--------------------------------------------------------------
void PatchObject::draw(ofTrueTypeFont *font){

    if(willErase) return;

    // Draw the specific object content ()
    drawObjectContent(font,(shared_ptr<ofBaseGLRenderer>&)ofGetCurrentRenderer());

}

//--------------------------------------------------------------
void PatchObject::drawImGuiNode(ImGuiEx::NodeCanvas& _nodeCanvas, map<int,shared_ptr<PatchObject>> &patchObjects){

    if(willErase) return;

    ImVec2 imPos( this->getPos() );
    ImVec2 imSize( this->width, this->height );

    // Begin Node
    string displayName = "";
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
            vector<ImGuiEx::ofxVPLinkData> tempLinkData;

            // if connected, get link origin (outlet origin position and link id)
            if(inletsConnected[i]){
                for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
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

            ImGuiEx::NodeConnectData connectData = _nodeCanvas.AddNodePin( nId, i, inletsNames.at(i).c_str(), tempLinkData, getInletTypeName(i), getInletWirelessReceive(i), inletsConnected[i], IM_COL32(pinCol.r,pinCol.g,pinCol.b,pinCol.a), ImGuiExNodePinsFlags_Left );

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
            vector<ImGuiEx::ofxVPLinkData> tempLinkData;

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

            _nodeCanvas.AddNodePin( nId, i, getOutletName(i).c_str(), tempLinkData, getOutletTypeName(i), getOutletWirelessSend(i), getIsOutletConnected(i), IM_COL32(pinCol.r,pinCol.g,pinCol.b,pinCol.a), ImGuiExNodePinsFlags_Right );

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
        objectsSelected = _nodeCanvas.getSelectedNodes();

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
bool PatchObject::connectTo(map<int,shared_ptr<PatchObject>> &patchObjects, int fromObjectID, int fromOutlet, int toInlet, int linkType){
    bool connected = false;

    if( (fromObjectID != -1) && (patchObjects[fromObjectID] != nullptr) && (fromObjectID!=this->getId()) && (this->getId() != -1) && (patchObjects[fromObjectID]->getOutletType(fromOutlet) == getInletType(toInlet)) && !inletsConnected[toInlet]){

        //cout << "Mosaic :: "<< "Connect object " << getName().c_str() << ":" << ofToString(getId()) << " to object " << getName().c_str() << ":" << ofToString(this->getId()) << endl;

        shared_ptr<PatchLink> tempLink = shared_ptr<PatchLink>(new PatchLink());

        string tmpID = ofToString(fromObjectID)+ofToString(fromOutlet)+ofToString(this->getId())+ofToString(toInlet);

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
            }else if(patchObjects[fromObjectID]->getName() == "audio device" && getIsPDSPPatchableObject()){
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
void PatchObject::disconnectFrom(map<int,shared_ptr<PatchObject>> &patchObjects, int objectInlet){

    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
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

                vector<shared_ptr<PatchLink>> tempBuffer;
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

//--------------------------------------------------------------
void PatchObject::disconnectLink(map<int,shared_ptr<PatchObject>> &patchObjects, int linkID){

    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
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

                vector<shared_ptr<PatchLink>> tempBuffer;
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

//---------------------------------------------------------------------------------- LOAD/SAVE
//--------------------------------------------------------------
bool PatchObject::loadConfig(shared_ptr<ofAppGLFWWindow> &mainWindow, pdsp::Engine &engine,int oTag, string &configFile){
    ofxXmlSettings XML;
    bool loaded = false;


#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
    if (XML.loadFile(configFile)){
#else
    if (XML.load(configFile)){
#endif

        patchFile = configFile;

        if(XML.pushTag("object", oTag)){

            nId = XML.getValue("id", 0);
            name = XML.getValue("name","none");
            filepath = XML.getValue("filepath","none");
            subpatchName = XML.getValue("subpatch","root");

            move(XML.getValue("position:x", 0),XML.getValue("position:y", 0));

            if(XML.pushTag("vars")){
                int totalCustomVars = XML.getNumTags("var");
                for (int i=0;i<totalCustomVars;i++){
                    if(XML.pushTag("var",i)){
                        customVars[XML.getValue("name", "")] = XML.getValue("value", 0.0);
                        XML.popTag();
                    }
                }
                XML.popTag();
            }

            if(XML.pushTag("inlets")){
                int totalInlets = XML.getNumTags("link");
                inletsPositions.clear();
                inletsIDs.clear();
                inletsWirelessReceive.clear();
                for (int i=0;i<totalInlets;i++){
                    if(XML.pushTag("link",i)){
                        inletsType.push_back(XML.getValue("type", 0));
                        inletsNames.push_back(XML.getValue("name", ""));
                        inletsIDs.push_back("");
                        inletsWirelessReceive.push_back(false);
                        inletsPositions.push_back( ImVec2(this->x, this->y + this->height*.5f) );
                        XML.popTag();
                    }
                }
                XML.popTag();
            }

            setup(mainWindow);
            setupDSP(engine);

            if(XML.pushTag("outlets")){
                int totalOutlets = XML.getNumTags("link");
                outletsPositions.clear();
                outletsIDs.clear();
                outletsWirelessSend.clear();
                for (int i=0;i<totalOutlets;i++){
                    if(XML.pushTag("link",i)){
                        outletsType.push_back(XML.getValue("type", 0));
                        outletsNames.push_back(XML.getValue("name", ""));
                        outletsIDs.push_back("");
                        outletsWirelessSend.push_back(false);
                        outletsPositions.push_back( ImVec2( this->x + this->width, this->y + this->height*.5f) );
                        XML.popTag();
                    }
                }
                XML.popTag();
            }

            XML.popTag(); // if(XML.pushTag("object", oTag))

            loaded = true;

        }
    }

    return loaded;

}

//--------------------------------------------------------------
bool PatchObject::saveConfig(bool newConnection){
    ofxXmlSettings XML;
    bool saved = false;

    if(patchFile != ""){
#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
        if (XML.loadFile(patchFile)){
#else
        if (XML.load(patchFile)){
#endif
            int totalObjects = XML.getNumTags("object");
            // first save of the object
            if(nId == -1){
                int freeId = 0;
                int maxId = 0;
                for (int i=0;i<totalObjects;i++){
                    if(XML.pushTag("object", i)){
                        if(XML.getValue("id",-1) > maxId){
                            maxId = XML.getValue("id",-1);
                        }
                        XML.popTag();
                    }
                }

                freeId = maxId+1;

                if(freeId >= 0){
                    nId = freeId;
                    int newObject = XML.addTag("object");

                    if(XML.pushTag("object",newObject)){
                        XML.addTag("id");
                        XML.setValue("id",nId);
                        XML.addTag("name");
                        XML.setValue("name",name);
                        XML.addTag("filepath");
                        XML.setValue("filepath",filepath);
                        XML.addTag("subpatch");
                        XML.setValue("subpatch",subpatchName);
                        XML.addTag("position");
                        XML.setValue("position:x",static_cast<double>(x));
                        XML.setValue("position:y",static_cast<double>(y));

                        // Save Custom Vars (GUI, vars, etc...)
                        int newCustomVars = XML.addTag("vars");
                        if(XML.pushTag("vars",newCustomVars)){
                            for(map<string,float>::iterator it = customVars.begin(); it != customVars.end(); it++ ){
                                int newVar = XML.addTag("var");
                                if(XML.pushTag("var",newVar)){
                                    XML.setValue("name",it->first);
                                    XML.setValue("value",it->second);
                                    XML.popTag();
                                }
                            }
                            XML.popTag();
                        }

                        // Save inlets
                        int newInlets = XML.addTag("inlets");
                        if(XML.pushTag("inlets",newInlets)){
                            for(int i=0;i<static_cast<int>(inletsType.size());i++){
                                int newLink = XML.addTag("link");
                                if(XML.pushTag("link",newLink)){
                                    XML.setValue("type",inletsType.at(i));
                                    XML.setValue("name",inletsNames.at(i));
                                    XML.popTag();
                                }
                            }
                            XML.popTag();
                        }

                        // Save oulets & links
                        int newOutlets = XML.addTag("outlets");
                        if(XML.pushTag("outlets",newOutlets)){
                            for(int i=0;i<static_cast<int>(outletsType.size());i++){
                                int newLink = XML.addTag("link");
                                if(XML.pushTag("link",newLink)){
                                    XML.setValue("type",outletsType.at(i));
                                    XML.setValue("name",outletsNames.at(i));
                                    XML.popTag();
                                }
                            }
                            XML.popTag();
                        }

                        XML.popTag();
                    }
                }

            }else{ // object previously saved
                for(int i=0;i<totalObjects;i++){
                    if(XML.pushTag("object", i)){
                        if(XML.getValue("id", -1) == nId){
                            XML.setValue("filepath",filepath);
                            XML.setValue("subpatch",subpatchName);
                            XML.setValue("position:x",static_cast<double>(x));
                            XML.setValue("position:y",static_cast<double>(y));

                            // Dynamic reloading custom vars (reconfig capabilities objects, as ShaderObject, etc...)
                            XML.removeTag("vars");
                            int newCustomVars = XML.addTag("vars");
                            if(XML.pushTag("vars",newCustomVars)){
                                for(map<string,float>::iterator it = customVars.begin(); it != customVars.end(); it++ ){
                                    int newLink = XML.addTag("var");
                                    if(XML.pushTag("var",newLink)){
                                        XML.setValue("name",it->first);
                                        XML.setValue("value",it->second);
                                        XML.popTag();
                                    }
                                }
                                XML.popTag();
                            }

                            // Dynamic reloading inlets (reconfig capabilities objects, as ShaderObject, etc...)
                            XML.removeTag("inlets");
                            int newInlets = XML.addTag("inlets");
                            if(XML.pushTag("inlets",newInlets)){
                                for(int i=0;i<static_cast<int>(inletsType.size());i++){
                                    int newLink = XML.addTag("link");
                                    if(XML.pushTag("link",newLink)){
                                        XML.setValue("type",inletsType.at(i));
                                        XML.setValue("name",inletsNames.at(i));
                                        XML.popTag();
                                    }
                                }
                                XML.popTag();
                            }

                            // Fixed static outlets
                            if(XML.pushTag("outlets")){
                                for(int j=0;j<static_cast<int>(outletsType.size());j++){
                                    if(XML.pushTag("link", j)){
                                        if(static_cast<int>(outPut.size()) > 0 && newConnection){
                                            int totalTo = XML.getNumTags("to");
                                            if(outPut.at(static_cast<int>(outPut.size())-1)->fromOutletID == j){
                                                int newTo = XML.addTag("to");
                                                if(XML.pushTag("to", newTo)){
                                                    XML.setValue("id",outPut.at(static_cast<int>(outPut.size())-1)->toObjectID);
                                                    XML.setValue("inlet",outPut.at(static_cast<int>(outPut.size())-1)->toInletID);
                                                    XML.popTag();
                                                }
                                            }
                                            if(static_cast<int>(outPut.size())<totalTo){
                                                for(int z=totalTo;z>static_cast<int>(outPut.size());z--){
                                                    XML.removeTag("to",j-1);
                                                }
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
            }
#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
            saved = XML.saveFile();
#else
            saved = XML.save();
#endif
        }
    }

    return saved;

}

//--------------------------------------------------------------
bool PatchObject::removeLinkFromConfig(int outlet, int toObjectID, int toInletID){
    ofxXmlSettings XML;
    bool saved = false;

    if(patchFile != ""){
#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
        if (XML.loadFile(patchFile)){
#else
        if (XML.load(patchFile)){
#endif
            int totalObjects = XML.getNumTags("object");
            for(int i=0;i<totalObjects;i++){
                if(XML.pushTag("object", i)){
                    if(XML.getValue("id", -1) == nId){
                        if(XML.pushTag("outlets")){
                            if(XML.pushTag("link", outlet)){
                                int totalTo = XML.getNumTags("to");
                                int linkToRemove = -1;
                                for(int z=0;z<totalTo;z++){
                                    if(XML.pushTag("to", z)){
                                        if(XML.getValue("id", -1) == toObjectID && XML.getValue("inlet", -1) == toInletID){
                                            linkToRemove = z;
                                        }
                                        XML.popTag();
                                    }
                                }
                                if(linkToRemove != -1){
                                    XML.removeTag("to",linkToRemove);
                                }
                                XML.popTag();
                            }
                            XML.popTag();
                        }
                    }
                    XML.popTag();
                }
            }
        }

#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
            saved = XML.saveFile();
#else
            saved = XML.save();
#endif
    }

    return saved;
}

//--------------------------------------------------------------
bool PatchObject::clearCustomVars(){
    ofxXmlSettings XML;
    bool saved = false;

    if(patchFile != ""){
#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
        if (XML.loadFile(patchFile)){
#else
        if (XML.load(patchFile)){
#endif
            int totalObjects = XML.getNumTags("object");
            for(int i=0;i<totalObjects;i++){
                if(XML.pushTag("object", i)){
                    if(XML.getValue("id", -1) == nId){
                        if(XML.pushTag("vars")){
                            int numVars = XML.getNumTags("var");
                            vector<bool> needErase;
                            for(int v=0;v<numVars;v++){
                                if(XML.pushTag("var",v)){
                                    if(ofIsStringInString(XML.getValue("name",""),"GUI_")){
                                        needErase.push_back(true);
                                        customVars.erase(XML.getValue("name",""));
                                        //ofLog(OF_LOG_NOTICE,"Removing var: %s",XML.getValue("name","").c_str());
                                    }else{
                                        needErase.push_back(false);
                                    }
                                    XML.popTag();
                                }
                            }
                            for(size_t r=0;r<needErase.size();r++){
                                if(needErase.at(r)){
                                    XML.removeTag("var",static_cast<int>(r));

                                }
                            }

                            XML.popTag();
                        }
                    }
                    XML.popTag();
                }
            }
        }

#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
            saved = XML.saveFile();
#else
            saved = XML.save();
#endif

    }

    return saved;
}

//--------------------------------------------------------------
map<string,float> PatchObject::loadCustomVars(){
    map<string,float> tempVars;

    ofxXmlSettings XML;

    if(patchFile != ""){
#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
        if (XML.loadFile(patchFile)){
#else
        if (XML.load(patchFile)){
#endif
            int totalObjects = XML.getNumTags("object");
            for(int i=0;i<totalObjects;i++){
                if(XML.pushTag("object", i)){
                    if(XML.getValue("id", -1) == nId){
                        if(XML.pushTag("vars")){
                            int numVars = XML.getNumTags("var");
                            for(int v=0;v<numVars;v++){
                                if(XML.pushTag("var",v)){
                                    tempVars[XML.getValue("name","")] = XML.getValue("value",0.0f);
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
string PatchObject::getInletTypeName(const int& iid) const{
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
string PatchObject::getOutletTypeName(const int& oid) const{
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
void PatchObject::setPatchfile(string pf) {
    patchFile = pf;
    ofFile temp(patchFile);
    patchFolderPath = temp.getEnclosingDirectory()+"data/";
    if(filepath != "none"){
        ofFile t2(filepath);
        if(t2.isDirectory()){
            string tst = filepath.substr(0, filepath.size()-1);
            size_t needle = tst.find_last_of("/");
            string folderName = filepath.substr(needle+1);
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
void PatchObject::keyPressed(ofKeyEventArgs &e,map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);
    if(!willErase){

    }
}

//--------------------------------------------------------------
void PatchObject::keyReleased(ofKeyEventArgs &e,map<int,shared_ptr<PatchObject>> &patchObjects){
    if(!willErase){
        if(e.key == OF_KEY_BACKSPACE){
            for (int j=0;j<static_cast<int>(linksToDisconnect.size());j++){
                disconnectLink(patchObjects,linksToDisconnect.at(j));
            }
            linksToDisconnect.clear();

            for(int j=0;j<static_cast<int>(objectsSelected.size());j++){
                if(objectsSelected.at(j) == this->nId){
                    ofNotifyEvent(removeEvent, objectsSelected.at(j));
                    this->setWillErase(true);
                }
            }
            objectsSelected.clear();
        // OSX: CMD-D, WIN/LINUX: CTRL-D    (DUPLICATE SELECTED OBJECTS)
        }else if(e.hasModifier(MOD_KEY) && e.keycode == 68){
            for(int j=0;j<static_cast<int>(objectsSelected.size());j++){
                if(objectsSelected.at(j) == this->nId){
                    ofNotifyEvent(duplicateEvent, objectsSelected.at(j));
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
