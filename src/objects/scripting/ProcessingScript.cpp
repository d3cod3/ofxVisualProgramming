/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2019 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#if defined(TARGET_WIN32)
    // Unavailable on windows.
#elif !defined(OFXVP_BUILD_WITH_MINIMAL_OBJECTS)

#include "ProcessingScript.h"

//--------------------------------------------------------------
ProcessingScript::ProcessingScript() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 2;

    _inletParams[0] = new vector<float>();      // data input

    _outletParams[0] = new ofTexture();         // processing texture
    _outletParams[1] = new vector<float>();     // processing data output

    this->initInletsState();

    jvm                 = new ofxJava();

    isNewObject         = false;

    isGUIObject         = true;
    this->isOverGUI     = true;

    lastProcessingScript        = "";
    loadProcessingScriptFlag    = false;
    saveProcessingScriptFlag    = false;
    processingScriptLoaded      = false;
    processingScriptSaved       = false;

    modalInfo                   = false;
    needToGrab                  = false;

}

//--------------------------------------------------------------
void ProcessingScript::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_ARRAY,"data");
    this->addOutlet(VP_LINK_TEXTURE,"processingTexture");
    this->addOutlet(VP_LINK_ARRAY,"processingData");
}

//--------------------------------------------------------------
void ProcessingScript::autoloadFile(string _fp){
    //lastProcessingScript = _fp;
    lastProcessingScript = copyFileToPatchFolder(this->patchFolderPath,_fp);
    processingScriptLoaded = true;
}

//--------------------------------------------------------------
void ProcessingScript::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onButtonEvent(this, &ProcessingScript::onButtonEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    scriptName = gui->addLabel("NONE");
    gui->addBreak();

    newButton = gui->addButton("NEW");
    newButton->setUseCustomMouse(true);

    loadButton = gui->addButton("OPEN");
    loadButton->setUseCustomMouse(true);

    //editButton = gui->addButton("EDIT");
    //editButton->setUseCustomMouse(true);

    gui->addBreak();
    clearButton = gui->addButton("CLEAR SCRIPT");
    clearButton->setUseCustomMouse(true);
    reloadButton = gui->addButton("RELOAD SCRIPT");
    reloadButton->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    // Setup ThreadedCommand var
    tempCommand.setup();

    // init processing
    watcher.start();

    if(filepath == "none"){
        isNewObject = true;
        ofFile file (ofToDataPath("scripts/Empty.java"));
        //filepath = file.getAbsolutePath();
        filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
    }

    loadScript(filepath);

}

//--------------------------------------------------------------
void ProcessingScript::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    // GUI
    gui->update();
    header->update();
    newButton->update();
    loadButton->update();
    //editButton->update();
    clearButton->update();
    reloadButton->update();

    if(loadProcessingScriptFlag){
        loadProcessingScriptFlag = false;
        //fd.openFile("load processing script"+ofToString(this->getId()),"Select a processing script");
    }

    if(saveProcessingScriptFlag){
        saveProcessingScriptFlag = false;
        string newFileName = "ProcessingScript_"+ofGetTimestampString("%y%m%d")+".java";
        //fd.saveFile("save processing script"+ofToString(this->getId()),"Save new Processing script as",newFileName);
    }

    while(watcher.waitingEvents()) {
        pathChanged(watcher.nextEvent());
    }

    // send data to processing
    if(this->inletsConnected[0]){
        dataIn.clear();
        for(int i=0;i<static_cast<int>(static_cast<vector<float> *>(_inletParams[0])->size());i++){
            dataIn.append(ofToString(static_cast<vector<float> *>(_inletParams[0])->at(i))+"\n");
        }
    }

    // receive data from processing
    static_cast<vector<float> *>(_outletParams[1])->clear();
    for (auto line : dataOut.getLines()){
        static_cast<vector<float> *>(_outletParams[1])->push_back(ofToFloat(line));
    }
}

//--------------------------------------------------------------
void ProcessingScript::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();

    ///////////////////////////////////////////
    // PROCESSING (JVM) LOGIC

    if(processingScriptLoaded){
        processingScriptLoaded = false;
        ofFile file (lastProcessingScript);

        //ofDirectory testJavalibdir;
        //testJavalibdir.open(std::filesystem::path(file.getEnclosingDirectory()+"javalibs/"));
        // && testJavalibdir.exists() && testJavalibdir.getFiles().size() >= 9

        if (file.exists()){
            string fileExtension = ofToUpper(file.getExtension());
            if(fileExtension == "JAVA") {
                jvm->closeJVM();
                filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
                //filepath = file.getAbsolutePath();
                loadScript(filepath);
            }
        }
    }

    if(processingScriptSaved){
        processingScriptSaved = false;
        ofFile fileToRead(ofToDataPath("scripts/Empty.java"));
        ofBuffer pfcontent = ofBufferFromFile(fileToRead);
        string tempString = pfcontent.getText();

        ofFile newProcessingFile (lastProcessingScript);

        string changeTo = newProcessingFile.getFileName().substr(0,checkFileExtension(newProcessingFile.getAbsolutePath(), ofToUpper(newProcessingFile.getExtension()), "JAVA").size()-5);
        size_t index;
        while ((index = tempString.find("Empty")) != string::npos){
            tempString.replace(index, 5, changeTo);
        }

        //ofDirectory testJavalibdir;
        //testJavalibdir.open(std::filesystem::path(fileToRead.getEnclosingDirectory()+"javalibs/"));

        // Save new processing/java script file
        ofBuffer newBuffer;
        newBuffer.append(tempString);

        ofBufferToFile(checkFileExtension(newProcessingFile.getAbsolutePath(), ofToUpper(newProcessingFile.getExtension()), "JAVA"),newBuffer,false);
        // Save javalibs folder
        /*ofDirectory newJavalibsdir;
        newJavalibsdir.createDirectory(std::filesystem::path(newProcessingFile.getEnclosingDirectory()+"javalibs/"));
        newJavalibsdir.open(std::filesystem::path(newProcessingFile.getEnclosingDirectory()+"javalibs/"));
        for(int i=0;i<testJavalibdir.getFiles().size();i++){
            ofFile::copyFromTo(testJavalibdir.getFile(i).getAbsolutePath(),newJavalibsdir.getAbsolutePath()+"/"+testJavalibdir.getFile(i).getFileName(),true,true);
        }*/

        jvm->closeJVM();
        filepath = copyFileToPatchFolder(this->patchFolderPath,checkFileExtension(newProcessingFile.getAbsolutePath(), ofToUpper(newProcessingFile.getExtension()), "JAVA"));
        //filepath = newProcessingFile.getAbsolutePath();
        loadScript(filepath);
    }

    if(jvm->sys_status == 0 && !jvm->compiled){
        jvm->setup();
        needToGrab = false;
    }

    if(jvm->compiled){
        jvm->update(dataIn,dataOut);

        if(jvm->loadedTxtInfo && jvm->renderTexture->isAllocated()){
            if(!needToGrab){
                needToGrab = true;
                _outletParams[0] = new ofTexture();
                static_cast<ofTexture *>(_outletParams[0])->allocate(static_cast<int>(jvm->renderReference->getWidth()),static_cast<int>(jvm->renderReference->getHeight()),GL_RGB);
            }
            *static_cast<ofTexture *>(_outletParams[0]) = *jvm->renderTexture;

            if(static_cast<ofTexture *>(_outletParams[0])->getWidth()/static_cast<ofTexture *>(_outletParams[0])->getHeight() >= this->width/this->height){
                if(static_cast<ofTexture *>(_outletParams[0])->getWidth() > static_cast<ofTexture *>(_outletParams[0])->getHeight()){   // horizontal texture
                    drawW           = this->width;
                    drawH           = (this->width/static_cast<ofTexture *>(_outletParams[0])->getWidth())*static_cast<ofTexture *>(_outletParams[0])->getHeight();
                    posX            = 0;
                    posY            = (this->height-drawH)/2.0f;
                }else{ // vertical texture
                    drawW           = (static_cast<ofTexture *>(_outletParams[0])->getWidth()*this->height)/static_cast<ofTexture *>(_outletParams[0])->getHeight();
                    drawH           = this->height;
                    posX            = (this->width-drawW)/2.0f;
                    posY            = 0;
                }
            }else{ // always considered vertical texture
                drawW           = (static_cast<ofTexture *>(_outletParams[0])->getWidth()*this->height)/static_cast<ofTexture *>(_outletParams[0])->getHeight();
                drawH           = this->height;
                posX            = (this->width-drawW)/2.0f;
                posY            = 0;
            }
            static_cast<ofTexture *>(_outletParams[0])->draw(posX,posY,drawW,drawH);
        }
    }
    ///////////////////////////////////////////
    // GUI
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void ProcessingScript::removeObjectContent(bool removeFileFromData){
    jvm->closeJVM();

    tempCommand.stop();

    if(removeFileFromData && filepath != ofToDataPath("scripts/Empty.java",true)){
        ofFile tempFile(filepath);
        string vsName = tempFile.getFileName();
        string fsName = tempFile.getEnclosingDirectory()+tempFile.getFileName().substr(0,vsName.find_last_of('.'))+".class";

        removeFile(fsName);

        fsName = tempFile.getEnclosingDirectory()+tempFile.getFileName().substr(0,vsName.find_last_of('.'))+"_in.txt";
        removeFile(fsName);

        fsName = tempFile.getEnclosingDirectory()+tempFile.getFileName().substr(0,vsName.find_last_of('.'))+"_out.txt";
        removeFile(fsName);

        removeFile(filepath);
    }
}

//--------------------------------------------------------------
/*void ProcessingScript::fileDialogResponse(ofxThreadedFileDialogResponse &response){
    if(response.id == "load processing script"+ofToString(this->getId())){
        lastProcessingScript = response.filepath;
        processingScriptLoaded = true;
    }else if(response.id == "save processing script"+ofToString(this->getId())){
        lastProcessingScript = response.filepath;
        processingScriptSaved = true;
    }
}*/

//--------------------------------------------------------------
void ProcessingScript::loadScript(string scriptFile){

    filepath = forceCheckMosaicDataPath(scriptFile);
    currentScriptFile.open(filepath);

    jvm->loadScript(filepath);

    watcher.removeAllPaths();
    watcher.addPath(filepath);
    ofLog(OF_LOG_NOTICE,"[verbose] processing/java script: %s loaded & running!",filepath.c_str());
    this->saveConfig(false);

    if(currentScriptFile.getFileName().size() > 22){
        scriptName->setLabel(currentScriptFile.getFileName().substr(0,21)+"...");
    }else{
        scriptName->setLabel(currentScriptFile.getFileName());
    }

}

//--------------------------------------------------------------
void ProcessingScript::clearScript(){
    jvm->closeJVM();

    ofFile file (ofToDataPath("scripts/Empty.java"));
    filepath = file.getAbsolutePath();
    loadScript(filepath);
}

//--------------------------------------------------------------
void ProcessingScript::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == newButton){
            saveProcessingScriptFlag = true;
        }else if(e.target == loadButton){
            loadProcessingScriptFlag = true;
        }else if(e.target == clearButton){
            clearScript();
        }else if(e.target == reloadButton){
            jvm->reload();
        }
    }
}

//--------------------------------------------------------------
void ProcessingScript::pathChanged(const PathWatcher::Event &event) {
    switch(event.change) {
    case PathWatcher::CREATED:
        //ofLogVerbose(PACKAGE) << "path created " << event.path;
        break;
    case PathWatcher::MODIFIED:
        //ofLogVerbose(PACKAGE) << "path modified " << event.path;
        filepath = event.path;
        jvm->reload();
        break;
    case PathWatcher::DELETED:
        //ofLogVerbose(PACKAGE) << "path deleted " << event.path;
        return;
    default: // NONE
        return;
    }

}


OBJECT_REGISTER( ProcessingScript, "processing script", OFXVP_OBJECT_CAT_SCRIPTING)

#endif
