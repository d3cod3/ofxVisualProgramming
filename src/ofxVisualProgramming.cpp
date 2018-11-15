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

#include "ofxVisualProgramming.h"

//--------------------------------------------------------------
void ofxVisualProgramming::initObjectMatrix(){
    vector<string> vecInit = {};

    vecInit = {};
    objectsMatrix["3d"] = vecInit;

    vecInit = {"audio analyzer","beat extractor","bpm extractor","centroid extractor","dissonance extractor","fft extractor","hfc extractor","hpcp extractor","inharmonicity extractor","mel bands extractor","mfcc extractor","onset extractor","pitch extractor","power extractor","rms extractor","rolloff extractor","tristimulus extractor"};
    objectsMatrix["audio_analysis"] = vecInit;

    vecInit = {"background subtraction","chroma key","color tracking","contour tracking","haar tracking","motion detection","optical flow"};
    objectsMatrix["computer vision"] = vecInit;

    vecInit = {"floats to vector","vector concat"};
    objectsMatrix["data"] = vecInit;

    vecInit = {};
    objectsMatrix["graphics"] = vecInit;

    vecInit = {"bang","comment","message","player controls","signal viewer","slider","trigger","video viewer"};
    objectsMatrix["gui"] = vecInit;

    vecInit = {};
    objectsMatrix["input/output"] = vecInit;

    vecInit = {"counter","delay bang","gate","loadbang"};
    objectsMatrix["logic"] = vecInit;

    vecInit = {};
    objectsMatrix["machine learning"] = vecInit;

    vecInit = {"add","clamp","constant","divide","metronome","multiply","simple noise","simple random","smooth","subtract"};
    objectsMatrix["math"] = vecInit;

    vecInit = {};
    objectsMatrix["midi"] = vecInit;

    vecInit = {};
    objectsMatrix["osc"] = vecInit;

    vecInit = {};
    objectsMatrix["physics"] = vecInit;

#if defined(TARGET_LINUX) || defined(TARGET_OSX)
    vecInit = {"bash script","lua script","python script","shader object"};
#elif defined(TARGET_WIN32)
    vecInit = {"lua script","shader object"};
#endif

    objectsMatrix["scripting"] = vecInit;

    vecInit = {"soundfile player"};
    objectsMatrix["sound"] = vecInit;

    vecInit = {"kinect grabber","video grabber","video player"};
    objectsMatrix["video"] = vecInit;

    vecInit = {};
    objectsMatrix["web"] = vecInit;

    vecInit = {"output window"};
    objectsMatrix["windowing"] = vecInit;

}

//--------------------------------------------------------------
ofxVisualProgramming::ofxVisualProgramming(){

    mainWindow = dynamic_pointer_cast<ofAppGLFWWindow>(ofGetCurrentWindow());

    // Profiler
    profilerActive          = false;
    TIME_SAMPLE_SET_DRAW_LOCATION(TIME_MEASUREMENTS_BOTTOM_RIGHT);
    TIME_SAMPLE_SET_AVERAGE_RATE(0.3);
    TIME_SAMPLE_SET_REMOVE_EXPIRED_THREADS(true);
    TIME_SAMPLE_GET_INSTANCE()->drawUiWithFontStash(MAIN_FONT);
    TIME_SAMPLE_GET_INSTANCE()->setSavesSettingsOnExit(false);
    TIME_SAMPLE_SET_ENABLED(profilerActive);

    //  Event listeners
    ofAddListener(ofEvents().mouseMoved, this, &ofxVisualProgramming::mouseMoved);
    ofAddListener(ofEvents().mouseDragged, this, &ofxVisualProgramming::mouseDragged);
    ofAddListener(ofEvents().mousePressed, this, &ofxVisualProgramming::mousePressed);
    ofAddListener(ofEvents().mouseReleased, this, &ofxVisualProgramming::mouseReleased);
    ofAddListener(ofEvents().mouseScrolled, this, &ofxVisualProgramming::mouseScrolled);
    ofAddListener(ofEvents().keyPressed, this, &ofxVisualProgramming::keyPressed);

    // System
    glVersion           = "OpenGL "+ofToString(glGetString(GL_VERSION));
    glShadingVersion    = "Shading Language "+ofToString(glGetString(GL_SHADING_LANGUAGE_VERSION));

    font        = new ofxFontStash();
    fontSize    = 12;
    isRetina    = false;
    scaleFactor = 1;
    isOverGui   = false;
    linkActivateDistance    = 5;

    selectedObjectLinkType  = -1;
    selectedObjectLink      = -1;
    selectedObjectID        = -1;
    draggingObjectID        = -1;
    draggingObject          = false;
    bLoadingNewObject       = false;

    currentPatchFile        = "empty_patch.xml";

    resetTime               = ofGetElapsedTimeMillis();
    wait                    = 2000;

    alphabet                = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXZY";
    newFileCounter          = 0;

    audioSampleRate         = 0;
    dspON                   = false;

}

//--------------------------------------------------------------
ofxVisualProgramming::~ofxVisualProgramming(){
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
        fontSize    = 26;
        linkActivateDistance *= scaleFactor;
        TIME_SAMPLE_GET_INSTANCE()->setUiScale(scaleFactor);
    }

    // Set pan-zoom canvas
    canvas.disableMouseInput();
    canvas.setbMouseInputEnabled(true);
    canvas.toggleOfCam();
    easyCam.enableOrtho();

    // RETINA FIX
    if(ofGetScreenWidth() >= RETINA_MIN_WIDTH && ofGetScreenHeight() >= RETINA_MIN_HEIGHT){
        canvas.setScale(2);
    }

    // GUI
    setupGUI();

    // Create new empty file patch
    newPatch();

}

//--------------------------------------------------------------
void ofxVisualProgramming::setupGUI(){

    initObjectMatrix();

    guiThemeRetina = new ofxDatGuiThemeRetina();
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_LEFT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(160*scaleFactor);
    guiHeader = gui->addHeader("OBJECTS");

    for(map<string,vector<string>>::iterator it = objectsMatrix.begin(); it != objectsMatrix.end(); it++ ){
        ofxDatGuiFolder* tgf;
        if(static_cast<int>(it->second.size()) == 0){
            tgf = new ofxDatGuiFolder(it->first, ofColor::black);
        }else{
            tgf = new ofxDatGuiFolder(it->first);
        }
        tgf->setLabelAlignment(ofxDatGuiAlignment::RIGHT);
        tgf->onButtonEvent(this, &ofxVisualProgramming::onButtonEvent);
        objectFolders.push_back(tgf);

        ofxDatGuiScrollView* tsw = new ofxDatGuiScrollView(it->first);
        objectNavigators.push_back(tsw);
        if(isRetina){
            objectNavigators.back()->setItemSpacing(27);
        }
        for(int j=0;j<static_cast<int>(it->second.size());j++){
            objectNavigators.back()->add(it->second.at(j));
        }

        for(int z=0;z<static_cast<int>(it->second.size());z++){
            objectNavigators.back()->children[z]->setUseCustomMouse(true);
        }

        objectNavigators.back()->onScrollViewEvent(this, &ofxVisualProgramming::onScrollViewEvent);

        objectFolders.back()->attachItem(objectNavigators.back());
        gui->addFolder(objectFolders.back());

    }

    ofxDatGuiFooter* footer = gui->addFooter();
    footer->setLabelWhenExpanded("collapse");
    footer->setLabelWhenCollapsed("objects");

    if(isRetina){
        gui->setTheme(guiThemeRetina);
    }

    gui->onButtonEvent(this, &ofxVisualProgramming::onButtonEvent);

}

//--------------------------------------------------------------
void ofxVisualProgramming::update(){

    // Sound Context
    unique_lock<std::mutex> lock(inputAudioMutex);
    {

    }

    // GUI
    if(!draggingObject){
        gui->update();
        for(int i=0;i<static_cast<int>(objectNavigators.size());i++){
            objectNavigators.at(i)->update();
        }
    }

    // Graphical Context
    canvas.update();

    for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        TS_START(it->second->getName()+ofToString(it->second->getId())+"_update");
        it->second->update(patchObjects);
        TS_STOP(it->second->getName()+ofToString(it->second->getId())+"_update");

        if(draggingObject && draggingObjectID == it->first){
            it->second->mouseDragged(actualMouse.x,actualMouse.y);
        }
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
        for(int x=0;x<static_cast<int>(eraseIndexes.size());x++){
            for(int p=0;p<static_cast<int>(patchObjects.at(eraseIndexes.at(x))->outPut.size());p++){
                patchObjects[patchObjects.at(eraseIndexes.at(x))->outPut.at(p)->toObjectID]->inletsConnected.at(patchObjects.at(eraseIndexes.at(x))->outPut.at(p)->toInletID) = false;
            }
            patchObjects.at(eraseIndexes.at(x))->removeObjectContent();
            patchObjects.erase(eraseIndexes.at(x));
        }

    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::draw(){

    TSGL_START("draw");

    ofPushView();
    ofPushStyle();
    ofPushMatrix();

    canvas.begin();

    ofEnableAlphaBlending();
    ofSetCircleResolution(6);
    ofSetCurveResolution(50);
    ofSetColor(255);
    ofSetLineWidth(1);

    for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        TS_START(it->second->getName()+ofToString(it->second->getId())+"_draw");
        it->second->draw(font);
        TS_STOP(it->second->getName()+ofToString(it->second->getId())+"_draw");
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
        case 5: ofSetColor(255,128,128); ofSetLineWidth(1);
            break;
        default: break;
        }
        ofDrawLine(patchObjects[selectedObjectID]->getOutletPosition(selectedObjectLink).x, patchObjects[selectedObjectID]->getOutletPosition(selectedObjectLink).y, canvas.getMovingPoint().x,canvas.getMovingPoint().y);

        // Draw outlet type name
        //ofSetColor(245);
        switch(lt) {
        case 0: patchObjects[selectedObjectID]->linkTypeName = "float";
            break;
        case 1: patchObjects[selectedObjectID]->linkTypeName = "string";
            break;
        case 2: patchObjects[selectedObjectID]->linkTypeName = "vector<float>";
            break;
        case 3: patchObjects[selectedObjectID]->linkTypeName = "ofTexture";
            break;
        case 4: patchObjects[selectedObjectID]->linkTypeName = "ofSoundBuffer";
            break;
        case 5: patchObjects[selectedObjectID]->linkTypeName = patchObjects[selectedObjectID]->specialLinkTypeName;
            break;
        default: patchObjects[selectedObjectID]->linkTypeName = "";
            break;
        }

        if(isRetina){
            font->draw(patchObjects[selectedObjectID]->linkTypeName,fontSize/2,canvas.getMovingPoint().x + (10*scaleFactor),canvas.getMovingPoint().y);
        }else{
            font->draw(patchObjects[selectedObjectID]->linkTypeName,fontSize,canvas.getMovingPoint().x + (10*scaleFactor),canvas.getMovingPoint().y);
        }

    }
    
    canvas.end();

    // Draw Bottom Bar
    ofSetColor(0,0,0,60);
    ofDrawRectangle(0,ofGetHeight() - (18*scaleFactor) - (240*scaleFactor),ofGetWidth(),(18*scaleFactor));
    ofSetColor(0,200,0);
    font->draw(glVersion,fontSize,10*scaleFactor,ofGetHeight() - (6*scaleFactor) - (240*scaleFactor));
    ofSetColor(200);
    font->draw(glError.getError(),fontSize,glVersion.length()*fontSize*0.5f + 10*scaleFactor,ofGetHeight() - (6*scaleFactor) - (240*scaleFactor));

    ofDisableBlendMode();

    ofPopMatrix();
    ofPopStyle();
    ofPopView();

    gui->draw();

    TSGL_STOP("draw");

}

//--------------------------------------------------------------
void ofxVisualProgramming::exit(){
    ofDirectory dir;
    dir.listDir(ofToDataPath("temp/"));
    for(size_t i = 0; i < dir.size(); i++){
        dir.getFile(i).remove();
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::mouseMoved(ofMouseEventArgs &e){

    actualMouse = ofVec2f(canvas.getMovingPoint().x,canvas.getMovingPoint().y);

    // GUI
    if(!draggingObject){
        gui->setCustomMousePos(e.x,e.y);
        isOverGui = gui->hitTest(ofPoint(e.x,e.y));
        for(int i=0;i<static_cast<int>(objectNavigators.size());i++){
            for(int z=0;z<static_cast<int>(objectNavigators.at(i)->children.size());z++){
                objectNavigators.at(i)->children[z]->setCustomMousePos(e.x-objectNavigators.at(i)->getX(),e.y-objectNavigators.at(i)->getY());
            }
        }
    }

    // CANVAS
    for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        it->second->mouseMoved(actualMouse.x,actualMouse.y);
        it->second->setIsActive(false);
        if (it->second->isOver(actualMouse)){
            activeObject(it->first);
        }
    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::mouseDragged(ofMouseEventArgs &e){

    actualMouse = ofVec2f(canvas.getMovingPoint().x,canvas.getMovingPoint().y);

    // CANVAS
    for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->second->isOver(ofPoint(actualMouse.x,actualMouse.y,0))){
            draggingObject = true;
            draggingObjectID = it->first;
        }
        for(int p=0;p<static_cast<int>(it->second->outPut.size());p++){
            if(it->second->outPut[p]->toObjectID == selectedObjectID){
                if(isRetina){
                    it->second->outPut[p]->linkVertices[2].move(it->second->outPut[p]->posTo.x-40,it->second->outPut[p]->posTo.y);
                }else{
                    it->second->outPut[p]->linkVertices[2].move(it->second->outPut[p]->posTo.x-20,it->second->outPut[p]->posTo.y);
                }
                it->second->outPut[p]->linkVertices[3].move(it->second->outPut[p]->posTo.x,it->second->outPut[p]->posTo.y);
            }
        }
        if(selectedObjectID != it->first){
            for (int j=0;j<it->second->getNumInlets();j++){
                if(it->second->getInletPosition(j).distance(actualMouse) < linkActivateDistance){
                    if(it->second->getInletType(j) == selectedObjectLinkType){
                        it->second->setInletMouseNear(j,true);
                    }
                }else{
                    it->second->setInletMouseNear(j,false);
                }
            }
        }
    }

    // GUI
    if(!draggingObject){
        gui->setCustomMousePos(e.x,e.y);
        isOverGui = gui->hitTest(ofPoint(e.x,e.y));
        for(int i=0;i<static_cast<int>(objectNavigators.size());i++){
            for(int z=0;z<static_cast<int>(objectNavigators.at(i)->children.size());z++){
                objectNavigators.at(i)->children[z]->setCustomMousePos(e.x-objectNavigators.at(i)->getX(),e.y-objectNavigators.at(i)->getY());
            }
        }
    }

    if(selectedObjectLink == -1 && !draggingObject && !isOverGui){
        canvas.mouseDragged(e);
    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::mousePressed(ofMouseEventArgs &e){

    actualMouse = ofVec2f(canvas.getMovingPoint().x,canvas.getMovingPoint().y);

    if(isOverGui){
        if(!draggingObject){
            gui->setCustomMousePos(e.x,e.y);
        }
    }else{
        canvas.mousePressed(e);

        selectedObjectLink = -1;
        selectedObjectLinkType = -1;

        for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if(patchObjects[it->first] != nullptr){
                for(int p=0;p<it->second->getNumOutlets();p++){
                    if(it->second->getOutletPosition(p).distance(actualMouse) < linkActivateDistance && !it->second->headerBox->inside(ofVec3f(actualMouse.x,actualMouse.y,0))){
                        selectedObjectID = it->first;
                        selectedObjectLink = p;
                        selectedObjectLinkType = it->second->getOutletType(p);
                        it->second->setIsActive(false);
                        break;
                    }
                }
                it->second->mousePressed(actualMouse.x,actualMouse.y);
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
    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::mouseReleased(ofMouseEventArgs &e){

    actualMouse = ofVec2f(canvas.getMovingPoint().x,canvas.getMovingPoint().y);

    if(isOverGui){
        if(!draggingObject){
            gui->setCustomMousePos(e.x,e.y);
            if(!gui->getFocused()){
                gui->focus();
            }
        }
    }else{
        canvas.mouseReleased(e);

        bool isLinked = false;

        for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            it->second->mouseReleased(actualMouse.x,actualMouse.y,patchObjects);
        }

        if(selectedObjectLinkType != -1 && selectedObjectLink != -1 && selectedObjectID != -1 && !patchObjects.empty()){
            for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
                if(selectedObjectID != it->first){
                    for (int j=0;j<it->second->getNumInlets();j++){
                        if(it->second->getInletPosition(j).distance(actualMouse) < linkActivateDistance){
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

        if(!isLinked && selectedObjectLinkType != -1 && selectedObjectLink != -1 && selectedObjectID != -1 && !patchObjects.empty() && patchObjects[selectedObjectID] != nullptr && patchObjects[selectedObjectID]->outPut.size()>0){
            vector<bool> tempEraseLinks;
            for(int j=0;j<static_cast<int>(patchObjects[selectedObjectID]->outPut.size());j++){
                //ofLog(OF_LOG_NOTICE,"Object %i have link to %i",selectedObjectID,patchObjects[selectedObjectID]->outPut[j]->toObjectID);
                if(patchObjects[selectedObjectID]->outPut[j]->fromOutletID == selectedObjectLink){
                    tempEraseLinks.push_back(true);
                }else{
                    tempEraseLinks.push_back(false);
                }
            }

            vector<PatchLink*> tempBuffer;
            tempBuffer.reserve(patchObjects[selectedObjectID]->outPut.size()-tempEraseLinks.size());

            for (size_t i=0; i<patchObjects[selectedObjectID]->outPut.size(); i++){
                if(!tempEraseLinks[i]){
                    tempBuffer.push_back(patchObjects[selectedObjectID]->outPut[i]);
                }else{
                    patchObjects[selectedObjectID]->removeLinkFromConfig(selectedObjectLink);
                    if(patchObjects[patchObjects[selectedObjectID]->outPut[i]->toObjectID] != nullptr){
                        patchObjects[patchObjects[selectedObjectID]->outPut[i]->toObjectID]->inletsConnected[patchObjects[selectedObjectID]->outPut[i]->toInletID] = false;
                    }
                    //ofLog(OF_LOG_NOTICE,"Removed link from %i to %i",selectedObjectID,patchObjects[selectedObjectID]->outPut[i]->toObjectID);
                }
            }

            patchObjects[selectedObjectID]->outPut = tempBuffer;

        }

        selectedObjectLinkType  = -1;
        selectedObjectLink      = -1;

        draggingObject          = false;
        draggingObjectID        = -1;

    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::mouseScrolled(ofMouseEventArgs &e){
    if(!isOverGui && e.y < (ofGetWindowHeight()-(240*scaleFactor))){
        canvas.mouseScrolled(e);
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::keyPressed(ofKeyEventArgs &e){
    for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        it->second->keyPressed(e.key);
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::audioProcess(float *input, int bufferSize, int nChannels){

    if(audioSampleRate != 0 && dspON){

        TS_START("ofxVP audioProcess");

        if(!bLoadingNewObject){
            if(audioDevices[audioINDev].inputChannels > 0){
                inputBuffer.copyFrom(input, bufferSize, nChannels, audioSampleRate);

                // compute audio input
                for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
                    it->second->audioIn(inputBuffer);
                }

                unique_lock<std::mutex> lock(inputAudioMutex);
                lastInputBuffer = inputBuffer;
            }
            if(audioDevices[audioOUTDev].outputChannels > 0){
                // compute audio output
                for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
                    it->second->audioOut(emptyBuffer);
                }
            }

        }

        TS_STOP("ofxVP audioProcess");

    }

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

    bLoadingNewObject       = true;

    PatchObject* tempObj = selectObject(name);

    tempObj->newObject();
    tempObj->setPatchfile(currentPatchFile);
    tempObj->setup(mainWindow);
    tempObj->setupDSP(engine);
    tempObj->move(static_cast<int>(pos.x-(OBJECT_STANDARD_WIDTH/2*scaleFactor)),static_cast<int>(pos.y-(OBJECT_STANDARD_HEIGHT/2*scaleFactor)));
    tempObj->setIsRetina(isRetina);
    ofAddListener(tempObj->dragEvent ,this,&ofxVisualProgramming::dragObject);
    ofAddListener(tempObj->removeEvent ,this,&ofxVisualProgramming::removeObject);
    ofAddListener(tempObj->resetEvent ,this,&ofxVisualProgramming::resetObject);
    ofAddListener(tempObj->iconifyEvent ,this,&ofxVisualProgramming::iconifyObject);
    ofAddListener(tempObj->duplicateEvent ,this,&ofxVisualProgramming::duplicateObject);

    actualObjectID++;

    bool saved = tempObj->saveConfig(false,actualObjectID);

    if(saved){
        patchObjects[tempObj->getId()] = tempObj;
        patchObjects[tempObj->getId()]->fixCollisions(patchObjects);
    }

    bLoadingNewObject       = false;

}

//--------------------------------------------------------------
void ofxVisualProgramming::dragObject(int &id){

}

//--------------------------------------------------------------
void ofxVisualProgramming::resetObject(int &id){
    if ((id != -1) && (patchObjects[id] != nullptr)){

        ofxXmlSettings XML;
        if (XML.loadFile(currentPatchFile)){

            for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
                vector<PatchLink*> tempBuffer;
                for(int j=0;j<static_cast<int>(it->second->outPut.size());j++){
                    if(it->second->outPut[j]->toObjectID == id){
                        if(it->second->outPut[j]->toInletID < patchObjects[id]->getNumInlets()){
                            tempBuffer.push_back(it->second->outPut[j]);
                        }
                    }else{
                        tempBuffer.push_back(it->second->outPut[j]);
                    }
                }
                it->second->outPut = tempBuffer;
            }

            int totalObjects = XML.getNumTags("object");

            // remove links to the resetted object
            for(int i=0;i<totalObjects;i++){
                if(XML.pushTag("object", i)){
                    if(XML.getValue("id", -1) != id){
                        if(XML.pushTag("outlets")){
                            int totalLinks = XML.getNumTags("link");
                            for(int l=0;l<totalLinks;l++){
                                if(XML.pushTag("link",l)){
                                    int totalTo = XML.getNumTags("to");
                                    for(int t=0;t<totalTo;t++){
                                        if(XML.pushTag("to",t)){
                                            bool delLink = false;
                                            if(XML.getValue("id", -1) == id && XML.getValue("inlet", -1) >= patchObjects[id]->getNumInlets()){
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

            XML.saveFile();

        }
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::resetObject(int id){
    if ((id != -1) && (patchObjects[id] != nullptr)){
        for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            vector<PatchLink*> tempBuffer;
            for(int j=0;j<static_cast<int>(it->second->outPut.size());j++){
                if(it->second->outPut[j]->toObjectID != id){
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

//--------------------------------------------------------------
void ofxVisualProgramming::deleteObject(int id){
    resetTime = ofGetElapsedTimeMillis();

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
            vector<PatchLink*> tempBuffer;
            for(int j=0;j<static_cast<int>(it->second->outPut.size());j++){
                if(it->second->outPut[j]->toObjectID != id){
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

//--------------------------------------------------------------
void ofxVisualProgramming::removeObject(int &id){
    resetTime = ofGetElapsedTimeMillis();

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
            vector<PatchLink*> tempBuffer;
            for(int j=0;j<static_cast<int>(it->second->outPut.size());j++){
                if(it->second->outPut[j]->toObjectID != id){
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

//--------------------------------------------------------------
void ofxVisualProgramming::iconifyObject(int &id){

}

//--------------------------------------------------------------
void ofxVisualProgramming::duplicateObject(int &id){
    // disable duplicate for hardware related objects
    if(patchObjects[id]->getName() != "video grabber" && patchObjects[id]->getName() != "kinect grabber"){
        ofVec2f newPos = ofVec2f(patchObjects[id]->getPos().x + patchObjects[id]->getObjectWidth(),patchObjects[id]->getPos().y);
        addObject(patchObjects[id]->getName(),patchObjects[id]->getPos());
    }
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

        if(tempLink->type == VP_LINK_TEXTURE){
            patchObjects[toID]->_inletParams[toInlet] = new ofTexture();
        }else if(tempLink->type == VP_LINK_AUDIO){
            patchObjects[toID]->_inletParams[toInlet] = new ofSoundBuffer();
        }

        checkSpecialConnection(fromID,toID,linkType);

        connected = true;
    }

    return connected;
}

//--------------------------------------------------------------
void ofxVisualProgramming::checkSpecialConnection(int fromID, int toID, int linkType){
    if(patchObjects[fromID]->getName() == "lua script" || patchObjects[fromID]->getName() == "python script"){
        if((patchObjects[fromID]->getName() == "shader object" || patchObjects[toID]->getName() == "output window") && linkType == VP_LINK_TEXTURE){
            patchObjects[fromID]->resetResolution(toID,patchObjects[toID]->getOutputWidth(),patchObjects[toID]->getOutputHeight());
        }
    }
    if(patchObjects[fromID]->getName() == "shader object"){
        if(patchObjects[toID]->getName() == "output window" && linkType == VP_LINK_TEXTURE){
            patchObjects[fromID]->resetResolution(toID,patchObjects[toID]->getOutputWidth(),patchObjects[toID]->getOutputHeight());
        }
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::resetSystemObjects(){
    for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->second->getIsSystemObject()){
            it->second->resetSystemObject();
            resetObject(it->second->getId());
            if(it->second->getIsAudioOUTObject()){
                it->second->setupAudioOutObjectContent(engine);
            }
        }
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::resetSpecificSystemObjects(string name){
    for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->second->getIsSystemObject() && it->second->getName() == name){
            it->second->resetSystemObject();
            resetObject(it->second->getId());
            if(it->second->getIsAudioOUTObject()){
                it->second->setupAudioOutObjectContent(engine);
            }
        }
    }
}

//--------------------------------------------------------------
PatchObject* ofxVisualProgramming::selectObject(string objname){
    PatchObject* tempObj;

#if defined(TARGET_LINUX) || defined(TARGET_OSX)
    if(objname == "bash script"){
        tempObj = new BashScript();
    }else if(objname == "python script"){
        tempObj = new PythonScript();
    }else
#endif

    if(objname == "lua script"){
        tempObj = new LuaScript();
    }else if(objname == "shader object"){
        tempObj = new ShaderObject();
    // -------------------------------------- Audio Analysis
    }else if(objname == "audio analyzer"){
        tempObj = new AudioAnalyzer();
    }else if(objname == "audio device"){
        tempObj = new AudioDevice();
    }else if(objname == "beat extractor"){
        tempObj = new BeatExtractor();
    }else if(objname == "bpm extractor"){
        tempObj = new BPMExtractor();
    }else if(objname == "centroid extractor"){
        tempObj = new CentroidExtractor();
    }else if(objname == "dissonance extractor"){
        tempObj = new DissonanceExtractor();
    }else if(objname == "fft extractor"){
        tempObj = new FftExtractor();
    }else if(objname == "hfc extractor"){
        tempObj = new HFCExtractor();
    }else if(objname == "hpcp extractor"){
        tempObj = new HPCPExtractor();
    }else if(objname == "inharmonicity extractor"){
        tempObj = new InharmonicityExtractor();
    }else if(objname == "mel bands extractor"){
        tempObj = new MelBandsExtractor();
    }else if(objname == "mfcc extractor"){
        tempObj = new MFCCExtractor();
    }else if(objname == "onset extractor"){
        tempObj = new OnsetExtractor();
    }else if(objname == "pitch extractor"){
        tempObj = new PitchExtractor();
    }else if(objname == "power extractor"){
        tempObj = new PowerExtractor();
    }else if(objname == "rms extractor"){
        tempObj = new RMSExtractor();
    }else if(objname == "rolloff extractor"){
        tempObj = new RollOffExtractor();
    }else if(objname == "tristimulus extractor"){
        tempObj = new TristimulusExtractor();
    // -------------------------------------- Data
    }else if(objname == "vector concat"){
        tempObj = new VectorConcat();
    }else if(objname == "floats to vector"){
        tempObj = new FloatsToVector();
    // -------------------------------------- Sound
    }else if(objname == "soundfile player"){
        tempObj = new SoundfilePlayer();
    // -------------------------------------- Math
    }else if(objname == "add"){
        tempObj = new Add();
    }else if(objname == "clamp"){
        tempObj = new Clamp();
    }else if(objname == "constant"){
        tempObj = new Constant();
    }else if(objname == "divide"){
        tempObj = new Divide();
    }else if(objname == "metronome"){
        tempObj = new Metronome();
    }else if(objname == "multiply"){
        tempObj = new Multiply();
    }else if(objname == "simple random"){
        tempObj = new SimpleRandom();
    }else if(objname == "simple noise"){
        tempObj = new SimpleNoise();
    }else if(objname == "smooth"){
        tempObj = new Smooth();
    }else if(objname == "subtract"){
        tempObj = new Subtract();
    // -------------------------------------- Logic
    }else if(objname == "counter"){
        tempObj = new Counter();
    }else if(objname == "delay bang"){
        tempObj = new DelayBang();
    }else if(objname == "gate"){
        tempObj = new Gate();
    }else if(objname == "loadbang"){
        tempObj = new LoadBang();
    // -------------------------------------- GUI
    }else if(objname == "bang"){
        tempObj = new moBang();
    }else if(objname == "comment"){
        tempObj = new moComment();
    }else if(objname == "message"){
        tempObj = new moMessage();
    }else if(objname == "player controls"){
        tempObj = new moPlayerControls();
    }else if(objname == "slider"){
        tempObj = new moSlider();
    }else if(objname == "trigger"){
        tempObj = new moTrigger();
    }else if(objname == "video viewer"){
        tempObj = new moVideoViewer();
    }else if(objname == "signal viewer"){
        tempObj = new moSignalViewer();
    // -------------------------------------- VIDEO
    }else if(objname == "kinect grabber"){
        tempObj = new KinectGrabber();
    }else if(objname == "video player"){
        tempObj = new VideoPlayer();
    }else if(objname == "video grabber"){
        tempObj = new VideoGrabber();
    // -------------------------------------- WINDOWING
    }else if(objname == "output window"){
        tempObj = new OutputWindow();
    // -------------------------------------- COMPUTER VISION
    }else if(objname == "background subtraction"){
        tempObj = new BackgroundSubtraction();
    }else if(objname == "chroma key"){
        tempObj = new ChromaKey();
    }else if(objname == "color tracking"){
        tempObj = new ColorTracking();
    }else if(objname == "contour tracking"){
        tempObj = new ContourTracking();
    }else if(objname == "haar tracking"){
        tempObj = new HaarTracking();
    }else if(objname == "motion detection"){
        tempObj = new MotionDetection();
    }else if(objname == "optical flow"){
        tempObj = new OpticalFlow();
    }else{
        tempObj = nullptr;
    }


    return tempObj;
}

//--------------------------------------------------------------
void ofxVisualProgramming::newPatch(){
    string newFileName = "patch_"+ofGetTimestampString("%y%m%d")+alphabet.at(newFileCounter)+".xml";
    ofFile fileToRead(ofToDataPath("empty_patch.xml",true));
    ofFile newPatchFile(ofToDataPath("temp/"+newFileName,true));
    ofFile::copyFromTo(fileToRead.getAbsolutePath(),newPatchFile.getAbsolutePath(),true,true);
    newFileCounter++;

    currentPatchFile = newPatchFile.getAbsolutePath();
    openPatch(currentPatchFile);

    tempPatchFile = currentPatchFile;

    // Add System Blocks (Audio Device,...)
    if(dspON){
        glm::vec3 temp = canvas.screenToWorld(glm::vec3(ofGetWindowWidth()/2,ofGetWindowHeight()/2 + 100,0));
        addObject("audio device",ofVec2f(temp.x,temp.y));
    }

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
            dspON = XML.getValue("dsp",0);
            audioINDev = XML.getValue("audio_in_device",0);
            audioOUTDev = XML.getValue("audio_out_device",0);
            audioBufferSize = XML.getValue("buffer_size",0);

            audioDevices = soundStreamIN.getDeviceList();
            ofLog(OF_LOG_NOTICE," ");
            ofLog(OF_LOG_NOTICE,"------------------- AUDIO DEVICES");
            for(size_t i=0;i<audioDevices.size();i++){
                if(audioDevices[i].inputChannels > 0){
                    audioDevicesStringIN.push_back("  "+audioDevices[i].name);
                    audioDevicesID_IN.push_back(i);
                }
                if(audioDevices[i].outputChannels > 0){
                    audioDevicesStringOUT.push_back("  "+audioDevices[i].name);
                    audioDevicesID_OUT.push_back(i);
                }
                ofLog(OF_LOG_NOTICE,"Device[%zu]: %s (IN:%i - OUT:%i)",i,audioDevices[i].name.c_str(),audioDevices[i].inputChannels,audioDevices[i].outputChannels);
            }
            ofLog(OF_LOG_NOTICE," ");

            audioSampleRate = audioDevices[audioOUTDev].sampleRates[0];

            if(audioSampleRate < 44100){
                audioSampleRate = 44100;
            }

            XML.setValue("sample_rate_in",audioSampleRate);
            XML.setValue("sample_rate_out",audioSampleRate);
            XML.setValue("input_channels",static_cast<int>(audioDevices[audioINDev].inputChannels));
            XML.setValue("output_channels",static_cast<int>(audioDevices[audioOUTDev].outputChannels));
            XML.saveFile();

            if(dspON){
                engine.setChannels(audioDevices[audioINDev].inputChannels, audioDevices[audioOUTDev].outputChannels);
                this->setChannels(audioDevices[audioINDev].inputChannels,0);

                for(int in=0;in<audioDevices[audioINDev].inputChannels;in++){
                    engine.audio_in(in) >> this->in(in);
                }
                this->out_silent() >> engine.blackhole();

                ofLog(OF_LOG_NOTICE," ");
                ofLog(OF_LOG_NOTICE,"------------------- Soundstream INPUT Started on");
                ofLog(OF_LOG_NOTICE,"Audio device: %s",audioDevices[audioINDev].name.c_str());
                ofLog(OF_LOG_NOTICE," ");
                ofLog(OF_LOG_NOTICE,"------------------- Soundstream OUTPUT Started on");
                ofLog(OF_LOG_NOTICE,"Audio device: %s",audioDevices[audioOUTDev].name.c_str());
                ofLog(OF_LOG_NOTICE," ");
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
                    loaded = tempObj->loadConfig(mainWindow,engine,i,patchFile);
                    if(loaded){
                        tempObj->setIsRetina(isRetina);
                        ofAddListener(tempObj->dragEvent ,this,&ofxVisualProgramming::dragObject);
                        ofAddListener(tempObj->removeEvent ,this,&ofxVisualProgramming::removeObject);
                        ofAddListener(tempObj->resetEvent ,this,&ofxVisualProgramming::resetObject);
                        ofAddListener(tempObj->iconifyEvent ,this,&ofxVisualProgramming::iconifyObject);
                        ofAddListener(tempObj->duplicateEvent ,this,&ofxVisualProgramming::duplicateObject);
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

                                    if(connect(fromID,j,toObjectID,toInletID,linkType)){
                                        //ofLog(OF_LOG_NOTICE,"Connected object %s, outlet %i TO object %s, inlet %i",patchObjects[fromID]->getName().c_str(),z,patchObjects[toObjectID]->getName().c_str(),toInletID);
                                    }

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

        if(dspON){
            engine.setOutputDeviceID(audioDevices[audioOUTDev].deviceID);
            engine.setInputDeviceID(audioDevices[audioINDev].deviceID);
            engine.setup(audioSampleRate, audioBufferSize, 3);
        }

    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::savePatchAs(string patchFile){

    string newFileName = patchFile;
    ofFile fileToRead(currentPatchFile);
    ofFile newPatchFile(newFileName);
    ofFile::copyFromTo(fileToRead.getAbsolutePath(),newPatchFile.getAbsolutePath(),true,true);

    currentPatchFile = newFileName;

    for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        it->second->setPatchfile(currentPatchFile);
    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::setPatchVariable(string var, int value){
    ofxXmlSettings XML;

    if (XML.loadFile(currentPatchFile)){
        if (XML.pushTag("settings")){
            XML.setValue(var,value);
            XML.saveFile();
            XML.popTag();
        }
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::setAudioInDevice(int ind){

    bool found = false;
    int index = audioDevicesID_IN.at(ind);
    for(int i=0;i<audioDevices[index].sampleRates.size();i++){
        if(audioDevices[index].sampleRates[i] == audioSampleRate){
            found = true;
        }
    }

    if(!found){
        ofLog(OF_LOG_NOTICE," ");
        ofLog(OF_LOG_NOTICE,"------------------- INCOMPATIBLE INPUT DEVICE Sample Rate: %i", audioDevices[index].sampleRates[0]);
        ofLog(OF_LOG_NOTICE,"------------------- PLEASE SELECT ANOTHER INPUT DEVICE");
        return;
    }else{
        setPatchVariable("audio_in_device",index);
        audioINDev = index;

        setPatchVariable("sample_rate_in",audioSampleRate);
        setPatchVariable("input_channels",static_cast<int>(audioDevices[audioINDev].inputChannels));

        if(dspON){
            engine.setChannels(audioDevices[audioINDev].inputChannels, audioDevices[audioOUTDev].outputChannels);
            this->setChannels(audioDevices[audioINDev].inputChannels,0);

            for(int in=0;in<audioDevices[audioINDev].inputChannels;in++){
                engine.audio_in(in) >> this->in(in);
            }
            this->out_silent() >> engine.blackhole();

            ofLog(OF_LOG_NOTICE," ");
            ofLog(OF_LOG_NOTICE,"------------------- Soundstream INPUT Selected Device");
            ofLog(OF_LOG_NOTICE,"Audio device: %s",audioDevices[audioINDev].name.c_str());
            ofLog(OF_LOG_NOTICE," ");

            resetSpecificSystemObjects("audio device");

            engine.setOutputDeviceID(audioDevices[audioOUTDev].deviceID);
            engine.setInputDeviceID(audioDevices[audioINDev].deviceID);
            engine.setup(audioSampleRate, audioBufferSize, 3);
        }
    }
    
}

//--------------------------------------------------------------
void ofxVisualProgramming::setAudioOutDevice(int ind){

    int index = audioDevicesID_OUT.at(ind);

    setPatchVariable("audio_out_device",index);
    audioOUTDev = index;

    audioSampleRate = audioDevices[audioOUTDev].sampleRates[0];
    if(audioSampleRate < 44100){
        audioSampleRate = 44100;
    }

    setPatchVariable("sample_rate_out",audioSampleRate);
    setPatchVariable("output_channels",static_cast<int>(audioDevices[audioOUTDev].outputChannels));

    if(dspON){
        engine.setChannels(audioDevices[audioINDev].inputChannels, audioDevices[audioOUTDev].outputChannels);
        this->setChannels(audioDevices[audioINDev].inputChannels,0);

        for(int in=0;in<audioDevices[audioINDev].inputChannels;in++){
            engine.audio_in(in) >> this->in(in);
        }
        this->out_silent() >> engine.blackhole();

        ofLog(OF_LOG_NOTICE," ");
        ofLog(OF_LOG_NOTICE,"------------------- Soundstream OUTPUT Selected Device");
        ofLog(OF_LOG_NOTICE,"Audio device: %s",audioDevices[audioOUTDev].name.c_str());
        ofLog(OF_LOG_NOTICE," ");

        resetSpecificSystemObjects("audio device");

        engine.setOutputDeviceID(audioDevices[audioOUTDev].deviceID);
        engine.setInputDeviceID(audioDevices[audioINDev].deviceID);
        engine.setup(audioSampleRate, audioBufferSize, 3);

        bool found = false;
        for(int i=0;i<audioDevices[audioINDev].sampleRates.size();i++){
            if(audioDevices[audioINDev].sampleRates[i] == audioSampleRate){
                found = true;
            }
        }

        if(!found){
            ofLog(OF_LOG_NOTICE," ");
            ofLog(OF_LOG_NOTICE,"------------------- INCOMPATIBLE INPUT DEVICE Sample Rate: %i", audioDevices[audioINDev].sampleRates[0]);
            ofLog(OF_LOG_NOTICE,"------------------- PLEASE SELECT ANOTHER INPUT DEVICE");
        }
    }
    
}

//--------------------------------------------------------------
void ofxVisualProgramming::activateDSP(){

    if(audioDevices[audioINDev].inputChannels > 0 && audioDevices[audioOUTDev].outputChannels > 0){
        engine.setChannels(audioDevices[audioINDev].inputChannels, audioDevices[audioOUTDev].outputChannels);
        this->setChannels(audioDevices[audioINDev].inputChannels,0);

        for(int in=0;in<audioDevices[audioINDev].inputChannels;in++){
            engine.audio_in(in) >> this->in(in);
        }
        this->out_silent() >> engine.blackhole();

        ofLog(OF_LOG_NOTICE," ");
        ofLog(OF_LOG_NOTICE,"------------------- Soundstream INPUT Started on");
        ofLog(OF_LOG_NOTICE,"Audio device: %s",audioDevices[audioINDev].name.c_str());
        ofLog(OF_LOG_NOTICE," ");
        ofLog(OF_LOG_NOTICE,"------------------- Soundstream OUTPUT Started on");
        ofLog(OF_LOG_NOTICE,"Audio device: %s",audioDevices[audioOUTDev].name.c_str());
        ofLog(OF_LOG_NOTICE," ");

        engine.setOutputDeviceID(audioDevices[audioOUTDev].deviceID);
        engine.setInputDeviceID(audioDevices[audioINDev].deviceID);
        engine.setup(audioSampleRate, audioBufferSize, 3);
        engine.start();

        bool found = false;

        for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if(it->second->getName() == "audio device"){
                found = true;
                break;
            }
        }

        if(!found){
            glm::vec3 temp = canvas.screenToWorld(glm::vec3(ofGetWindowWidth()/2,ofGetWindowHeight()/2 + 100,0));
            addObject("audio device",ofVec2f(temp.x,temp.y));
        }

        setPatchVariable("dsp",1);
        dspON = true;
    }else{
        ofLog(OF_LOG_ERROR,"The selected audio devices couldn't be compatible or couldn't be properly installed in your system!");
    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::deactivateDSP(){
    setPatchVariable("dsp",0);
    dspON = false;
}

//--------------------------------------------------------------
void ofxVisualProgramming::onButtonEvent(ofxDatGuiButtonEvent e){
    for(int i=0;i<static_cast<int>(objectFolders.size());i++){
        if(e.target->getLabel() != objectFolders[i]->getLabel()){
            if(objectFolders[i]->isExpanded()){
                objectFolders[i]->collapse();
            }
        }
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::onScrollViewEvent(ofxDatGuiScrollViewEvent e){
    int cc = 0;
    for(map<string,vector<string>>::iterator it = objectsMatrix.begin(); it != objectsMatrix.end(); it++ ){
        for(int j=0;j<static_cast<int>(it->second.size());j++){
            if(it->second.at(j) == e.target->getLabel()){
                if(objectFolders.at(cc)->isExpanded()){
                    addObject(e.target->getLabel(),ofVec2f(canvas.getMovingPoint().x + 200*scaleFactor,canvas.getMovingPoint().y));
                    break;
                }
            }
        }
        cc++;
    }
}
