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

#include "ShaderObject.h"

//--------------------------------------------------------------
ShaderObject::ShaderObject() : PatchObject("glsl shader"){

    this->numInlets  = 0;
    this->numOutlets = 1;

    _outletParams[0] = new ofTexture();     // output

    scriptLoaded        = false;
    isNewObject         = false;
    reloading           = false;

    isGUIObject         = true;
    this->isOverGUI     = true;

    fbo                 = new ofFbo();
    pingPong            = new ofxPingPong();
    shader              = new ofShader();
    needReset           = false;

    kuro        = new ofImage();

    posX = posY = drawW = drawH = 0.0f;

    output_width    = STANDARD_TEXTURE_WIDTH;
    output_height   = STANDARD_TEXTURE_HEIGHT;

    nTextures       = 0;
    internalFormat  = GL_RGBA;

    fragmentShader  = "";
    vertexShader    = "";

    lastShaderScript        = "";
    lastVertexShaderPath    = "";
    loadShaderScriptFlag    = false;
    saveShaderScriptFlag    = false;
    shaderScriptLoaded      = false;
    shaderScriptSaved       = false;
    oneBang                 = false;

    loaded                  = false;

    this->setIsResizable(true);
    this->setIsTextureObj(true);

    prevW                   = this->width;
    prevH                   = this->height;

}

//--------------------------------------------------------------
void ShaderObject::newObject(){
    PatchObject::setName( this->objectName );
    this->addOutlet(VP_LINK_TEXTURE,"output");

    this->setCustomVar(static_cast<float>(output_width),"OUTPUT_WIDTH");
    this->setCustomVar(static_cast<float>(output_height),"OUTPUT_HEIGHT");

    this->setCustomVar(static_cast<float>(prevW),"WIDTH");
    this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
}

//--------------------------------------------------------------
void ShaderObject::autoloadFile(string _fp){
    lastShaderScript = _fp;
    shaderScriptLoaded = true;
}

//--------------------------------------------------------------
void ShaderObject::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    fileDialog.setIsRetina(this->isRetina);

    initResolution();

    // load kuro
    kuro->load("images/kuro.jpg");

    // init path watcher
    watcher.start();

    if(filepath != "none"){
        loadScript(filepath);
    }

}

//--------------------------------------------------------------
void ShaderObject::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    // Recursive reset for shader objects chain
    if(needReset){
        needReset = false;
        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if(patchObjects[it->first] != nullptr && it->first != this->getId() && !patchObjects[it->first]->getWillErase()){
                for(int o=0;o<static_cast<int>(it->second->outPut.size());o++){
                    if(!it->second->outPut[o]->isDisabled && it->second->outPut[o]->toObjectID == this->getId()){
                        if(it->second->getName() == "lua script" || it->second->getName() == "python script" || it->second->getName() == "glsl shader"){
                            it->second->resetResolution(this->getId(),output_width,output_height);
                        }
                    }
                }
            }
        }
    }

    if(shaderScriptLoaded){
        shaderScriptLoaded = false;
        // open the new one
        currentScriptFile.open(lastShaderScript);
        if (currentScriptFile.exists()){
            string fileExtension = ofToUpper(currentScriptFile.getExtension());
            if(fileExtension == "FRAG") {
                filepath = copyFileToPatchFolder(this->patchFolderPath,currentScriptFile.getAbsolutePath());
                string fsName = currentScriptFile.getFileName();
                string vsName = currentScriptFile.getEnclosingDirectory()+currentScriptFile.getFileName().substr(0,fsName.find_last_of('.'))+".vert";
                ofFile newVertGLSLFile (vsName);
                if(newVertGLSLFile.exists()){
                    copyFileToPatchFolder(this->patchFolderPath,newVertGLSLFile.getAbsolutePath());
                }else{
                    ofFile vertToRead;
                    if(ofIsGLProgrammableRenderer()){
                      vertToRead.open(ofToDataPath("scripts/empty.vert"));
                    }else{
                      vertToRead.open(ofToDataPath("scripts/empty_120.vert"));
                    }
                    ofFile patchFolderNewFrag(filepath);
                    string pf_fsName = patchFolderNewFrag.getFileName();
                    string pf_vsName = patchFolderNewFrag.getEnclosingDirectory()+patchFolderNewFrag.getFileName().substr(0,pf_fsName.find_last_of('.'))+".vert";
                    ofFile::copyFromTo(vertToRead.getAbsolutePath(),pf_vsName,true,true);
                }
                loadScript(filepath);
                reloading = true;
            }else if(fileExtension == "VERT"){
                string newVertOpened = copyFileToPatchFolder(this->patchFolderPath,currentScriptFile.getAbsolutePath());
                string vsName = currentScriptFile.getFileName();
                string fsName = currentScriptFile.getEnclosingDirectory()+currentScriptFile.getFileName().substr(0,vsName.find_last_of('.'))+".frag";
                ofFile newFragGLSLFile (fsName);
                if(newFragGLSLFile.exists()){
                    filepath = copyFileToPatchFolder(this->patchFolderPath,newFragGLSLFile.getAbsolutePath());
                }else{
                    ofFile fragToRead;
                    if(ofIsGLProgrammableRenderer()){
                      fragToRead.open(ofToDataPath("scripts/empty.frag"));
                    }else{
                      fragToRead.open(ofToDataPath("scripts/empty_120.frag"));
                    }
                    ofFile patchFolderNewVert(newVertOpened);
                    string pf_vsName = patchFolderNewVert.getFileName();
                    string pf_fsName = patchFolderNewVert.getEnclosingDirectory()+patchFolderNewVert.getFileName().substr(0,pf_vsName.find_last_of('.'))+".frag";
                    ofFile::copyFromTo(fragToRead.getAbsolutePath(),pf_fsName,true,true);
                    filepath = pf_fsName;
                }
                loadScript(filepath);
                reloading = true;
            }
        }
    }

    if(shaderScriptSaved){
        shaderScriptSaved = false;
        // create and open the new one
        ofFile fileToRead;
        if(ofIsGLProgrammableRenderer()){
          fileToRead.open(ofToDataPath("scripts/empty.frag"));
        }else{
          fileToRead.open(ofToDataPath("scripts/empty_120.frag"));
        }
        ofFile newGLSLFile (lastShaderScript);
        ofFile::copyFromTo(fileToRead.getAbsolutePath(),checkFileExtension(newGLSLFile.getAbsolutePath(), ofToUpper(newGLSLFile.getExtension()), "FRAG"),true,true);
        ofFile correctedFileToRead(checkFileExtension(newGLSLFile.getAbsolutePath(), ofToUpper(newGLSLFile.getExtension()), "FRAG"));

        ofFile vertToRead;
        if(ofIsGLProgrammableRenderer()){
          vertToRead.open(ofToDataPath("scripts/empty.vert"));
        }else{
          vertToRead.open(ofToDataPath("scripts/empty_120.vert"));
        }
        string fsName = newGLSLFile.getFileName();
        string vsName = newGLSLFile.getEnclosingDirectory()+newGLSLFile.getFileName().substr(0,fsName.find_last_of('.'))+".vert";
        ofFile newVertGLSLFile (vsName);
        ofFile::copyFromTo(vertToRead.getAbsolutePath(),newVertGLSLFile.getAbsolutePath(),true,true);

        currentScriptFile = correctedFileToRead;

        if (currentScriptFile.exists()){
            filepath = currentScriptFile.getAbsolutePath();
            loadScript(filepath);
            reloading = true;
        }
    }

    while(watcher.waitingEvents()) {
        pathChanged(watcher.nextEvent());
    }


    if(!loaded){
        loaded = true;
        prevW = this->getCustomVar("WIDTH");
        prevH = this->getCustomVar("HEIGHT");
        this->width             = prevW;
        this->height            = prevH;
    }
}

//--------------------------------------------------------------
void ShaderObject::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){

    ///////////////////////////////////////////
    // SHADER UPDATE
    if(scriptLoaded){
        // receive external data
        for(int i=0;i<this->numInlets;i++){
            if(this->inletsConnected[i] && this->getInletType(i) == VP_LINK_TEXTURE && i < static_cast<int>(textures.size()) && static_cast<ofTexture *>(_inletParams[i])->isAllocated()){
                textures[i]->begin();
                ofSetColor(255);
                static_cast<ofTexture *>(_inletParams[i])->draw(0,0,output_width, output_height);
                textures[i]->end();
            }
        }
        pingPong->dst->begin();

        ofClear(0);
        shader->begin();
        shader->setUniformTexture("backbuffer", pingPong->src->getTexture(),0);
        for(int i=0;i<static_cast<int>(textures.size());i++){
            string texName = "tex" + ofToString(i);
            shader->setUniformTexture(texName.c_str(),textures[i]->getTexture(),i+1);
        }
        shader->setUniform2f("resolution",static_cast<float>(output_width),static_cast<float>(output_height));
        shader->setUniform1f("time",static_cast<float>(ofGetElapsedTimef()));

        for(int i=0;i<this->numInlets;i++){
            if(this->inletsConnected[i] && this->getInletType(i) == VP_LINK_NUMERIC){
                shaderSliders.at(i-static_cast<int>(textures.size())) = *(float *)&_inletParams[i];
            }
        }

        // set custom shader vars
        string paramName, tempVarName;
        for(size_t i=0;i<shaderSliders.size();i++){
            if(shaderSlidersType.at(i) == ShaderSliderType_FLOAT){
                paramName = "param1f"+ofToString(shaderSlidersIndex[i]);
                tempVarName = "GUI_FLOAT_"+shaderSlidersLabel.at(i);
                shader->setUniform1f(paramName.c_str(), static_cast<float>(shaderSliders.at(i)));
            }else if(shaderSlidersType.at(i) == ShaderSliderType_INT){
                paramName = "param1i"+ofToString(shaderSlidersIndex[i]);
                tempVarName = "GUI_INT_"+shaderSlidersLabel.at(i);
                shader->setUniform1i(paramName.c_str(), static_cast<int>(floor(shaderSliders.at(i))));
            }
        }

        ofSetColor(255,255);
        if (ofIsGLProgrammableRenderer()) {
            if (quad.getNumVertices() == 0) {
                quad.clear();
                quad.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
                quad.addVertex(glm::vec3(0, 0, 0));			 quad.addTexCoord(glm::vec2(0, 0));
                quad.addVertex(glm::vec3(output_width, 0, 0));		 quad.addTexCoord(glm::vec2(output_width, 0));
                quad.addVertex(glm::vec3(output_width, output_width, 0)); quad.addTexCoord(glm::vec2(output_width, output_width));
                quad.addVertex(glm::vec3(0, output_width, 0));	 quad.addTexCoord(glm::vec2(0, output_width));
            }
            quad.draw(OF_MESH_FILL);
        }else{
            glBegin(GL_QUADS);
            glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
            glTexCoord2f(output_width, 0); glVertex3f(output_width, 0, 0);
            glTexCoord2f(output_width, output_height); glVertex3f(output_width, output_height, 0);
            glTexCoord2f(0,output_height);  glVertex3f(0,output_height, 0);
            glEnd();
        }

        shader->end();

        pingPong->dst->end();

        pingPong->swap();

    }
    ///////////////////////////////////////////

    ///////////////////////////////////////////
    // SHADER DRAW
    fbo->begin();
    if(scriptLoaded){
        ofPushView();
        ofPushStyle();
        ofPushMatrix();
        ofEnableAlphaBlending();
        ofSetColor(255);
        pingPong->dst->draw(0,0,output_width, output_height);
        ofPopMatrix();
        ofPopStyle();
        ofPopView();
    }else{
        kuro->draw(0,0,fbo->getWidth(),fbo->getHeight());
    }
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
void ShaderObject::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
    loadShaderScriptFlag = false;
    saveShaderScriptFlag = false;

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
    /*string newFileName = "glslshader_"+ofGetTimestampString("%y%m%d")+".frag";
    if(ImGuiEx::getFileDialog(fileDialog, saveShaderScriptFlag, "Save new GLSL shader as", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ".frag", newFileName, scaleFactor)){
        lastShaderScript = fileDialog.selected_path;
        shaderScriptSaved = true;
    }*/

    if(ImGuiEx::getFileDialog(fileDialog, loadShaderScriptFlag, "Select a GLSL shader", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".frag,.vert", "", scaleFactor)){
        lastShaderScript = fileDialog.selected_path;
        shaderScriptLoaded = true;
    }

}

//--------------------------------------------------------------
void ShaderObject::drawObjectNodeConfig(){
    ofFile tempFilename(filepath);

    loadShaderScriptFlag = false;
    saveShaderScriptFlag = false;

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
        saveShaderScriptFlag = true;
    }

    if(saveShaderScriptFlag){
        newScriptName = "glslshader_"+ofGetTimestampString("%y%m%d")+".frag";
        ImGui::OpenPopup("Save new GLSL shader as");
    }

    if(ImGui::BeginPopup("Save new GLSL shader as")){

        if(ImGui::InputText("##NewFileNameInput", &newScriptName,ImGuiInputTextFlags_EnterReturnsTrue)){
            if(newScriptName != ""){
                // save file in data/ folder
                lastShaderScript = this->patchFolderPath+newScriptName;
                shaderScriptSaved = true;
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
                lastShaderScript = this->patchFolderPath+newScriptName;
                shaderScriptSaved = true;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();

    }

    ImGui::Spacing();
    if(ImGui::Button("Open",ImVec2(224*scaleFactor,26*scaleFactor))){
        loadShaderScriptFlag = true;
    }

    if(shaderSliders.size() > 0){

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::PushItemWidth(224*scaleFactor);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(120,255,255,30));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(120,255,255,60));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(120,255,255,60));
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(120,255,255,160));
        for(size_t i=0;i<shaderSliders.size();i++){
            if(shaderSlidersType.at(i) == ShaderSliderType_FLOAT){
                ImGui::SliderFloat(shaderSlidersLabel.at(i).c_str(),&shaderSliders.at(i),0.0f,10.0f);
                this->setCustomVar(shaderSliders.at(i),"GUI_FLOAT_"+shaderSlidersLabel.at(i));
            }else if(shaderSlidersType.at(i) == ShaderSliderType_INT){
                ImGui::SliderFloat(shaderSlidersLabel.at(i).c_str(),&shaderSliders.at(i),0.0f,30.0f,"%.0f");
                this->setCustomVar(static_cast<float>(static_cast<int>(floor(shaderSliders.at(i)))),"GUI_INT_"+shaderSlidersLabel.at(i));
            }
        }
        ImGui::PopStyleColor(4);
        ImGui::PopItemWidth();

    }

    ImGuiEx::ObjectInfo(
                "This object is a GLSL ( #version 150 ) container, capable of loading shaders and editing them in real-time. You can type code with the Mosaic code editor, or with your default code editor. Scripts will refresh automatically on save.",
                "https://mosaic.d3cod3.org/reference.php?r=glsl-shader", scaleFactor);

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, loadShaderScriptFlag, "Select a GLSL shader", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".frag,.vert", "", scaleFactor)){
        lastShaderScript = fileDialog.selected_path;
        shaderScriptLoaded = true;
    }
}

//--------------------------------------------------------------
void ShaderObject::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void ShaderObject::doFragmentShader(){

    // Looks how many textures we need to inject from fragment shader
    int num = 0;
    for (int i = 0; i < 16; i++){
        string searchFor = "tex" + ofToString(i);
        if(static_cast<int>(fragmentShader.find(searchFor)) != -1)
            num++;
        else
            break;
    }

    vector<bool> tempInletsConn;
    for(int i=0;i<this->numInlets;i++){
        if(this->inletsConnected[i]){
            tempInletsConn.push_back(true);
        }else{
            tempInletsConn.push_back(false);
        }
    }

    // reset inlets
    this->inletsType.clear();
    this->inletsNames.clear();
    this->numInlets = num;

    textures.clear();
    for( int i = 0; i < num; i++){
        _inletParams[i] = new ofTexture();

        ofFbo* tempFBO = new ofFbo();
        tempFBO->allocate(output_width,output_height,internalFormat,4);
        tempFBO->begin();
        ofClear(0,0,0,255);
        tempFBO->end();
        textures.push_back(tempFBO);
    }

    reloading = false;
    nTextures = num;

    // INJECT TEXTURE(s) INLETS
    for( int i = 0; i < nTextures; i++){
        this->addInlet(VP_LINK_TEXTURE,"texture"+ofToString((i+1)));
    }

    shaderSliders.clear();
    shaderSlidersLabel.clear();
    shaderSlidersIndex.clear();
    shaderSlidersType.clear();

    // OBJECT CUSTOM STANDARD VARS
    map<string,float> tempVars = this->loadCustomVars();
    this->clearCustomVars();

    // INJECT SHADER FLOAT PARAMETER IN OBJECT GUI
    for (int i = 0; i < 10; i++){
        string searchFor = "param1f" + ofToString(i);
        if(fragmentShader.find(searchFor) != string::npos){
            unsigned long subVarStart = fragmentShader.find(searchFor);
            unsigned long subVarMiddle = fragmentShader.find(";//",subVarStart);
            unsigned long subVarEnd = fragmentShader.find("@",subVarStart);
            string varName = fragmentShader.substr(subVarMiddle+3,subVarEnd-subVarMiddle-3);
            float tempValue = 0.0f;
            map<string,float>::const_iterator it = tempVars.find("GUI_FLOAT_"+varName);
            if(it!=tempVars.end()){
                this->setCustomVar(it->second,"GUI_FLOAT_"+varName);
                tempValue = it->second;
            }else{
                this->setCustomVar(0.0f,"GUI_FLOAT_"+varName);
            }

            shaderSliders.push_back(tempValue);
            shaderSlidersIndex.push_back(i);
            shaderSlidersLabel.push_back(varName);
            shaderSlidersType.push_back(ShaderSliderType_FLOAT);

            _inletParams[this->numInlets] = new float();
            *(float *)&_inletParams[this->numInlets] = 0.0f;
            this->numInlets++;
            this->addInlet(VP_LINK_NUMERIC,varName);

        }else{
            break;
        }
    }
    // INJECT SHADER INT PARAMETER IN OBJECT GUI
    for (int i = 0; i < 10; i++){
        string searchFor = "param1i" + ofToString(i);
        if(fragmentShader.find(searchFor) != string::npos){
            unsigned long subVarStart = fragmentShader.find(searchFor);
            unsigned long subVarMiddle = fragmentShader.find(";//",subVarStart);
            unsigned long subVarEnd = fragmentShader.find("@",subVarStart);
            string varName = fragmentShader.substr(subVarMiddle+3,subVarEnd-subVarMiddle-3);
            float tempValue = 0.0f;
            map<string,float>::const_iterator it = tempVars.find("GUI_INT_"+varName);
            if(it!=tempVars.end()){
                this->setCustomVar(it->second,"GUI_INT_"+varName);
                tempValue = it->second;
            }else{
                this->setCustomVar(0.0f,"GUI_INT_"+varName);
            }
            shaderSliders.push_back(tempValue);
            shaderSlidersIndex.push_back(i);
            shaderSlidersLabel.push_back(varName);
            shaderSlidersType.push_back(ShaderSliderType_INT);

            _inletParams[this->numInlets] = new float();
            *(float *)&_inletParams[this->numInlets] = 0.0f;
            this->numInlets++;
            this->addInlet(VP_LINK_NUMERIC,varName);

        }else{
            break;
        }
    }

    this->inletsConnected.clear();
    for(int i=0;i<this->numInlets;i++){
        if(i<static_cast<int>(tempInletsConn.size())){
            if(tempInletsConn.at(i)){
                this->inletsConnected.push_back(true);
            }else{
                this->inletsConnected.push_back(false);
            }
        }else{
            this->inletsConnected.push_back(false);
        }
    }

    ofNotifyEvent(this->resetEvent, this->nId);

    this->saveConfig(false);

    // Compile the shader and load it to the GPU
    if (ofIsGLProgrammableRenderer()) {
      quad.clear();
    }
    shader->unload();

    if (!ofIsGLProgrammableRenderer()) {
        if(vertexShader != ""){
            shader->setupShaderFromSource(GL_VERTEX_SHADER, vertexShader);
        }
        shader->setupShaderFromSource(GL_FRAGMENT_SHADER, fragmentShader);
        scriptLoaded = shader->linkProgram();
    }else{
        size_t lastindex = filepath.find_last_of(".");
        string rawname = filepath.substr(0, lastindex);
        shader->load(rawname);
        scriptLoaded = shader->isLoaded();
    }

    if(scriptLoaded){
        ofLog(OF_LOG_NOTICE,"[verbose] SHADER: %s [%ix%i] loaded on GPU!",filepath.c_str(),output_width,output_height);
    }
}

//--------------------------------------------------------------
void ShaderObject::initResolution(){
    output_width = static_cast<int>(floor(this->getCustomVar("OUTPUT_WIDTH")));
    output_height = static_cast<int>(floor(this->getCustomVar("OUTPUT_HEIGHT")));

    fbo = new ofFbo();
    fbo->allocate(output_width,output_height,GL_RGBA32F_ARB,4);
    fbo->begin();
    ofClear(0,0,0,255);
    fbo->end();

    // init shader
    pingPong = new ofxPingPong();
    pingPong->allocate(output_width,output_height);

}

//--------------------------------------------------------------
void ShaderObject::resetResolution(int fromID, int newWidth, int newHeight){
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

        // init shader
        pingPong = new ofxPingPong();
        pingPong->allocate(output_width,output_height);

        if(filepath != "none"){
            loadScript(filepath);
        }

        needReset = true;
    }

}

//--------------------------------------------------------------
void ShaderObject::loadScript(string scriptFile){

    oneBang = true;

    // Get FRAGMENT_SHADER
    filepath = scriptFile;
    // Check if we have VERTEX_SHADER too
    ofFile tempCurrentFrag(scriptFile);
    string fsName = tempCurrentFrag.getFileName();
    string vsName = tempCurrentFrag.getEnclosingDirectory()+tempCurrentFrag.getFileName().substr(0,fsName.find_last_of('.'))+".vert";
    ofFile vertexShaderFile(vsName);

    this->setSpecialName("| "+fsName.substr(0,fsName.find_last_of('.')));

    currentScriptFile.open(filepath);

    if(currentScriptFile.exists()){
        ofBuffer fscontent = ofBufferFromFile(filepath);

        fragmentShader.clear();
        fragmentShader = fscontent.getText();

        if(vertexShaderFile.exists()){
            lastVertexShaderPath = vertexShaderFile.getAbsolutePath();
            ofBuffer vscontent = ofBufferFromFile(lastVertexShaderPath);
            vertexShader.clear();
            vertexShader = vscontent.getText();
        }

        ofShader test;
        if (ofIsGLProgrammableRenderer()) {
            size_t lastindex = filepath.find_last_of(".");
            string rawname = filepath.substr(0, lastindex);
            test.load(rawname);

            if(test.isLoaded()){
                test.unload();
                watcher.removeAllPaths();
                watcher.addPath(filepath);
                if(vertexShader != ""){
                    watcher.addPath(vertexShaderFile.getAbsolutePath());
                }
                doFragmentShader();
            }
        }else{
            if(vertexShader != ""){
                test.setupShaderFromSource(GL_VERTEX_SHADER, vertexShader);
            }
            test.setupShaderFromSource(GL_FRAGMENT_SHADER, fragmentShader);

            if(test.linkProgram()){
                watcher.removeAllPaths();
                watcher.addPath(filepath);
                if(vertexShader != ""){
                    watcher.addPath(vertexShaderFile.getAbsolutePath());
                }
                doFragmentShader();
            }
        }

    }else{
        ofLog(OF_LOG_ERROR,"SHADER File %s do not exists",currentScriptFile.getFileName().c_str());
    }

    oneBang = false;

}

//--------------------------------------------------------------
void ShaderObject::pathChanged(const PathWatcher::Event &event) {
    switch(event.change) {
        case PathWatcher::CREATED:
            //ofLogVerbose(PACKAGE) << "path created " << event.path;
            break;
        case PathWatcher::MODIFIED:
            //ofLogVerbose(PACKAGE) << "path modified " << event.path;
            if(!oneBang){
                currentScriptFile.open(ofToDataPath(event.path,true));
                if(ofToUpper(currentScriptFile.getExtension()) == "FRAG") {
                    filepath = currentScriptFile.getAbsolutePath();
                    loadScript(filepath);
                }else if(ofToUpper(currentScriptFile.getExtension()) == "VERT"){
                    string vsName = currentScriptFile.getFileName();
                    string fsName = currentScriptFile.getEnclosingDirectory()+currentScriptFile.getFileName().substr(0,vsName.find_last_of('.'))+".frag";
                    filepath = fsName;
                    loadScript(filepath);
                }
            }
            break;
        case PathWatcher::DELETED:
            //ofLogVerbose(PACKAGE) << "path deleted " << event.path;
            return;
        default: // NONE
            return;
    }

}

OBJECT_REGISTER( ShaderObject, "glsl shader", OFXVP_OBJECT_CAT_SCRIPTING)

#endif
