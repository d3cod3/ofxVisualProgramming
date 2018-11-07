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

#include "BashScript.h"

//--------------------------------------------------------------
BashScript::BashScript() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new string();         // control
    *(string *)&_inletParams[0] = "";

    _outletParams[0] = new string();        // output
    *(string *)&_outletParams[0] = "";

    this->initInletsState();

    bashIcon            = new ofImage();

    nameLabelLoaded     = false;
    scriptLoaded        = false;
    isNewObject         = false;

    isGUIObject         = true;
    this->isOverGUI     = true;

    lastMessage         = "";

    threadLoaded    = false;
    needToLoadScript= true;
}

//--------------------------------------------------------------
void BashScript::newObject(){
    this->setName("bash script");
    this->addInlet(VP_LINK_STRING,"control");
    this->addOutlet(VP_LINK_STRING);
}

//--------------------------------------------------------------
void BashScript::threadedFunction(){
    while(isThreadRunning()){
        std::unique_lock<std::mutex> lock(mutex);
        if(needToLoadScript){
            needToLoadScript = false;
            loadScript(filepath);
            threadLoaded = true;
            nameLabelLoaded = true;
        }
        condition.wait(lock);
    }

}

//--------------------------------------------------------------
void BashScript::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    // GUI
    bashIcon->load("images/bash.png");

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onButtonEvent(this, &BashScript::onButtonEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    scriptName = gui->addLabel("NONE");
    gui->addBreak();

    newButton = gui->addButton("NEW");
    newButton->setUseCustomMouse(true);

    loadButton = gui->addButton("OPEN");
    loadButton->setUseCustomMouse(true);

    editButton = gui->addButton("EDIT");
    editButton->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    // Load script
    tempCommand.setup();
    watcher.start();
    if(filepath == "none"){
        isNewObject = true;
        ofFile file (ofToDataPath("scripts/empty.sh"));
        filepath = file.getAbsolutePath();
    }
    if(!isThreadRunning()){
        startThread();
    }
}

//--------------------------------------------------------------
void BashScript::updateObjectContent(map<int,PatchObject*> &patchObjects){

    if(tempCommand.getCmdExec() && tempCommand.getSysStatus() != 0){
        ofSystemAlertDialog("Mosaic works better with Atom [https://atom.io/] text editor, and it seems you do not have it installed on your system.");
    }

    // listen to message control (_inletParams[0])
    if(this->inletsConnected[0]){
        if(lastMessage != *static_cast<string *>(_inletParams[0])){
            lastMessage = *static_cast<string *>(_inletParams[0]);
        }

        if(lastMessage == "bang"){
            reloadScriptThreaded();
        }
    }

    // GUI
    gui->update();
    header->update();
    newButton->update();
    loadButton->update();
    editButton->update();

    if(nameLabelLoaded){
        nameLabelLoaded = false;
        if(currentScriptFile.getFileName().size() > 22){
            scriptName->setLabel(currentScriptFile.getFileName().substr(0,21)+"...");
        }else{
            scriptName->setLabel(currentScriptFile.getFileName());
        }
    }

    while(watcher.waitingEvents()) {
        pathChanged(watcher.nextEvent());
    }

    condition.notify_all();
}

//--------------------------------------------------------------
void BashScript::drawObjectContent(ofxFontStash *font){
    ofSetColor(255,150);
    ofEnableAlphaBlending();
    bashIcon->draw(this->width/2,this->headerHeight,((this->height/2.2f)/bashIcon->getHeight())*bashIcon->getWidth(),this->height/2.2f);
    // GUI
    ofSetColor(255);
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void BashScript::removeObjectContent(){
    tempCommand.stop();
}

//--------------------------------------------------------------
void BashScript::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    newButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    loadButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    editButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || newButton->hitTest(_m-this->getPos()) || loadButton->hitTest(_m-this->getPos()) || editButton->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void BashScript::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        newButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        loadButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        editButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void BashScript::loadScript(string scriptFile){

    filepath = scriptFile;
    currentScriptFile.open(filepath);

    string cmd = "";
    FILE *execFile;
#ifdef TARGET_LINUX
    cmd = "sh "+filepath;
    execFile = popen(cmd.c_str(), "r");
#elif defined(TARGET_OSX)
    cmd = "sh "+filepath;
    execFile = popen(cmd.c_str(), "r");
#elif defined(TARGET_WIN32)
    cmd = filepath;
    execFile = _popen(cmd.c_str(), "r");
#endif

    if (execFile){
        scriptLoaded = true;
        watcher.removeAllPaths();
        watcher.addPath(filepath);

        ofLog(OF_LOG_NOTICE,"[verbose] bash script: %s RUNNING!",filepath.c_str());
        ofLog(OF_LOG_NOTICE," ");

        char buffer[128];
        _outletParams[0] = new string();
        *static_cast<string *>(_outletParams[0]) = "";
        while(!feof(execFile)){
            if(fgets(buffer, sizeof(buffer), execFile) != nullptr){
                char *s = buffer;
                std::string tempstr(s);
                static_cast<string *>(_outletParams[0])->append(tempstr);
                static_cast<string *>(_outletParams[0])->append(" ");
            }
        }
        //ofLog(OF_LOG_NOTICE,"%s",static_cast<string *>(_outletParams[0])->c_str());
        ofLog(OF_LOG_NOTICE,"[verbose]bash script: %s EXECUTED!",filepath.c_str());

#ifdef TARGET_LINUX
        pclose(execFile);
#elif defined(TARGET_OSX)
        pclose(execFile);
#elif defined(TARGET_WIN32)
        _pclose(execFile);
#endif

    }

}

//--------------------------------------------------------------
void BashScript::reloadScriptThreaded(){
    scriptLoaded = false;
    needToLoadScript = true;
}

//--------------------------------------------------------------
void BashScript::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == newButton){
            string newFileName = "bashScript_"+ofGetTimestampString("%y%m%d")+".sh";
            ofFileDialogResult saveFileResult = ofSystemSaveDialog(newFileName,"Save new Bash script as");
            if (saveFileResult.bSuccess){
                ofFile fileToRead(ofToDataPath("scripts/empty.sh"));
                ofFile newBashFile (saveFileResult.getPath());
                ofFile::copyFromTo(fileToRead.getAbsolutePath(),newBashFile.getAbsolutePath(),true,true);
                threadLoaded = false;
                filepath = newBashFile.getAbsolutePath();
                reloadScriptThreaded();
            }
        }else if(e.target == loadButton){
            ofFileDialogResult openFileResult= ofSystemLoadDialog("Select a bash script");
            if (openFileResult.bSuccess){
                ofFile file (openFileResult.getPath());
                if (file.exists()){
                    string fileExtension = ofToUpper(file.getExtension());
                    if(fileExtension == "SH" || fileExtension == "CMD") {
                        threadLoaded = false;
                        filepath = file.getAbsolutePath();
                        reloadScriptThreaded();
                    }
                }
            }
        }else if(e.target == editButton){
            if(filepath != "none" && scriptLoaded){
                string cmd = "";
#ifdef TARGET_LINUX
                cmd = "atom "+filepath;
#elif defined(TARGET_OSX)
                cmd = "open -a /Applications/Atom.app "+filepath;
#elif defined(TARGET_WIN32)
                cmd = "atom "+filepath;
#endif
                tempCommand.execCommand(cmd);

            }
        }
    }
}

//--------------------------------------------------------------
void BashScript::pathChanged(const PathWatcher::Event &event) {
    switch(event.change) {
        case PathWatcher::CREATED:
            //ofLogVerbose(PACKAGE) << "path created " << event.path;
            break;
        case PathWatcher::MODIFIED:
            //ofLogVerbose(PACKAGE) << "path modified " << event.path;
            filepath = event.path;
            reloadScriptThreaded();
            break;
        case PathWatcher::DELETED:
            //ofLogVerbose(PACKAGE) << "path deleted " << event.path;
            return;
        default: // NONE
            return;
    }

}
