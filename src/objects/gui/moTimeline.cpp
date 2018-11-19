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

//--------------------------------------------------------------
moTimeline::moTimeline() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new string();  // control
    *static_cast<string *>(_inletParams[0]) = "";
    _inletParams[1] = new float();  // playhead
    *(float *)&_inletParams[1] = 0.0f;
    

    _outletParams[0] = new vector<float>();  // timeline tracks data vector

    this->initInletsState();

    timeline    = new ofxTimeline();
    localFont   = new ofxFontStash();

    actualTrackName     = "track_name";
    sameNameAvoider     = 0;
    lastTrackID         = 0;
    durationInSeconds   = 60;
    timelineLoaded      = false;
    isFullscreen        = false;

}

//--------------------------------------------------------------
void moTimeline::newObject(){
    this->setName("timeline");
    this->addInlet(VP_LINK_STRING,"control");
    this->addInlet(VP_LINK_NUMERIC,"playhead");
    this->addOutlet(VP_LINK_ARRAY);

    this->setCustomVar(static_cast<float>(lastTrackID),"LAST_TRACK_ID");
    this->setCustomVar(static_cast<float>(durationInSeconds),"DURATION");
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
    window->setWindowTitle("Timeline "+ofToString(this->getId()));
    window->setVerticalSync(true);

    ofAddListener(window->events().draw,this,&moTimeline::drawInWindow);
    ofAddListener(window->events().keyPressed,this,&moTimeline::keyPressed);
    ofAddListener(window->events().keyReleased,this,&moTimeline::keyReleased);
    ofAddListener(window->events().mouseMoved ,this,&moTimeline::mouseMoved);
    ofAddListener(window->events().mouseDragged ,this,&moTimeline::mouseDragged);
    ofAddListener(window->events().mousePressed ,this,&moTimeline::mousePressed);
    ofAddListener(window->events().mouseReleased ,this,&moTimeline::mouseReleased);
    ofAddListener(window->events().windowResized ,this,&moTimeline::windowResized);

    initTimeline();

    // GUI
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onButtonEvent(this, &moTimeline::onButtonEvent);
    gui->onTextInputEvent(this, &moTimeline::onTextInputEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    guiDuration = gui->addTextInput("LENGTH","60"); // duration in seconds
    guiDuration->setText(ofToString(static_cast<int>(floor(this->getCustomVar("DURATION")))));
    guiDuration->setUseCustomMouse(true);
    setDuration = gui->addButton("SET DURATION");
    setDuration->setUseCustomMouse(true);
    setDuration->setLabelAlignment(ofxDatGuiAlignment::CENTER);
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
    saveTimeline = gui->addButton("SAVE TIMELINE");
    saveTimeline->setUseCustomMouse(true);
    saveTimeline->setLabelAlignment(ofxDatGuiAlignment::CENTER);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

}

//--------------------------------------------------------------
void moTimeline::updateObjectContent(map<int,PatchObject*> &patchObjects){
    gui->update();
    header->update();
    guiTrackName->update();
    addCurveTrack->update();
    addBangTrack->update();
    addColorTrack->update();
    addLFOTrack->update();
    guiDuration->update();
    setDuration->update();
    loadTimeline->update();
    saveTimeline->update();

    if(!timelineLoaded){
        timelineLoaded = true;
        timeline->setDurationInSeconds(static_cast<int>(floor(this->getCustomVar("DURATION"))));
    }
}

//--------------------------------------------------------------
void moTimeline::drawObjectContent(ofxFontStash *font){
    localFont = font;
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void moTimeline::removeObjectContent(){
    window->setWindowShouldClose();
}

//--------------------------------------------------------------
void moTimeline::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    guiTrackName->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    addCurveTrack->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    addBangTrack->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    addColorTrack->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    addLFOTrack->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    guiDuration->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    setDuration->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    loadTimeline->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    saveTimeline->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || guiTrackName->hitTest(_m-this->getPos()) || addCurveTrack->hitTest(_m-this->getPos())
                            || addBangTrack->hitTest(_m-this->getPos()) || addColorTrack->hitTest(_m-this->getPos()) || addLFOTrack->hitTest(_m-this->getPos())
                            || guiDuration->hitTest(_m-this->getPos()) || setDuration->hitTest(_m-this->getPos()) || loadTimeline->hitTest(_m-this->getPos())  || saveTimeline->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void moTimeline::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        guiTrackName->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        addCurveTrack->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        addBangTrack->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        addColorTrack->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        addLFOTrack->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        guiDuration->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        setDuration->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void moTimeline::initTimeline(){
    sameNameAvoider += static_cast<int>(floor(this->getCustomVar("LAST_TRACK_ID")));

    if(this->filepath == "none"){
        this->filepath = "temp/";
    }

    timeline->setWorkingFolder(this->filepath);

    timeline->setup();
    timeline->disableEvents();
    timeline->setFrameRate(30);
    timeline->setDurationInSeconds(static_cast<int>(floor(this->getCustomVar("DURATION"))));
    timeline->setLoopType(OF_LOOP_NORMAL);

    autoAddTracks(this->filepath);

    timeline->loadTracksFromFolder(this->filepath);

}

//--------------------------------------------------------------
void moTimeline::autoAddTracks(string path){
    filesystem::path temppath = filesystem::path(path);
    ofDirectory dir(temppath);
    if(dir.isDirectory()){
        dir.listDir();
        for(int i = 0; i < dir.size(); i++){
            size_t found = dir.getPath(i).find_last_of("/\\");
            string fileStr = dir.getPath(i).substr(found+1);
            size_t found2 = fileStr.find_first_of("_");
            string trackName = fileStr.substr(found2+1);
            string tempName = trackName.erase(trackName.length()-4);
            if(trackName.at(0) == '_'){
                if(trackName.at(1) == 'F'){
                    timeline->addCurves(tempName);
                }else if(trackName.at(1) == 'B'){
                    timeline->addBangs(tempName);
                }else if(trackName.at(1) == 'C'){
                    timeline->addColors(tempName);
                }else if(trackName.at(1) == 'L'){
                    timeline->addLFO(tempName);
                }
            }
        }
    }
}

//--------------------------------------------------------------
void moTimeline::loadTimelineData(string folder){
    this->filepath = forceCheckMosaicDataPath(folder);

    timeline->clear();
    timeline->setWorkingFolder(this->filepath);

    autoAddTracks(this->filepath);

    timeline->loadTracksFromFolder(this->filepath);

    timelineLoaded = false;
}

//--------------------------------------------------------------
void moTimeline::saveTimelineData(string folder){
    this->filepath = forceCheckMosaicDataPath(folder);

    timeline->setWorkingFolder(this->filepath);
    timeline->save();
    timeline->saveTracksToFolder(this->filepath);
}

//--------------------------------------------------------------
void moTimeline::drawInWindow(ofEventArgs &e){
    ofBackground(20);
    ofPushStyle();
    ofSetColor(255);
    timeline->draw();
    ofSetColor(120);
    // RETINA FIX
    if(ofGetScreenWidth() >= RETINA_MIN_WIDTH && ofGetScreenHeight() >= RETINA_MIN_HEIGHT){
        localFont->draw("forked ofxTimeline modded for Mosaic, original ofxaddon from James George co-developed by YCAM Interlab",26,40,window->getHeight()-20);
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
    timeline->mouseMoved(e);
}

//--------------------------------------------------------------
void moTimeline::mouseDragged(ofMouseEventArgs &e){
    timeline->mouseDragged(e);
}

//--------------------------------------------------------------
void moTimeline::mousePressed(ofMouseEventArgs &e){
    timeline->mousePressed(e);
}

//--------------------------------------------------------------
void moTimeline::mouseReleased(ofMouseEventArgs &e){
    timeline->mouseReleased(e);
}

//--------------------------------------------------------------
void moTimeline::windowResized(ofResizeEventArgs &e){
    timeline->windowResized(e);
}

//--------------------------------------------------------------
void moTimeline::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == addCurveTrack){
            string tempName = "_F_"+actualTrackName+"_"+ofToString(sameNameAvoider);
            timeline->addCurves(tempName,ofToDataPath(this->filepath + tempName + ".xml"));
            sameNameAvoider++;
            this->setCustomVar(static_cast<float>(sameNameAvoider),"LAST_TRACK_ID");
        }else if(e.target == addBangTrack){
            string tempName = "_B_"+actualTrackName+"_"+ofToString(sameNameAvoider);
            timeline->addBangs(tempName,ofToDataPath(this->filepath + tempName + ".xml"));
            sameNameAvoider++;
            this->setCustomVar(static_cast<float>(sameNameAvoider),"LAST_TRACK_ID");
        }else if(e.target == addColorTrack){
            string tempName = "_C_"+actualTrackName+"_"+ofToString(sameNameAvoider);
            timeline->addColors(tempName,ofToDataPath(this->filepath + tempName + ".xml"));
            sameNameAvoider++;
            this->setCustomVar(static_cast<float>(sameNameAvoider),"LAST_TRACK_ID");
        }else if(e.target == addLFOTrack){
            string tempName = "_L_"+actualTrackName+"_"+ofToString(sameNameAvoider);
            timeline->addLFO(tempName,ofToDataPath(this->filepath + tempName + ".xml"));
            sameNameAvoider++;
            this->setCustomVar(static_cast<float>(sameNameAvoider),"LAST_TRACK_ID");
        }else if(e.target == setDuration){
            timeline->setDurationInSeconds(durationInSeconds);
        }else if(e.target == loadTimeline){
            ofFileDialogResult folderSelectResult = ofSystemLoadDialog("Select folder for loading timeline data",true);
            if(folderSelectResult.bSuccess){
                ofFile tempfile (folderSelectResult.getPath());
                if(tempfile.exists() && tempfile.isDirectory()){
                    loadTimelineData(tempfile.getAbsolutePath()+"/");
                }
            }
        }else if(e.target == saveTimeline){
            ofFileDialogResult folderSelectResult = ofSystemLoadDialog("Select folder for saving timeline data",true);
            if(folderSelectResult.bSuccess){
                ofFile tempfile (folderSelectResult.getPath());
                if(tempfile.exists() && tempfile.isDirectory()){
                     saveTimelineData(tempfile.getAbsolutePath()+"/");
                }
            }
        }
    }
}

//--------------------------------------------------------------
void moTimeline::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == guiTrackName){
            actualTrackName = e.text;
        }if(e.target == guiDuration){
            if(isInteger(e.text)){
                this->setCustomVar(static_cast<float>(ofToInt(e.text)),"DURATION");
                durationInSeconds = ofToInt(e.text);
            }
        }
    }
}
