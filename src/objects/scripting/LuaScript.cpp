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

#include "LuaScript.h"

//--------------------------------------------------------------
LuaScript::LuaScript() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 2;

    _inletParams[0] = new vector<float>();      // data

    _outletParams[0] = new ofTexture();         // output
    _outletParams[1] = new ofxLua();            // lua script reference (for keyboard and mouse events on external windows)

    for(int i=0;i<this->numInlets;i++){
        this->inletsConnected.push_back(false);
    }

    nameLabelLoaded     = false;
    scriptLoaded        = false;
    isNewObject         = false;

    isGUIObject         = true;
    this->isOverGUI     = true;

    fbo = new ofFbo();

    kuro = new ofImage();

    posX = posY = drawW = drawH = 0.0f;

    output_width        = 320;
    output_height       = 240;

    mosaicTableName = "_mosaic_data_table";
    tempstring      = "";

    threadLoaded    = false;
    needToLoadScript= true;
}

//--------------------------------------------------------------
void LuaScript::newObject(){
    this->setName("lua script");
    this->addInlet(VP_LINK_ARRAY,"data");
    this->addOutlet(VP_LINK_TEXTURE);
    this->addOutlet(VP_LINK_SCRIPT);

    this->setCustomVar(static_cast<float>(output_width),"OUTPUT_WIDTH");
    this->setCustomVar(static_cast<float>(output_height),"OUTPUT_HEIGHT");
}

//--------------------------------------------------------------
void LuaScript::threadedFunction(){
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
void LuaScript::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    initResolution();

    // load kuro
    kuro->load("images/kuro.jpg");

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onButtonEvent(this, &LuaScript::onButtonEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    scriptName = gui->addLabel("NONE");
    gui->addBreak();

    loadButton = gui->addButton("OPEN");
    loadButton->setUseCustomMouse(true);

    editButton = gui->addButton("EDIT");
    editButton->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    // Setup ThreadedCommand var
    tempCommand.setup();

    // init lua
    lua.init(true);
    lua.addListener(this);
    watcher.start();

    if(filepath == "none"){
        isNewObject = true;
        ofFile file (ofToDataPath("scripts/empty.lua"));
        filepath = file.getAbsolutePath();
    }
    if(!isThreadRunning()){
        startThread();
    }

}

//--------------------------------------------------------------
void LuaScript::updateObjectContent(map<int,PatchObject*> &patchObjects){

    if(tempCommand.getCmdExec() && tempCommand.getSysStatus() != 0){
        string cmd = "";
        ofSystemAlertDialog("Mosaic works better with Atom [https://atom.io/] text editor, and it seems you do not have it installed on your system. Opening script with default text editor!");
#ifdef TARGET_LINUX
        cmd = "nano "+filepath;
#elif defined(TARGET_OSX)
        cmd = "open -a /Applications/TextEdit.app "+filepath;
#elif defined(TARGET_WIN32)
        cmd = "start "+filepath;
#endif
        tempCommand.execCommand(cmd);
    }

    // GUI
    gui->update();
    header->update();
    loadButton->update();
    editButton->update();

    while(watcher.waitingEvents()) {
        pathChanged(watcher.nextEvent());
    }

    ///////////////////////////////////////////
    // LUA UPDATE
    if(scriptLoaded && threadLoaded){
        if(nameLabelLoaded){
            nameLabelLoaded = false;
            if(currentScriptFile.getFileName().size() > 22){
                scriptName->setLabel(currentScriptFile.getFileName().substr(0,21)+"...");
            }else{
                scriptName->setLabel(currentScriptFile.getFileName());
            }
        }
        // receive external data
        if(this->inletsConnected[0]){
            for(int i=0;i<static_cast<int>(static_cast<vector<float> *>(_inletParams[0])->size());i++){
                lua_getglobal(lua, "_updateMosaicData");
                lua_pushnumber(lua,i+1);
                lua_pushnumber(lua,static_cast<vector<float> *>(_inletParams[0])->at(i));
                lua_pcall(lua,2,0,0);
            }
        }
        // update lua state
        lua.scriptUpdate();
        // send script reference (for events)
        *static_cast<ofxLua *>(_outletParams[1]) = lua;
    }
    ///////////////////////////////////////////

    ///////////////////////////////////////////
    // LUA DRAW
    fbo->begin();
    if(scriptLoaded && threadLoaded){
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
        ofPushView();
        ofPushStyle();
        ofPushMatrix();
        lua.scriptDraw();
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
void LuaScript::drawObjectContent(ofxFontStash *font){
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
void LuaScript::removeObjectContent(){
    tempCommand.stop();
    ///////////////////////////////////////////
    // LUA EXIT
    lua.scriptExit();
    lua.clear();
    ///////////////////////////////////////////
}

//--------------------------------------------------------------
void LuaScript::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    loadButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    editButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || loadButton->hitTest(_m-this->getPos()) || editButton->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void LuaScript::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void LuaScript::initResolution(){
    output_width = static_cast<int>(floor(this->getCustomVar("OUTPUT_WIDTH")));
    output_height = static_cast<int>(floor(this->getCustomVar("OUTPUT_HEIGHT")));

    fbo = new ofFbo();
    fbo->allocate(output_width,output_height,GL_RGBA32F_ARB);
    fbo->begin();
    ofClear(255,255,255, 0);
    fbo->end();

}

//--------------------------------------------------------------
void LuaScript::resetResolution(int fromID, int newWidth, int newHeight){
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
        fbo->allocate(output_width,output_height,GL_RGBA32F_ARB);
        fbo->begin();
        ofClear(255,255,255, 0);
        fbo->end();

        tempstring = "OUTPUT_WIDTH = "+ofToString(output_width);
        lua.doString(tempstring);
        tempstring = "OUTPUT_HEIGHT = "+ofToString(output_height);
        lua.doString(tempstring);
    }

}

//--------------------------------------------------------------
void LuaScript::loadScript(string scriptFile){

    filepath = scriptFile;
    currentScriptFile.open(filepath);

    lua.scriptExit();
    lua.init(true);
    lua.doScript(filepath, true);

    // inject incoming data vector to lua
    string tempstring = mosaicTableName+" = {}";
    lua.doString(tempstring);
    tempstring = "function _updateMosaicData(i,data) "+mosaicTableName+"[i] = data  end";
    lua.doString(tempstring);

    // set Mosaic scripting vars
    tempstring = "OUTPUT_WIDTH = "+ofToString(output_width);
    lua.doString(tempstring);
    tempstring = "OUTPUT_HEIGHT = "+ofToString(output_height);
    lua.doString(tempstring);

    scriptLoaded = lua.isValid();

    ///////////////////////////////////////////
    // LUA SETUP
    if(scriptLoaded){
        ofLog(OF_LOG_NOTICE,"[verbose] lua script: %s loaded & running!",filepath.c_str());
        watcher.removeAllPaths();
        watcher.addPath(filepath);
        lua.scriptSetup();
    }
    ///////////////////////////////////////////

}

//--------------------------------------------------------------
void LuaScript::reloadScriptThreaded(){
    scriptLoaded = false;
    needToLoadScript = true;
}

//--------------------------------------------------------------
void LuaScript::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == loadButton){
            ofFileDialogResult openFileResult= ofSystemLoadDialog("Select a lua script");
            if (openFileResult.bSuccess){
                ofFile file (openFileResult.getPath());
                if (file.exists()){
                    string fileExtension = ofToUpper(file.getExtension());
                    if(fileExtension == "LUA") {
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
void LuaScript::pathChanged(const PathWatcher::Event &event) {
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

//--------------------------------------------------------------
void LuaScript::errorReceived(std::string& msg) {
    ofLog(OF_LOG_ERROR,"LUA script error: %s",msg.c_str());
}
