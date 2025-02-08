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

#if defined(TARGET_WIN32)
    // Unavailable on windows.
#elif !defined(OFXVP_BUILD_WITH_MINIMAL_OBJECTS)

#include "BashScript.h"

//--------------------------------------------------------------
BashScript::BashScript() : PatchObject("bash script"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new string();         // control
    *ofxVP_CAST_PIN_PTR<string>(this->_inletParams[0]) = "";

    _outletParams[0] = new string();        // output
    *ofxVP_CAST_PIN_PTR<string>(this->_outletParams[0]) = "";

    this->initInletsState();

    posX = posY = drawW = drawH = 0.0f;

    bashIcon            = new ofImage();

    scriptLoaded        = false;
    isNewObject         = false;

    lastMessage         = "";

    threadLoaded        = false;
    needToLoadScript    = true;

    loadScriptFlag      = false;
    saveScriptFlag      = false;

    this->setIsTextureObj(true);

}

//--------------------------------------------------------------
void BashScript::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_STRING,"control");

    this->addOutlet(VP_LINK_STRING,"scriptSTDOutput");
}

//--------------------------------------------------------------
void BashScript::autoloadFile(string _fp){
    threadLoaded = false;
    filepath = copyFileToPatchFolder(this->patchFolderPath,_fp);
    reloadScript();
}

//--------------------------------------------------------------
void BashScript::threadedFunction(){
    while(isThreadRunning()){
        std::unique_lock<std::mutex> lock(mutex);
        if(needToLoadScript){
            needToLoadScript = false;
            loadScript(filepath);
            threadLoaded = true;
        }
        condition.wait(lock);
        sleep(10);
    }

}

//--------------------------------------------------------------
void BashScript::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    // GUI
    fileDialog.setIsRetina(this->isRetina);

    ofDisableArbTex();
    bashIcon->load("images/bash.png");
    ofEnableArbTex();

    // Load script
    watcher.start();

    if(!isThreadRunning()){
        startThread();
    }

}

//--------------------------------------------------------------
void BashScript::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    // listen to message control (_inletParams[0])
    if(this->inletsConnected[0]){
        if(lastMessage != *ofxVP_CAST_PIN_PTR<string>(this->_inletParams[0])){
            lastMessage = *ofxVP_CAST_PIN_PTR<string>(this->_inletParams[0]);
        }

        if(lastMessage == "bang"){
            reloadScript();
        }
    }

    // path watcher
    while(watcher.waitingEvents()) {
        pathChanged(watcher.nextEvent());
    }

    condition.notify_all();

}

//--------------------------------------------------------------
void BashScript::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);
}

//--------------------------------------------------------------
void BashScript::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
    loadScriptFlag = false;
    saveScriptFlag = false;

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

        ImVec2 window_pos = ImGui::GetWindowPos()+ImVec2(IMGUI_EX_NODE_PINS_WIDTH_NORMAL, IMGUI_EX_NODE_HEADER_HEIGHT);
        _nodeCanvas.getNodeDrawList()->AddRectFilled(window_pos,window_pos+ImVec2(scaledObjW*this->scaleFactor*_nodeCanvas.GetCanvasScale(), scaledObjH*this->scaleFactor*_nodeCanvas.GetCanvasScale()),ImGui::GetColorU32(ImVec4(34.0/255.0, 34.0/255.0, 34.0/255.0, 1.0f)));

        if(bashIcon->getTexture().isAllocated()){
            calcTextureDims(bashIcon->getTexture(), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
            ImGui::SetCursorPos(ImVec2(posX+(IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor), posY+(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor)));
            ImGui::Image((ImTextureID)(uintptr_t)bashIcon->getTexture().getTextureData().textureID, ImVec2(drawW, drawH));
        }

        // get imgui node translated/scaled position/dimension for drawing textures in OF
        //objOriginX = (ImGui::GetWindowPos().x + ((IMGUI_EX_NODE_PINS_WIDTH_NORMAL - 1)*this->scaleFactor) - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale();
        //objOriginY = (ImGui::GetWindowPos().y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale();
        scaledObjW = this->width - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL+IMGUI_EX_NODE_PINS_WIDTH_SMALL)*this->scaleFactor/_nodeCanvas.GetCanvasScale();
        scaledObjH = this->height - (IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor/_nodeCanvas.GetCanvasScale();

        _nodeCanvas.EndNodeContent();
    }

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, loadScriptFlag, "Select a bash script", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".sh", "", scaleFactor)){
        ofFile bashFile (fileDialog.selected_path);
        filepath = copyFileToPatchFolder(this->patchFolderPath,bashFile.getAbsolutePath());
        threadLoaded = false;
        reloadScript();
    }

}

//--------------------------------------------------------------
void BashScript::drawObjectNodeConfig(){
    ofFile tempFilename(filepath);

    loadScriptFlag = false;
    saveScriptFlag = false;

    ImGui::Spacing();
    ImGui::Text("Loaded File:");
    if(filepath == "none"){
        ImGui::Text("%s",filepath.c_str());
    }else{
        ImGui::Text("%s",tempFilename.getFileName().c_str());
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",tempFilename.getAbsolutePath().c_str());
    }
    ImGui::Spacing();
    if(ImGui::Button("New",ImVec2(224*scaleFactor,26*scaleFactor))){
        saveScriptFlag = true;
    }

    if(saveScriptFlag){
        newScriptName = "bashScript_"+ofGetTimestampString("%y%m%d")+".sh";
        ImGui::OpenPopup("Save new bash script as");
    }

    if(ImGui::BeginPopup("Save new bash script as")){

        if(ImGui::InputText("##NewFileNameInput", &newScriptName,ImGuiInputTextFlags_EnterReturnsTrue)){
            if(newScriptName != ""){
                // save file in data/ folder
                ofFile fileToRead(ofToDataPath("scripts/empty.sh"));
                ofFile newBashFile (this->patchFolderPath+newScriptName);
                ofFile::copyFromTo(fileToRead.getAbsolutePath(),checkFileExtension(newBashFile.getAbsolutePath(), ofToUpper(newBashFile.getExtension()), "SH"),true,true);
                filepath = this->patchFolderPath+newScriptName;
                threadLoaded = false;
                reloadScript();
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
                ofFile fileToRead(ofToDataPath("scripts/empty.sh"));
                ofFile newBashFile (this->patchFolderPath+newScriptName);
                ofFile::copyFromTo(fileToRead.getAbsolutePath(),checkFileExtension(newBashFile.getAbsolutePath(), ofToUpper(newBashFile.getExtension()), "SH"),true,true);
                filepath = this->patchFolderPath+newScriptName;
                threadLoaded = false;
                reloadScript();
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();

    }

    ImGui::Spacing();
    if(ImGui::Button("Open",ImVec2(224*scaleFactor,26*scaleFactor))){
        loadScriptFlag = true;
    }


    ImGuiEx::ObjectInfo(
                "Load and run a bash script files (Bourne-Again SHell). You can type code with the Mosaic code editor, or with your default code editor. Scripts will refresh automatically on save.",
                "https://mosaic.d3cod3.org/reference.php?r=bash-script", scaleFactor);


    if(ImGuiEx::getFileDialog(fileDialog, loadScriptFlag, "Select a bash script", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".sh", "", scaleFactor)){
        ofFile bashFile (fileDialog.selected_path);
        filepath = copyFileToPatchFolder(this->patchFolderPath,bashFile.getAbsolutePath());
        threadLoaded = false;
        reloadScript();
    }
}

//--------------------------------------------------------------
void BashScript::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);

    std::unique_lock<std::mutex> lck(mutex);
    stopThread();
    condition.notify_all();
}


//--------------------------------------------------------------
void BashScript::loadScript(string scriptFile){

    ofFile tmpf(scriptFile);

    if(tmpf.isFile() && ofToUpper(tmpf.getExtension()) == "SH"){
        filepath = forceCheckMosaicDataPath(scriptFile);
        currentScriptFile.open(filepath);

        string cmd = "";
        FILE *execFile;

        cmd = "sh "+filepath;
        execFile = popen(cmd.c_str(), "r");

        if (execFile){
            scriptLoaded = true;
            watcher.removeAllPaths();
            watcher.addPath(filepath);

            ofLog(OF_LOG_NOTICE,"-- bash script: %s RUNNING!",filepath.c_str());
            ofLog(OF_LOG_NOTICE,"%s"," ");

            char buffer[128];
            _outletParams[0] = new string();
            *ofxVP_CAST_PIN_PTR<string>(_outletParams[0]) = "";
            while(!feof(execFile)){
                if(fgets(buffer, sizeof(buffer), execFile) != nullptr){
                    char *s = buffer;
                    std::string tempstr(s);
                    static_cast<string *>(_outletParams[0])->append(tempstr);
                    static_cast<string *>(_outletParams[0])->append(" ");
                }
            }
            //ofLog(OF_LOG_NOTICE,"%s",static_cast<string *>(_outletParams[0])->c_str());
            ofLog(OF_LOG_NOTICE,"-- bash script: %s EXECUTED!",filepath.c_str());

            this->saveConfig(false);

            pclose(execFile);

        }
    }

}

//--------------------------------------------------------------
void BashScript::reloadScript(){
    scriptLoaded = false;
    needToLoadScript = true;
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
            reloadScript();
            break;
        case PathWatcher::DELETED:
            //ofLogVerbose(PACKAGE) << "path deleted " << event.path;
            return;
        default: // NONE
            return;
    }

}

OBJECT_REGISTER( BashScript, "bash script", OFXVP_OBJECT_CAT_SCRIPTING)

#endif
