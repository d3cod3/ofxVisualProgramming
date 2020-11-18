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


#include "PythonScript.h"

//--------------------------------------------------------------
PythonScript::PythonScript() : PatchObject("python script"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new vector<float>();      // data input

    _outletParams[0] = new vector<float>();         // data output

    this->initInletsState();

    isNewObject         = false;

    pythonIcon          = new ofImage();
    posX = posY = drawW = drawH = 0.0f;

    mosaicTableName = "_mosaic_data_inlet";
    pythonTableName = "_mosaic_data_outlet";
    tempstring      = "";
    needToLoadScript= true;

    lastPythonScript       = "";
    loadPythonScriptFlag   = false;
    savePythonScriptFlag   = false;

    this->setIsTextureObj(true);

}

//--------------------------------------------------------------
void PythonScript::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"_mosaic_data_inlet");

    this->addOutlet(VP_LINK_ARRAY,"_mosaic_data_outlet");
}

//--------------------------------------------------------------
void PythonScript::autoloadFile(string _fp){
    lastPythonScript = _fp;
    ofFile file (lastPythonScript);
    filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
    reloadScript();
}

//--------------------------------------------------------------
void PythonScript::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    pythonIcon->load("images/python.png");

    // init python
    python.init();
    watcher.start();

    /*if(filepath == "none"){
        isNewObject = true;
        ofFile file (ofToDataPath("scripts/empty.py"));
        filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
    }*/

}

//--------------------------------------------------------------
void PythonScript::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    while(watcher.waitingEvents()) {
        pathChanged(watcher.nextEvent());
    }

    if(needToLoadScript && filepath != "none"){
        needToLoadScript = false;
        loadScript(filepath);
    }

}

//--------------------------------------------------------------
void PythonScript::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){

    ///////////////////////////////////////////
    // PYTHON UPDATE
    if(script){
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
            updatePythonList = python.getObject("_getPYOutletTableAt");
            getPythonListSize = python.getObject("_getPYOutletSize");
            if(updatePythonList && !updatePythonList.isPythonError() && getPythonListSize && !getPythonListSize.isPythonError()){
                static_cast<vector<float> *>(_outletParams[0])->clear();
                for(int i=0;i<static_cast<int>(getPythonListSize(ofxPythonObject::fromInt(static_cast<int>(0))).asInt());i++){
                    static_cast<vector<float> *>(_outletParams[0])->push_back(updatePythonList(ofxPythonObject::fromInt(static_cast<int>(i))).asFloat());
                }
            }

            updatePython();
        }
    }
    ///////////////////////////////////////////

    ofSetColor(255);
    // draw node texture preview with OF
    if(scaledObjW*canvasZoom > 90.0f){
        drawNodeOFTexture(pythonIcon->getTexture(), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
    }

}

//--------------------------------------------------------------
void PythonScript::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
    loadPythonScriptFlag = false;
    savePythonScriptFlag = false;

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

        _nodeCanvas.EndNodeContent();
    }

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();

    // file dialog
    /*string newFileName = "pythonScript_"+ofGetTimestampString("%y%m%d")+".py";
    if(ImGuiEx::getFileDialog(fileDialog, savePythonScriptFlag, "Save new python script as", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ".py", newFileName, scaleFactor)){
        lastPythonScript = fileDialog.selected_path;

        ofFile fileToRead(ofToDataPath("scripts/empty.py"));
        ofFile newPyFile (lastPythonScript);
        ofFile::copyFromTo(fileToRead.getAbsolutePath(),checkFileExtension(newPyFile.getAbsolutePath(), ofToUpper(newPyFile.getExtension()), "PY"),true,true);
        filepath = copyFileToPatchFolder(this->patchFolderPath,checkFileExtension(newPyFile.getAbsolutePath(), ofToUpper(newPyFile.getExtension()), "PY"));
        reloadScript();
    }*/

    if(ImGuiEx::getFileDialog(fileDialog, loadPythonScriptFlag, "Select a python script", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".py", "", scaleFactor)){
        lastPythonScript = fileDialog.selected_path;

        ofFile file (lastPythonScript);
        filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
        reloadScript();
    }

}

//--------------------------------------------------------------
void PythonScript::drawObjectNodeConfig(){
    ofFile tempFilename(filepath);

    loadPythonScriptFlag = false;
    savePythonScriptFlag = false;

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
        savePythonScriptFlag = true;
    }

    if(savePythonScriptFlag){
        newScriptName = "pythonScript_"+ofGetTimestampString("%y%m%d")+".py";
        ImGui::OpenPopup("Save new python script as");
    }

    if(ImGui::BeginPopup("Save new python script as")){

        if(ImGui::InputText("##NewFileNameInput", &newScriptName,ImGuiInputTextFlags_EnterReturnsTrue)){
            if(newScriptName != ""){
                // save file in data/ folder
                ofFile fileToRead(ofToDataPath("scripts/empty.py"));
                ofFile newPyFile (this->patchFolderPath+newScriptName);
                ofFile::copyFromTo(fileToRead.getAbsolutePath(),checkFileExtension(newPyFile.getAbsolutePath(), ofToUpper(newPyFile.getExtension()), "PY"),true,true);
                filepath = this->patchFolderPath+newScriptName;
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
                ofFile fileToRead(ofToDataPath("scripts/empty.py"));
                ofFile newPyFile (this->patchFolderPath+newScriptName);
                ofFile::copyFromTo(fileToRead.getAbsolutePath(),checkFileExtension(newPyFile.getAbsolutePath(), ofToUpper(newPyFile.getExtension()), "PY"),true,true);
                filepath = this->patchFolderPath+newScriptName;
                reloadScript();
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();

    }

    ImGui::Spacing();
    if(ImGui::Button("Open",ImVec2(224*scaleFactor,26*scaleFactor))){
        loadPythonScriptFlag = true;
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
                "Load and run a python ( 2.7 on osx, 3.8 on linux ) script files. You can type code with the Mosaic code editor, or with your default code editor. Scripts will refresh automatically on save.",
                "https://mosaic.d3cod3.org/reference.php?r=python-script", scaleFactor);

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, loadPythonScriptFlag, "Select a python script", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".py", "", scaleFactor)){
        lastPythonScript = fileDialog.selected_path;

        ofFile file (lastPythonScript);
        filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
        reloadScript();
    }
}

//--------------------------------------------------------------
void PythonScript::removeObjectContent(bool removeFileFromData){
    script = ofxPythonObject::_None();
}

//--------------------------------------------------------------
void PythonScript::loadScript(string scriptFile){

    filepath = forceCheckMosaicDataPath(scriptFile);
    currentScriptFile.open(filepath);

    python.reset();
    python.addPath(currentScriptFile.getEnclosingDirectory());
    python.executeScript(filepath);

    // inject incoming data vector to python as list
    string tempstring = mosaicTableName+" = [];\n"+mosaicTableName+".append(0)";
    python.executeString(tempstring);
    // tabs and newlines are really important in python!
    tempstring = "def _updateMosaicData( i,data ):\n\t if len("+mosaicTableName+") < i:\n\t\t "+mosaicTableName+".append(0)\n\t elif 0 <= i < len("+mosaicTableName+"):\n\t\t "+mosaicTableName+"[i] = data\n";
    python.executeString(tempstring);

    // inject outgoing data list to mosaic as vector<float>
    tempstring = pythonTableName+" = [];\n"+pythonTableName+".append(0)";
    python.executeString(tempstring);
    tempstring = "def _getPYOutletTableAt( i ):\n\t return "+pythonTableName+"[i]\n";
    python.executeString(tempstring);
    tempstring = "def _getPYOutletSize( i ):\n\t return len("+pythonTableName+")\n";
    python.executeString(tempstring);

    // set Mosaic scripting vars
    ofFile tempFileScript(filepath);

    string temppath = tempFileScript.getEnclosingDirectory().substr(0,tempFileScript.getEnclosingDirectory().size()-1);

    #ifdef TARGET_WIN32
        std::replace(temppath.begin(),temppath.end(),'\\','/');
    #endif

    tempstring = "SCRIPT_PATH = '"+temppath+"'\n";
    python.executeString(tempstring);

    ///////////////////////////////////////////
    // PYTHON SETUP
    klass = python.getObject("mosaicApp");
    if(klass){
        script = klass();
        ofLog(OF_LOG_NOTICE,"[verbose] python script: %s loaded & running!",filepath.c_str());
        watcher.removeAllPaths();
        watcher.addPath(filepath);

        this->saveConfig(false);
    }else{
        script = ofxPythonObject::_None();
    }
    ///////////////////////////////////////////

}

//--------------------------------------------------------------
void PythonScript::clearScript(){
    python.reset();
    script = ofxPythonObject::_None();
}

//--------------------------------------------------------------
void PythonScript::reloadScript(){
    script = ofxPythonObject::_None();
    needToLoadScript = true;
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
            reloadScript();
            break;
        case PathWatcher::DELETED:
            //ofLogVerbose(PACKAGE) << "path deleted " << event.path;
            return;
        default: // NONE
            return;
    }

}


OBJECT_REGISTER( PythonScript, "python script", OFXVP_OBJECT_CAT_SCRIPTING)

#endif
