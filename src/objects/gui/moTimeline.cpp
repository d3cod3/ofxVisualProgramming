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

#ifndef OFXVP_BUILD_WITH_MINIMAL_OBJECTS

#include "moTimeline.h"

#include "GLFW/glfw3.h"

//--------------------------------------------------------------
moTimeline::moTimeline() : PatchObject("timeline"){

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
    fps                 = 30;
    bpm                 = 120;
    timelineLoaded      = false;
    resetTimelineOutlets= false;

    isFullscreen        = false;
    scrolledDisplacement= 0;

    lastMessage         = "";
    lastPlayheadPos     = 0;

    lastTimelineFolder      = "";
    loadTimelineConfigFlag  = false;
    saveTimelineConfigFlag  = false;
    loadedTimelineConfig    = false;
    savedTimelineConfig     = false;

    retinaScreen            = false;
    showBPMGrid             = false;
    isPaused                = false;
    isLoop                  = false;

    loaded                  = false;
    loadedObjectFromXML     = false;
    autoRemove              = false;
    startTime               = ofGetElapsedTimeMillis();
    waitTime                = 100;

}

//--------------------------------------------------------------
void moTimeline::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_STRING,"control");
    this->addInlet(VP_LINK_NUMERIC,"playhead");

    this->setCustomVar(static_cast<float>(lastTrackID),"LAST_TRACK_ID");
    this->setCustomVar(static_cast<float>(durationInSeconds),"DURATION");
    this->setCustomVar(static_cast<float>(fps),"FPS");
    this->setCustomVar(bpm,"BPM");
}

//--------------------------------------------------------------
void moTimeline::customReset(){
    timeline->setWorkingFolder(this->filepath);
    timeline->setName("timeline"+ofToString(this->nId));
    timeline->saveTracksToFolder(this->filepath);

    this->saveConfig(false);
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
        settings.setSize(1600, 1200);
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

}

//--------------------------------------------------------------
void moTimeline::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(!timelineLoaded){
        timelineLoaded = true;
        timeline->setDurationInSeconds(static_cast<int>(floor(this->getCustomVar("DURATION"))));
        timeline->setFrameRate(static_cast<float>(floor(this->getCustomVar("FPS"))));
    }

    /*if(loadTimelineConfigFlag){
        loadTimelineConfigFlag = false;
        //fd.openFolder("load timeline config","Select folder for loading timeline data");
    }*/

    /*if(saveTimelineConfigFlag){
        saveTimelineConfigFlag = false;
        //fd.openFolder("save timeline config","Select folder for saving timeline data");
    }*/

    /*if(loadedTimelineConfig){
        loadedTimelineConfig = false;
        if(lastTimelineFolder != ""){
            ofFile tempfile (lastTimelineFolder);
            if(tempfile.exists() && tempfile.isDirectory()){
                loadTimelineData(tempfile.getAbsolutePath()+"/");
            }
        }
    }*/

    /*if(savedTimelineConfig){
        savedTimelineConfig = false;
        if(lastTimelineFolder != ""){
            ofFile tempfile (lastTimelineFolder);
            if(tempfile.exists() && tempfile.isDirectory()){
                 saveTimelineData(tempfile.getAbsolutePath()+"/");
            }
        }
    }*/

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
            if(actualTracks->at(i).at(2) == 'S' || (actualTracks->at(i).at(0) == '_' && actualTracks->at(i).at(3) == 'S')){ // SWITCHES
                ofxTLSwitches* tempMT = (ofxTLSwitches*)timeline->getTrack(actualTracks->at(i));
                *(float *)&_outletParams[i] = static_cast<float>(tempMT->isOn());
            }else{
                *(float *)&_outletParams[i] = static_cast<float>(timeline->getValue(actualTracks->at(i)));
            }
        }else if(this->getOutletType(i) == VP_LINK_ARRAY){
            if(actualTracks->at(i).at(2) == 'C' || (actualTracks->at(i).at(0) == '_' && actualTracks->at(i).at(3) == 'C')){ // COLOR
                static_cast<vector<float> *>(_outletParams[i])->at(0) = timeline->getColor(actualTracks->at(i)).r; // RED
                static_cast<vector<float> *>(_outletParams[i])->at(1) = timeline->getColor(actualTracks->at(i)).g; // GREEN
                static_cast<vector<float> *>(_outletParams[i])->at(2) = timeline->getColor(actualTracks->at(i)).b; // BLUE
            }
        }
    }

    // auto set/load timeline data folder
    if(!loaded){
        loaded = true;
        ofLog(OF_LOG_NOTICE,"Timeline settings loaded from folder: %s",this->patchFolderPath.c_str());
        ofFile temp(this->patchFolderPath+"timeline"+ofToString(this->nId));
        if(!temp.exists()){
            saveTimelineData(this->patchFolderPath+"timeline"+ofToString(this->nId)+"/");
        }else{
            this->filepath = this->patchFolderPath+"timeline"+ofToString(this->nId)+"/";
            loadTimelineData(this->filepath);
        }
        // force init for retina screens
        ofResizeEventArgs te;
        te.width = window->getWidth();
        te.height = window->getHeight();
        timeline->windowResized(te);
        timeline->setWidth(window->getWidth());
    }

    // auto remove
    if(window->getGLFWWindow() == nullptr && !autoRemove){
        autoRemove = true;
        ofNotifyEvent(this->removeEvent, this->nId);
        this->willErase = true;
    }

}

//--------------------------------------------------------------
void moTimeline::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    localFont = font;
}

//--------------------------------------------------------------
void moTimeline::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){

        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            drawObjectNodeConfig();

            ImGui::EndMenu();
        }

        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 window_size = ImGui::GetWindowSize();
        ImVec2 ph_pos = ImVec2(window_pos.x + (20*this->scaleFactor), window_pos.y + (20*this->scaleFactor));


        // draw position (timecode)
        _nodeCanvas.getNodeDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize()*_nodeCanvas.GetCanvasScale()/this->scaleFactor, ImVec2(window_pos.x +(40*_nodeCanvas.GetCanvasScale()), window_pos.y + (40*_nodeCanvas.GetCanvasScale())), IM_COL32_WHITE, timeline->getTimecode().timecodeForFrame(timeline->getCurrentFrame()).c_str(), NULL, 0.0f);

        // draw player state
        if(timeline->getIsPlaying()){ // play
            _nodeCanvas.getNodeDrawList()->AddTriangleFilled(ImVec2(window_pos.x+window_size.x-(50*_nodeCanvas.GetCanvasScale()),window_pos.y+window_size.y-(40*_nodeCanvas.GetCanvasScale())), ImVec2(window_pos.x+window_size.x-(50*_nodeCanvas.GetCanvasScale()), window_pos.y+window_size.y-(20*_nodeCanvas.GetCanvasScale())), ImVec2(window_pos.x+window_size.x-(30*_nodeCanvas.GetCanvasScale()), window_pos.y+window_size.y-(30*_nodeCanvas.GetCanvasScale())), IM_COL32(255, 255, 255, 120));
        }else if(!timeline->getIsPlaying() && timeline->getCurrentFrame() > 1){ // pause
            _nodeCanvas.getNodeDrawList()->AddRectFilled(ImVec2(window_pos.x+window_size.x-(50*_nodeCanvas.GetCanvasScale()),window_pos.y+window_size.y-(40*_nodeCanvas.GetCanvasScale())),ImVec2(window_pos.x+window_size.x-(42*_nodeCanvas.GetCanvasScale()),window_pos.y+window_size.y-(20*_nodeCanvas.GetCanvasScale())),IM_COL32(255, 255, 255, 120));
            _nodeCanvas.getNodeDrawList()->AddRectFilled(ImVec2(window_pos.x+window_size.x-(38*_nodeCanvas.GetCanvasScale()),window_pos.y+window_size.y-(40*_nodeCanvas.GetCanvasScale())),ImVec2(window_pos.x+window_size.x-(30*_nodeCanvas.GetCanvasScale()),window_pos.y+window_size.y-(20*_nodeCanvas.GetCanvasScale())),IM_COL32(255, 255, 255, 120));
        }else if(!timeline->getIsPlaying() && timeline->getCurrentFrame() <= 1){ // stop
            _nodeCanvas.getNodeDrawList()->AddRectFilled(ImVec2(window_pos.x+window_size.x-(50*_nodeCanvas.GetCanvasScale()),window_pos.y+window_size.y-(40*_nodeCanvas.GetCanvasScale())),ImVec2(window_pos.x+window_size.x-(30*_nodeCanvas.GetCanvasScale()),window_pos.y+window_size.y-(20*_nodeCanvas.GetCanvasScale())),IM_COL32(255, 255, 255, 120));
        }

        // draw playhead
        float phx = ofMap( ofClamp(timeline->getPercentComplete(),0.0f,1.0f), 0.0f, 1.0f, 1, (this->width*0.98f*_nodeCanvas.GetCanvasScale())-(31*this->scaleFactor) );
        _nodeCanvas.getNodeDrawList()->AddLine(ImVec2(ph_pos.x + phx, ph_pos.y),ImVec2(ph_pos.x + phx, window_size.y+ph_pos.y-(26*this->scaleFactor)),IM_COL32(255, 255, 255, 160), 2.0f);

        _nodeCanvas.EndNodeContent();
    }
}

//--------------------------------------------------------------
void moTimeline::drawObjectNodeConfig(){
    ImGui::Spacing();
    ImGui::Text("%s - %s", timeline->getTimecode().timecodeForFrame(timeline->getCurrentFrame()).c_str(), timeline->getDurationInTimecode().c_str());
    ImGui::Spacing();
    ImGui::Text("%s - %s", ofToString(timeline->getCurrentFrame()).c_str(), ofToString(timeline->getDurationInFrames()).c_str());

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button, VHS_BLUE);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, VHS_BLUE_OVER);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, VHS_BLUE_OVER);
    if(ImGui::Button(ICON_FA_PLAY,ImVec2(69*this->scaleFactor,26*this->scaleFactor))){
        timeline->play();
    }
    ImGui::SameLine();
    if(ImGui::Button(ICON_FA_STOP,ImVec2(69*this->scaleFactor,26*this->scaleFactor))){
        timeline->setCurrentTimeSeconds(0);
        timeline->stop();
    }
    ImGui::PopStyleColor(3);
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, VHS_YELLOW);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, VHS_YELLOW_OVER);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, VHS_YELLOW_OVER);
    if(ImGui::Button(ICON_FA_PAUSE,ImVec2(69*this->scaleFactor,26*this->scaleFactor))){
        isPaused = !isPaused;
        if(isPaused){
            timeline->stop();
        }else{
            timeline->play();
        }

    }
    ImGui::PopStyleColor(3);
    ImGui::Spacing();
    if(ImGui::Checkbox("LOOP " ICON_FA_REDO,&isLoop)){
        if(isLoop){
            timeline->setLoopType(OF_LOOP_NORMAL);
        }else{
            timeline->setLoopType(OF_LOOP_NONE);
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();

    if(ImGui::Checkbox("Retina Screen",&retinaScreen)){
        timeline->forceRetina = retinaScreen;
        ofResizeEventArgs te;
        te.width = window->getWidth();
        te.height = window->getHeight();
        timeline->windowResized(te);
        timeline->setWidth(window->getWidth());
    }
    ImGui::Spacing();
    if(ImGui::InputInt("Duration",&durationInSeconds)){
        if(durationInSeconds < 1){
            durationInSeconds = 1;
        }
        this->setCustomVar(static_cast<float>(durationInSeconds),"DURATION");
        timeline->setDurationInSeconds(durationInSeconds);
    }
    ImGui::SameLine(); ImGuiEx::HelpMarker("Duration in seconds.");
    ImGui::Spacing();
    if(ImGui::InputInt("FPS",&fps)){
        if(fps < 1){
            fps = 1;
        }
        this->setCustomVar(static_cast<float>(fps),"FPS");
        timeline->setFrameRate(fps);
    }
    ImGui::Spacing();
    if(ImGui::InputInt("BPM",&bpm)){
        if(bpm < 1){
            bpm = 1;
        }
        this->setCustomVar(static_cast<float>(bpm),"BPM");
        timeline->setBPM(static_cast<float>(bpm));
    }
    ImGui::Spacing();
    if(ImGui::Checkbox("Toggle BPM grid",&showBPMGrid)){
        timeline->setShowBPMGrid(showBPMGrid);
        if(showBPMGrid){
            ofLog(OF_LOG_NOTICE,"Zoom IN on your timeline to make BPM Grid appear!");
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::InputText("Track Name",&actualTrackName);

    ImGui::Spacing();
    if(ImGui::Button("ADD CURVE TRACK",ImVec2(224*scaleFactor,26*scaleFactor))){
        addTrack(TIMELINE_CURVE_TRACK);
    }
    ImGui::Spacing();
    if(ImGui::Button("ADD BANG TRACK",ImVec2(224*scaleFactor,26*scaleFactor))){
        addTrack(TIMELINE_BANG_TRACK);
    }
    ImGui::Spacing();
    if(ImGui::Button("ADD SWITCH TRACK",ImVec2(224*scaleFactor,26*scaleFactor))){
        addTrack(TIMELINE_SWITCH_TRACK);
    }
    ImGui::Spacing();
    if(ImGui::Button("ADD COLOR TRACK",ImVec2(224*scaleFactor,26*scaleFactor))){
        addTrack(TIMELINE_COLOR_TRACK);
    }
    ImGui::Spacing();
    if(ImGui::Button("ADD LFO TRACK",ImVec2(224*scaleFactor,26*scaleFactor))){
        addTrack(TIMELINE_LFO_TRACK);
    }

    ImGuiEx::ObjectInfo(
                "Advanced GUI module, it allows you to visualize/ edit a timeline with different keyframes controlled tracks : curves, bangs, switches, colors, and lfo.",
                "https://mosaic.d3cod3.org/reference.php?r=timeline", scaleFactor);
}

//--------------------------------------------------------------
void moTimeline::removeObjectContent(bool removeFileFromData){
    if(window->getGLFWWindow() != nullptr){
        window->setWindowShouldClose();
    }
    if(removeFileFromData){
        removeFile(filepath);
    }
}

//--------------------------------------------------------------
void moTimeline::initTimeline(){
    sameNameAvoider += static_cast<int>(floor(this->getCustomVar("LAST_TRACK_ID")));

    if(this->filepath == "none"){
        this->filepath = "temp/";
    }else{
        loadedObjectFromXML = true;
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
    timeline->setBPM(this->getCustomVar("BPM"));
    timeline->enableSnapToBPM(true);
    timeline->setShowBPMGrid(false);

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
                }else if(trackName.at(2) == 'S'){
                    timeline->addSwitches(tempName);
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
        case TIMELINE_SWITCH_TRACK:
            tempName = "_"+preCode+"S-"+actualTrackName;
            timeline->addSwitches(tempName,ofToDataPath(this->filepath + timeline->getName() + tempName + ".xml"));
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

    this->saveConfig(false);
}

//--------------------------------------------------------------
void moTimeline::saveTimelineData(string folder){
    this->filepath = forceCheckMosaicDataPath(folder);

    timeline->setWorkingFolder(this->filepath);
    timeline->setName("timeline"+ofToString(this->nId));
    timeline->saveTracksToFolder(this->filepath);

    this->saveConfig(false);
}

//--------------------------------------------------------------
void moTimeline::updateOutletsConfig(){

    resetTimelineOutlets = true;

}

//--------------------------------------------------------------
void moTimeline::saveOutletConfig(){
    ofxXmlSettings XML;
    if(XML.loadFile(this->patchFile)){
        int totalObjects = XML.getNumTags("object");

        // Load Links
        vector<ofVec3f> tempLinks;
        for(int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                if(XML.getValue("id", -1) == this->nId){
                    if (XML.pushTag("outlets")){
                        int totalOutlets = XML.getNumTags("link");
                        for(int j=0;j<totalOutlets;j++){
                            if (XML.pushTag("link",j)){
                                int totalLinks = XML.getNumTags("to");
                                for(int z=0;z<totalLinks;z++){
                                    if(XML.pushTag("to",z)){
                                        int toObjectID = XML.getValue("id", 0);
                                        int toInletID = XML.getValue("inlet", 0);
                                        tempLinks.push_back(ofVec3f(j,toObjectID,toInletID));
                                        XML.popTag();
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

        // Save new object outlet config
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

                                // re-add previous links
                                for(int z=0;z<tempLinks.size();z++){
                                    if(static_cast<int>(floor(tempLinks.at(z).x)) == j){
                                        int newTo = XML.addTag("to");
                                        if(XML.pushTag("to", newTo)){
                                            XML.setValue("id",static_cast<int>(floor(tempLinks.at(z).y)));
                                            XML.setValue("inlet",static_cast<int>(floor(tempLinks.at(z).z)));
                                            XML.popTag();
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

    ofNotifyEvent(this->reconnectOutletsEvent, this->nId);
}

//--------------------------------------------------------------
void moTimeline::resetOutlets(){
    vector<ofxTLTrack*> tempTracks = timeline->getPage("Page One")->getTracks();

    this->outPut.clear();
    this->outletsType.clear();

    this->numOutlets = tempTracks.size();

    for( int i = 0; i < tempTracks.size(); i++){
        if(tempTracks.at(i)->getTrackType() == "Colors"){
            _outletParams[i] = new vector<float>();
            static_cast<vector<float> *>(_outletParams[i])->assign(3,0.0f);
            this->addOutlet(VP_LINK_ARRAY,"colorTrackRGB");
        }else{
            _outletParams[i] = new float();
            *(float *)&_outletParams[i] = 0.0f;
            this->addOutlet(VP_LINK_NUMERIC,"trackData");
        }
    }

    this->height      = OBJECT_HEIGHT;

    if(this->numOutlets > 6){
        this->height          *= 2;
    }

    if(this->numOutlets > 12){
        this->height          *= 2;
    }

    saveOutletConfig();

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
    if(ofGetElapsedTimeMillis()-startTime > waitTime){
        timeline->draw();
    }
    ofPopMatrix();
    ofSetColor(120);
    // RETINA FIX
    if(timeline->forceRetina){
        localFont->draw("forked ofxTimeline modded for Mosaic, original ofxaddon from James George co-developed by YCAM Interlab",24,40,window->getHeight()-20);
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
        if(ofGetScreenWidth() >= RETINA_MIN_WIDTH && ofGetScreenHeight() >= RETINA_MIN_HEIGHT){
            window->setWindowShape(1600,1200);
        }else{
            window->setWindowShape(800,600);
        }

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


OBJECT_REGISTER( moTimeline, "timeline", OFXVP_OBJECT_CAT_GUI)

#endif
