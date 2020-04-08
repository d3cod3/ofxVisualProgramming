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
    iconified               = false;
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
    color               = new ofColor(55,55,55);

    output_width        = 320;
    output_height       = 240;

}

//--------------------------------------------------------------
PatchObject::~PatchObject(){

    // free memory and point lost pointer to null
    for(int i=0;i<static_cast<int>(headerButtons.size());i++){
        delete headerButtons.at(i);
        headerButtons.at(i) = nullptr;
    }

    delete box;
    delete headerBox;
    delete color;

    box = nullptr;
    headerBox = nullptr;
    color = nullptr;
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
    addButton('x', nullptr,buttonOffset);
    addButton('d', nullptr,buttonOffset);

    inletsMouseNear.assign(MAX_OUTLETS,false);

    inletsPositionOF.assign(inlets.size(),ofVec2f(0));
    outletsPositionOF.assign(outlets.size(),ofVec2f(0));

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

    // update links positions
    if(!willErase){
        for(int out=0;out<getNumOutlets();out++){
            for(int i=0;i<static_cast<int>(outPut.size());i++){
                if(!outPut[i]->isDisabled && outPut[i]->fromOutletID == out && patchObjects[outPut[i]->toObjectID]!=nullptr && !patchObjects[outPut[i]->toObjectID]->getWillErase()){
                    outPut[i]->posFrom = getOutletPosition(out);
                    outPut[i]->posTo = patchObjects[outPut[i]->toObjectID]->getInletPosition(outPut[i]->toInletID);
                    // send data through links
                    patchObjects[outPut[i]->toObjectID]->_inletParams[outPut[i]->toInletID] = _outletParams[out];
                }

            }
        }
        updateObjectContent(patchObjects,fd);
    }

}

//--------------------------------------------------------------
void PatchObject::draw(ofxFontStash *font){

    if(!willErase){
        ofPushStyle();
        if(!iconified){
            ofSetCircleResolution(50);

            // Draw the object box
            if(isBigGuiViewer){
                ofSetColor(0);
            }else if(isBigGuiComment){
                ofSetColor(0,0,0,30);
            }else{
                ofSetColor(*color);
            }
            ofSetLineWidth(1);
            ofDrawRectangle(*box);

            // Draw the specific object content ()
            ofPushStyle();
            ofPushMatrix();
            ofTranslate(box->getPosition().x,box->getPosition().y);
            drawObjectContent(font,(shared_ptr<ofBaseGLRenderer>&)ofGetCurrentRenderer());
            ofPopMatrix();
            ofPopStyle();

        }

        // Draw the header
        ofFill();
        if(isBigGuiViewer || isBigGuiComment){
            ofSetColor(0,0,0,0);
        }else{
            ofSetColor(50);
        }

        ofDrawRectangle(*headerBox);

        if(!isBigGuiViewer && !isBigGuiComment){
            ofSetColor(230);
            font->draw(name+" | id:"+ofToString(this->nId),fontSize,headerBox->x + 6, headerBox->y + letterHeight);
        }

        if(!isSystemObject){
            for (unsigned int i=0;i<headerButtons.size();i++){
                ofSetColor(230);
                if (headerButtons[i]->state != nullptr){
                    if ((*headerButtons[i]->state) == true)
                        ofSetColor(220,20,60);
                }

                font->draw(ofToString(headerButtons[i]->letter) , fontSize,headerBox->getPosition().x + headerBox->getWidth() - headerButtons[i]->offset - i*letterWidth, headerBox->getPosition().y + letterHeight);
            }
        }

        ofPopStyle();
    }
}

//--------------------------------------------------------------
void PatchObject::drawImGuiNode(ImGuiEx::NodeCanvas& _nodeCanvas){

    ImVec2 imPos( this->getPos() );
    ImVec2 imSize( this->width, this->height );
    if(_nodeCanvas.BeginNode( PatchObject::getUID().c_str(), imPos, imSize, this->getNumInlets(), this->getNumOutlets() )){

        // Inlets
        for(int i=0;i<static_cast<int>(inlets.size());i++){
            auto pinCol = getInletColor(i);
            vector<ImVec2> tempLinksToPos;
            vector<int> tempLinksIDs;

            _nodeCanvas.AddNodePin( inletsNames.at(i).c_str(), inletsPositions[0], tempLinksToPos, tempLinksIDs, getInletTypeName(i), inletsConnected[i], IM_COL32(pinCol.r,pinCol.g,pinCol.b,pinCol.a), ImGuiExNodePinsFlags_Left );

            inletsPositionOF[i] = ofVec2f((inletsPositions[0].x - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale(), (inletsPositions[0].y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale());
        }

        // Outlets
        for(int i=0;i<static_cast<int>(outlets.size());i++){
            auto pinCol = getOutletColor(i);

            // links
            vector<ImVec2> tempLinksToPos;
            vector<int> tempLinksIDs;

            for(int j=0;j<static_cast<int>(outPut.size());j++){
                if(!outPut[j]->isDisabled && outPut[j]->fromOutletID == i){
                    tempLinksToPos.push_back(ImVec2(outPut[j]->linkVertices[1].x,outPut[j]->linkVertices[1].y));
                    tempLinksIDs.push_back(outPut[j]->id);
                }
            }

            _nodeCanvas.AddNodePin( getOutletName(i).c_str(), outletsPositions[0], tempLinksToPos, tempLinksIDs, getOutletTypeName(i), getIsOutletConnected(i), IM_COL32(pinCol.r,pinCol.g,pinCol.b,pinCol.a), ImGuiExNodePinsFlags_Right );

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
void PatchObject::bezierLink(DraggableVertex from, DraggableVertex to, float _width){
    ofNoFill();
    ofDrawBezier(from.x, from.y+(_width/2), ((to.x-from.x)*.5f)+from.x,from.y+(_width/2), ((to.x-from.x)*.5f)+from.x, to.y+(_width/2), to.x,to.y+(_width/2));
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
void PatchObject::iconify(){
    iconified = !iconified;
    if(iconified){
        box->setHeight(headerHeight);
    }else{
        box->setHeight(height);
    }
}

//--------------------------------------------------------------
void PatchObject::duplicate(){

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


//---------------------------------------------------------------------------------- OBJECT HEADER
//--------------------------------------------------------------
void PatchObject::addButton(char letter, bool *variableToControl, int offset){
    PushButton *newBut = new PushButton();
    newBut->letter  = letter;
    newBut->state   = variableToControl;
    newBut->offset  = offset;
    headerButtons.push_back(newBut);
}

//---------------------------------------------------------------------------------- MOUSE EVENTS
//--------------------------------------------------------------
void PatchObject::mouseMoved(float mx, float my){
    if(!willErase){
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

    }
}

//--------------------------------------------------------------
void PatchObject::mouseDragged(float mx, float my){
    if(!willErase){
        ofVec3f m = ofVec3f(mx,my,0);
        if (isMouseOver && !isGUIObject){
            ofNotifyEvent(dragEvent, nId);

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

    }

}

//--------------------------------------------------------------
void PatchObject::mousePressed(float mx, float my){
    if(!willErase){
        ofVec3f m = ofVec3f(mx, my,0);
        if(box->inside(m)){
            mousePressedObjectContent(m);
        }
        if(isMouseOver && headerBox->inside(m) && !isSystemObject){
            for (unsigned int i=0;i<headerButtons.size();i++){
                if (m.x > (headerBox->getPosition().x + headerBox->getWidth() - headerButtons[i]->offset - i*letterWidth)  && m.x < (headerBox->getPosition().x + headerBox->getWidth() - i*letterWidth) ){
                    if(i == 0){ // REMOVE
                        ofNotifyEvent(removeEvent, nId);
                        willErase = true;
                    }else if(i == 1){ // DUPLICATE
                        ofNotifyEvent(duplicateEvent, nId);
                        duplicate();
                    }else{
                        if(headerButtons[i]->state != nullptr){
                            (*headerButtons[i]->state) = !(*headerButtons[i]->state);
                        }
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------
void PatchObject::mouseReleased(float mx, float my,map<int,shared_ptr<PatchObject>> &patchObjects){
    if(!willErase){
        ofVec3f m = ofVec3f(mx, my,0);

        if (box->inside(m)){
            mouseReleasedObjectContent(m);

            x = box->getPosition().x;
            y = box->getPosition().y;

            fixCollisions(patchObjects);

            saveConfig(false,nId);
        }

        for(size_t m=0;m<inletsMouseNear.size();m++){
            inletsMouseNear.at(m) = false;
        }

    }

}

//--------------------------------------------------------------
void PatchObject::keyPressed(int key){
    if(!willErase && isMouseOver){
        keyPressedObjectContent(key);
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


