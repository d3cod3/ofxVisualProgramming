#include "PythonScript.h"

//--------------------------------------------------------------
PythonScript::PythonScript() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 2;

    _inletParams[0] = new vector<float>();      // data

    _outletParams[0] = new ofTexture();         // output
    _outletParams[1] = new ofxPythonObject();   // python script reference (for keyboard and mouse events on external windows)

    for(int i=0;i<this->numInlets;i++){
        this->inletsConnected.push_back(false);
    }

    isNewObject         = false;

    isGUIObject         = true;
    isOverGui           = true;

    fbo = new ofFbo();

    kuro = new ofImage();

    scaleH = 0.0f;

    output_width    = 320;
    output_height   = 240;

    mosaicTableName = "_mosaic_data_list";
    tempstring      = "";

    threadLoaded    = false;
    needToLoadScript= true;
}

//--------------------------------------------------------------
void PythonScript::newObject(){
    this->setName("python script");
    this->addInlet(VP_LINK_ARRAY,"data");
    this->addOutlet(VP_LINK_TEXTURE);
    this->addOutlet(VP_LINK_SCRIPT);
}

//--------------------------------------------------------------
void PythonScript::threadedFunction(){
    while(isThreadRunning()){
        std::unique_lock<std::mutex> lock(mutex);
        if(needToLoadScript){
            needToLoadScript = false;
            loadScript(filepath);
            threadLoaded = true;
        }
        condition.wait(lock);
    }

}

//--------------------------------------------------------------
void PythonScript::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadProjectorSettings();

    // load kuro
    kuro->load("images/kuro.jpg");

    // init output texture container
    fbo->allocate(output_width,output_height,GL_RGBA32F_ARB);
    fbo->begin();
    ofClear(255,255,255, 0);
    fbo->end();

    // init python
    python.init();
    watcher.start();

    if(filepath != "none"){
        if(!isThreadRunning()){
            startThread();
        }
    }else{
        isNewObject = true;
    }


    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth((this->width/3 * 2) + 1);
    gui->onButtonEvent(this, &PythonScript::onButtonEvent);

    loadButton = gui->addButton("OPEN");
    loadButton->setUseCustomMouse(true);

    editButton = gui->addButton("EDIT");
    editButton->setUseCustomMouse(true);

    gui->setPosition(this->width/3,this->height - (loadButton->getHeight()*2));
}

//--------------------------------------------------------------
void PythonScript::updateObjectContent(map<int,PatchObject*> &patchObjects){
    // GUI
    gui->update();
    loadButton->update();
    editButton->update();

    while(watcher.waitingEvents()) {
        pathChanged(watcher.nextEvent());
    }

    ///////////////////////////////////////////
    // PYTHON UPDATE
    if(script && threadLoaded){
        updatePython = script.attr("update");
        if(updatePython){
            updateMosaicList = python.getObject("_updateMosaicData");
            // receive external data (this is slow, fix it using python obj direct access)
            if(this->inletsConnected[0] && updateMosaicList){
                for(int i=0;i<static_cast<vector<float> *>(_inletParams[0])->size();i++){
                    updateMosaicList(ofxPythonObject::fromInt(static_cast<int>(i)),ofxPythonObject::fromFloat(static_cast<double>(static_cast<vector<float> *>(_inletParams[0])->at(i))));
                }
            }
            updatePython();
            // send script reference (for events)
            *static_cast<ofxPythonObject *>(_outletParams[1]) = script;
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
        drawPython = script.attr("draw");
        if (drawPython) drawPython();
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
    scaleH = (this->width/fbo->getWidth())*fbo->getHeight();
    static_cast<ofTexture *>(_outletParams[0])->draw(0,this->height/2 - scaleH/2,this->width,scaleH);
    // GUI
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void PythonScript::removeObjectContent(){
    ///////////////////////////////////////////
    // PYTHON EXIT
    python.reset();
    script = ofxPythonObject::_None();
    ///////////////////////////////////////////

}

//--------------------------------------------------------------
void PythonScript::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    loadButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    editButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    isOverGui = loadButton->hitTest(_m-this->getPos());
}

//--------------------------------------------------------------
void PythonScript::dragGUIObject(ofVec3f _m){
    if(!isOverGui){
        ofNotifyEvent(dragEvent, nId);

        box->setFromCenter(_m.x, _m.y,box->getWidth(),box->getHeight());
        headerBox->set(box->getPosition().x,box->getPosition().y,box->getWidth(),headerHeight);

        x = box->getPosition().x;
        y = box->getPosition().y;

        for(int j=0;j<outPut.size();j++){
            outPut[j]->linkVertices[0].move(outPut[j]->posFrom.x,outPut[j]->posFrom.y);
            outPut[j]->linkVertices[1].move(outPut[j]->posFrom.x+20,outPut[j]->posFrom.y);
        }
    }
}

//--------------------------------------------------------------
bool PythonScript::loadProjectorSettings(){
    ofxXmlSettings XML;
    bool loaded = false;

    if (XML.loadFile(patchFile)){
        if (XML.pushTag("settings")){
            output_width = XML.getValue("output_width",0);
            output_height = XML.getValue("output_height",0);
            XML.popTag();
        }

        loaded = true;
    }

    return loaded;
}

//--------------------------------------------------------------
void PythonScript::loadScript(string scriptFile){

    filepath = scriptFile;

    ofFile tempfile (filepath);

    python.reset();
    python.addPath(tempfile.getEnclosingDirectory());
    python.executeScript(filepath);

    // inject incoming data vector to python as list
    string tempstring = mosaicTableName+" = [];\n"+mosaicTableName+".append(0)";
    python.executeString(tempstring);
    // tabs and newlines are really important in python!
    tempstring = "def _updateMosaicData( i,data ):\n\t if len("+mosaicTableName+") < i:\n\t\t"+mosaicTableName+".append(0)\n\t elif 0 <= i < len("+mosaicTableName+"):\n\t\t"+mosaicTableName+"[i] = data\n";
    python.executeString(tempstring);

    // set Mosaic scripting vars
    tempstring = "OUTPUT_WIDTH = "+ofToString(output_width)+"\nOUTPUT_HEIGHT = "+ofToString(output_height)+"\n";
    python.executeString(tempstring);

    ///////////////////////////////////////////
    // PYTHON SETUP
    klass = python.getObject("mosaicApp");
    if(klass){
        script = klass();
        ofLog(OF_LOG_NOTICE,"python script: %s loaded & running!",filepath.c_str());
        watcher.removeAllPaths();
        watcher.addPath(filepath);
    }else{
        script = ofxPythonObject::_None();
    }
    ///////////////////////////////////////////

}

//--------------------------------------------------------------
void PythonScript::reloadScriptThreaded(){
    script = ofxPythonObject::_None();
    needToLoadScript = true;
}

//--------------------------------------------------------------
void PythonScript::onButtonEvent(ofxDatGuiButtonEvent e){
    if(e.target == loadButton){
        ofFileDialogResult openFileResult= ofSystemLoadDialog("Select a python script");
        if (openFileResult.bSuccess){
            ofFile file (openFileResult.getPath());
            if (file.exists()){
                string fileExtension = ofToUpper(file.getExtension());
                if(fileExtension == "PY") {
                    threadLoaded = false;
                    filepath = file.getAbsolutePath();
                    if(!isThreadRunning()){
                        startThread();
                    }else{
                        reloadScriptThreaded();
                    }
                }
            }
        }
    }else if(e.target == editButton){
        if(filepath != "none" && script){
            string cmd = "";
#ifdef TARGET_LINUX
            cmd = "atom "+filepath;
#elif defined(TARGET_OSX)
            cmd = "open -a /Applications/Atom.app "+filepath;
#elif defined(TARGET_WIN32)
            cmd = "atom "+filepath;
#endif
            if(system(cmd.c_str()) != 0){ // error
                ofSystemAlertDialog("Mosaic works better with Atom [https://atom.io/] text editor, and it seems you do not have it installed on your system. Opening script with default text editor!");
#ifdef TARGET_LINUX
                cmd = "nano "+filepath;
#elif defined(TARGET_OSX)
                cmd = "open -a /Applications/TextEdit.app "+filepath;
#elif defined(TARGET_WIN32)
                cmd = "start "+filepath;
#endif
                system(cmd.c_str());
            }
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

