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
    isOverGui           = true;

    fbo         = new ofFbo();
    pingPong    = new ofxPingPong();
    shader      = new ofShader();

    kuro        = new ofImage();

    scaleH = 0.0f;

    output_width    = 320;
    output_height   = 240;

    nTextures       = 0;
    internalFormat  = GL_RGBA;

    fragmentShader  = "";

}

//--------------------------------------------------------------
void ShaderObject::newObject(){
    this->setName("shader object");
    this->addOutlet(VP_LINK_TEXTURE);
}

//--------------------------------------------------------------
void ShaderObject::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadProjectorSettings();

    // load kuro
    kuro->load("images/kuro.jpg");

    // Setup ThreadedCommand var
    tempCommand.setup();

    // init output texture container
    fbo->allocate(output_width,output_height,GL_RGBA32F_ARB);
    fbo->begin();
    ofClear(255,255,255, 0);
    fbo->end();

    // init shader
    pingPong->allocate(output_width,output_height);

    // init path watcher
    watcher.start();

    if(filepath != "none"){
        loadScript(filepath);
    }else{
        isNewObject = true;
    }

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth((this->width/3 * 2) + 1);
    gui->onButtonEvent(this, &ShaderObject::onButtonEvent);

    loadButton = gui->addButton("OPEN");
    loadButton->setUseCustomMouse(true);

    editButton = gui->addButton("EDIT");
    editButton->setUseCustomMouse(true);

    gui->setPosition(this->width/3,this->height - (loadButton->getHeight()*2));
}

//--------------------------------------------------------------
void ShaderObject::updateObjectContent(map<int,PatchObject*> &patchObjects){
    // GUI
    gui->update();
    loadButton->update();
    editButton->update();

    while(watcher.waitingEvents()) {
        pathChanged(watcher.nextEvent());
    }

    ///////////////////////////////////////////
    // SHADER UPDATE
    if(scriptLoaded){
        // receive external data
        for(int i=0;i<this->numInlets;i++){
            if(this->inletsConnected[i]){
                textures[i]->begin();
                static_cast<ofTexture *>(_inletParams[i])->draw(0,0,output_width, output_height);
                textures[i]->end();
            }
        }
        pingPong->dst->begin();

        ofClear(0);
        shader->begin();
        shader->setUniformTexture("backbuffer", pingPong->src->getTexture(),0);
        for(int i=0;i<textures.size();i++){
            string texName = "tex" + ofToString(i);
            shader->setUniformTexture(texName.c_str(),textures[i]->getTexture(),i+1);
        }
        shader->setUniform2f("resolution",static_cast<float>(output_width),static_cast<float>(output_height));
        shader->setUniform1f("time",static_cast<float>(ofGetElapsedTimef()));

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
void ShaderObject::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    scaleH = (this->width/fbo->getWidth())*fbo->getHeight();
    static_cast<ofTexture *>(_outletParams[0])->draw(0,this->height/2 - scaleH/2,this->width,scaleH);
    // GUI
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void ShaderObject::removeObjectContent(){
    
}

//--------------------------------------------------------------
void ShaderObject::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    loadButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    editButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    isOverGui = loadButton->hitTest(_m-this->getPos());
}

//--------------------------------------------------------------
void ShaderObject::dragGUIObject(ofVec3f _m){
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
void ShaderObject::doFragmentShader(){
    ofBuffer content = ofBufferFromFile(filepath);

    fragmentShader = content.getText();
    // Looks how many textures it´s need on the injected fragment shader
    int num = 0;
    for (int i = 0; i < 16; i++){
        string searchFor = "tex" + ofToString(i);
        if (fragmentShader.find(searchFor)!= -1)
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
        tempFBO->allocate(output_width,output_height,internalFormat);
        tempFBO->begin();
        ofClear(0,0,0,255);
        tempFBO->end();
        textures.push_back(tempFBO);
    }

    // Check if it´s the same number of textures already created and allocated
    if (num != nTextures && (reloading || isNewObject)){

        nTextures = num;

        this->inlets.clear();
        this->inletsNames.clear();

        ofNotifyEvent(this->resetEvent, this->nId);

        for( int i = 0; i < nTextures; i++){
            this->addInlet(VP_LINK_TEXTURE,"texture"+ofToString((i+1)));
        }

    }

    this->inletsConnected.clear();
    for(int i=0;i<this->numInlets;i++){
        if(i<tempInletsConn.size()){
            if(tempInletsConn.at(i)){
                this->inletsConnected.push_back(true);
            }else{
                this->inletsConnected.push_back(false);
            }
        }else{
            this->inletsConnected.push_back(false);
        }
    }

    // Compile the shader and load it to the GPU
    shader->unload();
    shader->setupShaderFromSource(GL_FRAGMENT_SHADER, fragmentShader);
    scriptLoaded = shader->linkProgram();

    if(scriptLoaded){
        ofLog(OF_LOG_NOTICE,"[verbose] SHADER: %s loaded on GPU!",filepath.c_str());
    }
}

//--------------------------------------------------------------
bool ShaderObject::loadProjectorSettings(){
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
void ShaderObject::loadScript(string scriptFile){

    filepath = scriptFile;

    ofShader test;
    test.setupShaderFromFile(GL_FRAGMENT_SHADER, filepath);

    if(test.linkProgram()){
        watcher.removeAllPaths();
        watcher.addPath(filepath);
        doFragmentShader();
    }

    if(!reloading){
        reloading = true;
    }

}

//--------------------------------------------------------------
void ShaderObject::onButtonEvent(ofxDatGuiButtonEvent e){
    if(e.target == loadButton){
        ofFileDialogResult openFileResult= ofSystemLoadDialog("Select a shader");
        if (openFileResult.bSuccess){
            ofFile file (openFileResult.getPath());
            if (file.exists()){
                string fileExtension = ofToUpper(file.getExtension());
                if(fileExtension == "FS" || fileExtension == "FRAG") {
                    filepath = file.getAbsolutePath();
                    loadScript(filepath);
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

            if(tempCommand.getSysStatus() != 0){ // error
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
            filepath = event.path;
            loadScript(filepath);
            break;
        case PathWatcher::DELETED:
            //ofLogVerbose(PACKAGE) << "path deleted " << event.path;
            return;
        default: // NONE
            return;
    }

}
