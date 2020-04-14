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
ShaderObject::ShaderObject() : PatchObject(){

    this->numInlets  = 0;
    this->numOutlets = 1;

    _outletParams[0] = new ofTexture();     // output

    scriptLoaded        = false;
    isNewObject         = false;
    reloading           = false;

    isGUIObject         = true;
    this->isOverGUI     = true;

    fbo         = new ofFbo();
    pingPong    = new ofxPingPong();
    shader      = new ofShader();
    needReset   = false;

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

    modalInfo               = false;

}

//--------------------------------------------------------------
void ShaderObject::newObject(){
    this->setName(this->objectName);
    this->addOutlet(VP_LINK_TEXTURE,"output");

    this->setCustomVar(static_cast<float>(output_width),"OUTPUT_WIDTH");
    this->setCustomVar(static_cast<float>(output_height),"OUTPUT_HEIGHT");
}

//--------------------------------------------------------------
void ShaderObject::autoloadFile(string _fp){
    lastShaderScript = _fp;
    //lastShaderScript = copyFileToPatchFolder(this->patchFolderPath,_fp);
    shaderScriptLoaded = true;
}

//--------------------------------------------------------------
void ShaderObject::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    initResolution();

    // load kuro
    kuro->load("images/kuro.jpg");

    // Setup ThreadedCommand var
    tempCommand.setup();

    // init path watcher
    watcher.start();

    if(filepath == "none"){
        currentScriptFile.open(ofToDataPath("scripts/empty.frag"));
        //filepath = currentScriptFile.getAbsolutePath();
        filepath = copyFileToPatchFolder(this->patchFolderPath,currentScriptFile.getAbsolutePath());
        isNewObject = true;
    }
    loadScript(filepath);

}

//--------------------------------------------------------------
void ShaderObject::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    if(tempCommand.getCmdExec() && tempCommand.getSysStatus() != 0 && !modalInfo){
        modalInfo = true;
        fd.notificationPopup("Mosaic files editing","Mosaic works better with Atom [https://atom.io/] text editor, and it seems you do not have it installed on your system.");
    }

    // Recursive reset for shader objects chain
    if(needReset){
        needReset = false;
        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if(patchObjects[it->first] != nullptr && it->first != this->getId() && !patchObjects[it->first]->getWillErase()){
                for(int o=0;o<static_cast<int>(it->second->outPut.size());o++){
                    if(!it->second->outPut[o]->isDisabled && it->second->outPut[o]->toObjectID == this->getId()){
                        if(it->second->getName() == "lua script" || it->second->getName() == "python script" || it->second->getName() == "shader object"){
                            it->second->resetResolution(this->getId(),output_width,output_height);
                        }
                    }
                }
            }
        }
    }

    // GUI
    gui->update();
    header->update();
    newButton->update();
    loadButton->update();
    //editButton->update();
    for(size_t i=0;i<shaderSliders.size();i++){
        shaderSliders.at(i)->update();
    }

    if(loadShaderScriptFlag){
        loadShaderScriptFlag = false;
        fd.openFile("load shader"+ofToString(this->getId()),"Select a shader");
    }

    if(saveShaderScriptFlag){
        saveShaderScriptFlag = false;
        string newFileName = "shader_"+ofGetTimestampString("%y%m%d")+".frag";
        fd.saveFile("save shader"+ofToString(this->getId()),"Save new GLSL shader as",newFileName);
    }

    if(shaderScriptLoaded){
        shaderScriptLoaded = false;
        // open the new one
        currentScriptFile.open(lastShaderScript);
        if (currentScriptFile.exists()){
            string fileExtension = ofToUpper(currentScriptFile.getExtension());
            if(fileExtension == "FRAG") {
                //filepath = currentScriptFile.getAbsolutePath();
                filepath = copyFileToPatchFolder(this->patchFolderPath,currentScriptFile.getAbsolutePath());
                loadScript(filepath);
                reloading = true;
            }else if(fileExtension == "VERT"){
                string vsName = currentScriptFile.getFileName();
                string fsName = currentScriptFile.getEnclosingDirectory()+currentScriptFile.getFileName().substr(0,vsName.find_last_of('.'))+".frag";
                filepath = fsName;
                filepath = copyFileToPatchFolder(this->patchFolderPath,filepath);
                loadScript(filepath);
                reloading = true;
            }
        }
    }

    if(shaderScriptSaved){
        shaderScriptSaved = false;
        // create and open the new one
        ofFile fileToRead(ofToDataPath("scripts/empty.frag"));
        ofFile newGLSLFile (lastShaderScript);
        ofFile::copyFromTo(fileToRead.getAbsolutePath(),checkFileExtension(newGLSLFile.getAbsolutePath(), ofToUpper(newGLSLFile.getExtension()), "FRAG"),true,true);
        ofFile correctedFileToRead(checkFileExtension(newGLSLFile.getAbsolutePath(), ofToUpper(newGLSLFile.getExtension()), "FRAG"));
        currentScriptFile = correctedFileToRead;
        if (currentScriptFile.exists()){
            string fileExtension = ofToUpper(currentScriptFile.getExtension());
            if(fileExtension == "FRAG") {
                // filepath = currentScriptFile.getAbsolutePath();
                filepath = copyFileToPatchFolder(this->patchFolderPath,currentScriptFile.getAbsolutePath());
                loadScript(filepath);
                reloading = true;
            }else if(fileExtension == "VERT"){
                string vsName = currentScriptFile.getFileName();
                string fsName = currentScriptFile.getEnclosingDirectory()+currentScriptFile.getFileName().substr(0,vsName.find_last_of('.'))+".frag";
                filepath = fsName;
                filepath = copyFileToPatchFolder(this->patchFolderPath,filepath);
                loadScript(filepath);
                reloading = true;
            }
        }
    }

    while(watcher.waitingEvents()) {
        pathChanged(watcher.nextEvent());
    }

    ///////////////////////////////////////////
    // SHADER UPDATE
    if(scriptLoaded){
        // receive external data
        for(int i=0;i<this->numInlets;i++){
            if(this->inletsConnected[i] && this->getInletType(i) == VP_LINK_TEXTURE && i < static_cast<int>(textures.size()) && static_cast<ofTexture *>(_inletParams[i])->isAllocated()){
                textures[i]->begin();
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
                shaderSliders.at(i-static_cast<int>(textures.size()))->setValue(*(float *)&_inletParams[i]);
            }
        }

        // set custom shader vars
        string paramName, tempVarName;
        for(size_t i=0;i<shaderSliders.size();i++){
            if(shaderSliders.at(i)->getPrecision() != 0){// FLOAT
                paramName = "param1f"+ofToString(shaderSlidersIndex[i]);
                tempVarName = "GUI_FLOAT_"+shaderSliders.at(i)->getLabel();
                shader->setUniform1f(paramName.c_str(), static_cast<float>(this->getCustomVar(tempVarName)));
            }else{ // INT
                paramName = "param1i"+ofToString(shaderSlidersIndex[i]);
                tempVarName = "GUI_INT_"+shaderSliders.at(i)->getLabel();
                shader->setUniform1i(paramName.c_str(), static_cast<int>(floor(this->getCustomVar(tempVarName))));
            }
        }

        ofSetColor(255,255);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
        glTexCoord2f(output_width, 0); glVertex3f(output_width, 0, 0);
        glTexCoord2f(output_width, output_height); glVertex3f(output_width, output_height, 0);
        glTexCoord2f(0,output_height);  glVertex3f(0,output_height, 0);
        glEnd();

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
}

//--------------------------------------------------------------
void ShaderObject::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
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
    // GUI
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void ShaderObject::removeObjectContent(bool removeFileFromData){
    tempCommand.stop();

    if(currentScriptFile.getAbsolutePath() != ofToDataPath("scripts/empty.frag",true) && currentScriptFile.exists() && removeFileFromData){
        removeFile(filepath);
        //ofLog(OF_LOG_NOTICE,"%s",lastVertexShaderPath.c_str());
        /*if(lastVertexShaderPath != ofToDataPath("scripts/empty.vert",true)){
            removeFile(lastVertexShaderPath);
        }*/
    }

}

//--------------------------------------------------------------
void ShaderObject::mouseMovedObjectContent(ofVec3f _m){
    int testingOver = 0;
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    newButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    loadButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    //editButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    for(size_t i=0;i<shaderSliders.size();i++){
        shaderSliders.at(i)->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        if(shaderSliders.at(i)->hitTest(_m-this->getPos())){
            testingOver++;
        }
    }

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || newButton->hitTest(_m-this->getPos()) || loadButton->hitTest(_m-this->getPos()) || testingOver>0;
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void ShaderObject::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        for(size_t i=0;i<shaderSliders.size();i++){
            shaderSliders.at(i)->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        }
    }else{
        

        box->setFromCenter(_m.x, _m.y,box->getWidth(),box->getHeight());
        headerBox->set(box->getPosition().x,box->getPosition().y,box->getWidth(),headerHeight);

        x = box->getPosition().x;
        y = box->getPosition().y;

        for(int j=0;j<static_cast<int>(outPut.size());j++){
            // (outPut[j]->posFrom.x,outPut[j]->posFrom.y);
            // (outPut[j]->posFrom.x+20,outPut[j]->posFrom.y);
        }
    }
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

    if (num != nTextures || reloading || isNewObject){
        reloading = false;
        nTextures = num;

        this->inlets.clear();
        this->inletsNames.clear();
        this->inletsPositionOF.clear();

        // add texture(s) inlets
        for( int i = 0; i < nTextures; i++){
            this->addInlet(VP_LINK_TEXTURE,"texture"+ofToString((i+1)));
        }

    }

    loadGUI();

    shaderSliders.clear();
    shaderSlidersIndex.clear();

    // OBJECT CUSTOM STANDARD VARS
    map<string,float> tempVars = this->loadCustomVars();
    this->clearCustomVars();

    // INJECT SHADER FLOAT PARAMETER IN OBJECT GUI
    for (int i = 0; i < 10; i++){
        string searchFor = "param1f" + ofToString(i);
        if(fragmentShader.find(searchFor) != static_cast<unsigned long>(-1)){
            unsigned long subVarStart = fragmentShader.find(searchFor);
            unsigned long subVarMiddle = fragmentShader.find(";//",subVarStart);
            unsigned long subVarEnd = fragmentShader.find("@",subVarStart);
            string varName = fragmentShader.substr(subVarMiddle+3,subVarEnd-subVarMiddle-3);
            ofxDatGuiSlider* tempSlider = gui->addSlider(varName,0.0f,10.0f,0.0f);
            tempSlider->setUseCustomMouse(true);
            map<string,float>::const_iterator it = tempVars.find("GUI_FLOAT_"+varName);
            if(it!=tempVars.end()){
                this->setCustomVar(it->second,"GUI_FLOAT_"+varName);
                tempSlider->setValue(it->second);
            }else{
                this->setCustomVar(0.0f,"GUI_FLOAT_"+varName);
            }

            shaderSliders.push_back(tempSlider);
            shaderSlidersIndex.push_back(i);

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
        if(fragmentShader.find(searchFor) != static_cast<unsigned long>(-1)){
            unsigned long subVarStart = fragmentShader.find(searchFor);
            unsigned long subVarMiddle = fragmentShader.find(";//",subVarStart);
            unsigned long subVarEnd = fragmentShader.find("@",subVarStart);
            string varName = fragmentShader.substr(subVarMiddle+3,subVarEnd-subVarMiddle-3);
            ofxDatGuiSlider* tempSlider = gui->addSlider(varName,0,30,0);
            tempSlider->setUseCustomMouse(true);
            tempSlider->setPrecision(0);
            map<string,float>::const_iterator it = tempVars.find("GUI_INT_"+varName);
            if(it!=tempVars.end()){
                this->setCustomVar(it->second,"GUI_INT_"+varName);
                tempSlider->setValue(it->second);
            }else{
                this->setCustomVar(0.0f,"GUI_INT_"+varName);
            }
            shaderSliders.push_back(tempSlider);
            shaderSlidersIndex.push_back(i);

            _inletParams[this->numInlets] = new float();
            *(float *)&_inletParams[this->numInlets] = 0.0f;
            this->numInlets++;
            this->addInlet(VP_LINK_NUMERIC,varName);

        }else{
            break;
        }
    }
    gui->setWidth(this->width);

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

    this->saveConfig(false,this->nId);

    // Compile the shader and load it to the GPU
    shader->unload();
    if(vertexShader != ""){
        shader->setupShaderFromSource(GL_VERTEX_SHADER, vertexShader);
    }
    shader->setupShaderFromSource(GL_FRAGMENT_SHADER, fragmentShader);
    scriptLoaded = shader->linkProgram();

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
        this->saveConfig(false,this->nId);

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
void ShaderObject::fileDialogResponse(ofxThreadedFileDialogResponse &response){
    if(response.id == "load shader"+ofToString(this->getId())){
        lastShaderScript = response.filepath;
        shaderScriptLoaded = true;
    }else if(response.id == "save shader"+ofToString(this->getId())){
        lastShaderScript = response.filepath;
        shaderScriptSaved = true;
    }
}

//--------------------------------------------------------------
void ShaderObject::loadGUI(){
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    //gui->setWidth(this->width);
    gui->onButtonEvent(this, &ShaderObject::onButtonEvent);
    gui->onSliderEvent(this, &ShaderObject::onSliderEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    shaderName = gui->addLabel("NONE");
    ofFile tempFile(filepath);
    if(tempFile.getFileName().size() > 22){
        shaderName->setLabel(tempFile.getFileName().substr(0,21)+"...");
    }else{
        shaderName->setLabel(tempFile.getFileName());
    }
    gui->addBreak();

    newButton = gui->addButton("NEW");
    newButton->setUseCustomMouse(true);

    loadButton = gui->addButton("OPEN");
    loadButton->setUseCustomMouse(true);

    //editButton = gui->addButton("EDIT");
    //editButton->setUseCustomMouse(true);
    gui->addBreak();

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);
}

//--------------------------------------------------------------
void ShaderObject::loadScript(string scriptFile){

    oneBang = true;

    // Get FRAGMENT_SHADER
    //filepath = copyFileToPatchFolder(this->patchFolderPath,scriptFile);
    filepath = scriptFile;
    // Check if we have VERTEX_SHADER too
    ofFile tempCurrentFrag(scriptFile);
    string fsName = tempCurrentFrag.getFileName();
    string vsName = tempCurrentFrag.getEnclosingDirectory()+tempCurrentFrag.getFileName().substr(0,fsName.find_last_of('.'))+".vert";
    ofFile vertexShaderFile(vsName);

    currentScriptFile.open(filepath);

    if(currentScriptFile.exists()){
        ofBuffer fscontent = ofBufferFromFile(filepath);

        fragmentShader = fscontent.getText();

        if(vertexShaderFile.exists()){
            lastVertexShaderPath = vertexShaderFile.getAbsolutePath();
            ofBuffer vscontent = ofBufferFromFile(lastVertexShaderPath);
            vertexShader = vscontent.getText();
        }

        ofShader test;
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
    }else{
        ofLog(OF_LOG_ERROR,"SHADER File %s do not exists",currentScriptFile.getFileName().c_str());
    }

    oneBang = false;

}

//--------------------------------------------------------------
void ShaderObject::onSliderEvent(ofxDatGuiSliderEvent e){
    for(size_t i=0;i<shaderSliders.size();i++){
        if(e.target == shaderSliders.at(i)){
            string sliderName = shaderSliders.at(i)->getLabel();
            if(shaderSliders.at(i)->getPrecision() == 0){// INT
                sliderName = "GUI_INT_"+sliderName;
            }else{ // FLOAT
                sliderName = "GUI_FLOAT_"+sliderName;
            }
            this->setCustomVar(static_cast<float>(shaderSliders.at(i)->getValue()),sliderName);
            this->saveConfig(false,this->nId);
        }
    }
}

//--------------------------------------------------------------
void ShaderObject::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == newButton){
            saveShaderScriptFlag = true;
        }else if(e.target == loadButton){
            loadShaderScriptFlag = true;
        }
    }
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

OBJECT_REGISTER( ShaderObject, "shader object", OFXVP_OBJECT_CAT_SCRIPTING)

#endif
