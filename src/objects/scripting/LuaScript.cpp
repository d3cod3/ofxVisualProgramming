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

#include "LuaScript.h"

//--------------------------------------------------------------
LuaScript::LuaScript() : PatchObject("lua script"){

    this->numInlets  = 2;
    this->numOutlets = 3;

    _inletParams[0] = new vector<float>();      // data

    _inletParams[1] = new string();             // string
    *static_cast<string *>(_inletParams[1]) = "";

    _outletParams[0] = new ofTexture();         // output
    _outletParams[1] = new LiveCoding();        // lua script reference (for keyboard and mouse events on external windows)
    _outletParams[2] = new vector<float>();     // outlet vector from lua

    this->specialLinkTypeName = "LiveCoding";

    this->initInletsState();

    scriptLoaded        = false;
    isNewObject         = false;

    fbo = new ofFbo();

    kuro = new ofImage();

    posX = posY = drawW = drawH = 0.0f;

    output_width        = STANDARD_TEXTURE_WIDTH;
    output_height       = STANDARD_TEXTURE_HEIGHT;

    mosaicTableName     = "_mosaic_data_inlet";
    luaTablename        = "_mosaic_data_outlet";
    mosaicStringName    = "_mosaic_string_inlet";
    tempstring      = "";

    needToLoadScript= true;

    isError         = false;
    setupTrigger    = false;

    lastLuaScript       = "";
    loadLuaScriptFlag   = false;
    saveLuaScriptFlag   = false;
    luaScriptLoaded     = false;
    luaScriptSaved      = false;
    loaded              = false;
    loadTime            = ofGetElapsedTimeMillis();

    static_cast<LiveCoding *>(_outletParams[1])->hide = true;

    this->setIsResizable(true);
    this->setIsTextureObj(true);

    prevW                   = this->width;
    prevH                   = this->height;
}

//--------------------------------------------------------------
void LuaScript::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"_mosaic_data_inlet");
    this->addInlet(VP_LINK_STRING,"_mosaic_string_inlet");

    this->addOutlet(VP_LINK_TEXTURE,"generatedTexture");
    this->addOutlet(VP_LINK_SPECIAL,"mouseKeyboardInteractivity");
    this->addOutlet(VP_LINK_ARRAY,"_mosaic_data_outlet");

    this->setCustomVar(static_cast<float>(output_width),"OUTPUT_WIDTH");
    this->setCustomVar(static_cast<float>(output_height),"OUTPUT_HEIGHT");

    this->setCustomVar(static_cast<float>(prevW),"WIDTH");
    this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
}

//--------------------------------------------------------------
void LuaScript::autoloadFile(string _fp){
    openScript(_fp);
}

//--------------------------------------------------------------
void LuaScript::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    fileDialog.setIsRetina(this->isRetina);

    initResolution();

    // load kuro
    kuro->load("images/kuro.jpg");

    // init lua
    static_cast<LiveCoding *>(_outletParams[1])->lua.init(true);
    static_cast<LiveCoding *>(_outletParams[1])->lua.addListener(this);
    watcher.start();

}

//--------------------------------------------------------------
void LuaScript::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(needToLoadScript  && filepath != "none"){
        needToLoadScript = false;
        loadScript(filepath);
        setupTrigger = false;
    }

    // path watcher
    while(watcher.waitingEvents()) {
        pathChanged(watcher.nextEvent());
    }

    ///////////////////////////////////////////
    // LUA SETUP
    if(scriptLoaded && !isError){
        if(!setupTrigger){
            setupTrigger = true;
            static_cast<LiveCoding *>(_outletParams[1])->lua.scriptSetup();
        }
    }
    ///////////////////////////////////////////

    if(luaScriptLoaded){
        luaScriptLoaded = false;
        openScript(lastLuaScript);
    }

    if(luaScriptSaved){
        luaScriptSaved = false;
        newScript(lastLuaScript);
    }

    if(!loaded && ofGetElapsedTimeMillis()-loadTime > 1000){
        loaded = true;
        prevW = this->getCustomVar("WIDTH");
        prevH = this->getCustomVar("HEIGHT");
        this->width             = prevW;
        this->height            = prevH;
        reloadScript();
    }
}

//--------------------------------------------------------------
void LuaScript::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){

    ///////////////////////////////////////////
    // LUA UPDATE
    if(scriptLoaded && !isError){

        // receive external data
        if(this->inletsConnected[0]){
            for(int i=0;i<static_cast<int>(static_cast<vector<float> *>(_inletParams[0])->size());i++){
                lua_getglobal(static_cast<LiveCoding *>(_outletParams[1])->lua, "_updateMosaicData");
                lua_pushnumber(static_cast<LiveCoding *>(_outletParams[1])->lua,i+1);
                lua_pushnumber(static_cast<LiveCoding *>(_outletParams[1])->lua,static_cast<vector<float> *>(_inletParams[0])->at(i));
                lua_pcall(static_cast<LiveCoding *>(_outletParams[1])->lua,2,0,0);
            }
            static_cast<LiveCoding *>(_outletParams[1])->lua.doString("USING_DATA_INLET = true");
        }else{
            static_cast<LiveCoding *>(_outletParams[1])->lua.doString("USING_DATA_INLET = false");
        }


        if(this->inletsConnected[1]){
            string temp = mosaicStringName+" = '";
            temp.append(*static_cast<string *>(_inletParams[1]));
            temp.append("'");
            static_cast<LiveCoding *>(_outletParams[1])->lua.doString(temp);
        }else{
            string temp = mosaicStringName+" = ''";
            static_cast<LiveCoding *>(_outletParams[1])->lua.doString(temp);
        }

        // send internal data
        size_t len = static_cast<LiveCoding *>(_outletParams[1])->lua.tableSize(luaTablename);
        if(len > 0){
            static_cast<vector<float> *>(_outletParams[2])->clear();
            for(size_t s=0;s<len;s++){
                lua_getglobal(static_cast<LiveCoding *>(_outletParams[1])->lua, "_getLUAOutletTableAt");
                lua_pushnumber(static_cast<LiveCoding *>(_outletParams[1])->lua,s+1);
                lua_pcall(static_cast<LiveCoding *>(_outletParams[1])->lua,1,1,0);
                lua_Number tn = lua_tonumber(static_cast<LiveCoding *>(_outletParams[1])->lua, -2);
                static_cast<vector<float> *>(_outletParams[2])->push_back(static_cast<float>(tn));
            }
            std::rotate(static_cast<vector<float> *>(_outletParams[2])->begin(),static_cast<vector<float> *>(_outletParams[2])->begin()+1,static_cast<vector<float> *>(_outletParams[2])->end());
        }


        // update lua state
        ofSoundUpdate();
        static_cast<LiveCoding *>(_outletParams[1])->lua.scriptUpdate();
    }
    ///////////////////////////////////////////

    ///////////////////////////////////////////
    // LUA DRAW
    fbo->begin();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    ofPushView();
    ofPushStyle();
    ofPushMatrix();
    if(!static_cast<LiveCoding *>(_outletParams[1])->hide){
        ofBackground(0);
    }
    if(scriptLoaded && !isError){
        static_cast<LiveCoding *>(_outletParams[1])->lua.scriptDraw();
    }else{
        kuro->draw(0,0,fbo->getWidth(),fbo->getHeight());
    }
    ofPopMatrix();
    ofPopStyle();
    ofPopView();
    glPopAttrib();
    fbo->end();

    *static_cast<ofTexture *>(_outletParams[0]) = fbo->getTexture();
    ///////////////////////////////////////////

    ofSetColor(255);
    if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
        // draw node texture preview with OF
        if(scaledObjW*canvasZoom > 90.0f){
            drawNodeOFTexture(*static_cast<ofTexture *>(_outletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
        }
    }else{
        // background
        if(scaledObjW*canvasZoom > 90.0f){
            ofSetColor(34,34,34);
            ofDrawRectangle(objOriginX - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/canvasZoom), objOriginY-(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor/canvasZoom),scaledObjW + (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/canvasZoom),scaledObjH + (((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor)/canvasZoom) );
        }
    }
}

//--------------------------------------------------------------
void LuaScript::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    loadLuaScriptFlag = false;
    saveLuaScriptFlag = false;

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){

        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            drawObjectNodeConfig(); this->configMenuWidth = ImGui::GetWindowWidth();

            ImGui::EndMenu();
        }

        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        // get imgui node translated/scaled position/dimension for drawing textures in OF
        objOriginX = (ImGui::GetWindowPos().x + ((IMGUI_EX_NODE_PINS_WIDTH_NORMAL - 1)*this->scaleFactor) - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale();
        objOriginY = (ImGui::GetWindowPos().y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale();
        scaledObjW = this->width - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/_nodeCanvas.GetCanvasScale());
        scaledObjH = this->height - ((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor/_nodeCanvas.GetCanvasScale());

        if(this->width != prevW){
            prevW = this->width;
            this->setCustomVar(static_cast<float>(prevW),"WIDTH");
        }
        if(this->height != prevH){
            prevH = this->height;
            this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
        }

        _nodeCanvas.EndNodeContent();
    }

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();

    // file dialog
    /*string newFileName = "luaScript_"+ofGetTimestampString("%y%m%d")+".lua";
    if(ImGuiEx::getFileDialog(fileDialog, saveLuaScriptFlag, "Save new Lua script as", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ".lua", newFileName, scaleFactor)){
        lastLuaScript = fileDialog.selected_path;
        luaScriptSaved = true;
    }*/

    if(ImGuiEx::getFileDialog(fileDialog, loadLuaScriptFlag, "Select a lua script", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".lua", "", scaleFactor)){
        lastLuaScript = fileDialog.selected_path;
        luaScriptLoaded = true;
    }

}

//--------------------------------------------------------------
void LuaScript::drawObjectNodeConfig(){
    ofFile tempFilename(filepath);

    loadLuaScriptFlag = false;
    saveLuaScriptFlag = false;

    ImGui::Spacing();
    ImGui::Text("Loaded File:");
    if(filepath == "none"){
        ImGui::Text("%s",filepath.c_str());
    }else{
        ImGui::Text("%s",tempFilename.getFileName().c_str());
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",tempFilename.getAbsolutePath().c_str());
    }
    ImGui::Spacing();
    ImGui::Text("Rendering at: %.0fx%.0f",static_cast<ofTexture *>(_outletParams[0])->getWidth(),static_cast<ofTexture *>(_outletParams[0])->getHeight());
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    if(ImGui::Button("New",ImVec2(224*scaleFactor,26*scaleFactor))){
        saveLuaScriptFlag = true;
    }

    if(saveLuaScriptFlag){
        newScriptName = "luaScript_"+ofGetTimestampString("%y%m%d")+".lua";
        ImGui::OpenPopup("Save new lua script as");
    }

    if(ImGui::BeginPopup("Save new lua script as")){

        if(ImGui::InputText("##NewFileNameInput", &newScriptName,ImGuiInputTextFlags_EnterReturnsTrue)){
            if(newScriptName != ""){
                // save file in data/ folder
                lastLuaScript = this->patchFolderPath+newScriptName;
                luaScriptSaved = true;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if(ImGui::Button("Cancel")){
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if(ImGui::Button("Create")){
            if(newScriptName != ""){
                // save file in data/ folder
                lastLuaScript = this->patchFolderPath+newScriptName;
                luaScriptSaved = true;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();

    }

    ImGui::Spacing();
    if(ImGui::Button("Open",ImVec2(224*scaleFactor,26*scaleFactor))){
        loadLuaScriptFlag = true;
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Separator();
    ImGui::Spacing();
    if(ImGui::Button("Clear Script",ImVec2(224*scaleFactor,26*scaleFactor))){
        clearScript();
    }
    ImGui::Spacing();
    if(ImGui::Button("Reload Script",ImVec2(224*scaleFactor,26*scaleFactor))){
        reloadScript();
    }

    ImGuiEx::ObjectInfo(
                "This object is a live-coding lua script container, with OF bindings mimicking the OF programming structure. You can type code with the Mosaic code editor, or with your default code editor. Scripts will refresh automatically on save.",
                "https://mosaic.d3cod3.org/reference.php?r=lua-script", scaleFactor);

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, loadLuaScriptFlag, "Select a lua script", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".lua", "", scaleFactor)){
        lastLuaScript = fileDialog.selected_path;
        luaScriptLoaded = true;
    }
}

//--------------------------------------------------------------
void LuaScript::removeObjectContent(bool removeFileFromData){
    ///////////////////////////////////////////
    // LUA EXIT
    static_cast<LiveCoding *>(_outletParams[1])->lua.scriptExit();
    ///////////////////////////////////////////
}

//--------------------------------------------------------------
void LuaScript::initResolution(){
    output_width = static_cast<int>(floor(this->getCustomVar("OUTPUT_WIDTH")));
    output_height = static_cast<int>(floor(this->getCustomVar("OUTPUT_HEIGHT")));

    fbo = new ofFbo();
    fbo->allocate(output_width,output_height,GL_RGBA32F_ARB,4);
    fbo->begin();
    ofClear(0,0,0,255);
    fbo->end();

    ofTextureData texData;
    texData.width = this->output_width;
    texData.height = this->output_height;
    texData.textureTarget = GL_TEXTURE_2D;
    texData.bFlipTexture = true;

    _outletParams[0] = new ofTexture();
    static_cast<ofTexture *>(_outletParams[0])->allocate(texData);

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
        this->saveConfig(false);

        fbo = new ofFbo();
        fbo->allocate(output_width,output_height,GL_RGBA32F_ARB,4);
        fbo->begin();
        ofClear(0,0,0,255);
        fbo->end();

        ofTextureData texData;
        texData.width = this->output_width;
        texData.height = this->output_height;
        texData.textureTarget = GL_TEXTURE_2D;
        texData.bFlipTexture = true;

        _outletParams[0] = new ofTexture();
        static_cast<ofTexture *>(_outletParams[0])->allocate(texData);

        tempstring = "OUTPUT_WIDTH = "+ofToString(output_width);
        static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);
        tempstring = "OUTPUT_HEIGHT = "+ofToString(output_height);
        static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);
        if(this->inletsConnected[0]){
            tempstring = "USING_DATA_INLET = true";
        }else{
            tempstring = "USING_DATA_INLET = false";
        }
        static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);

        tempstring = mosaicStringName+" = ''";
        static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);

        ofFile tempFileScript(filepath);
        tempstring = "SCRIPT_PATH = '"+tempFileScript.getEnclosingDirectory().substr(0,tempFileScript.getEnclosingDirectory().size()-1)+"'";
        static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);

    }

}

//--------------------------------------------------------------
void LuaScript::openScript(string scriptFile){
    ofFile file (scriptFile);
    if (file.exists()){
        string fileExtension = ofToUpper(file.getExtension());
        if(fileExtension == "LUA") {
            // check if others lua files exists in script folder (multiple files lua script) and import them
            ofDirectory td;
            td.allowExt("lua");
            td.listDir(file.getEnclosingDirectory());
            for(int i = 0; i < (int)td.size(); i++){
                if(td.getPath(i) != file.getAbsolutePath()){
                    copyFileToPatchFolder(this->patchFolderPath,td.getPath(i));
                }
            }
            // import all subfolders -- NOT RECURSIVE
            ofDirectory tf;
            tf.listDir(file.getEnclosingDirectory());
            for(int i = 0; i < (int)tf.size(); i++){
                ofDirectory ttf(tf.getPath(i));
                if(ttf.isDirectory() && tf.getName(i) != "data"){
                    filesystem::path tpa(this->patchFolderPath+tf.getName(i)+"/");
                    ttf.copyTo(tpa,false,false);
                    ttf.listDir();
                    for(int j = 0; j < (int)ttf.size(); j++){
                        ofFile ftf(ttf.getPath(j));
                        if(ftf.isFile()){
                            filesystem::path tpa(this->patchFolderPath+tf.getName(i)+"/"+ftf.getFileName());
                            ftf.copyTo(tpa,false,false);
                        }
                    }
                    //ofLog(OF_LOG_NOTICE,"%s - %s",this->patchFolderPath.c_str(),tf.getName(i).c_str());
                }
            }

            // then import the main script file
            filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
            reloadScript();
        }
    }
}

//--------------------------------------------------------------
void LuaScript::newScript(string scriptFile){
    ofFile fileToRead(ofToDataPath("scripts/empty.lua"));
    ofFile newLuaFile (scriptFile);
    ofFile::copyFromTo(fileToRead.getAbsolutePath(),checkFileExtension(newLuaFile.getAbsolutePath(), ofToUpper(newLuaFile.getExtension()), "LUA"),true,true);
    filepath = scriptFile;
    reloadScript();
}

//--------------------------------------------------------------
void LuaScript::loadScript(string scriptFile){

    currentScriptFile.open(filepath);

    static_cast<LiveCoding *>(_outletParams[1])->filepath = filepath;

    // inject incoming data vector to lua
    string tempstring = mosaicTableName+" = {}";
    static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);
    tempstring = "function _updateMosaicData(i,data) "+mosaicTableName+"[i] = data  end";
    static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);

    // inject outgoing data vector
    tempstring = luaTablename+" = {}";
    static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);
    tempstring = "function _getLUAOutletTableAt(i) return "+luaTablename+"[i] end";
    static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);

    // inject incoming string
    tempstring = mosaicStringName+" = ''";
    static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);

    // set Mosaic scripting vars
    tempstring = "OUTPUT_WIDTH = "+ofToString(output_width);
    static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);
    tempstring = "OUTPUT_HEIGHT = "+ofToString(output_height);
    static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);
    if(this->inletsConnected[0]){
        tempstring = "USING_DATA_INLET = true";
    }else{
        tempstring = "USING_DATA_INLET = false";
    }
    static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);
    ofFile tempFileScript(currentScriptFile.getAbsolutePath());
    tempstring = "SCRIPT_PATH = '"+tempFileScript.getEnclosingDirectory().substr(0,tempFileScript.getEnclosingDirectory().size()-1)+"'";

    #ifdef TARGET_WIN32
        std::replace(tempstring.begin(),tempstring.end(),'\\','/');
    #endif

    static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);

    // load lua Mosaic lib
    tempstring = ofBufferFromFile("livecoding/lua_mosaicLib.lua").getText();
    static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);

    // finally load the script
    static_cast<LiveCoding *>(_outletParams[1])->lua.doScript(filepath, true);

    scriptLoaded = static_cast<LiveCoding *>(_outletParams[1])->lua.isValid();

    ///////////////////////////////////////////
    // LUA SETUP
    if(scriptLoaded  && !isError){
        watcher.removeAllPaths();
        watcher.addPath(filepath);
        ofLog(OF_LOG_NOTICE,"[verbose] lua script: %s loaded & running!",filepath.c_str());
        this->saveConfig(false);
    }
    ///////////////////////////////////////////

}

//--------------------------------------------------------------
void LuaScript::unloadScript(){
    static_cast<LiveCoding *>(_outletParams[1])->lua.scriptExit();
    static_cast<LiveCoding *>(_outletParams[1])->lua.init(true);
}

//--------------------------------------------------------------
void LuaScript::clearScript(){
    unloadScript();
    static_cast<LiveCoding *>(_outletParams[1])->lua.doString("function draw() of.background(0) end");

    // inject incoming data vector to lua
    string tempstring = mosaicTableName+" = {}";
    static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);
    tempstring = "function _updateMosaicData(i,data) "+mosaicTableName+"[i] = data  end";
    static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);

    // inject outgoing data vector
    tempstring = luaTablename+" = {}";
    static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);
    tempstring = "function _getLUAOutletTableAt(i) return "+luaTablename+"[i] end";
    static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);

    // inject incoming string
    tempstring = mosaicStringName+" = ''";
    static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);

    // set Mosaic scripting vars
    tempstring = "OUTPUT_WIDTH = "+ofToString(output_width);
    static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);
    tempstring = "OUTPUT_HEIGHT = "+ofToString(output_height);
    static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);
    if(this->inletsConnected[0]){
        tempstring = "USING_DATA_INLET = true";
    }else{
        tempstring = "USING_DATA_INLET = false";
    }
    static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);
    ofFile tempFileScript(filepath);
    tempstring = "SCRIPT_PATH = '"+tempFileScript.getEnclosingDirectory().substr(0,tempFileScript.getEnclosingDirectory().size()-1)+"'";

    #ifdef TARGET_WIN32
        std::replace(tempstring.begin(),tempstring.end(),'\\','/');
    #endif

    static_cast<LiveCoding *>(_outletParams[1])->lua.doString(tempstring);

    scriptLoaded = static_cast<LiveCoding *>(_outletParams[1])->lua.isValid();
}

//--------------------------------------------------------------
void LuaScript::reloadScript(){

    unloadScript();

    scriptLoaded = false;
    needToLoadScript = true;
    isError = false;
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
            reloadScript();
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
    isError = true;

    if(!msg.empty()){
        size_t found = msg.find_first_of("\n");
        if(found == string::npos){
            ofLog(OF_LOG_ERROR,"LUA SCRIPT error: %s",msg.c_str());
        }
    }
}

OBJECT_REGISTER( LuaScript, "lua script", OFXVP_OBJECT_CAT_SCRIPTING)

#endif
