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

    vecInit = {"audio analyzer","beat extractor","bpm extractor","centroid extractor","dissonance extractor","fft extractor","hfc extractor","hpcp extractor","inharmonicity extractor","mel bands extractor","mfcc extractor","onset extractor","pitch extractor","power extractor","rms extractor","rolloff extractor","tristimulus extractor"};
    objectsMatrix["audio_analysis"] = vecInit;

    vecInit = {"arduino serial","key pressed","key released","midi key","midi knob","midi pad","midi receiver","midi score","midi sender","osc receiver","osc sender"};
    objectsMatrix["communications"] = vecInit;

#if defined(TARGET_LINUX) || defined(TARGET_OSX)
    vecInit = {"background subtraction","chroma key","color tracking","contour tracking","face tracker","haar tracking","motion detection","optical flow"};
#elif defined(TARGET_WIN32)
    vecInit = {"background subtraction","chroma key","color tracking","contour tracking","haar tracking","motion detection","optical flow"};
#endif
    objectsMatrix["computer vision"] = vecInit;

    vecInit = {"bang multiplexer","bang to float","data to file","data to texture","file to data","floats to vector","texture to data","vector at","vector concat","vector gate","vector multiply"};
    objectsMatrix["data"] = vecInit;

    vecInit = {"image exporter","image loader"};
    objectsMatrix["graphics"] = vecInit;

    vecInit = {"2d pad","bang","comment","message","player controls","signal viewer","slider","sonogram","timeline","trigger","video viewer","vu meter"};
    objectsMatrix["gui"] = vecInit;

    vecInit = {"&&","||","==","!=",">","<","counter","delay bang","delay float","gate","inverter","loadbang","select","spigot","timed semaphore"};
    objectsMatrix["logic"] = vecInit;

    vecInit = {"add","clamp","constant","divide","map","metronome","modulus","multiply","range","simple noise","simple random","smooth","subtract"};
    objectsMatrix["math"] = vecInit;

#if defined(TARGET_LINUX) || defined(TARGET_OSX)
    vecInit = {"bash script","lua script","processing script","python script","shader object"};
#elif defined(TARGET_WIN32)
    vecInit = {"lua script","processing script","python script","shader object"};
#endif
    objectsMatrix["scripting"] = vecInit;

    vecInit = {"ADSR envelope","AHR envelope","amp","audio exporter","audio gate","bit cruncher","bit noise","chorus","comb filter","compressor","crossfader","data oscillator","decimator","delay","ducker","hi pass","lfo","low pass","mixer","note to frequency","panner","pd patch","pulse","quad panner","resonant 2pole filter","reverb","saw","signal trigger","sine","soundfile player","triangle","white noise"};
    objectsMatrix["sound"] = vecInit;

#if defined(TARGET_LINUX) || defined(TARGET_OSX)
    vecInit = {"kinect grabber","video crop","video feedback","video exporter","video gate","video grabber","video player","video receiver","video sender","video streaming","video timedelay","video transform"};
#elif defined(TARGET_WIN32)
   vecInit = {"kinect grabber","video crop","video feedback","video exporter","video gate","video grabber","video player","video streaming","video timedelay","video transform"};
#endif
    objectsMatrix["video"] = vecInit;

    vecInit = {"live patching","output window","projection mapping"};
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
    TIME_SAMPLE_GET_INSTANCE()->setAutoDraw(false);
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

    engine                  = new pdsp::Engine();

    font                    = new ofxFontStash();
    fontSize                = 12;
    isRetina                = false;
    scaleFactor             = 1;
    linkActivateDistance    = 5;
    isVPDragging            = false;

    isOutletSelected        = false;
    selectedObjectLinkType  = -1;
    selectedObjectLink      = -1;
    selectedObjectID        = -1;
    draggingObjectID        = -1;
    pressedObjectID         = -1;
    lastAddedObjectID       = -1;
    draggingObject          = false;
    bLoadingNewObject       = false;
    bLoadingNewPatch        = false;

    livePatchingObiID       = -1;

    currentPatchFile        = "empty_patch.xml";
    currentPatchFolderPath  = ofToDataPath("temp/");

    resetTime               = ofGetElapsedTimeMillis();
    wait                    = 2000;

    alphabet                = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXZY";
    newFileCounter          = 0;

    audioSampleRate         = 44100;
    dspON                   = false;

    inited                  = false;

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

    // RESET TEMP FOLDER
    resetTempFolder();

    // Threaded File Dialogs
    fileDialog.setup();
    ofAddListener(fileDialog.fileDialogEvent, this, &ofxVisualProgramming::onFileDialogResponse);

    // INIT OBJECTS
    initObjectMatrix();

    // Create new empty file patch
    newPatch();

}

//--------------------------------------------------------------
void ofxVisualProgramming::update(){

    // canvas init
    if(!inited){
        inited = true;
        canvasViewport.set(0,20,ofGetWindowWidth(),ofGetWindowHeight());

        // RETINA FIX
        if(ofGetScreenWidth() >= RETINA_MIN_WIDTH && ofGetScreenHeight() >= RETINA_MIN_HEIGHT){
            canvas.setScale(2);
        }
    }

    // Sound Context
    unique_lock<std::mutex> lock(inputAudioMutex);
    {

    }

    // Graphical Context
    canvas.update();

    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        TS_START(it->second->getName()+ofToString(it->second->getId())+"_update");
        it->second->update(patchObjects,fileDialog);
        TS_STOP(it->second->getName()+ofToString(it->second->getId())+"_update");

        if(draggingObject && draggingObjectID == it->first){
            it->second->mouseDragged(actualMouse.x,actualMouse.y);
        }

        // update scripts objects files map
        ofFile tempsofp(it->second->getFilepath());
        string fileExt = ofToUpper(tempsofp.getExtension());
        if(fileExt == "LUA" || fileExt == "PY" || fileExt == "SH" || fileExt == "JAVA" || fileExt == "FRAG"){
            map<string,string>::iterator sofpIT = scriptsObjectsFilesPaths.find(tempsofp.getFileName());
            if (sofpIT == scriptsObjectsFilesPaths.end()){
                // not found, insert it
                scriptsObjectsFilesPaths.insert( pair<string,string>(tempsofp.getFileName(),tempsofp.getAbsolutePath()) );
            }
        }

    }
    // Clear map from deleted objects
    if(ofGetElapsedTimeMillis()-resetTime > wait){
        resetTime = ofGetElapsedTimeMillis();
        eraseIndexes.clear();
        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if(it->second->getWillErase()){
                eraseIndexes.push_back(it->first);

            }
        }
        for(int x=0;x<static_cast<int>(eraseIndexes.size());x++){
            for(int p=0;p<static_cast<int>(patchObjects.at(eraseIndexes.at(x))->outPut.size());p++){
                patchObjects[patchObjects.at(eraseIndexes.at(x))->outPut.at(p)->toObjectID]->inletsConnected.at(patchObjects.at(eraseIndexes.at(x))->outPut.at(p)->toInletID) = false;
            }

            // remove scripts objects filepath reference from scripts objects files map
            ofFile tempsofp(patchObjects.at(eraseIndexes.at(x))->getFilepath());
            string fileExt = ofToUpper(tempsofp.getExtension());
            if(fileExt == "LUA" || fileExt == "PY" || fileExt == "SH" || fileExt == "JAVA" || fileExt == "FRAG"){
                map<string,string>::iterator sofpIT = scriptsObjectsFilesPaths.find(tempsofp.getFileName());
                if (sofpIT != scriptsObjectsFilesPaths.end()){
                    // found it, remove it
                    scriptsObjectsFilesPaths.erase(sofpIT);
                }
            }

            patchObjects.at(eraseIndexes.at(x))->removeObjectContent(true);
            patchObjects.erase(eraseIndexes.at(x));
        }

    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::updateCanvasViewport(){
    canvasViewport.set(0,20,ofGetWindowWidth(),ofGetWindowHeight());
}

//--------------------------------------------------------------
void ofxVisualProgramming::draw(){

    TSGL_START("draw");

    ofPushView();
    ofPushStyle();
    ofPushMatrix();

    canvas.begin(canvasViewport);

    ofEnableAlphaBlending();
    ofSetCurveResolution(50);
    ofSetColor(255);
    ofSetLineWidth(1);

    livePatchingObiID = -1;

    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        TS_START(it->second->getName()+ofToString(it->second->getId())+"_draw");
        if(it->second->getName() == "live patching"){
           livePatchingObiID = it->second->getId();
        }
        it->second->draw(font);
        TS_STOP(it->second->getName()+ofToString(it->second->getId())+"_draw");
    }

    // draw outlet cables with var name
    if(selectedObjectLink >= 0 && isOutletSelected){
        int lt = patchObjects[selectedObjectID]->getOutletType(selectedObjectLink);
        switch(lt) {
        case 0: ofSetColor(COLOR_NUMERIC_LINK);
            break;
        case 1: ofSetColor(COLOR_STRING_LINK);
            break;
        case 2: ofSetColor(COLOR_ARRAY_LINK);
            break;
        case 3: ofSetColor(COLOR_TEXTURE_LINK); ofSetLineWidth(2);
            break;
        case 4: ofSetColor(COLOR_AUDIO_LINK); ofSetLineWidth(2);
            break;
        case 5: ofSetColor(COLOR_SCRIPT_LINK); ofSetLineWidth(1);
            break;
        default: break;
        }
        ofDrawLine(patchObjects[selectedObjectID]->getOutletPosition(selectedObjectLink).x, patchObjects[selectedObjectID]->getOutletPosition(selectedObjectLink).y, canvas.getMovingPoint().x,canvas.getMovingPoint().y);

        // Draw outlet type name
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
            font->draw(patchObjects[selectedObjectID]->linkTypeName+" "+patchObjects[selectedObjectID]->getOutletName(selectedObjectLink),fontSize/2,canvas.getMovingPoint().x + (10*scaleFactor),canvas.getMovingPoint().y);
        }else{
            font->draw(patchObjects[selectedObjectID]->linkTypeName+" "+patchObjects[selectedObjectID]->getOutletName(selectedObjectLink),fontSize,canvas.getMovingPoint().x + (10*scaleFactor),canvas.getMovingPoint().y);
        }

    }
    
    canvas.end();

    // Draw Bottom Bar
    ofSetColor(0,0,0,60);
    ofDrawRectangle(0,ofGetHeight() - (18*scaleFactor),ofGetWidth(),(18*scaleFactor));

    // DSP flag
    if(dspON){
        ofSetColor(ofColor::fromHex(0xFFD00B));
        font->draw("DSP ON",fontSize,10*scaleFactor,ofGetHeight() - (6*scaleFactor));
    }else{
        ofSetColor(ofColor::fromHex(0x777777));
        font->draw("DSP OFF",fontSize,10*scaleFactor,ofGetHeight() - (6*scaleFactor));
    }


    ofDisableAlphaBlending();

    ofPopMatrix();
    ofPopStyle();
    ofPopView();

    // LIVE PATCHING SESSION
    drawLivePatchingSession();

    // Profiler
    TIME_SAMPLE_GET_INSTANCE()->draw(ofGetWidth() / TIME_SAMPLE_GET_INSTANCE()->getUiScale() - TIME_SAMPLE_GET_INSTANCE()->getWidth() - (TIME_SAMPLE_GET_INSTANCE()->getUiScale()*5),ofGetHeight() / TIME_SAMPLE_GET_INSTANCE()->getUiScale() - TIME_SAMPLE_GET_INSTANCE()->getHeight() - (TIME_SAMPLE_GET_INSTANCE()->getUiScale()*5) - TIME_SAMPLE_GET_INSTANCE()->getPlotsHeight());

    TSGL_STOP("draw");

}

//--------------------------------------------------------------
void ofxVisualProgramming::drawLivePatchingSession(){
    if(weAlreadyHaveObject("live patching") && livePatchingObiID != -1){
        if(patchObjects[livePatchingObiID]->inletsConnected[0] && static_cast<ofTexture *>(patchObjects[livePatchingObiID]->_inletParams[0])->isAllocated()){
            float lpDrawW, lpDrawH, lpPosX, lpPosY;
            if(static_cast<ofTexture *>(patchObjects[livePatchingObiID]->_inletParams[0])->getWidth() >= static_cast<ofTexture *>(patchObjects[livePatchingObiID]->_inletParams[0])->getHeight()){   // horizontal texture
                lpDrawW           = ofGetWidth();
                lpDrawH           = (ofGetWidth()/static_cast<ofTexture *>(patchObjects[livePatchingObiID]->_inletParams[0])->getWidth())*static_cast<ofTexture *>(patchObjects[livePatchingObiID]->_inletParams[0])->getHeight();
                lpPosX            = 0;
                lpPosY            = (ofGetHeight()-lpDrawH)/2.0f;
            }else{ // vertical texture
                lpDrawW           = (static_cast<ofTexture *>(patchObjects[livePatchingObiID]->_inletParams[0])->getWidth()*ofGetHeight())/static_cast<ofTexture *>(patchObjects[livePatchingObiID]->_inletParams[0])->getHeight();
                lpDrawH           = ofGetHeight();
                lpPosX            = (ofGetWidth()-lpDrawW)/2.0f;
                lpPosY            = 0;
            }
            ofSetColor(255,127);
            static_cast<ofTexture *>(patchObjects[livePatchingObiID]->_inletParams[0])->draw(lpPosX,lpPosY,lpDrawW,lpDrawH);
        }
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::resetTempFolder(){
    ofDirectory dir;
    dir.listDir(ofToDataPath("temp/"));
    for(size_t i = 0; i < dir.size(); i++){
        if(dir.getFile(i).isDirectory()){
            ofDirectory temp;
            temp.removeDirectory(dir.getFile(i).getAbsolutePath(),true,true);
        }else{
            dir.getFile(i).remove();
        }

    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::cleanPatchDataFolder(){
    ofDirectory dir;
    // get patch data folder
    dir.listDir(currentPatchFolderPath+"data/");

    for(size_t i = 0; i < dir.size(); i++){
        if(dir.getFile(i).isFile()){
            map<string,string>::iterator sofpIT = scriptsObjectsFilesPaths.find(dir.getFile(i).getFileName());
            if (sofpIT == scriptsObjectsFilesPaths.end()){
                // not found in patch scripts map, remove it from patch data folder
                //ofLog(OF_LOG_NOTICE,"%s",dir.getFile(i).getAbsolutePath().c_str());
                string fileExt = ofToUpper(dir.getFile(i).getExtension());
                if(fileExt == "PY" || fileExt == "SH" || fileExt == "JAVA" || fileExt == "FRAG"){
                    dir.getFile(i).remove();
                }
                // remove if filename is empty
                string tfn = dir.getFile(i).getFileName();
                if(dir.getFile(i).getFileName().substr(0,tfn.find_last_of('.')) == "empty"){
                    dir.getFile(i).remove();
                }
                // remove alone .vert files
                if(fileExt == "VERT"){
                    string vsName = dir.getFile(i).getFileName();
                    string fsName = dir.getFile(i).getFileName().substr(0,vsName.find_last_of('.'))+".frag";
                    map<string,string>::iterator sofpIT2 = scriptsObjectsFilesPaths.find(fsName);
                    if (sofpIT2 == scriptsObjectsFilesPaths.end()){
                        // related fragment shader not found in patch scripts map, remove it from patch data folder
                        dir.getFile(i).remove();
                    }
                }
            }
        }
    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::exit(){

    //savePatchAsLast();

    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        it->second->removeObjectContent();
    }

    fileDialog.stop();

    if(dspON){
        deactivateDSP();
    }

    cleanPatchDataFolder();

    resetTempFolder();
}

//--------------------------------------------------------------
void ofxVisualProgramming::mouseMoved(ofMouseEventArgs &e){

    actualMouse = ofVec2f(canvas.getMovingPoint().x,canvas.getMovingPoint().y);

    // CANVAS
    if(!isHoverMenu && !isHoverLogger && !isHoverCodeEditor){
        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            it->second->mouseMoved(actualMouse.x,actualMouse.y);
            it->second->setIsActive(false);
            if (it->second->isOver(actualMouse)){
                activeObject(it->first);
            }
        }
    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::mouseDragged(ofMouseEventArgs &e){

    isVPDragging = true;

    actualMouse = ofVec2f(canvas.getMovingPoint().x,canvas.getMovingPoint().y);

    // CANVAS
    if(!isHoverMenu && !isHoverLogger && !isHoverCodeEditor){

        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){

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

    }

    if(selectedObjectLink == -1 && !draggingObject && !isHoverMenu && !isHoverLogger && !isHoverCodeEditor){
        canvas.mouseDragged(e);
    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::mousePressed(ofMouseEventArgs &e){

    actualMouse = ofVec2f(canvas.getMovingPoint().x,canvas.getMovingPoint().y);

    canvas.mousePressed(e);

    isOutletSelected = false;
    selectedObjectLink = -1;
    selectedObjectLinkType = -1;
    pressedObjectID = -1;

    if(!isHoverMenu && !isHoverLogger && !isHoverCodeEditor){

        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if(patchObjects[it->first] != nullptr){
                it->second->setIsObjectSelected(false);
                if(it->second->isOver(ofPoint(actualMouse.x,actualMouse.y,0))){
                    it->second->setIsObjectSelected(true);
                    pressedObjectID = it->first;
                }
                for(int p=0;p<it->second->getNumInlets();p++){
                    if(it->second->getInletPosition(p).distance(actualMouse) < linkActivateDistance && !it->second->headerBox->inside(ofVec3f(actualMouse.x,actualMouse.y,0))){
                        isOutletSelected = false;
                        selectedObjectID = it->first;
                        selectedObjectLink = p;
                        selectedObjectLinkType = it->second->getInletType(p);
                        it->second->setIsActive(false);
                        break;
                    }
                }
                for(int p=0;p<it->second->getNumOutlets();p++){
                    if(it->second->getOutletPosition(p).distance(actualMouse) < linkActivateDistance && !it->second->headerBox->inside(ofVec3f(actualMouse.x,actualMouse.y,0))){
                        isOutletSelected = true;
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
            for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
                if(patchObjects[it->first] != nullptr){
                    if(it->second->getIsActive()){
                        isOutletSelected = false;
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

    isVPDragging = false;

    actualMouse = ofVec2f(canvas.getMovingPoint().x,canvas.getMovingPoint().y);

    canvas.mouseReleased(e);

    bool isLinked = false;

    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        it->second->mouseReleased(actualMouse.x,actualMouse.y,patchObjects);
    }

    if(selectedObjectLinkType != -1 && selectedObjectLink != -1 && selectedObjectID != -1 && !patchObjects.empty() && isOutletSelected){
        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if(selectedObjectID != it->first){
                for (int j=0;j<it->second->getNumInlets();j++){
                    if(it->second->getInletPosition(j).distance(actualMouse) < linkActivateDistance){
                        // if previously connected, disconnect and refresh connection
                        if(it->second->inletsConnected.at(j)){
                            // Disconnect selected --> inlet link
                            disconnectSelected(it->first,j);
                        }
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

    if(!isLinked && selectedObjectLinkType != -1 && selectedObjectLink != -1 && selectedObjectID != -1 && !patchObjects.empty() && patchObjects[selectedObjectID] != nullptr && patchObjects[selectedObjectID]->outPut.size()>0 && isOutletSelected){
        vector<bool> tempEraseLinks;
        for(int j=0;j<static_cast<int>(patchObjects[selectedObjectID]->outPut.size());j++){
            //ofLog(OF_LOG_NOTICE,"Object %i have link to %i",selectedObjectID,patchObjects[selectedObjectID]->outPut[j]->toObjectID);
            if(patchObjects[selectedObjectID]->outPut[j]->fromOutletID == selectedObjectLink){
                tempEraseLinks.push_back(true);
            }else{
                tempEraseLinks.push_back(false);
            }
        }

        vector<shared_ptr<PatchLink>> tempBuffer;
        tempBuffer.reserve(patchObjects[selectedObjectID]->outPut.size()-tempEraseLinks.size());

        for (size_t i=0; i<patchObjects[selectedObjectID]->outPut.size(); i++){
            if(!tempEraseLinks[i]){
                tempBuffer.push_back(patchObjects[selectedObjectID]->outPut[i]);
            }else{
                patchObjects[selectedObjectID]->removeLinkFromConfig(selectedObjectLink);
                if(patchObjects[patchObjects[selectedObjectID]->outPut[i]->toObjectID] != nullptr){
                    patchObjects[patchObjects[selectedObjectID]->outPut[i]->toObjectID]->inletsConnected[patchObjects[selectedObjectID]->outPut[i]->toInletID] = false;
                    if(patchObjects[selectedObjectID]->getIsPDSPPatchableObject() || patchObjects[selectedObjectID]->getName() == "audio device"){
                        patchObjects[selectedObjectID]->pdspOut[i].disconnectOut();
                    }
                    if(patchObjects[patchObjects[selectedObjectID]->outPut[i]->toObjectID]->getIsPDSPPatchableObject()){
                        patchObjects[patchObjects[selectedObjectID]->outPut[i]->toObjectID]->pdspIn[patchObjects[selectedObjectID]->outPut[i]->toInletID].disconnectIn();
                    }
                }
                //ofLog(OF_LOG_NOTICE,"Removed link from %i to %i",selectedObjectID,patchObjects[selectedObjectID]->outPut[i]->toObjectID);
            }
        }

        patchObjects[selectedObjectID]->outPut = tempBuffer;

    }else if(!isLinked && selectedObjectLinkType != -1 && selectedObjectLink != -1 && selectedObjectID != -1 && !patchObjects.empty() && patchObjects[selectedObjectID] != nullptr && !isOutletSelected){
        // Disconnect selected --> inlet link
        disconnectSelected(selectedObjectID,selectedObjectLink);

    }

    isOutletSelected        = false;
    selectedObjectLinkType  = -1;
    selectedObjectLink      = -1;

    draggingObject          = false;
    draggingObjectID        = -1;

}

//--------------------------------------------------------------
void ofxVisualProgramming::mouseScrolled(ofMouseEventArgs &e){
    if(!isHoverLogger && !isHoverMenu && !isHoverCodeEditor){
        canvas.mouseScrolled(e);
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::keyPressed(ofKeyEventArgs &e){
    if(!isHoverCodeEditor){
        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            it->second->keyPressed(e.key);
        }
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::onFileDialogResponse(ofxThreadedFileDialogResponse &response){
    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        it->second->fileDialogResponse(response);
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
                if(!bLoadingNewPatch){
                    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
                        it->second->audioIn(inputBuffer);
                    }
                }


                unique_lock<std::mutex> lock(inputAudioMutex);
                lastInputBuffer = inputBuffer;
            }
            if(audioDevices[audioOUTDev].outputChannels > 0){
                // compute audio output
                if(!bLoadingNewPatch){
                    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
                        it->second->audioOut(emptyBuffer);
                    }
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

        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
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

    // check if object exists
    bool exists = false;
    for(map<string,vector<string>>::iterator it = objectsMatrix.begin(); it != objectsMatrix.end(); it++ ){
        for(int j=0;j<static_cast<int>(it->second.size());j++){
            if(it->second.at(j) == name){
                exists = true;
                break;
            }
        }
    }
    if(!exists && name != "audio device"){
        return;
    }

    // discard special unique objects
    if(name == "live patching" && weAlreadyHaveObject(name)){
        ofLog(OF_LOG_WARNING,"%s object is a single instance type of object, a Mosaic patch can't contain more than one.",name.c_str());
        return;
    }

    bLoadingNewObject       = true;

    shared_ptr<PatchObject> tempObj = selectObject(name);

    tempObj->newObject();
    tempObj->setPatchfile(currentPatchFile);
    tempObj->setup(mainWindow);
    tempObj->setupDSP(*engine);
    tempObj->move(static_cast<int>(pos.x-(OBJECT_STANDARD_WIDTH/2*scaleFactor)),static_cast<int>(pos.y-(OBJECT_STANDARD_HEIGHT/2*scaleFactor)));
    tempObj->setIsRetina(isRetina);
    ofAddListener(tempObj->dragEvent ,this,&ofxVisualProgramming::dragObject);
    ofAddListener(tempObj->removeEvent ,this,&ofxVisualProgramming::removeObject);
    ofAddListener(tempObj->resetEvent ,this,&ofxVisualProgramming::resetObject);
    ofAddListener(tempObj->reconnectOutletsEvent ,this,&ofxVisualProgramming::reconnectObjectOutlets);
    ofAddListener(tempObj->iconifyEvent ,this,&ofxVisualProgramming::iconifyObject);
    ofAddListener(tempObj->duplicateEvent ,this,&ofxVisualProgramming::duplicateObject);

    actualObjectID++;

    bool saved = tempObj->saveConfig(false,actualObjectID);

    if(saved){
        patchObjects[tempObj->getId()] = tempObj;
        patchObjects[tempObj->getId()]->fixCollisions(patchObjects);
        lastAddedObjectID = tempObj->getId();
    }

    bLoadingNewObject       = false;

}

//--------------------------------------------------------------
shared_ptr<PatchObject> ofxVisualProgramming::getLastAddedObject(){
    if(lastAddedObjectID != -1 && patchObjects.count(lastAddedObjectID) > 0){
        return patchObjects[lastAddedObjectID];
    }else{
        return nullptr;
    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::dragObject(int &id){

}

//--------------------------------------------------------------
void ofxVisualProgramming::resetObject(int &id){
    if ((id != -1) && (patchObjects[id] != nullptr)){

        ofxXmlSettings XML;
        if (XML.loadFile(currentPatchFile)){

            for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
                vector<shared_ptr<PatchLink>> tempBuffer;
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
        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            vector<shared_ptr<PatchLink>> tempBuffer;
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
void ofxVisualProgramming::reconnectObjectOutlets(int &id){
    if ((id != -1) && (patchObjects[id] != nullptr)){

        ofxXmlSettings XML;
        if (XML.loadFile(currentPatchFile)){
            int totalObjects = XML.getNumTags("object");

            // relink object outlets from XML
            for(int i=0;i<totalObjects;i++){
                if(XML.pushTag("object", i)){
                    if(XML.getValue("id", -1) == id){
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
                    XML.popTag();
                }
            }

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

        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            vector<shared_ptr<PatchLink>> tempBuffer;
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
void ofxVisualProgramming::deleteSelectedObject(){
    if(pressedObjectID != -1){
        if(!patchObjects[pressedObjectID]->getIsSystemObject()){
            deleteObject(pressedObjectID);
            patchObjects[pressedObjectID]->setWillErase(true);
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

        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            vector<shared_ptr<PatchLink>> tempBuffer;
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
    // disable duplicate for hardware&system related objects
    if(patchObjects[id]->getName() != "video grabber" && patchObjects[id]->getName() != "kinect grabber" && patchObjects[id]->getName() != "live patching" && patchObjects[id]->getName() != "projection mapping"){
        ofVec2f newPos = ofVec2f(patchObjects[id]->getPos().x + patchObjects[id]->getObjectWidth(),patchObjects[id]->getPos().y);
        addObject(patchObjects[id]->getName(),patchObjects[id]->getPos());
    }else{
        ofLog(OF_LOG_NOTICE,"'%s' is one of the Mosaic objects that can't (for now) be duplicated due to hardware/system related issues.",patchObjects[id]->getName().c_str());
    }
}

//--------------------------------------------------------------
bool ofxVisualProgramming::connect(int fromID, int fromOutlet, int toID,int toInlet, int linkType){
    bool connected = false;

    if((fromID != -1) && (patchObjects[fromID] != nullptr) && (toID != -1) && (patchObjects[toID] != nullptr) && (patchObjects[fromID]->getOutletType(fromOutlet) == patchObjects[toID]->getInletType(toInlet)) && !patchObjects[toID]->inletsConnected[toInlet]){

        //cout << "Mosaic :: "<< "Connect object " << patchObjects[fromID]->getName().c_str() << ":" << ofToString(fromID) << " to object " << patchObjects[toID]->getName().c_str() << ":" << ofToString(toID) << endl;

        shared_ptr<PatchLink> tempLink = shared_ptr<PatchLink>(new PatchLink());

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

        if(tempLink->type == VP_LINK_NUMERIC){
            patchObjects[toID]->_inletParams[toInlet] = new float();
        }else if(tempLink->type == VP_LINK_STRING){
            patchObjects[toID]->_inletParams[toInlet] = new string();
        }else if(tempLink->type == VP_LINK_ARRAY){
            patchObjects[toID]->_inletParams[toInlet] = new vector<float>();
        }else if(tempLink->type == VP_LINK_TEXTURE){
            patchObjects[toID]->_inletParams[toInlet] = new ofTexture();
        }else if(tempLink->type == VP_LINK_AUDIO){
            patchObjects[toID]->_inletParams[toInlet] = new ofSoundBuffer();
            if(patchObjects[fromID]->getIsPDSPPatchableObject() && patchObjects[toID]->getIsPDSPPatchableObject()){
                patchObjects[fromID]->pdspOut[fromOutlet] >> patchObjects[toID]->pdspIn[toInlet];
            }else if(patchObjects[fromID]->getName() == "audio device" && patchObjects[toID]->getIsPDSPPatchableObject()){
                patchObjects[fromID]->pdspOut[fromOutlet] >> patchObjects[toID]->pdspIn[toInlet];
            }
        }

        checkSpecialConnection(fromID,toID,linkType);

        connected = true;
    }

    return connected;
}

//--------------------------------------------------------------
void ofxVisualProgramming::disconnectSelected(int objID, int objLink){

    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        for(int j=0;j<static_cast<int>(it->second->outPut.size());j++){
            if(it->second->outPut[j]->toObjectID == objID && it->second->outPut[j]->toInletID == objLink){
                // remove link
                vector<bool> tempEraseLinks;
                for(int s=0;s<static_cast<int>(it->second->outPut.size());s++){
                    if(it->second->outPut[s]->toObjectID == objID && it->second->outPut[s]->toInletID == objLink){
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
                        if(patchObjects[objID] != nullptr){
                            patchObjects[objID]->inletsConnected[objLink] = false;
                            if(patchObjects[objID]->getIsPDSPPatchableObject()){
                                patchObjects[objID]->pdspIn[objLink].disconnectIn();
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
    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->second->getIsSystemObject()){
            it->second->resetSystemObject();
            resetObject(it->second->getId());
            if(it->second->getIsAudioOUTObject()){
                it->second->setupAudioOutObjectContent(*engine);
            }
        }
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::resetSpecificSystemObjects(string name){
    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->second->getIsSystemObject() && it->second->getName() == name){
            it->second->resetSystemObject();
            resetObject(it->second->getId());
            if(it->second->getIsAudioOUTObject()){
                it->second->setupAudioOutObjectContent(*engine);
            }
        }
    }
}

//--------------------------------------------------------------
bool ofxVisualProgramming::weAlreadyHaveObject(string name){
    bool found = false;

    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->second->getName() == name){
            found = true;
            break;
        }
    }

    return found;
}

//--------------------------------------------------------------
shared_ptr<PatchObject> ofxVisualProgramming::selectObject(string objname){
    shared_ptr<PatchObject> tempObj;

#if defined(TARGET_LINUX) || defined(TARGET_OSX)
    if(objname == "bash script"){
        tempObj = shared_ptr<PatchObject>(new BashScript());
    }else if(objname == "face tracker"){
        tempObj = shared_ptr<PatchObject>(new FaceTracker());
    }else if(objname == "video receiver"){
        tempObj = shared_ptr<PatchObject>(new VideoReceiver());
    }else if(objname == "video sender"){
        tempObj = shared_ptr<PatchObject>(new VideoSender());
    }else
#endif

    if(objname == "python script"){
        tempObj = shared_ptr<PatchObject>(new PythonScript());
    }else if(objname == "lua script"){
        tempObj = shared_ptr<PatchObject>(new LuaScript());
    }else if(objname == "processing script"){
        tempObj = shared_ptr<PatchObject>(new ProcessingScript());
    }else if(objname == "shader object"){
        tempObj = shared_ptr<PatchObject>(new ShaderObject());
    // -------------------------------------- Audio Analysis
    }else if(objname == "audio analyzer"){
        tempObj = shared_ptr<PatchObject>(new AudioAnalyzer());
    }else if(objname == "audio device"){
        tempObj = shared_ptr<PatchObject>(new AudioDevice());
    }else if(objname == "beat extractor"){
        tempObj = shared_ptr<PatchObject>(new BeatExtractor());
    }else if(objname == "bpm extractor"){
        tempObj = shared_ptr<PatchObject>(new BPMExtractor());
    }else if(objname == "centroid extractor"){
        tempObj = shared_ptr<PatchObject>(new CentroidExtractor());
    }else if(objname == "dissonance extractor"){
        tempObj = shared_ptr<PatchObject>(new DissonanceExtractor());
    }else if(objname == "fft extractor"){
        tempObj = shared_ptr<PatchObject>(new FftExtractor());
    }else if(objname == "hfc extractor"){
        tempObj = shared_ptr<PatchObject>(new HFCExtractor());
    }else if(objname == "hpcp extractor"){
        tempObj = shared_ptr<PatchObject>(new HPCPExtractor());
    }else if(objname == "inharmonicity extractor"){
        tempObj = shared_ptr<PatchObject>(new InharmonicityExtractor());
    }else if(objname == "mel bands extractor"){
        tempObj = shared_ptr<PatchObject>(new MelBandsExtractor());
    }else if(objname == "mfcc extractor"){
        tempObj = shared_ptr<PatchObject>(new MFCCExtractor());
    }else if(objname == "onset extractor"){
        tempObj = shared_ptr<PatchObject>(new OnsetExtractor());
    }else if(objname == "pitch extractor"){
        tempObj = shared_ptr<PatchObject>(new PitchExtractor());
    }else if(objname == "power extractor"){
        tempObj = shared_ptr<PatchObject>(new PowerExtractor());
    }else if(objname == "rms extractor"){
        tempObj = shared_ptr<PatchObject>(new RMSExtractor());
    }else if(objname == "rolloff extractor"){
        tempObj = shared_ptr<PatchObject>(new RollOffExtractor());
    }else if(objname == "tristimulus extractor"){
        tempObj = shared_ptr<PatchObject>(new TristimulusExtractor());
    // -------------------------------------- Communications
    }else if(objname == "arduino serial"){
        tempObj = shared_ptr<PatchObject>(new ArduinoSerial());
    }else if(objname == "key pressed"){
        tempObj = shared_ptr<PatchObject>(new KeyPressed());
    }else if(objname == "key released"){
        tempObj = shared_ptr<PatchObject>(new KeyReleased());
    }else if(objname == "midi key"){
        tempObj = shared_ptr<PatchObject>(new MidiKey());
    }else if(objname == "midi knob"){
        tempObj = shared_ptr<PatchObject>(new MidiKnob());
    }else if(objname == "midi pad"){
        tempObj = shared_ptr<PatchObject>(new MidiPad());
    }else if(objname == "midi receiver"){
        tempObj = shared_ptr<PatchObject>(new MidiReceiver());
    }else if(objname == "midi score"){
        tempObj = shared_ptr<PatchObject>(new MidiScore());
    }else if(objname == "midi sender"){
        tempObj = shared_ptr<PatchObject>(new MidiSender());
    }else if(objname == "osc receiver"){
        tempObj = shared_ptr<PatchObject>(new OscReceiver());
    }else if(objname == "osc sender"){
        tempObj = shared_ptr<PatchObject>(new OscSender());
    // -------------------------------------- Data
    }else if(objname == "bang multiplexer"){
        tempObj = shared_ptr<PatchObject>(new BangMultiplexer());
    }else if(objname == "bang to float"){
        tempObj = shared_ptr<PatchObject>(new BangToFloat());
    }else if(objname == "data to file"){
        tempObj = shared_ptr<PatchObject>(new DataToFile());
    }else if(objname == "file to data"){
        tempObj = shared_ptr<PatchObject>(new FileToData());
    }else if(objname == "data to texture"){
        tempObj = shared_ptr<PatchObject>(new DataToTexture());
    }else if(objname == "vector at"){
        tempObj = shared_ptr<PatchObject>(new VectorAt());
    }else if(objname == "vector concat"){
        tempObj = shared_ptr<PatchObject>(new VectorConcat());
    }else if(objname == "floats to vector"){
        tempObj = shared_ptr<PatchObject>(new FloatsToVector());
    }else if(objname == "texture to data"){
        tempObj = shared_ptr<PatchObject>(new TextureToData());
    }else if(objname == "vector gate"){
        tempObj = shared_ptr<PatchObject>(new VectorGate());
    }else if(objname == "vector multiply"){
        tempObj = shared_ptr<PatchObject>(new VectorMultiply());
    // -------------------------------------- Graphics
    }else if(objname == "image exporter"){
        tempObj = shared_ptr<PatchObject>(new ImageExporter());
    }else if(objname == "image loader"){
        tempObj = shared_ptr<PatchObject>(new ImageLoader());
    // -------------------------------------- Sound
    }else if(objname == "ADSR envelope"){
        tempObj = shared_ptr<PatchObject>(new pdspADSR());
    }else if(objname == "AHR envelope"){
        tempObj = shared_ptr<PatchObject>(new pdspAHR());
    }else if(objname == "amp"){
        tempObj = shared_ptr<PatchObject>(new SigMult());
    }else if(objname == "audio exporter"){
        tempObj = shared_ptr<PatchObject>(new AudioExporter());
    }else if(objname == "audio gate"){
        tempObj = shared_ptr<PatchObject>(new AudioGate());
    }else if(objname == "bit cruncher"){
        tempObj = shared_ptr<PatchObject>(new pdspBitCruncher());
    }else if(objname == "bit noise"){
        tempObj = shared_ptr<PatchObject>(new pdspBitNoise());
    }else if(objname == "crossfader"){
        tempObj = shared_ptr<PatchObject>(new Crossfader());
    }else if(objname == "chorus"){
        tempObj = shared_ptr<PatchObject>(new pdspChorusEffect());
    }else if(objname == "comb filter"){
        tempObj = shared_ptr<PatchObject>(new pdspCombFilter());
    }else if(objname == "compressor"){
        tempObj = shared_ptr<PatchObject>(new pdspCompressor());
    }else if(objname == "decimator"){
        tempObj = shared_ptr<PatchObject>(new pdspDecimator());
    }else if(objname == "ducker"){
        tempObj = shared_ptr<PatchObject>(new pdspDucker());
    }else if(objname == "lfo"){
        tempObj = shared_ptr<PatchObject>(new pdspLFO());
    }else if(objname == "mixer"){
        tempObj = shared_ptr<PatchObject>(new Mixer());
    }else if(objname == "note to frequency"){
        tempObj = shared_ptr<PatchObject>(new NoteToFrequency());
    }else if(objname == "panner"){
        tempObj = shared_ptr<PatchObject>(new Panner());
    }else if(objname == "resonant 2pole filter"){
        tempObj = shared_ptr<PatchObject>(new pdspResonant2PoleFilter());
    }else if(objname == "pd patch"){
        tempObj = shared_ptr<PatchObject>(new PDPatch());
    }else if(objname == "quad panner"){
        tempObj = shared_ptr<PatchObject>(new QuadPanner());
    }else if(objname == "pulse"){
        tempObj = shared_ptr<PatchObject>(new OscPulse());
    }else if(objname == "reverb"){
        tempObj = shared_ptr<PatchObject>(new pdspReverb());
    }else if(objname == "saw"){
        tempObj = shared_ptr<PatchObject>(new OscSaw());
    }else if(objname == "sine"){
        tempObj = shared_ptr<PatchObject>(new Oscillator());
    }else if(objname == "triangle"){
        tempObj = shared_ptr<PatchObject>(new OscTriangle());
    }else if(objname == "signal trigger"){
        tempObj = shared_ptr<PatchObject>(new SignalTrigger());
    }else if(objname == "soundfile player"){
        tempObj = shared_ptr<PatchObject>(new SoundfilePlayer());
    }else if(objname == "delay"){
        tempObj = shared_ptr<PatchObject>(new pdspDelay());
    }else if(objname == "data oscillator"){
        tempObj = shared_ptr<PatchObject>(new pdspDataOscillator());
    }else if(objname == "low pass"){
        tempObj = shared_ptr<PatchObject>(new pdspHiCut());
    }else if(objname == "hi pass"){
        tempObj = shared_ptr<PatchObject>(new pdspLowCut());
    }else if(objname == "white noise"){
        tempObj = shared_ptr<PatchObject>(new pdspWhiteNoise());
    // -------------------------------------- Math
    }else if(objname == "add"){
        tempObj = shared_ptr<PatchObject>(new Add());
    }else if(objname == "clamp"){
        tempObj = shared_ptr<PatchObject>(new Clamp());
    }else if(objname == "constant"){
        tempObj = shared_ptr<PatchObject>(new Constant());
    }else if(objname == "divide"){
        tempObj = shared_ptr<PatchObject>(new Divide());
    }else if(objname == "map"){
        tempObj = shared_ptr<PatchObject>(new Map());
    }else if(objname == "metronome"){
        tempObj = shared_ptr<PatchObject>(new Metronome());
    }else if(objname == "modulus"){
        tempObj = shared_ptr<PatchObject>(new Module());
    }else if(objname == "multiply"){
        tempObj = shared_ptr<PatchObject>(new Multiply());
    }else if(objname == "range"){
        tempObj = shared_ptr<PatchObject>(new Range());
    }else if(objname == "simple random"){
        tempObj = shared_ptr<PatchObject>(new SimpleRandom());
    }else if(objname == "simple noise"){
        tempObj = shared_ptr<PatchObject>(new SimpleNoise());
    }else if(objname == "smooth"){
        tempObj = shared_ptr<PatchObject>(new Smooth());
    }else if(objname == "subtract"){
        tempObj = shared_ptr<PatchObject>(new Subtract());
    // -------------------------------------- Logic
    }else if(objname == "&&"){
        tempObj = shared_ptr<PatchObject>(new AND());
    }else if(objname == "||"){
        tempObj = shared_ptr<PatchObject>(new OR());
    }else if(objname == "=="){
        tempObj = shared_ptr<PatchObject>(new Equality());
    }else if(objname == "!="){
        tempObj = shared_ptr<PatchObject>(new Inequality());
    }else if(objname == ">"){
        tempObj = shared_ptr<PatchObject>(new BiggerThan());
    }else if(objname == "<"){
        tempObj = shared_ptr<PatchObject>(new SmallerThan());
    }else if(objname == "counter"){
        tempObj = shared_ptr<PatchObject>(new Counter());
    }else if(objname == "delay bang"){
        tempObj = shared_ptr<PatchObject>(new DelayBang());
    }else if(objname == "delay float"){
        tempObj = shared_ptr<PatchObject>(new DelayFloat());
    }else if(objname == "gate"){
        tempObj = shared_ptr<PatchObject>(new Gate());
    }else if(objname == "inverter"){
        tempObj = shared_ptr<PatchObject>(new Inverter());
    }else if(objname == "loadbang"){
        tempObj = shared_ptr<PatchObject>(new LoadBang());
    }else if(objname == "select"){
        tempObj = shared_ptr<PatchObject>(new Select());
    }else if(objname == "spigot"){
        tempObj = shared_ptr<PatchObject>(new Spigot());
    }else if(objname == "timed semaphore"){
        tempObj = shared_ptr<PatchObject>(new TimedSemaphore());
    // -------------------------------------- GUI
    }else if(objname == "2d pad"){
        tempObj = shared_ptr<PatchObject>(new mo2DPad());
    }else if(objname == "bang"){
        tempObj = shared_ptr<PatchObject>(new moBang());
    }else if(objname == "comment"){
        tempObj = shared_ptr<PatchObject>(new moComment());
    }else if(objname == "message"){
        tempObj = shared_ptr<PatchObject>(new moMessage());
    }else if(objname == "player controls"){
        tempObj = shared_ptr<PatchObject>(new moPlayerControls());
    }else if(objname == "slider"){
        tempObj = shared_ptr<PatchObject>(new moSlider());
    }else if(objname == "sonogram"){
        tempObj = shared_ptr<PatchObject>(new moSonogram());
    }else if(objname == "timeline"){
        tempObj = shared_ptr<PatchObject>(new moTimeline());
    }else if(objname == "trigger"){
        tempObj = shared_ptr<PatchObject>(new moTrigger());
    }else if(objname == "video viewer"){
        tempObj = shared_ptr<PatchObject>(new moVideoViewer());
    }else if(objname == "signal viewer"){
        tempObj = shared_ptr<PatchObject>(new moSignalViewer());
    }else if(objname == "vu meter"){
        tempObj = shared_ptr<PatchObject>(new moVUMeter());
    // -------------------------------------- VIDEO
    }else if(objname == "kinect grabber"){
        tempObj = shared_ptr<PatchObject>(new KinectGrabber());
    }else if(objname == "video player"){
        tempObj = shared_ptr<PatchObject>(new VideoPlayer());
    }else if(objname == "video grabber"){
        tempObj = shared_ptr<PatchObject>(new VideoGrabber());
    }else if(objname == "video feedback"){
        tempObj = shared_ptr<PatchObject>(new VideoDelay());
    }else if(objname == "video exporter"){
        tempObj = shared_ptr<PatchObject>(new VideoExporter());
    }else if(objname == "video crop"){
        tempObj = shared_ptr<PatchObject>(new VideoCrop());
    }else if(objname == "video gate"){
        tempObj = shared_ptr<PatchObject>(new VideoGate());
    }else if(objname == "video transform"){
        tempObj = shared_ptr<PatchObject>(new VideoTransform());
    }else if(objname == "video streaming"){
        tempObj = shared_ptr<PatchObject>(new VideoStreaming());
    }else if(objname == "video timedelay"){
        tempObj = shared_ptr<PatchObject>(new VideoTimelapse());
    // -------------------------------------- WINDOWING
    }else if(objname == "live patching"){
        tempObj = shared_ptr<PatchObject>(new LivePatching());
    }else if(objname == "output window"){
        tempObj = shared_ptr<PatchObject>(new OutputWindow());
    }else if(objname == "projection mapping"){
        tempObj = shared_ptr<PatchObject>(new ProjectionMapping());
    // -------------------------------------- COMPUTER VISION
    }else if(objname == "background subtraction"){
        tempObj = shared_ptr<PatchObject>(new BackgroundSubtraction());
    }else if(objname == "chroma key"){
        tempObj = shared_ptr<PatchObject>(new ChromaKey());
    }else if(objname == "color tracking"){
        tempObj = shared_ptr<PatchObject>(new ColorTracking());
    }else if(objname == "contour tracking"){
        tempObj = shared_ptr<PatchObject>(new ContourTracking());
    }else if(objname == "haar tracking"){
        tempObj = shared_ptr<PatchObject>(new HaarTracking());
    }else if(objname == "motion detection"){
        tempObj = shared_ptr<PatchObject>(new MotionDetection());
    }else if(objname == "optical flow"){
        tempObj = shared_ptr<PatchObject>(new OpticalFlow());
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

}

//--------------------------------------------------------------
void ofxVisualProgramming::newTempPatchFromFile(string patchFile){
    string newFileName = "patch_"+ofGetTimestampString("%y%m%d")+alphabet.at(newFileCounter)+".xml";
    ofFile fileToRead(patchFile);
    ofFile newPatchFile(ofToDataPath("temp/"+newFileName,true));
    ofFile::copyFromTo(fileToRead.getAbsolutePath(),newPatchFile.getAbsolutePath(),true,true);
    newFileCounter++;

    currentPatchFile = newPatchFile.getAbsolutePath();
    openPatch(currentPatchFile);

    tempPatchFile = currentPatchFile;
}

//--------------------------------------------------------------
void ofxVisualProgramming::openPatch(string patchFile){

    bLoadingNewPatch = true;

    currentPatchFile = patchFile;
    ofFile temp(currentPatchFile);
    currentPatchFolderPath  = temp.getEnclosingDirectory();

    ofFile patchDataFolder(currentPatchFolderPath+"data/");
    if(!patchDataFolder.exists()){
        patchDataFolder.create();
    }

    // clear previous patch
    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        it->second->removeObjectContent();
    }
    /*for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        delete it->second;
    }*/
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
            audioDevicesStringIN.clear();
            audioDevicesID_IN.clear();
            audioDevicesStringOUT.clear();
            audioDevicesID_OUT.clear();
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
                if(audioINDev == i){
                   audioGUIINIndex = audioDevicesID_IN.size()-1;
                }
                if(audioOUTDev == i){
                   audioGUIOUTIndex = audioDevicesID_OUT.size()-1;
                }
                string tempSR = "";
                for(size_t sr=0;sr<audioDevices[i].sampleRates.size();sr++){
                    if(sr < audioDevices[i].sampleRates.size()-1){
                        tempSR += ofToString(audioDevices[i].sampleRates.at(sr))+", ";
                    }else{
                        tempSR += ofToString(audioDevices[i].sampleRates.at(sr));
                    }
                }
                ofLog(OF_LOG_NOTICE,"Device[%zu]: %s (IN:%i - OUT:%i), Sample Rates: %s",i,audioDevices[i].name.c_str(),audioDevices[i].inputChannels,audioDevices[i].outputChannels,tempSR.c_str());
            }

            audioSampleRate = audioDevices[audioOUTDev].sampleRates[0];

            if(audioSampleRate < 44100){
                audioSampleRate = 44100;
            }

            XML.setValue("sample_rate_in",audioSampleRate);
            XML.setValue("sample_rate_out",audioSampleRate);
            XML.setValue("input_channels",static_cast<int>(audioDevices[audioINDev].inputChannels));
            XML.setValue("output_channels",static_cast<int>(audioDevices[audioOUTDev].outputChannels));
            XML.saveFile();

            delete engine;
            engine = nullptr;
            engine = new pdsp::Engine();

            if(dspON){
                engine->setChannels(audioDevices[audioINDev].inputChannels, audioDevices[audioOUTDev].outputChannels);
                this->setChannels(audioDevices[audioINDev].inputChannels,0);

                for(int in=0;in<audioDevices[audioINDev].inputChannels;in++){
                    engine->audio_in(in) >> this->in(in);
                }
                this->out_silent() >> engine->blackhole();

                engine->setOutputDeviceID(audioDevices[audioOUTDev].deviceID);
                engine->setInputDeviceID(audioDevices[audioINDev].deviceID);
                engine->setup(audioSampleRate, audioBufferSize, 3);

                ofLog(OF_LOG_NOTICE,"[verbose]------------------- Soundstream INPUT Started on");
                ofLog(OF_LOG_NOTICE,"Audio device: %s",audioDevices[audioINDev].name.c_str());
                ofLog(OF_LOG_NOTICE,"[verbose]------------------- Soundstream OUTPUT Started on");
                ofLog(OF_LOG_NOTICE,"Audio device: %s",audioDevices[audioOUTDev].name.c_str());

                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }

            XML.popTag();
        }

        int totalObjects = XML.getNumTags("object");

        // Load all the patch objects
        for(int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                string objname = XML.getValue("name","");
                bool loaded = false;
                shared_ptr<PatchObject> tempObj = selectObject(objname);
                if(tempObj != nullptr){
                    loaded = tempObj->loadConfig(mainWindow,*engine,i,patchFile);
                    if(loaded){
                        tempObj->setPatchfile(currentPatchFile);
                        tempObj->setIsRetina(isRetina);
                        ofAddListener(tempObj->dragEvent ,this,&ofxVisualProgramming::dragObject);
                        ofAddListener(tempObj->removeEvent ,this,&ofxVisualProgramming::removeObject);
                        ofAddListener(tempObj->resetEvent ,this,&ofxVisualProgramming::resetObject);
                        ofAddListener(tempObj->reconnectOutletsEvent ,this,&ofxVisualProgramming::reconnectObjectOutlets);
                        ofAddListener(tempObj->iconifyEvent ,this,&ofxVisualProgramming::iconifyObject);
                        ofAddListener(tempObj->duplicateEvent ,this,&ofxVisualProgramming::duplicateObject);
                        // Insert the new patch into the map
                        patchObjects[tempObj->getId()] = tempObj;
                        actualObjectID = tempObj->getId();
                        lastAddedObjectID = tempObj->getId();

                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
                                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
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

    }

    bLoadingNewPatch = false;

}

//--------------------------------------------------------------
void ofxVisualProgramming::savePatchAs(string patchFile){

    // Mosaic patch folder structure:
    //
    // > PATCH_NAME/
    //      > DATA/
    //        PATCH_NAME.xml


    // sanitize filename
    ofFile tempPF(patchFile);
    string preSanitizeFN = tempPF.getFileName();
    sanitizeFilename(preSanitizeFN);

    string sanitizedPatchFile = tempPF.getEnclosingDirectory()+preSanitizeFN;
    //ofLog(OF_LOG_NOTICE,"%s",patchFile.c_str());
    //ofLog(OF_LOG_NOTICE,"%s",sanitizedPatchFile.c_str());

    // copy patch file & patch data folder
    ofFile tempFile(sanitizedPatchFile);
    string tempFileName = tempFile.getFileName();
    string finalTempFileName = tempFile.getFileName().substr(0,tempFileName.find_last_of('.'));

    string newFileName = checkFileExtension(sanitizedPatchFile, ofToUpper(tempFile.getExtension()), "XML");
    ofFile fileToRead(currentPatchFile);
    ofDirectory dataFolderOrigin;
    dataFolderOrigin.listDir(currentPatchFolderPath+"data/");
    ofFile newPatchFile(newFileName);

    currentPatchFile = newPatchFile.getEnclosingDirectory()+finalTempFileName+"/"+newPatchFile.getFileName();
    ofFile temp(currentPatchFile);
    currentPatchFolderPath  = temp.getEnclosingDirectory();

    ofFile::copyFromTo(fileToRead.getAbsolutePath(),currentPatchFile,true,true);

    std::filesystem::path tp = currentPatchFolderPath+"/data/";
    dataFolderOrigin.copyTo(tp,true,true);

    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        it->second->setPatchfile(currentPatchFile);
    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::openLastPatch(){
    newTempPatchFromFile("last_patch.xml");
}

//--------------------------------------------------------------
void ofxVisualProgramming::savePatchAsLast(){
    string newFileName = "last_patch.xml";
    ofFile fileToRead(currentPatchFile);
    ofFile newPatchFile(newFileName);
    ofFile::copyFromTo(fileToRead.getAbsolutePath(),newPatchFile.getAbsolutePath(),true,true);
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
        ofLog(OF_LOG_WARNING,"------------------- INCOMPATIBLE INPUT DEVICE Sample Rate: %i", audioDevices[index].sampleRates[0]);
        ofLog(OF_LOG_WARNING,"------------------- PLEASE SELECT ANOTHER INPUT DEVICE");
        return;
    }else{
        delete engine;
        engine = nullptr;
        engine = new pdsp::Engine();

        setPatchVariable("audio_in_device",index);
        audioINDev = index;

        setPatchVariable("sample_rate_in",audioSampleRate);
        setPatchVariable("input_channels",static_cast<int>(audioDevices[audioINDev].inputChannels));

        if(dspON){
            engine->setChannels(audioDevices[audioINDev].inputChannels, audioDevices[audioOUTDev].outputChannels);
            this->setChannels(audioDevices[audioINDev].inputChannels,0);

            for(int in=0;in<audioDevices[audioINDev].inputChannels;in++){
                engine->audio_in(in) >> this->in(in);
            }
            this->out_silent() >> engine->blackhole();

            ofLog(OF_LOG_NOTICE,"[verbose]------------------- Soundstream INPUT Selected Device");
            ofLog(OF_LOG_NOTICE,"Audio device: %s",audioDevices[audioINDev].name.c_str());

            resetSpecificSystemObjects("audio device");

            engine->setOutputDeviceID(audioDevices[audioOUTDev].deviceID);
            engine->setInputDeviceID(audioDevices[audioINDev].deviceID);
            engine->setup(audioSampleRate, audioBufferSize, 3);
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

    delete engine;
    engine = nullptr;
    engine = new pdsp::Engine();

    setPatchVariable("sample_rate_out",audioSampleRate);
    setPatchVariable("output_channels",static_cast<int>(audioDevices[audioOUTDev].outputChannels));

    if(dspON){
        engine->setChannels(audioDevices[audioINDev].inputChannels, audioDevices[audioOUTDev].outputChannels);
        this->setChannels(audioDevices[audioINDev].inputChannels,0);

        for(int in=0;in<audioDevices[audioINDev].inputChannels;in++){
            engine->audio_in(in) >> this->in(in);
        }
        this->out_silent() >> engine->blackhole();

        ofLog(OF_LOG_NOTICE,"[verbose]------------------- Soundstream OUTPUT Selected Device");
        ofLog(OF_LOG_NOTICE,"Audio device: %s",audioDevices[audioOUTDev].name.c_str());

        resetSpecificSystemObjects("audio device");

        engine->setOutputDeviceID(audioDevices[audioOUTDev].deviceID);
        engine->setInputDeviceID(audioDevices[audioINDev].deviceID);
        engine->setup(audioSampleRate, audioBufferSize, 3);

        bool found = false;
        for(int i=0;i<audioDevices[audioINDev].sampleRates.size();i++){
            if(audioDevices[audioINDev].sampleRates[i] == audioSampleRate){
                found = true;
            }
        }

        if(!found){
            ofLog(OF_LOG_NOTICE,"------------------- INCOMPATIBLE INPUT DEVICE Sample Rate: %i", audioDevices[audioINDev].sampleRates[0]);
            ofLog(OF_LOG_NOTICE,"------------------- PLEASE SELECT ANOTHER INPUT DEVICE");
        }
    }
    
}

//--------------------------------------------------------------
void ofxVisualProgramming::activateDSP(){

    engine->setChannels(0,0);

    if(audioDevices[audioINDev].inputChannels > 0 && audioDevices[audioOUTDev].outputChannels > 0){
        engine->setChannels(audioDevices[audioINDev].inputChannels, audioDevices[audioOUTDev].outputChannels);
        this->setChannels(audioDevices[audioINDev].inputChannels,0);

        for(int in=0;in<audioDevices[audioINDev].inputChannels;in++){
            engine->audio_in(in) >> this->in(in);
        }
        this->out_silent() >> engine->blackhole();

        ofLog(OF_LOG_NOTICE,"[verbose]------------------- Soundstream INPUT Started on");
        ofLog(OF_LOG_NOTICE,"Audio device: %s",audioDevices[audioINDev].name.c_str());
        ofLog(OF_LOG_NOTICE,"[verbose]------------------- Soundstream OUTPUT Started on");
        ofLog(OF_LOG_NOTICE,"Audio device: %s",audioDevices[audioOUTDev].name.c_str());

        engine->setOutputDeviceID(audioDevices[audioOUTDev].deviceID);
        engine->setInputDeviceID(audioDevices[audioINDev].deviceID);
        engine->setup(audioSampleRate, audioBufferSize, 3);

        bool found = weAlreadyHaveObject("audio device");

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
