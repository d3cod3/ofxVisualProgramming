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

#include "moTimeline.h"

#include "GLFW/glfw3.h"

//--------------------------------------------------------------
moTimeline::moTimeline() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 0;

    _inletParams[0] = new string();  // control
    *static_cast<string *>(_inletParams[0]) = "";
    _inletParams[1] = new float();  // playhead
    *(float *)&_inletParams[1] = 0.0f;

    this->initInletsState();

    timeline    = new ofxTimeline();
    actualTracks= new vector<string>();
    localFont   = new ofxFontStash();

    actualTrackName     = "trackName";
    sameNameAvoider     = 0;
    lastTrackID         = 0;
    durationInSeconds   = 60;
    fps                 = static_cast<int>(ofGetFrameRate());
    timelineLoaded      = false;
    resetTimelineOutlets= false;

    isFullscreen        = false;
    scrolledDisplacement= 0;

    isGUIObject         = true;
    this->isOverGUI     = true;

    lastMessage         = "";
    lastPlayheadPos     = 0;

    lastTimelineFolder      = "";
    loadTimelineConfigFlag  = false;
    saveTimelineConfigFlag  = false;
    loadedTimelineConfig    = false;
    savedTimelineConfig     = false;

    autoRemove              = false;

}

//--------------------------------------------------------------
void moTimeline::newObject(){
    this->setName("timeline");
    this->addInlet(VP_LINK_STRING,"control");
    this->addInlet(VP_LINK_NUMERIC,"playhead");

    this->setCustomVar(static_cast<float>(lastTrackID),"LAST_TRACK_ID");
    this->setCustomVar(static_cast<float>(durationInSeconds),"DURATION");
    this->setCustomVar(static_cast<float>(fps),"FPS");
}

//--------------------------------------------------------------
void moTimeline::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    ofGLFWWindowSettings settings;
    settings.setGLVersion(2,1);
    settings.shareContextWith = mainWindow;
    settings.decorated = true;
    settings.resizable = true;
    settings.stencilBits = 0;
    // RETINA FIX
    if(ofGetScreenWidth() >= RETINA_MIN_WIDTH && ofGetScreenHeight() >= RETINA_MIN_HEIGHT){
        settings.setPosition(ofDefaultVec2(mainWindow->getScreenSize().x-(1600+100),400));
        settings.setSize(800, 600);
    }else{
        settings.setPosition(ofDefaultVec2(mainWindow->getScreenSize().x-(800+50),200));
        settings.setSize(800, 600);
    }

    window = dynamic_pointer_cast<ofAppGLFWWindow>(ofCreateWindow(settings));
    window->setWindowTitle("Timeline"+ofToString(this->getId()));
    window->setVerticalSync(true);

    glfwSetWindowCloseCallback(window->getGLFWWindow(),GL_FALSE);

    ofAddListener(window->events().draw,this,&moTimeline::drawInWindow);
    ofAddListener(window->events().keyPressed,this,&moTimeline::keyPressed);
    ofAddListener(window->events().keyReleased,this,&moTimeline::keyReleased);
    ofAddListener(window->events().mouseMoved ,this,&moTimeline::mouseMoved);
    ofAddListener(window->events().mouseDragged ,this,&moTimeline::mouseDragged);
    ofAddListener(window->events().mousePressed ,this,&moTimeline::mousePressed);
    ofAddListener(window->events().mouseReleased ,this,&moTimeline::mouseReleased);
    ofAddListener(window->events().mouseScrolled ,this,&moTimeline::mouseScrolled);
    ofAddListener(window->events().windowResized ,this,&moTimeline::windowResized);

    initTimeline();

    // GUI
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onButtonEvent(this, &moTimeline::onButtonEvent);
    gui->onToggleEvent(this, &moTimeline::onToggleEvent);
    gui->onTextInputEvent(this, &moTimeline::onTextInputEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    setRetina = gui->addToggle("RETINA SCREEN",false);
    setRetina->setUseCustomMouse(true);
    gui->addBreak();
    guiDuration = gui->addTextInput("LENGTH","60"); // duration in seconds
    guiDuration->setText(ofToString(static_cast<int>(floor(this->getCustomVar("DURATION")))));
    guiDuration->setUseCustomMouse(true);
    setDuration = gui->addButton("SET DURATION");
    setDuration->setUseCustomMouse(true);
    setDuration->setLabelAlignment(ofxDatGuiAlignment::CENTER);
    gui->addBreak();
    guiFPS = gui->addTextInput("FPS",ofToString(fps)); // fps
    guiFPS->setText(ofToString(static_cast<int>(floor(this->getCustomVar("FPS")))));
    guiFPS->setUseCustomMouse(true);
    setFPS = gui->addButton("SET FPS");
    setFPS->setUseCustomMouse(true);
    setFPS->setLabelAlignment(ofxDatGuiAlignment::CENTER);
    gui->addBreak();
    guiTrackName = gui->addTextInput("NAME",actualTrackName);
    guiTrackName->setUseCustomMouse(true);
    gui->addBreak();
    addCurveTrack = gui->addButton("ADD CURVE TRACK");
    addCurveTrack->setUseCustomMouse(true);
    addCurveTrack->setLabelAlignment(ofxDatGuiAlignment::CENTER);
    addBangTrack = gui->addButton("ADD BANG TRACK");
    addBangTrack->setUseCustomMouse(true);
    addBangTrack->setLabelAlignment(ofxDatGuiAlignment::CENTER);
    addColorTrack = gui->addButton("ADD COLOR TRACK");
    addColorTrack->setUseCustomMouse(true);
    addColorTrack->setLabelAlignment(ofxDatGuiAlignment::CENTER);
    addLFOTrack = gui->addButton("ADD LFO TRACK");
    addLFOTrack->setUseCustomMouse(true);
    addLFOTrack->setLabelAlignment(ofxDatGuiAlignment::CENTER);
    gui->addBreak();
    loadTimeline = gui->addButton("LOAD TIMELINE");
    loadTimeline->setUseCustomMouse(true);
    loadTimeline->setLabelAlignment(ofxDatGuiAlignment::CENTER);
    saveTimeline = gui->addButton("SAVE TIMELINE AS");
    saveTimeline->setUseCustomMouse(true);
    saveTimeline->setLabelAlignment(ofxDatGuiAlignment::CENTER);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

}

//--------------------------------------------------------------
void moTimeline::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){
    gui->update();
    header->update();
    setRetina->update();
    guiTrackName->update();
    addCurveTrack->update();
    addBangTrack->update();
    addColorTrack->update();
    addLFOTrack->update();
    guiDuration->update();
    setDuration->update();
    guiFPS->update();
    setFPS->update();
    loadTimeline->update();
    saveTimeline->update();

    if(!timelineLoaded){
        timelineLoaded = true;
        timeline->setDurationInSeconds(static_cast<int>(floor(this->getCustomVar("DURATION"))));
        timeline->setFrameRate(static_cast<float>(floor(this->getCustomVar("FPS"))));
    }

    if(loadTimelineConfigFlag){
        loadTimelineConfigFlag = false;
        fd.openFolder("load timeline config","Select folder for loading timeline data");
    }

    if(saveTimelineConfigFlag){
        saveTimelineConfigFlag = false;
        fd.openFolder("save timeline config","Select folder for saving timeline data");
    }

    if(loadedTimelineConfig){
        loadedTimelineConfig = false;
        if(lastTimelineFolder != ""){
            ofFile tempfile (lastTimelineFolder);
            if(tempfile.exists() && tempfile.isDirectory()){
                loadTimelineData(tempfile.getAbsolutePath()+"/");
            }
        }
    }

    if(savedTimelineConfig){
        savedTimelineConfig = false;
        if(lastTimelineFolder != ""){
            ofFile tempfile (lastTimelineFolder);
            if(tempfile.exists() && tempfile.isDirectory()){
                 saveTimelineData(tempfile.getAbsolutePath()+"/");
            }
        }
    }

    if(resetTimelineOutlets){
        resetTimelineOutlets = false;
        for(int j=0;j<static_cast<int>(this->outPut.size());j++){
            patchObjects[this->outPut[j]->toObjectID]->inletsConnected[this->outPut[j]->toInletID] = false;
        }
        resetOutlets();
    }

    // listen to message control (_inletParams[0])
    if(this->inletsConnected[0]){
        if(lastMessage != *static_cast<string *>(_inletParams[0])){
            lastMessage = *static_cast<string *>(_inletParams[0]);

            if(lastMessage == "play"){
                timeline->play();
            }else if(lastMessage == "pause"){
                timeline->stop();
            }else if(lastMessage == "unpause"){
                timeline->play();
            }else if(lastMessage == "stop"){
                timeline->setCurrentTimeSeconds(0);
                timeline->stop();
            }else if(lastMessage == "loop_normal"){
                timeline->setLoopType(OF_LOOP_NORMAL);
            }else if(lastMessage == "loop_none"){
                timeline->setLoopType(OF_LOOP_NONE);
            }

        }
    }

    // listen to playhead control (_inletParams[1])
    if(this->inletsConnected[1]){
        if(lastPlayheadPos != *(float *)&_inletParams[1]){
            lastPlayheadPos = *(float *)&_inletParams[1];
            timeline->setCurrentTimeSeconds(lastPlayheadPos*timeline->getDurationInSeconds());
        }
    }

    // pass timeline data to outlets (if any)
    for(int i=0;i<actualTracks->size();i++){
        if(this->getOutletType(i) == VP_LINK_NUMERIC){
            *(float *)&_outletParams[i] = static_cast<float>(timeline->getValue(actualTracks->at(i)));
        }else if(this->getOutletType(i) == VP_LINK_ARRAY){
            static_cast<vector<float> *>(_outletParams[i])->at(0) = timeline->getColor(actualTracks->at(i)).r; // RED
            static_cast<vector<float> *>(_outletParams[i])->at(1) = timeline->getColor(actualTracks->at(i)).g; // GREEN
            static_cast<vector<float> *>(_outletParams[i])->at(2) = timeline->getColor(actualTracks->at(i)).b; // BLUE
        }
    }

    // auto remove
    if(window->getGLFWWindow() == nullptr && !autoRemove){
        autoRemove = true;
        ofNotifyEvent(this->removeEvent, this->nId);
        this->willErase = true;
    }

}

//--------------------------------------------------------------
void moTimeline::drawObjectContent(ofxFontStash *font){
    localFont = font;
    ofSetColor(255);
    ofEnableAlphaBlending();

    // draw timeline state
    ofSetColor(255,60);
    if(timeline->getIsPlaying()){ // play
        ofBeginShape();
        ofVertex(this->width - 30,this->height - 50);
        ofVertex(this->width - 30,this->height - 30);
        ofVertex(this->width - 10,this->height - 40);
        ofEndShape();
    }else if(!timeline->getIsPlaying() && timeline->getCurrentFrame() > 1){ // pause
        ofDrawRectangle(this->width - 30, this->height - 50,8,20);
        ofDrawRectangle(this->width - 18, this->height - 50,8,20);
    }else if(timeline->getCurrentFrame() <= 1){ // stop
        ofDrawRectangle(this->width - 30, this->height - 50,20,20);
    }

    // draw timeline playhead
    ofSetColor(255,100);
    ofSetLineWidth(2);
    float phx = ofMap( timeline->getPercentComplete(), 0, 1, 0, this->width-2 );
    if(timeline->getCurrentFrame() > 1){
        ofDrawLine( phx, this->headerHeight, phx, this->height-this->headerHeight);
    }

    // draw timeline time and timecode
    ofSetColor(255);
    font->draw(timeline->getTimecode().timecodeForFrame(timeline->getCurrentFrame()),static_cast<int>(floor(this->fontSize*1.4)),this->width/3 + 8,this->headerHeight*2.3);
    font->draw(timeline->getDurationInTimecode(),static_cast<int>(floor(this->fontSize*1.4)),this->width/3 + 8,this->headerHeight*3.3);
    font->draw(ofToString(timeline->getCurrentFrame()),static_cast<int>(floor(this->fontSize*1.3)),this->width/3 + 8,this->headerHeight*5);
    font->draw(ofToString(timeline->getDurationInFrames()),static_cast<int>(floor(this->fontSize*1.3)),this->width/3 + 8,this->headerHeight*6);

    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void moTimeline::removeObjectContent(){
    if(window->getGLFWWindow() != nullptr){
        window->setWindowShouldClose();
    }
}

//--------------------------------------------------------------
void moTimeline::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    setRetina->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    guiTrackName->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    addCurveTrack->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    addBangTrack->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    addColorTrack->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    addLFOTrack->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    guiDuration->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    setDuration->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    guiFPS->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    setFPS->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    loadTimeline->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    saveTimeline->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || setRetina->hitTest(_m-this->getPos()) || guiTrackName->hitTest(_m-this->getPos()) || addCurveTrack->hitTest(_m-this->getPos())
                            || addBangTrack->hitTest(_m-this->getPos()) || addColorTrack->hitTest(_m-this->getPos()) || addLFOTrack->hitTest(_m-this->getPos())
                            || guiDuration->hitTest(_m-this->getPos()) || setDuration->hitTest(_m-this->getPos()) || loadTimeline->hitTest(_m-this->getPos())
                            || guiFPS->hitTest(_m-this->getPos()) || setFPS->hitTest(_m-this->getPos()) || saveTimeline->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void moTimeline::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        setRetina->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        guiTrackName->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        addCurveTrack->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        addBangTrack->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        addColorTrack->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        addLFOTrack->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        guiDuration->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        setDuration->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        guiFPS->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        setFPS->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        loadTimeline->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        saveTimeline->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }else{
        ofNotifyEvent(dragEvent, nId);

        box->setFromCenter(_m.x, _m.y,box->getWidth(),box->getHeight());
        headerBox->set(box->getPosition().x,box->getPosition().y,box->getWidth(),headerHeight);

        x = box->getPosition().x;
        y = box->getPosition().y;

        for(int j=0;j<static_cast<int>(outPut.size());j++){
            outPut[j]->linkVertices[0].move(outPut[j]->posFrom.x,outPut[j]->posFrom.y);
            outPut[j]->linkVertices[1].move(outPut[j]->posFrom.x+20,outPut[j]->posFrom.y);
        }
    }
}

//--------------------------------------------------------------
void moTimeline::fileDialogResponse(ofxThreadedFileDialogResponse &response){
    if(response.id == "load timeline config"){
        lastTimelineFolder = response.filepath;
        loadedTimelineConfig = true;
    }else if(response.id == "save timeline config"){
        lastTimelineFolder = response.filepath;
        savedTimelineConfig = true;
    }
}

//--------------------------------------------------------------
void moTimeline::initTimeline(){
    sameNameAvoider += static_cast<int>(floor(this->getCustomVar("LAST_TRACK_ID")));

    if(this->filepath == "none"){
        this->filepath = "temp/";
    }

    timeline->setWorkingFolder(this->filepath);

    timeline->setup();
    timeline->setName(getLoadingTimelineName(this->filepath));
    timeline->disableEvents();
    timeline->setFrameRate(static_cast<float>(floor(this->getCustomVar("FPS"))));
    timeline->setDurationInSeconds(static_cast<int>(floor(this->getCustomVar("DURATION"))));
    timeline->setLoopType(OF_LOOP_NORMAL);
    timeline->setShowTimeControls(true);
    timeline->setFrameBased(true);

    autoAddTracks(this->filepath);

    timeline->loadTracksFromFolder(this->filepath);

}

//--------------------------------------------------------------
string moTimeline::getLoadingTimelineName(string path){
    filesystem::path temppath = filesystem::path(path);
    ofDirectory dir(temppath);
    if(dir.isDirectory()){
        dir.listDir();
        for(int i = 0; i < dir.size(); i++){
            size_t found = dir.getPath(i).find_last_of("/\\");
            string fileStr = dir.getPath(i).substr(found+1);
            size_t found2 = fileStr.find_first_of("_");
            if(found != string::npos && found2 != string::npos && fileStr.find("timeline") != string::npos && dir.getFile(i).getExtension() == "xml"){
                string timelineName = fileStr.substr(0,fileStr.length()-(fileStr.length()-found2));
                //ofLog(OF_LOG_NOTICE,"%s",timelineName.c_str());
                return timelineName;
            }
        }
    }

    return "";
}

//--------------------------------------------------------------
void moTimeline::autoAddTracks(string path){
    actualTracks->clear();

    filesystem::path temppath = filesystem::path(path);
    ofDirectory dir(temppath);
    if(dir.isDirectory()){
        dir.listDir();
        for(int i = 0; i < dir.size(); i++){
            size_t found = dir.getPath(i).find_last_of("/\\");
            string fileStr = dir.getPath(i).substr(found+1);
            size_t found2 = fileStr.find_first_of("_");
            if(found2 != string::npos && fileStr.find("timeline") != string::npos && dir.getFile(i).getExtension() == "xml"){
                string trackName = fileStr.substr(found2+1);
                string tempName = trackName.erase(trackName.length()-4);
                if(trackName.at(2) == 'F'){
                    timeline->addCurves(tempName);
                    actualTracks->push_back(tempName);
                    ofAddListener(timeline->getTrackHeader(timeline->getTrack(tempName))->removeTrackEvent ,this,&moTimeline::removeTrack);
                    sameNameAvoider++;
                }else if(trackName.at(2) == 'B'){
                    timeline->addBangs(tempName);
                    actualTracks->push_back(tempName);
                    ofAddListener(timeline->getTrackHeader(timeline->getTrack(tempName))->removeTrackEvent ,this,&moTimeline::removeTrack);
                    sameNameAvoider++;
                }else if(trackName.at(2) == 'C'){
                    timeline->addColors(tempName);
                    actualTracks->push_back(tempName);
                    ofAddListener(timeline->getTrackHeader(timeline->getTrack(tempName))->removeTrackEvent ,this,&moTimeline::removeTrack);
                    sameNameAvoider++;
                }else if(trackName.at(2) == 'L'){
                    timeline->addLFO(tempName);
                    actualTracks->push_back(tempName);
                    ofAddListener(timeline->getTrackHeader(timeline->getTrack(tempName))->removeTrackEvent ,this,&moTimeline::removeTrack);
                    sameNameAvoider++;
                }
            }
        }

        updateOutletsConfig();
        this->setCustomVar(static_cast<float>(sameNameAvoider),"LAST_TRACK_ID");
        timeline->setWidth(window->getWidth());
    }
}

//--------------------------------------------------------------
void moTimeline::addTrack(int type){

    if(timeline->getPage("Page One")->getTracks().size() < (MAX_OUTLETS/2)){
        string preCode  = "";
        string tempName = "";
        sameNameAvoider<10 ? preCode = "0"+ofToString(sameNameAvoider) : preCode = ofToString(sameNameAvoider);

        switch (type) {
        case TIMELINE_CURVE_TRACK:
            tempName = "_"+preCode+"F-"+actualTrackName;
            timeline->addCurves(tempName,ofToDataPath(this->filepath + timeline->getName() + tempName + ".xml"));
            actualTracks->push_back(tempName);
            ofAddListener(timeline->getTrackHeader(timeline->getTrack(tempName))->removeTrackEvent ,this,&moTimeline::removeTrack);
            break;
        case TIMELINE_BANG_TRACK:
            tempName = "_"+preCode+"B-"+actualTrackName;
            timeline->addBangs(tempName,ofToDataPath(this->filepath + timeline->getName() + tempName + ".xml"));
            actualTracks->push_back(tempName);
            ofAddListener(timeline->getTrackHeader(timeline->getTrack(tempName))->removeTrackEvent ,this,&moTimeline::removeTrack);
            break;
        case TIMELINE_COLOR_TRACK:
            tempName = "_"+preCode+"C-"+actualTrackName;
            timeline->addColors(tempName,ofToDataPath(this->filepath + timeline->getName() + tempName + ".xml"));
            actualTracks->push_back(tempName);
            ofAddListener(timeline->getTrackHeader(timeline->getTrack(tempName))->removeTrackEvent ,this,&moTimeline::removeTrack);
            break;
        case TIMELINE_LFO_TRACK:
            tempName = "_"+preCode+"L-"+actualTrackName;
            timeline->addLFO(tempName,ofToDataPath(this->filepath + timeline->getName() + tempName + ".xml"));
            actualTracks->push_back(tempName);
            ofAddListener(timeline->getTrackHeader(timeline->getTrack(tempName))->removeTrackEvent ,this,&moTimeline::removeTrack);
            break;
        default:
            break;
        }

        updateOutletsConfig();
        sameNameAvoider++;
        this->setCustomVar(static_cast<float>(sameNameAvoider),"LAST_TRACK_ID");

        timeline->setWidth(window->getWidth());

    }
}

//--------------------------------------------------------------
void moTimeline::loadTimelineData(string folder){
    this->filepath = forceCheckMosaicDataPath(folder);

    sameNameAvoider = 0;
    timeline->clear();

    vector<ofxTLTrack*> tempTracks = timeline->getPage("Page One")->getTracks();
    for(int i=0;i<tempTracks.size();i++){
        timeline->removeTrack(tempTracks.at(i));
    }

    timeline->setName(getLoadingTimelineName(this->filepath));
    timeline->setWorkingFolder(this->filepath);

    autoAddTracks(this->filepath);

    timeline->loadTracksFromFolder(this->filepath);

    timeline->setWidth(window->getWidth());

    timelineLoaded = false;
}

//--------------------------------------------------------------
void moTimeline::saveTimelineData(string folder){
    this->filepath = forceCheckMosaicDataPath(folder);

    timeline->setWorkingFolder(this->filepath);
    timeline->setName("timeline"+ofToString(this->nId));
    timeline->saveTracksToFolder(this->filepath);
}

//--------------------------------------------------------------
void moTimeline::updateOutletsConfig(){

    resetTimelineOutlets = true;

}

//--------------------------------------------------------------
void moTimeline::resetOutlets(){
    vector<ofxTLTrack*> tempTracks = timeline->getPage("Page One")->getTracks();

    this->outPut.clear();
    this->outlets.clear();

    this->numOutlets = tempTracks.size();

    for( int i = 0; i < tempTracks.size(); i++){
        if(tempTracks.at(i)->getTrackType() == "Colors"){
            _outletParams[i] = new vector<float>();
            static_cast<vector<float> *>(_outletParams[i])->assign(3,0.0f);
            this->addOutlet(VP_LINK_ARRAY);
        }else{
            _outletParams[i] = new float();
            *(float *)&_outletParams[i] = 0.0f;
            this->addOutlet(VP_LINK_NUMERIC);
        }
    }

    this->height      = OBJECT_HEIGHT;

    if(this->numOutlets > 6){
        this->height          *= 2;
    }

    if(this->numOutlets > 12){
        this->height          *= 2;
    }
    this->box->setHeight(this->height);
    gui->setPosition(0,this->height - header->getHeight());

    ofxXmlSettings XML;
    if(XML.loadFile(this->patchFile)){
        int totalObjects = XML.getNumTags("object");

        // Save new object outlet config
        for(int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                if(XML.getValue("id", -1) == this->nId){
                    // Dynamic reloading outlets
                    XML.removeTag("outlets");
                    int newOutlets = XML.addTag("outlets");
                    if(XML.pushTag("outlets",newOutlets)){
                        for(int j=0;j<static_cast<int>(this->outlets.size());j++){
                            int newLink = XML.addTag("link");
                            if(XML.pushTag("link",newLink)){
                                XML.setValue("type",this->outlets.at(j));
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

//--------------------------------------------------------------
void moTimeline::removeTrack(string &trackName){

    //ofLog(OF_LOG_NOTICE,"Removing Track %s",trackName.c_str());

    ofRemoveListener(timeline->getTrackHeader(timeline->getTrack(trackName))->removeTrackEvent,this,&moTimeline::removeTrack);
    ofFile temp(timeline->getWorkingFolder()+timeline->getName()+"_"+timeline->getTrack(trackName)->getName()+".xml");
    if(temp.exists()){
        ofFile::removeFile(temp.getAbsolutePath());
    }
    timeline->removeTrack(trackName);

    auto it = std::find(actualTracks->begin(), actualTracks->end(), trackName);
    if(it != actualTracks->end()) actualTracks->erase(it);

    updateOutletsConfig();

    timeline->setWidth(window->getWidth());
}

//--------------------------------------------------------------
void moTimeline::drawInWindow(ofEventArgs &e){
    ofBackground(20);
    ofPushStyle();
    ofSetColor(255);
    ofPushMatrix();
    ofTranslate(0,scrolledDisplacement,0);
    timeline->draw();
    ofPopMatrix();
    ofSetColor(120);
    // RETINA FIX
    if(timeline->forceRetina){
        localFont->draw("forked ofxTimeline modded for Mosaic, original ofxaddon from James George co-developed by YCAM Interlab",22,40,window->getHeight()-20);
    }else{
        localFont->draw("forked ofxTimeline modded for Mosaic, original ofxaddon from James George co-developed by YCAM Interlab",12,20,window->getHeight()-10);
    }
    ofPopStyle();
}

//--------------------------------------------------------------
void moTimeline::toggleWindowFullscreen(){
    isFullscreen = !isFullscreen;
    window->toggleFullscreen();

    if(!isFullscreen){
        window->setWindowShape(800,600);
    }
}

//--------------------------------------------------------------
void moTimeline::keyPressed(ofKeyEventArgs &e){
    timeline->keyPressed(e);

    // OSX: CMD-F, WIN/LINUX: CTRL-F    (FULLSCREEN)
    if(e.hasModifier(MOD_KEY) && e.keycode == 70){
        toggleWindowFullscreen();
    }
}

//--------------------------------------------------------------
void moTimeline::keyReleased(ofKeyEventArgs &e){
    timeline->keyReleased(e);
}

//--------------------------------------------------------------
void moTimeline::mouseMoved(ofMouseEventArgs &e){

    ofMouseEventArgs temp;
    temp = e;
    temp.y -= scrolledDisplacement;

    timeline->mouseMoved(temp);
}

//--------------------------------------------------------------
void moTimeline::mouseDragged(ofMouseEventArgs &e){
    ofMouseEventArgs temp;
    temp = e;
    temp.y -= scrolledDisplacement;

    timeline->mouseDragged(temp);
}

//--------------------------------------------------------------
void moTimeline::mousePressed(ofMouseEventArgs &e){
    ofMouseEventArgs temp;
    temp = e;
    temp.y -= scrolledDisplacement;

    timeline->mousePressed(temp);
}

//--------------------------------------------------------------
void moTimeline::mouseReleased(ofMouseEventArgs &e){
    ofMouseEventArgs temp;
    temp = e;
    temp.y -= scrolledDisplacement;

    timeline->mouseReleased(temp);
}

//--------------------------------------------------------------
void moTimeline::mouseScrolled(ofMouseEventArgs &e){
    if(timeline->getHeight() > window->getHeight()){
        scrolledDisplacement += e.scrollY;

        if(scrolledDisplacement < -(timeline->getHeight() - window->getHeight() + 30)){
            scrolledDisplacement = -(timeline->getHeight() - window->getHeight() + 30);
        }
        if(scrolledDisplacement>0){
            scrolledDisplacement = 0;
        }
    }
}

//--------------------------------------------------------------
void moTimeline::windowResized(ofResizeEventArgs &e){
    timeline->windowResized(e);
    timeline->setWidth(window->getWidth());
}

//--------------------------------------------------------------
void moTimeline::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == addCurveTrack){
            addTrack(TIMELINE_CURVE_TRACK);
        }else if(e.target == addBangTrack){
            addTrack(TIMELINE_BANG_TRACK);
        }else if(e.target == addColorTrack){
            addTrack(TIMELINE_COLOR_TRACK);
        }else if(e.target == addLFOTrack){
            addTrack(TIMELINE_LFO_TRACK);
        }else if(e.target == setDuration){
            timeline->setDurationInSeconds(durationInSeconds);
        }else if(e.target == setFPS){
            timeline->setFrameRate(fps);
        }else if(e.target == loadTimeline){
            loadTimelineConfigFlag = true;
        }else if(e.target == saveTimeline){
            saveTimelineConfigFlag = true;
        }
    }
}

//--------------------------------------------------------------
void moTimeline::onToggleEvent(ofxDatGuiToggleEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == setRetina){
            timeline->forceRetina = e.checked;
        }
    }
}

//--------------------------------------------------------------
void moTimeline::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == guiTrackName){
            actualTrackName = e.text;
        }else if(e.target == guiDuration){
            if(isInteger(e.text)){
                this->setCustomVar(static_cast<float>(ofToInt(e.text)),"DURATION");
                durationInSeconds = ofToInt(e.text);
            }
        }else if(e.target == guiFPS){
            if(isInteger(e.text)){
                this->setCustomVar(static_cast<float>(ofToInt(e.text)),"FPS");
                fps = ofToInt(e.text);
            }
        }
    }
}
