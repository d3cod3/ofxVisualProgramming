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

#include "PythonScript.h"

//--------------------------------------------------------------
PythonScript::PythonScript() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 2;

    _inletParams[0] = new vector<float>();      // data

    _outletParams[0] = new ofTexture();         // output
    _outletParams[1] = new LiveCoding();        // python script reference (for keyboard and mouse events on external windows)

    this->specialLinkTypeName = "LiveCoding";

    this->initInletsState();

    nameLabelLoaded     = false;
    isNewObject         = false;

    isGUIObject         = true;
    this->isOverGUI     = true;

    fbo = new ofFbo();

    kuro = new ofImage();

    posX = posY = drawW = drawH = 0.0f;

    output_width    = 800;
    output_height   = 600;

    mosaicTableName = "_mosaic_data_list";
    tempstring      = "";

    threadLoaded    = false;
    needToLoadScript= true;

    lastPythonScript       = "";
    loadPythonScriptFlag   = false;
    savePythonScriptFlag   = false;
    pythonScriptLoaded     = false;
    pythonScriptSaved      = false;

    static_cast<LiveCoding *>(_outletParams[1])->hide = true;
}

//--------------------------------------------------------------
void PythonScript::newObject(){
    this->setName("python script");
    this->addInlet(VP_LINK_ARRAY,"data");
    this->addOutlet(VP_LINK_TEXTURE);
    this->addOutlet(VP_LINK_SPECIAL);

    this->setCustomVar(static_cast<float>(output_width),"OUTPUT_WIDTH");
    this->setCustomVar(static_cast<float>(output_height),"OUTPUT_HEIGHT");
}

//--------------------------------------------------------------
void PythonScript::threadedFunction(){
    while(isThreadRunning()){
        std::unique_lock<std::mutex> lock(mutex);
        if(needToLoadScript){
            needToLoadScript = false;
            loadScript(filepath);
            threadLoaded = true;
            nameLabelLoaded = true;
        }
        condition.wait(lock);
        sleep(10);
    }

}

//--------------------------------------------------------------
void PythonScript::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    initResolution();

    // load kuro
    kuro->load("images/kuro.jpg");

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onButtonEvent(this, &PythonScript::onButtonEvent);

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

    // init live coding editor
    ofxEditor::loadFont(ofToDataPath(LIVECODING_FONT), 32);
    liveEditorSyntax.loadFile(ofToDataPath(PYTHON_SYNTAX));
    static_cast<LiveCoding *>(_outletParams[1])->liveEditor.getSettings().addSyntax(&liveEditorSyntax);
    liveEditorColors.loadFile(ofToDataPath(LIVECODING_COLORS));
    static_cast<LiveCoding *>(_outletParams[1])->liveEditor.setColorScheme(&liveEditorColors);
    static_cast<LiveCoding *>(_outletParams[1])->liveEditor.setLineNumbers(true);
    static_cast<LiveCoding *>(_outletParams[1])->liveEditor.setAutoFocus(true);
    static_cast<LiveCoding *>(_outletParams[1])->liveEditor.resize(output_width,output_height);

    // init python
    python.init();
    watcher.start();

    if(filepath == "none"){
        isNewObject = true;
        ofFile file (ofToDataPath("scripts/empty.py"));
        filepath = file.getAbsolutePath();
    }
    if(!isThreadRunning()){
        startThread();
    }

}

//--------------------------------------------------------------
void PythonScript::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){

    if(tempCommand.getCmdExec() && tempCommand.getSysStatus() != 0){
        fd.notificationPopup("Mosaic files editing","Mosaic works better with Atom [https://atom.io/] text editor, and it seems you do not have it installed on your system.");
    }

    // GUI
    gui->update();
    header->update();
    newButton->update();
    loadButton->update();
    editButton->update();
    clearButton->update();
    reloadButton->update();

    if(loadPythonScriptFlag){
        loadPythonScriptFlag = false;
        fd.openFile("load python script","Select a python script");
    }

    if(savePythonScriptFlag){
        savePythonScriptFlag = false;
        string newFileName = "pythonScript_"+ofGetTimestampString("%y%m%d")+".py";
        fd.saveFile("save python script","Save new Python script as",newFileName);
    }

    if(pythonScriptLoaded){
        pythonScriptLoaded = false;
        ofFile file (lastPythonScript);
        if (file.exists()){
            string fileExtension = ofToUpper(file.getExtension());
            if(fileExtension == "PY") {
                threadLoaded = false;
                filepath = file.getAbsolutePath();
                static_cast<LiveCoding *>(_outletParams[1])->liveEditor.openFile(filepath);
                static_cast<LiveCoding *>(_outletParams[1])->liveEditor.reset();
                reloadScriptThreaded();
            }
        }
    }

    if(pythonScriptSaved){
        pythonScriptSaved = false;
        ofFile fileToRead(ofToDataPath("scripts/empty.py"));
        ofFile newPyFile (lastPythonScript);
        ofFile::copyFromTo(fileToRead.getAbsolutePath(),newPyFile.getAbsolutePath(),true,true);
        threadLoaded = false;
        filepath = newPyFile.getAbsolutePath();
        static_cast<LiveCoding *>(_outletParams[1])->liveEditor.openFile(filepath);
        static_cast<LiveCoding *>(_outletParams[1])->liveEditor.reset();
        reloadScriptThreaded();
    }

    while(watcher.waitingEvents()) {
        pathChanged(watcher.nextEvent());
    }

    ///////////////////////////////////////////
    // PYTHON UPDATE
    if(script && threadLoaded){
        if(nameLabelLoaded){
            nameLabelLoaded = false;
            if(currentScriptFile.getFileName().size() > 22){
                scriptName->setLabel(currentScriptFile.getFileName().substr(0,21)+"...");
            }else{
                scriptName->setLabel(currentScriptFile.getFileName());
            }
        }
        updatePython = script.attr("update");
        if(updatePython && !script.isPythonError() && !updatePython.isPythonError()){
            updateMosaicList = python.getObject("_updateMosaicData");
            if(this->inletsConnected[0] && updateMosaicList && !updateMosaicList.isPythonError()){
                for(int i=0;i<static_cast<int>(static_cast<vector<float> *>(_inletParams[0])->size());i++){
                    updateMosaicList(ofxPythonObject::fromInt(static_cast<int>(i)),ofxPythonObject::fromFloat(static_cast<double>(static_cast<vector<float> *>(_inletParams[0])->at(i))));
                }
            }else{
                for(int i=0;i<1000;i++){
                    updateMosaicList(ofxPythonObject::fromInt(static_cast<int>(i)),ofxPythonObject::fromFloat(static_cast<double>(0.0)));
                }
            }
            updatePython();
            // send script reference (for events)
            static_cast<LiveCoding *>(_outletParams[1])->python = script;
        }
    }
    ///////////////////////////////////////////

    ///////////////////////////////////////////
    // PYTHON DRAW
    fbo->begin();
    if(script && threadLoaded){
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
        ofPushView();
        ofPushStyle();
        ofPushMatrix();
        if(!static_cast<LiveCoding *>(_outletParams[1])->hide) {
            ofBackground(0);
        }
        drawPython = script.attr("draw");
        if (drawPython && !script.isPythonError() && !drawPython.isPythonError()) drawPython();
        if(!static_cast<LiveCoding *>(_outletParams[1])->hide) {
            static_cast<LiveCoding *>(_outletParams[1])->liveEditor.draw();
        }
        ofPopMatrix();
        ofPopStyle();
        ofPopView();
        glPopAttrib();
    }else{
        kuro->draw(0,0,fbo->getWidth(),fbo->getHeight());
    }
    fbo->end();
    *static_cast<ofTexture *>(_outletParams[0]) = fbo->getTexture();
    ///////////////////////////////////////////
    condition.notify_one();

}

//--------------------------------------------------------------
void PythonScript::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(static_cast<ofTexture *>(_outletParams[0])->getWidth() >= static_cast<ofTexture *>(_outletParams[0])->getHeight()){   // horizontal texture
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
    static_cast<ofTexture *>(_outletParams[0])->draw(posX,posY,drawW,drawH);
    // GUI
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void PythonScript::removeObjectContent(){
    tempCommand.stop();
    script = ofxPythonObject::_None();
    if(isThreadRunning()){
        stopThread();
    }
}

//--------------------------------------------------------------
void PythonScript::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    newButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    loadButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    editButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    clearButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    reloadButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || newButton->hitTest(_m-this->getPos()) || loadButton->hitTest(_m-this->getPos()) || editButton->hitTest(_m-this->getPos()) || clearButton->hitTest(_m-this->getPos()) || reloadButton->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void PythonScript::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        newButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        loadButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        editButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        clearButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        reloadButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void PythonScript::initResolution(){
    output_width = static_cast<int>(floor(this->getCustomVar("OUTPUT_WIDTH")));
    output_height = static_cast<int>(floor(this->getCustomVar("OUTPUT_HEIGHT")));

    fbo = new ofFbo();
    fbo->allocate(output_width,output_height,GL_RGBA32F_ARB,4);
    fbo->begin();
    ofClear(0,0,0,255);
    fbo->end();

    static_cast<LiveCoding *>(_outletParams[1])->liveEditor.resize(output_width,output_height);

}

//--------------------------------------------------------------
void PythonScript::resetResolution(int fromID, int newWidth, int newHeight){
    bool reset = false;

    // Check if we are connected to signaling object
    for(int j=0;j<static_cast<int>(outPut.size());j++){
        if(outPut[j]->toObjectID == fromID){
            reset = true;
        }
    }


    if(reset && fromID != -1 && newWidth != -1 && newHeight != -1 && (output_width!=newWidth || output_height!=newHeight)){
        output_width    = newWidth;
        output_height   = newHeight;

        this->setCustomVar(static_cast<float>(output_width),"OUTPUT_WIDTH");
        this->setCustomVar(static_cast<float>(output_height),"OUTPUT_HEIGHT");
        this->saveConfig(false,this->nId);

        fbo = new ofFbo();
        fbo->allocate(output_width,output_height,GL_RGBA32F_ARB,4);
        fbo->begin();
        ofClear(0,0,0,255);
        fbo->end();

        static_cast<LiveCoding *>(_outletParams[1])->liveEditor.resize(output_width,output_height);

        ofFile tempFileScript(filepath);
        tempstring = "OUTPUT_WIDTH = "+ofToString(output_width)+"\nOUTPUT_HEIGHT = "+ofToString(output_height)+"\nSCRIPT_PATH = '"+tempFileScript.getEnclosingDirectory().substr(0,tempFileScript.getEnclosingDirectory().size()-1)+"'\n";
        python.executeString(tempstring);
    }

}

//--------------------------------------------------------------
void PythonScript::fileDialogResponse(ofxThreadedFileDialogResponse &response){
    if(response.id == "load python script"){
        lastPythonScript = response.filepath;
        pythonScriptLoaded = true;
    }else if(response.id == "save python script"){
        lastPythonScript = response.filepath;
        pythonScriptSaved = true;
    }
}

//--------------------------------------------------------------
void PythonScript::loadScript(string scriptFile){

    filepath = forceCheckMosaicDataPath(scriptFile);
    currentScriptFile.open(filepath);

    static_cast<LiveCoding *>(_outletParams[1])->filepath = filepath;

    python.reset();
    python.addPath(currentScriptFile.getEnclosingDirectory());
    python.executeScript(filepath);

    // inject incoming data vector to python as list
    string tempstring = mosaicTableName+" = [];\n"+mosaicTableName+".append(0)";
    python.executeString(tempstring);
    // tabs and newlines are really important in python!
    tempstring = "def _updateMosaicData( i,data ):\n\t if len("+mosaicTableName+") < i:\n\t\t"+mosaicTableName+".append(0)\n\t elif 0 <= i < len("+mosaicTableName+"):\n\t\t"+mosaicTableName+"[i] = data\n";
    python.executeString(tempstring);

    // set Mosaic scripting vars
    ofFile tempFileScript(filepath);

    string temppath = tempFileScript.getEnclosingDirectory().substr(0,tempFileScript.getEnclosingDirectory().size()-1);

    #ifdef TARGET_WIN32
        std::replace(temppath.begin(),temppath.end(),'\\','/');
    #endif

    tempstring = "OUTPUT_WIDTH = "+ofToString(output_width)+"\nOUTPUT_HEIGHT = "+ofToString(output_height)+"\nSCRIPT_PATH = '"+temppath+"'\n";
    python.executeString(tempstring);

    ///////////////////////////////////////////
    // PYTHON SETUP
    klass = python.getObject("mosaicApp");
    if(klass){
        script = klass();
        ofLog(OF_LOG_NOTICE,"[verbose] python script: %s loaded & running!",filepath.c_str());
        watcher.removeAllPaths();
        watcher.addPath(filepath);
    }else{
        script = ofxPythonObject::_None();
    }
    if(static_cast<LiveCoding *>(_outletParams[1])->hide){
        static_cast<LiveCoding *>(_outletParams[1])->liveEditor.openFile(filepath);
        static_cast<LiveCoding *>(_outletParams[1])->liveEditor.reset();
    }
    ///////////////////////////////////////////

}

//--------------------------------------------------------------
void PythonScript::clearScript(){
    python.reset();
    script = ofxPythonObject::_None();
}

//--------------------------------------------------------------
void PythonScript::reloadScriptThreaded(){
    script = ofxPythonObject::_None();
    needToLoadScript = true;
}

//--------------------------------------------------------------
void PythonScript::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == newButton){
            savePythonScriptFlag = true;
        }else if(e.target == loadButton){
            loadPythonScriptFlag = true;
        }else if(e.target == editButton){
            bool nameError = checkFilenameError(filepath);
            if(filepath != "none" && !nameError && script){
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
            if(nameError){
                ofLog(OF_LOG_ERROR,"PYTHON SCRIPT NAME ERROR: your script filename: %s contains irregular characters,",filepath.c_str());
                ofLog(OF_LOG_ERROR,"please use alphanumerical and underscore (_) characters only!");
            }
        }else if(e.target == clearButton){
            clearScript();
        }else if(e.target == reloadButton){
            reloadScriptThreaded();
        }
    }
}

//--------------------------------------------------------------
void PythonScript::pathChanged(const PathWatcher::Event &event) {
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

