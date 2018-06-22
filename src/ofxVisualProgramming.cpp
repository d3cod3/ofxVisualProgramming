/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2018 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

    Mosaic is distributed under the MIT License. This gives everyone the
    freedoms to use Mosaic in any context: commercial or non-commercial,
    public or private, open or closed source.

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

#include "ofxVisualProgramming.h"

ofxVisualProgramming::ofxVisualProgramming(){

    mainWindow = dynamic_pointer_cast<ofAppGLFWWindow>(ofGetCurrentWindow());

    // Performance Measurement
    TIME_SAMPLE_SET_DRAW_LOCATION(TIME_MEASUREMENTS_BOTTOM_RIGHT);
    TIME_SAMPLE_SET_AVERAGE_RATE(0.3);
    TIME_SAMPLE_SET_REMOVE_EXPIRED_THREADS(true);
    TIME_SAMPLE_GET_INSTANCE()->drawUiWithFontStash(MAIN_FONT);
    TIME_SAMPLE_GET_INSTANCE()->setSavesSettingsOnExit(false);

    //  Event listeners
    ofAddListener(ofEvents().mouseMoved, this, &ofxVisualProgramming::mouseMoved);
    ofAddListener(ofEvents().mouseDragged, this, &ofxVisualProgramming::mouseDragged);
    ofAddListener(ofEvents().mousePressed, this, &ofxVisualProgramming::mousePressed);
    ofAddListener(ofEvents().mouseReleased, this, &ofxVisualProgramming::mouseReleased);
    ofAddListener(ofEvents().mouseScrolled, this, &ofxVisualProgramming::mouseScrolled);

    // System
    glVersion           = "OpenGL "+ofToString(glGetString(GL_VERSION));
    glShadingVersion    = "Shading Language "+ofToString(glGetString(GL_SHADING_LANGUAGE_VERSION));

    font        = new ofxFontStash();
    fontSize    = 12;
    isRetina    = false;
    scaleFactor = 1;

    selectedObjectLinkType  = -1;
    selectedObjectLink      = -1;
    selectedObjectID        = -1;
    draggingObject          = false;

    currentPatchFile        = "empty_patch.xml";

    resetTime               = ofGetElapsedTimeMillis();
    wait                    = 3000;
}

ofxVisualProgramming::~ofxVisualProgramming(){
    soundStream.stop();

    delete font;
    font = nullptr;
}

//--------------------------------------------------------------
void ofxVisualProgramming::setup(){

    // Load resources
    font->setup(MAIN_FONT,1.0,2048,true,8,3.0f);

    // Check retina screens
    if(ofGetScreenWidth() >= RETINA_MIN_WIDTH && ofGetScreenHeight() >= RETINA_MIN_HEIGHT){
        isRetina = true;
        scaleFactor = 2;
        fontSize    = 16;
        TIME_SAMPLE_GET_INSTANCE()->setUiScale(scaleFactor);
    }

    // Set pan-zoom canvas
    canvas.disableMouseInput();
    canvas.setbMouseInputEnabled(true);
    canvas.toggleOfCam();
    easyCam.enableOrtho();

    loadPatch(currentPatchFile);

}

//--------------------------------------------------------------
void ofxVisualProgramming::update(){

    TS_START("ofxVP update");

    // Sound Context
    unique_lock<std::mutex> lock(inputAudioMutex);
    {
        unique_lock<std::mutex> lock(outputAudioMutex);
        {
        }
    }


    // Graphical Context
    canvas.update();

    for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        it->second->update(patchObjects);
    }
    // Clear map from deleted objects
    if(ofGetElapsedTimeMillis()-resetTime > wait){
        resetTime = ofGetElapsedTimeMillis();
        eraseIndexes.clear();
        for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if(it->second->getWillErase()){
                eraseIndexes.push_back(it->first);

            }
        }
        for(int x=0;x<eraseIndexes.size();x++){
            for(int p=0;p<patchObjects.at(eraseIndexes.at(x))->outPut.size();p++){
                patchObjects[patchObjects.at(eraseIndexes.at(x))->outPut.at(p)->toObjectID]->inletsConnected.at(patchObjects.at(eraseIndexes.at(x))->outPut.at(p)->toInletID) = false;
            }
            patchObjects.at(eraseIndexes.at(x))->removeObjectContent();
            patchObjects.erase(eraseIndexes.at(x));
        }
    }

    TS_STOP("ofxVP update");

}

//--------------------------------------------------------------
void ofxVisualProgramming::draw(){

    TSGL_START("draw");
    TS_START("ofxVP draw")

    ofPushView();
    ofPushStyle();
    ofPushMatrix();

    canvas.begin();

    ofEnableAlphaBlending();
    ofSetCircleResolution(6);
    ofSetColor(255);
    ofSetLineWidth(1);

    for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        it->second->draw(font);
    }

    if(selectedObjectLink >= 0){
        int lt = patchObjects[selectedObjectID]->getOutletType(selectedObjectLink);
        switch(lt) {
            case 0: ofSetColor(210,210,210);
            break;
            case 1: ofSetColor(230,210,255);
            break;
            case 2: ofSetColor(255,255,200);
            break;
            case 3: ofSetColor(200,255,255); ofSetLineWidth(2);
            break;
            case 4: ofSetColor(255,255,120); ofSetLineWidth(2);
            break;
            default: break;
        }
        ofDrawLine(patchObjects[selectedObjectID]->getOutletPosition(selectedObjectLink).x, patchObjects[selectedObjectID]->getOutletPosition(selectedObjectLink).y, canvas.getMovingPoint().x,canvas.getMovingPoint().y);
    }
    
    canvas.end();

    // Draw Bottom Bar
    ofSetColor(0,0,0,60);
    ofDrawRectangle(0,ofGetHeight() - (18*scaleFactor),ofGetWidth(),(18*scaleFactor));
    ofSetColor(0,200,0);
    font->draw(glVersion,fontSize,10*scaleFactor,ofGetHeight() - (6*scaleFactor));
    ofSetColor(200);
    font->draw(glError.getError(),fontSize,(glVersion.length()*fontSize*0.5f + 10)*scaleFactor,ofGetHeight() - (6*scaleFactor));

    ofDisableBlendMode();

    ofPopMatrix();
    ofPopStyle();
    ofPopView();

    TS_STOP("ofxVP draw");
    TSGL_STOP("draw");
    
}

//--------------------------------------------------------------
void ofxVisualProgramming::mouseMoved(ofMouseEventArgs &e){

    //TS_START("ofxVP mouseMoved");

    ofVec2f  mouse = ofVec2f(canvas.getMovingPoint().x,canvas.getMovingPoint().y);

    for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        it->second->mouseMoved(mouse.x,mouse.y);
        it->second->setIsActive(false);
        if (it->second->isOver(mouse)){
            activeObject(it->first);
        }
    }

    //TS_STOP("ofxVP mouseMoved");

}

//--------------------------------------------------------------
void ofxVisualProgramming::mouseDragged(ofMouseEventArgs &e){

    //TS_START("ofxVP mouseDragged");

    ofVec2f  mouse = ofVec2f(canvas.getMovingPoint().x,canvas.getMovingPoint().y);

    for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->second->isOver(ofPoint(mouse.x,mouse.y,0))){
            it->second->mouseDragged(mouse.x,mouse.y);
            draggingObject = true;
        }
        for(int p=0;p<it->second->outPut.size();p++){
            if(it->second->outPut[p]->toObjectID == selectedObjectID){

                it->second->outPut[p]->linkVertices[2].move(it->second->outPut[p]->posTo.x-20,it->second->outPut[p]->posTo.y);
                it->second->outPut[p]->linkVertices[3].move(it->second->outPut[p]->posTo.x,it->second->outPut[p]->posTo.y);
            }
        }
    }

    if(selectedObjectLink == -1 && !draggingObject){
        canvas.mouseDragged(e);
    }

    //TS_STOP("ofxVP mouseDragged");

}

//--------------------------------------------------------------
void ofxVisualProgramming::mousePressed(ofMouseEventArgs &e){

    //TS_START("ofxVP mousePressed");

    canvas.mousePressed(e);

    ofVec2f  mouse = ofVec2f(canvas.getMovingPoint().x,canvas.getMovingPoint().y);

    selectedObjectLink = -1;
    selectedObjectLinkType = -1;

    for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(patchObjects[it->first] != nullptr){
            for(int p=0;p<it->second->getNumOutlets();p++){
                if(it->second->getOutletPosition(p).distance(mouse) < 5 && !it->second->headerBox->inside(ofVec3f(mouse.x,mouse.y,0))){
                    selectedObjectID = it->first;
                    selectedObjectLink = p;
                    selectedObjectLinkType = it->second->getOutletType(p);
                    it->second->setIsActive(false);
                    break;
                }
            }
            it->second->mousePressed(mouse.x,mouse.y);
        }
    }

    if(selectedObjectLink == -1 && !patchObjects.empty()){
        for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if(patchObjects[it->first] != nullptr){
                if(it->second->getIsActive()){
                    selectedObjectID = it->first;
                    selectedObjectLinkType = -1;
                    break;
                }
            }
        }
    }

    //TS_STOP("ofxVP mousePressed");

}

//--------------------------------------------------------------
void ofxVisualProgramming::mouseReleased(ofMouseEventArgs &e){

    //TS_START("ofxVP mouseReleased");

    canvas.mouseReleased(e);

    ofVec2f  mouse = ofVec2f(canvas.getMovingPoint().x,canvas.getMovingPoint().y);

    bool isLinked = false;

    for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        it->second->mouseReleased(mouse.x,mouse.y);
    }

    if(selectedObjectLinkType != -1 && selectedObjectLink != -1 && selectedObjectID != -1 && !patchObjects.empty()){
        for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if(selectedObjectID != it->first){
                for (int j=0;j<it->second->getNumInlets();j++){
                    if(it->second->getInletPosition(j).distance(mouse) < 5){
                        if(it->second->getInletType(j) == selectedObjectLinkType){
                            connect(selectedObjectID,selectedObjectLink,it->first,j,selectedObjectLinkType);
                            patchObjects[selectedObjectID]->saveConfig(true,selectedObjectID);
                            isLinked = true;
                            break;
                        }

                    }
                }
            }
        }
    }

    if(!isLinked && selectedObjectLinkType != -1 && selectedObjectLink != -1 && selectedObjectID != -1 && !patchObjects.empty() && patchObjects[selectedObjectID] != nullptr){
        for(int j=0;j<patchObjects[selectedObjectID]->outPut.size();j++){
            if(patchObjects[selectedObjectID]->outPut[j]->fromOutletID == selectedObjectLink){
                patchObjects[selectedObjectID]->outPut.erase(patchObjects[selectedObjectID]->outPut.begin()+j);
                patchObjects[selectedObjectID]->removeLinkFromConfig(selectedObjectLink);
                patchObjects[patchObjects[selectedObjectID]->outPut[j]->toObjectID]->inletsConnected[patchObjects[selectedObjectID]->outPut[j]->toInletID] = false;
            }
        }

    }

    selectedObjectLinkType  = -1;
    selectedObjectLink      = -1;

    draggingObject          = false;

    //TS_STOP("ofxVP mouseReleased");
}

//--------------------------------------------------------------
void ofxVisualProgramming::mouseScrolled(ofMouseEventArgs &e){
    canvas.mouseScrolled(e);
}

//--------------------------------------------------------------
void ofxVisualProgramming::audioIn(ofSoundBuffer &inputBuffer){

    TS_START("ofxVP audioIn");

    // compute audio input
    for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        it->second->audioIn(inputBuffer);
    }

    unique_lock<std::mutex> lock(inputAudioMutex);
    lastInputBuffer = inputBuffer;

    TS_STOP("ofxVP audioIn");

}

//--------------------------------------------------------------
void ofxVisualProgramming::audioOut(ofSoundBuffer &outBuffer){

    TS_START("ofxVP audioOut");

    // Compute audio output
    for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        it->second->audioOut(outBuffer);
    }

    unique_lock<std::mutex> lock(outputAudioMutex);
    lastOutputBuffer = outBuffer;

    TS_STOP("ofxVP audioOut");
}

//--------------------------------------------------------------
void ofxVisualProgramming::activeObject(int oid){
    if ((oid != -1) && (patchObjects[oid] != nullptr)){
        selectedObjectID = oid;

        for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if (it->first == oid){
                it->second->setIsActive(true);
            }else{
                it->second->setIsActive(false);
            }
        }
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::addObject(string name,ofVec2f pos){
    PatchObject* tempObj = selectObject(name);

    tempObj->newObject();
    tempObj->setPatchfile(currentPatchFile);
    tempObj->setup(mainWindow);
    tempObj->move(static_cast<int>(pos.x-(OBJECT_STANDARD_WIDTH/2*scaleFactor)),static_cast<int>(pos.y-(OBJECT_STANDARD_HEIGHT/2*scaleFactor)));
    tempObj->setIsRetina(isRetina);
    ofAddListener(tempObj->dragEvent ,this,&ofxVisualProgramming::dragObject);
    ofAddListener(tempObj->removeEvent ,this,&ofxVisualProgramming::removeObject);
    ofAddListener(tempObj->iconifyEvent ,this,&ofxVisualProgramming::iconifyObject);

    actualObjectID++;

    bool saved = tempObj->saveConfig(false,actualObjectID);

    if(saved){
        patchObjects[tempObj->getId()] = tempObj;
    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::dragObject(int &id){

}

//--------------------------------------------------------------
void ofxVisualProgramming::removeObject(int &id){
    if ((id != -1) && (patchObjects[id] != nullptr)){

        int targetID = id;
        bool found = false;
        ofxXmlSettings XML;
        if (XML.loadFile(currentPatchFile)){
            int totalObjects = XML.getNumTags("object");

            for(int i=0;i<totalObjects;i++){
                if(XML.pushTag("object", i)){
                    if(XML.getValue("id", -1) == id){
                        targetID = i;
                        found = true;
                    }
                    XML.popTag();
                }
            }

            // remove links to the removed object
            for(int i=0;i<totalObjects;i++){
                if(XML.pushTag("object", i)){
                    if(XML.getValue("id", -1) != id){
                        //ofLogNotice("id",ofToString(XML.getValue("id", -1)));
                        if(XML.pushTag("outlets")){
                            int totalLinks = XML.getNumTags("link");
                            for(int l=0;l<totalLinks;l++){
                                if(XML.pushTag("link",l)){
                                    int totalTo = XML.getNumTags("to");
                                    for(int t=0;t<totalTo;t++){
                                        if(XML.pushTag("to",t)){
                                            bool delLink = false;
                                            if(XML.getValue("id", -1) == id){
                                                //ofLogNotice("remove link id",ofToString(XML.getValue("id", -1)));
                                                delLink = true;
                                            }
                                            XML.popTag();
                                            if(delLink){
                                                XML.removeTag("to",t);
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
            // remove object
            if(found){
                XML.removeTag("object", targetID);
                XML.saveFile();
            }
        }

        for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            for(int j=0;j<it->second->outPut.size();j++){
                if(it->second->outPut[j]->toObjectID == id){
                    it->second->outPut[j]->isDisabled = true;
                    patchObjects[it->second->outPut[j]->toObjectID]->inletsConnected[it->second->outPut[j]->toInletID] = false;
                }
            }
        }

    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::iconifyObject(int &id){

}

//--------------------------------------------------------------
bool ofxVisualProgramming::connect(int fromID, int fromOutlet, int toID,int toInlet, int linkType){
    bool connected = false;

    if((fromID != -1) && (patchObjects[fromID] != nullptr) && (toID != -1) && (patchObjects[toID] != nullptr) && (patchObjects[fromID]->getOutletType(fromOutlet) == patchObjects[toID]->getInletType(toInlet)) && !patchObjects[toID]->inletsConnected[toInlet]){
        PatchLink   *tempLink = new PatchLink();

        tempLink->posFrom = patchObjects[fromID]->getOutletPosition(fromOutlet);
        tempLink->posTo = patchObjects[toID]->getInletPosition(toInlet);
        tempLink->type = patchObjects[toID]->getInletType(toInlet);
        tempLink->fromOutletID = fromOutlet;
        tempLink->toObjectID = toID;
        tempLink->toInletID = toInlet;
        tempLink->isDisabled = false;

        tempLink->linkVertices.push_back(DraggableVertex(tempLink->posFrom.x,tempLink->posFrom.y));
        tempLink->linkVertices.push_back(DraggableVertex(tempLink->posFrom.x+20,tempLink->posFrom.y));
        tempLink->linkVertices.push_back(DraggableVertex(tempLink->posTo.x-20,tempLink->posTo.y));
        tempLink->linkVertices.push_back(DraggableVertex(tempLink->posTo.x,tempLink->posTo.y));

        patchObjects[fromID]->outPut.push_back(tempLink);

        patchObjects[toID]->inletsConnected[toInlet] = true;

        connected = true;
    }

    return connected;
}

//--------------------------------------------------------------
PatchObject* ofxVisualProgramming::selectObject(string objname){
    PatchObject* tempObj;
    if(objname == "lua script"){
        tempObj = new LuaScript();
    }else if(objname == "audio analyzer"){
        tempObj = new AudioAnalyzer();
    }else if(objname == "message"){
        tempObj = new moMessage();
    }else if(objname == "simple random"){
        tempObj = new SimpleRandom();
    }else if(objname == "simple noise"){
        tempObj = new SimpleNoise();
    }else if(objname == "slider"){
        tempObj = new moSlider();
    }else if(objname == "video player"){
        tempObj = new VideoPlayer();
    }else if(objname == "output window"){
        tempObj = new OutputWindow();
    }else{
        tempObj = nullptr;
    }

    return tempObj;
}

//--------------------------------------------------------------
void ofxVisualProgramming::openPatch(string patchFile){
    currentPatchFile = patchFile;

    // clear previous patch
    for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        it->second->removeObjectContent();
    }
    patchObjects.clear();

    // load new patch
    loadPatch(currentPatchFile);
}

//--------------------------------------------------------------
void ofxVisualProgramming::loadPatch(string patchFile){

    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){

        // Load main settings
        if (XML.pushTag("settings")){
            // Setup projector dimension
            output_width = XML.getValue("output_width",0);
            output_height = XML.getValue("output_height",0);

            // setup audio
            if(patchFile != "empty_patch.xml"){
                //soundStream.printDeviceList();
                int audioDev = XML.getValue("audio_device",0);

                soundStream.close();

                auto devices = soundStream.getDeviceList();
                soundStreamSettings.setInDevice(devices[audioDev]);
                soundStreamSettings.setOutDevice(devices[audioDev]);
                soundStreamSettings.setInListener(this);
                soundStreamSettings.sampleRate = XML.getValue("sample_rate",0);
                soundStreamSettings.numOutputChannels = devices[audioDev].outputChannels;
                soundStreamSettings.numInputChannels = devices[audioDev].inputChannels;
                XML.setValue("input_channels",static_cast<int>(devices[audioDev].inputChannels));
                XML.setValue("output_channels",static_cast<int>(devices[audioDev].outputChannels));
                soundStreamSettings.bufferSize = XML.getValue("buffer_size",0);

                XML.saveFile();

                bool startingSoundstream = soundStream.setup(soundStreamSettings);

                soundStream.start();

                if(startingSoundstream){
                    ofLog(OF_LOG_NOTICE,"------------------- Soundstream Started on");
                    ofLog(OF_LOG_NOTICE,"Audio device: %s",devices[audioDev].name.c_str());
                }else{
                    ofLog(OF_LOG_ERROR,"There was a problem starting the Soundstream on selected audio devices.");
                }
            }

            XML.popTag();
        }

        int totalObjects = XML.getNumTags("object");

        // Load all the patch objects
        for(int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                string objname = XML.getValue("name","");
                bool loaded = false;
                PatchObject* tempObj = selectObject(objname);
                if(tempObj != nullptr){
                    loaded = tempObj->loadConfig(mainWindow,i,patchFile);
                    if(loaded){
                        tempObj->setIsRetina(isRetina);
                        ofAddListener(tempObj->dragEvent ,this,&ofxVisualProgramming::dragObject);
                        ofAddListener(tempObj->removeEvent ,this,&ofxVisualProgramming::removeObject);
                        ofAddListener(tempObj->iconifyEvent ,this,&ofxVisualProgramming::iconifyObject);
                        // Insert the new patch into the map
                        patchObjects[tempObj->getId()] = tempObj;
                        actualObjectID = tempObj->getId();
                    }
                }
                XML.popTag();
            }
        }

        // Load Links
        for(int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                int fromID = XML.getValue("id", -1);
                if (XML.pushTag("outlets")){
                    int totalOutlets = XML.getNumTags("link");
                    for(int j=0;j<totalOutlets;j++){
                        if (XML.pushTag("link",j)){
                            int linkType = XML.getValue("type", 0);
                            int totalLinks = XML.getNumTags("to");
                            for(int z=0;z<totalLinks;z++){
                                if(XML.pushTag("to",z)){
                                    int toObjectID = XML.getValue("id", 0);
                                    int toInletID = XML.getValue("inlet", 0);

                                    connect(fromID,j,toObjectID,toInletID,linkType);

                                    XML.popTag();
                                }
                            }
                            XML.popTag();
                        }
                    }

                    XML.popTag();
                }
                XML.popTag();
            }
        }

    }


}

//--------------------------------------------------------------
void ofxVisualProgramming::savePatch(){

}
