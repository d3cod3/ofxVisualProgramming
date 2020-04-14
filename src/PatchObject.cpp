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
    nId             = -1;
    name            = "none";
    filepath        = "none";
    patchFile       = "";
    patchFolderPath = "";

    linkTypeName        = "";
    specialLinkTypeName = "";

    numInlets   = 0;
    numOutlets  = 0;

    isSystemObject          = false;
    bActive                 = false;
    isMouseOver             = false;
    isObjectSelected        = false;
    isOverGUI               = false;
    isRetina                = false;
    isGUIObject             = false;
    isBigGuiViewer          = false;
    isBigGuiComment         = false;
    isAudioINObject         = false;
    isAudioOUTObject        = false;
    isPDSPPatchableObject   = false;
    isResizable             = false;
    willErase               = false;

    width       = OBJECT_WIDTH;
    height      = OBJECT_HEIGHT;
    headerHeight= HEADER_HEIGHT;
    x           = 0.0f;
    y           = 0.0f;
    letterWidth = 12;
    letterHeight= 11;
    offSetWidth = 5;
    fontSize    = 12;
    buttonOffset= 10;

    retinaScale = 1.0f;

    // Dynamically allocate pointers
    // so we'll be able to call delete on them
    box                 = new ofRectangle();
    headerBox           = new ofRectangle();

    output_width        = 320;
    output_height       = 240;

}

//--------------------------------------------------------------
PatchObject::~PatchObject(){

    // free memory and point lost pointer to null
    delete box;
    delete headerBox;

    box = nullptr;
    headerBox = nullptr;
}

//--------------------------------------------------------------
void PatchObject::setup(shared_ptr<ofAppGLFWWindow> &mainWindow){

    if(isRetina){
        width           *= 2;
        height          *= 2;
        headerHeight    *= 2;
        letterWidth     *= 2;
        letterHeight    *= 2;
        buttonOffset    *= 2;
        fontSize         = 16;
        retinaScale      = 2.0f;
    }

    box->set(x,y,width,height);
    headerBox->set(x,y,width,headerHeight);

    inletsMouseNear.assign(MAX_OUTLETS,false);

    setupObjectContent(mainWindow);

}

//--------------------------------------------------------------
void PatchObject::setupDSP(pdsp::Engine &engine){
    if(this->isPDSPPatchableObject){
        for(int i=0;i<static_cast<int>(inlets.size());i++){
            int it = getInletType(i);
            if(it == 4){ // VP_LINK_AUDIO
                pdsp::PatchNode *temp = new pdsp::PatchNode();
                this->pdspIn[i] = *temp;
            }
        }
        for(int i=0;i<static_cast<int>(outlets.size());i++){
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
void PatchObject::update(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    if(willErase) return;

    // update links
    for(int out=0;out<getNumOutlets();out++){
        for(int i=0;i<static_cast<int>(outPut.size());i++){
            if(!outPut[i]->isDisabled && outPut[i]->fromOutletID == out && patchObjects[outPut[i]->toObjectID]!=nullptr && !patchObjects[outPut[i]->toObjectID]->getWillErase()){
                outPut[i]->posFrom = getOutletPosition(out);
                outPut[i]->posTo = patchObjects[outPut[i]->toObjectID]->getInletPosition(outPut[i]->toInletID);
                outPut[i]->linkVertices[0].set(outPut[i]->posFrom.x,outPut[i]->posFrom.y);
                outPut[i]->linkVertices[1].set(outPut[i]->posTo.x,outPut[i]->posTo.y);
                // send data through links
                patchObjects[outPut[i]->toObjectID]->_inletParams[outPut[i]->toInletID] = _outletParams[out];
            }
        }
    }
    updateObjectContent(patchObjects,fd);

}

//--------------------------------------------------------------
void PatchObject::draw(ofxFontStash *font){

    if(willErase) return;

    // Draw the specific object content ()
    ofPushStyle();
    ofPushMatrix();
    ofTranslate(box->getPosition().x,box->getPosition().y);
    drawObjectContent(font,(shared_ptr<ofBaseGLRenderer>&)ofGetCurrentRenderer());
    ofPopMatrix();
    ofPopStyle();
}

//--------------------------------------------------------------
void PatchObject::drawImGuiNode(ImGuiEx::NodeCanvas& _nodeCanvas, map<int,shared_ptr<PatchObject>> &patchObjects){

    if(willErase) return;

    ImVec2 imPos( this->getPos() );
    ImVec2 imSize( this->width, this->height );
    if(_nodeCanvas.BeginNode( PatchObject::getUID().c_str(), imPos, imSize, this->getNumInlets(), this->getNumOutlets(), this->getIsResizable() )){

        // Check menu state
        if( _nodeCanvas.doNodeMenuAction(ImGuiExNodeMenuActionFlags_DeleteNode) ){
            ofNotifyEvent(removeEvent, nId);
            this->setWillErase(true);

            // todo: tell ImGui explicitly to close/remove window instance ? This doesn't seem to be available (yet).
        }
        else if( _nodeCanvas.doNodeMenuAction(ImGuiExNodeMenuActionFlags_CopyNode) ){
        //          ofGetWindowPtr()->setClipboardString( this->serialize() );
        }
        else if( _nodeCanvas.doNodeMenuAction(ImGuiExNodeMenuActionFlags_DuplicateNode) ){
            ofNotifyEvent(duplicateEvent, nId);
        }

        // Inlets
        for(int i=0;i<static_cast<int>(inlets.size());i++){
            auto pinCol = getInletColor(i);
            vector<ImGuiEx::ofxVPLinkData> tempLinkData;

            // if connected, get link origin (outlet origin position and link id)
            if(inletsConnected[i]){
                for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
                    for(int j=0;j<static_cast<int>(it->second->outPut.size());j++){
                        if(it->second->outPut[j]->toObjectID == nId && it->second->outPut[j]->toInletID == i){
                            ImGuiEx::ofxVPLinkData tvpld;
                            tvpld._toPinPosition = ImVec2(it->second->outPut[j]->linkVertices[0].x,it->second->outPut[j]->linkVertices[0].y);
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

            ImGuiEx::NodeConnectData connectData = _nodeCanvas.AddNodePin( nId, i, inletsNames.at(i).c_str(), inletsPositions[0], tempLinkData, getInletTypeName(i), inletsConnected[i], IM_COL32(pinCol.r,pinCol.g,pinCol.b,pinCol.a), ImGuiExNodePinsFlags_Left );

            // check for inbound connections
            if(connectData.connectType == 1){
                //cout << "Connect object " << nId << " from outlet " << i << " to object " << connectData.toObjectID << " at inlet " << connectData.toInletPinID << endl;
                // if previously connected, disconnect and refresh connection
                if(patchObjects[connectData.toObjectID]->inletsConnected.at(connectData.toInletPinID)){
                    // Disconnect from --> inlet link
                    disconnectFrom(patchObjects,connectData.toObjectID,connectData.toInletPinID);
                }
                // if compatible type, connect
                if(getInletType(connectData.toInletPinID) == patchObjects[connectData.fromObjectID]->getOutletType(connectData.fromOutletPinID)){
                    connectTo(patchObjects,connectData.fromObjectID,connectData.fromOutletPinID,connectData.toObjectID,connectData.toInletPinID,getOutletType(connectData.fromOutletPinID));
                    saveConfig(true,connectData.fromObjectID);
                }
            }else if(connectData.connectType == 2){
                // disconnect from elsewhere ( if another object have this connection )
                if(patchObjects[connectData.toObjectID]->inletsConnected.at(connectData.toInletPinID)){
                    disconnectFrom(patchObjects,connectData.toObjectID,connectData.toInletPinID);
                }
                // disconnect previous link
                disconnectLink(patchObjects,connectData.linkID);
                // if compatible type, connect
                if(getInletType(connectData.toInletPinID) == patchObjects[connectData.fromObjectID]->getOutletType(connectData.fromOutletPinID)){
                    connectTo(patchObjects,connectData.fromObjectID,connectData.fromOutletPinID,connectData.toObjectID,connectData.toInletPinID,getOutletType(connectData.fromOutletPinID));
                    saveConfig(true,connectData.fromObjectID);
                }

            }else if(connectData.connectType == 3){
                // disconnect link
                disconnectLink(patchObjects,connectData.linkID);
            }

            inletsPositionOF[i] = ofVec2f((inletsPositions[0].x - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale(), (inletsPositions[0].y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale());
        }

        // Outlets
        for(int i=0;i<static_cast<int>(outlets.size());i++){
            auto pinCol = getOutletColor(i);

            // links
            vector<ImGuiEx::ofxVPLinkData> tempLinkData;

            for(int j=0;j<static_cast<int>(outPut.size());j++){
                if(!outPut[j]->isDisabled && outPut[j]->fromOutletID == i){
                    ImGuiEx::ofxVPLinkData tvpld;
                    tvpld._toPinPosition = ImVec2(outPut[j]->linkVertices[1].x,outPut[j]->linkVertices[1].y);
                    tvpld._linkID = outPut[j]->id;
                    tvpld._linkLabel = getOutletName(outPut[j]->fromOutletID);
                    tvpld._fromObjectID = nId;
                    tvpld._fromPinID = outPut[j]->fromOutletID;

                    tempLinkData.push_back(tvpld);

                }
            }

            _nodeCanvas.AddNodePin( nId, i, getOutletName(i).c_str(), outletsPositions[0], tempLinkData, getOutletTypeName(i), getIsOutletConnected(i), IM_COL32(pinCol.r,pinCol.g,pinCol.b,pinCol.a), ImGuiExNodePinsFlags_Right );

            outletsPositionOF[i] = ofVec2f((outletsPositions[0].x - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale(), (outletsPositions[0].y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale());

        }

        // Refresh links to eventually disconnect (backspace key or right click menu)
        linksToDisconnect = _nodeCanvas.getSelectedLinks();

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

    box->setPosition(this->x,this->y);
    headerBox->setPosition(this->x,this->y);

}

//--------------------------------------------------------------
void PatchObject::move(int _x, int _y){
    int px = _x;
    int py = _y;
    if(isRetina){
        px *= 2;
        py *= 2;
    }

    x = px;
    y = py;

    box->setPosition(px,py);
    headerBox->setPosition(px,py);
}

//--------------------------------------------------------------
bool PatchObject::isOver(ofPoint pos){
    return box->inside(pos) || isOverGUI;
}

//--------------------------------------------------------------
void PatchObject::fixCollisions(map<int,shared_ptr<PatchObject>> &patchObjects){

    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->first != getId()){
            if(getPos().x >= it->second->getPos().x && getPos().x < it->second->getPos().x + it->second->getObjectWidth() && getPos().y >= it->second->getPos().y-it->second->getObjectHeight() && getPos().y < it->second->getPos().y+it->second->getObjectHeight()){
                if(isRetina){
                    move((it->second->getPos().x+it->second->getObjectWidth()+20)/2,getPos().y/2);
                }else{
                    move(it->second->getPos().x+it->second->getObjectWidth()+10,getPos().y);
                }
                break;
            }else if(getPos().x+getObjectWidth() >= it->second->getPos().x && getPos().x+getObjectWidth() < it->second->getPos().x+it->second->getObjectWidth() && getPos().y >= it->second->getPos().y-it->second->getObjectHeight() && getPos().y < it->second->getPos().y+it->second->getObjectHeight()){
                if(isRetina){
                    move((it->second->getPos().x-getObjectWidth()-20)/2,getPos().y/2);
                }else{
                    move(it->second->getPos().x-getObjectWidth()-10,getPos().y);
                }
                break;
            }
        }
    }

}

//--------------------------------------------------------------
ofVec2f PatchObject::getInletPosition(int iid){
    return inletsPositionOF[iid];
}

//--------------------------------------------------------------
ofVec2f PatchObject::getOutletPosition(int oid){
    return outletsPositionOF[oid];
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
bool PatchObject::connectTo(map<int,shared_ptr<PatchObject>> &patchObjects, int fromObjectID, int fromOutlet, int toObjectID,int toInlet, int linkType){
    bool connected = false;

    if( (fromObjectID != -1) && (patchObjects[fromObjectID] != nullptr) && (fromObjectID!=toObjectID) && (toObjectID != -1) && (patchObjects[toObjectID] != nullptr) && (patchObjects[fromObjectID]->getOutletType(fromOutlet) == patchObjects[toObjectID]->getInletType(toInlet)) && !patchObjects[toObjectID]->inletsConnected[toInlet]){

        //cout << "Mosaic :: "<< "Connect object " << getName().c_str() << ":" << ofToString(getId()) << " to object " << patchObjects[toObjectID]->getName().c_str() << ":" << ofToString(toObjectID) << endl;

        shared_ptr<PatchLink> tempLink = shared_ptr<PatchLink>(new PatchLink());

        string tmpID = ofToString(fromObjectID)+ofToString(fromOutlet)+ofToString(toObjectID)+ofToString(toInlet);

        tempLink->id            = stoi(tmpID);
        tempLink->posFrom       = patchObjects[fromObjectID]->getOutletPosition(fromOutlet);
        tempLink->posTo         = patchObjects[toObjectID]->getInletPosition(toInlet);
        tempLink->type          = patchObjects[toObjectID]->getInletType(toInlet);
        tempLink->fromOutletID  = fromOutlet;
        tempLink->toObjectID    = toObjectID;
        tempLink->toInletID     = toInlet;
        tempLink->isDisabled    = false;

        tempLink->linkVertices.push_back(ofVec2f(tempLink->posFrom.x,tempLink->posFrom.y));
        tempLink->linkVertices.push_back(ofVec2f(tempLink->posTo.x,tempLink->posTo.y));

        patchObjects[fromObjectID]->outPut.push_back(tempLink);

        patchObjects[toObjectID]->inletsConnected[toInlet] = true;

        if(tempLink->type == VP_LINK_NUMERIC){
            patchObjects[toObjectID]->_inletParams[toInlet] = new float();
        }else if(tempLink->type == VP_LINK_STRING){
            patchObjects[toObjectID]->_inletParams[toInlet] = new string();
        }else if(tempLink->type == VP_LINK_ARRAY){
            patchObjects[toObjectID]->_inletParams[toInlet] = new vector<float>();
        }else if(tempLink->type == VP_LINK_PIXELS){
            patchObjects[toObjectID]->_inletParams[toInlet] = new ofPixels();
        }else if(tempLink->type == VP_LINK_TEXTURE){
            patchObjects[toObjectID]->_inletParams[toInlet] = new ofTexture();
        }else if(tempLink->type == VP_LINK_AUDIO){
            patchObjects[toObjectID]->_inletParams[toInlet] = new ofSoundBuffer();
            if(patchObjects[fromObjectID]->getIsPDSPPatchableObject() && patchObjects[toObjectID]->getIsPDSPPatchableObject()){
                patchObjects[fromObjectID]->pdspOut[fromOutlet] >> patchObjects[toObjectID]->pdspIn[toInlet];
            }else if(patchObjects[fromObjectID]->getName() == "audio device" && patchObjects[toObjectID]->getIsPDSPPatchableObject()){
                patchObjects[fromObjectID]->pdspOut[fromOutlet] >> patchObjects[toObjectID]->pdspIn[toInlet];
            }
        }

        // check special connections
        if(getName() == "lua script"){
            if((patchObjects[toObjectID]->getName() == "shader object" || patchObjects[toObjectID]->getName() == "output window") && linkType == VP_LINK_TEXTURE){
                patchObjects[fromObjectID]->resetResolution(toObjectID,patchObjects[toObjectID]->getOutputWidth(),patchObjects[toObjectID]->getOutputHeight());
            }
        }
        if(getName() == "shader object"){
            if(patchObjects[toObjectID]->getName() == "output window" && linkType == VP_LINK_TEXTURE){
                patchObjects[fromObjectID]->resetResolution(toObjectID,patchObjects[toObjectID]->getOutputWidth(),patchObjects[toObjectID]->getOutputHeight());
            }
        }

        connected = true;
    }

    return connected;
}

//--------------------------------------------------------------
void PatchObject::disconnectFrom(map<int,shared_ptr<PatchObject>> &patchObjects, int objectID, int objectInlet){

    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        for(int j=0;j<static_cast<int>(it->second->outPut.size());j++){
            if(it->second->outPut[j]->toObjectID == objectID && it->second->outPut[j]->toInletID == objectInlet){
                // remove link
                vector<bool> tempEraseLinks;
                for(int s=0;s<static_cast<int>(it->second->outPut.size());s++){
                    if(it->second->outPut[s]->toObjectID == objectID && it->second->outPut[s]->toInletID == objectInlet){
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
                        it->second->removeLinkFromConfig(it->second->outPut[s]->fromOutletID);
                        if(patchObjects[objectID] != nullptr){
                            patchObjects[objectID]->inletsConnected[objectInlet] = false;
                            if(patchObjects[objectID]->getIsPDSPPatchableObject()){
                                patchObjects[objectID]->pdspIn[objectInlet].disconnectIn();
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
                        it->second->removeLinkFromConfig(it->second->outPut[s]->fromOutletID);
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

    if (XML.loadFile(configFile)){

        patchFile = configFile;

        if(XML.pushTag("object", oTag)){

            nId = XML.getValue("id", 0);
            name = XML.getValue("name","none");
            filepath = XML.getValue("filepath","none");

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
                for (int i=0;i<totalInlets;i++){
                    if(XML.pushTag("link",i)){
                        inlets.push_back(XML.getValue("type", 0));
                        inletsNames.push_back(XML.getValue("name", ""));
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
                for (int i=0;i<totalOutlets;i++){
                    if(XML.pushTag("link",i)){
                        outlets.push_back(XML.getValue("type", 0));
                        outletsNames.push_back(XML.getValue("name", ""));
                        outletsPositions.push_back( ImVec2( this->x + this->width, this->y + this->height*.5f) );
                        XML.popTag();
                    }
                }
                XML.popTag();
            }

            loaded = true;

        }
    }

    return loaded;

}

//--------------------------------------------------------------
bool PatchObject::saveConfig(bool newConnection,int objID){
    ofxXmlSettings XML;
    bool saved = false;

    if(patchFile != ""){
        if(XML.loadFile(patchFile)){
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
                //freeId = objID;

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
                            for(int i=0;i<static_cast<int>(inlets.size());i++){
                                int newLink = XML.addTag("link");
                                if(XML.pushTag("link",newLink)){
                                    XML.setValue("type",inlets.at(i));
                                    XML.setValue("name",inletsNames.at(i));
                                    XML.popTag();
                                }
                            }
                            XML.popTag();
                        }

                        // Save oulets & links
                        int newOutlets = XML.addTag("outlets");
                        if(XML.pushTag("outlets",newOutlets)){
                            for(int i=0;i<static_cast<int>(outlets.size());i++){
                                int newLink = XML.addTag("link");
                                if(XML.pushTag("link",newLink)){
                                    XML.setValue("type",outlets.at(i));
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
                                for(int i=0;i<static_cast<int>(inlets.size());i++){
                                    int newLink = XML.addTag("link");
                                    if(XML.pushTag("link",newLink)){
                                        XML.setValue("type",inlets.at(i));
                                        XML.setValue("name",inletsNames.at(i));
                                        XML.popTag();
                                    }
                                }
                                XML.popTag();
                            }

                            // Fixed static outlets
                            if(XML.pushTag("outlets")){
                                for(int j=0;j<static_cast<int>(outlets.size());j++){
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
            saved = XML.saveFile();
        }
    }

    return saved;

}

//--------------------------------------------------------------
bool PatchObject::removeLinkFromConfig(int outlet){
    ofxXmlSettings XML;
    bool saved = false;

    if(patchFile != ""){
        if(XML.loadFile(patchFile)){
            int totalObjects = XML.getNumTags("object");
            for(int i=0;i<totalObjects;i++){
                if(XML.pushTag("object", i)){
                    if(XML.getValue("id", -1) == nId){
                        if(XML.pushTag("outlets")){
                            if(XML.pushTag("link", outlet)){
                                int totalTo = XML.getNumTags("to");
                                for(int z=0;z<totalTo;z++){
                                    XML.removeTag("to",z);
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

        saved = XML.saveFile();
    }

    return saved;
}

//--------------------------------------------------------------
bool PatchObject::clearCustomVars(){
    ofxXmlSettings XML;
    bool saved = false;

    if(patchFile != ""){
        if(XML.loadFile(patchFile)){
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

        saved = XML.saveFile();

    }

    return saved;
}

//--------------------------------------------------------------
map<string,float> PatchObject::loadCustomVars(){
    map<string,float> tempVars;

    ofxXmlSettings XML;

    if(patchFile != ""){
        if(XML.loadFile(patchFile)){
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

        saveConfig(false,nId);
    }
}

//---------------------------------------------------------------------------------- MOUSE EVENTS
//--------------------------------------------------------------
void PatchObject::mouseMoved(float mx, float my){
    /*if(!willErase){
        ofVec3f m = ofVec3f(mx, my,0);
        mouseMovedObjectContent(m);
        if(!isGUIObject){
            if(isOver(m) && bActive){
                isMouseOver = true;
            }else{
                isMouseOver = false;
            }
        }else{
            if((isOver(m) || isOverGUI) && bActive){
                isMouseOver = true;
            }else{
                isMouseOver = false;
            }
        }

    }*/
}

//--------------------------------------------------------------
void PatchObject::mouseDragged(float mx, float my){
    /*if(!willErase){
        ofVec3f m = ofVec3f(mx,my,0);
        if (isMouseOver && !isGUIObject){


            box->setFromCenter(m.x, m.y,box->getWidth(),box->getHeight());
            headerBox->set(box->getPosition().x,box->getPosition().y,box->getWidth(),headerHeight);

            x = box->getPosition().x;
            y = box->getPosition().y;

            for(int j=0;j<static_cast<int>(outPut.size());j++){
                outPut[j]->linkVertices[0].set(outPut[j]->posFrom.x,outPut[j]->posFrom.y);
            }
        }else if(isMouseOver && isGUIObject){
            dragGUIObject(m);
        }

    }*/

}

//--------------------------------------------------------------
void PatchObject::mousePressed(float mx, float my){
    /*if(!willErase){
        ofVec3f m = ofVec3f(mx, my,0);
        if(box->inside(m)){
            mousePressedObjectContent(m);
        }
    }*/
}

//--------------------------------------------------------------
void PatchObject::mouseReleased(float mx, float my,map<int,shared_ptr<PatchObject>> &patchObjects){
    /*if(!willErase){
        ofVec3f m = ofVec3f(mx, my,0);

        if (box->inside(m)){
            mouseReleasedObjectContent(m);

            //x = box->getPosition().x;
            //y = box->getPosition().y;

            fixCollisions(patchObjects);

            saveConfig(false,nId);
        }

        for(size_t m=0;m<inletsMouseNear.size();m++){
            inletsMouseNear.at(m) = false;
        }

    }*/

}

//--------------------------------------------------------------
void PatchObject::keyPressed(int key,map<int,shared_ptr<PatchObject>> &patchObjects){
    if(!willErase){
        if(isMouseOver){
            keyPressedObjectContent(key);
        }
        if(key == OF_KEY_BACKSPACE){
            for (int j=0;j<linksToDisconnect.size();j++){
                disconnectLink(patchObjects,linksToDisconnect.at(j));
            }
            linksToDisconnect.clear();
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


