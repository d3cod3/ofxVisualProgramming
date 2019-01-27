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

#include "PDPatch.h"

//--------------------------------------------------------------
PDPatch::PDPatch() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();  // Audio stream

    this->initInletsState();

    pdIcon            = new ofImage();

    isNewObject         = false;

    isGUIObject         = true;
    this->isOverGUI     = true;

    isAudioINObject     = true;
    isAudioOUTObject    = true;

}

//--------------------------------------------------------------
void PDPatch::newObject(){
    this->setName("pd patch");
    this->addInlet(VP_LINK_AUDIO,"audio in");
    this->addOutlet(VP_LINK_AUDIO);
}

//--------------------------------------------------------------
void PDPatch::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    // init PD engine
    loadAudioSettings();

    // GUI
    pdIcon->load("images/pd.png");

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onButtonEvent(this, &PDPatch::onButtonEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    patchName = gui->addLabel("NONE");
    gui->addBreak();

    loadButton = gui->addButton("OPEN");
    loadButton->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    // Load patch
    watcher.start();
    if(filepath == "none"){
        isNewObject = true;
        ofFile file (ofToDataPath("scripts/empty.pd"));
        filepath = file.getAbsolutePath();
    }

    loadPatch(filepath);
}

//--------------------------------------------------------------
void PDPatch::updateObjectContent(map<int,PatchObject*> &patchObjects){

    // GUI
    gui->update();
    header->update();
    loadButton->update();

    while(watcher.waitingEvents()) {
        pathChanged(watcher.nextEvent());
    }

}

//--------------------------------------------------------------
void PDPatch::drawObjectContent(ofxFontStash *font){
    ofSetColor(255,150);
    ofEnableAlphaBlending();
    pdIcon->draw(this->width/2,this->headerHeight,((this->height/2.2f)/pdIcon->getHeight())*pdIcon->getWidth(),this->height/2.2f);
    // GUI
    ofSetColor(255);
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void PDPatch::removeObjectContent(){
    pd.clear();
}

//--------------------------------------------------------------
void PDPatch::audioInObject(ofSoundBuffer &inputBuffer){
    if(this->inletsConnected[0] && pd.isInited() && pd.isComputingAudio() && currentPatch.isValid()){
        lastInputBuffer = *static_cast<ofSoundBuffer *>(_inletParams[0]);
        pd.audioIn(lastInputBuffer.getBuffer().data(), lastInputBuffer.getNumFrames(), lastInputBuffer.getNumChannels());
    }
}

//--------------------------------------------------------------
void PDPatch::audioOutObject(ofSoundBuffer &outputBuffer){
    if(pd.isInited() && pd.isComputingAudio() && currentPatch.isValid()){
        pd.audioOut(lastOutputBuffer.getBuffer().data(), lastOutputBuffer.getNumFrames(), 1);
    }else{
        lastOutputBuffer *= 0.0f;
    }

    *static_cast<ofSoundBuffer *>(_outletParams[0]) = lastOutputBuffer;
}

//--------------------------------------------------------------
void PDPatch::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    loadButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || loadButton->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void PDPatch::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        loadButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void PDPatch::loadAudioSettings(){
    ofxXmlSettings XML;

    if (XML.loadFile(this->patchFile)){
        if (XML.pushTag("settings")){
            sampleRate = XML.getValue("sample_rate_in",0);
            bufferSize = XML.getValue("buffer_size",0);
            XML.popTag();
        }
    }

    lastOutputBuffer.allocate(bufferSize,1);

    pd.init(1,1,sampleRate,4,false);
    pd.addReceiver(*this);
    pd.addMidiReceiver(*this);

    short *shortBuffer = new short[bufferSize];
    for (int i = 0; i < bufferSize; i++){
        shortBuffer[i] = 0;
    }

    _outletParams[0] = new ofSoundBuffer(shortBuffer,static_cast<size_t>(bufferSize),1,static_cast<unsigned int>(sampleRate));

}

//--------------------------------------------------------------
void PDPatch::loadPatch(string scriptFile){

    if(currentPatchFile.exists()){
        pd.closePatch(currentPatch);
        pd.clearSearchPath();
        pd.stop();
    }

    filepath = forceCheckMosaicDataPath(scriptFile);
    currentPatchFile.open(filepath);
    currentPatch = pd.openPatch(currentPatchFile.getAbsolutePath());

    pd.addToSearchPath(currentPatchFile.getEnclosingDirectory());

    pd.start();

    if(currentPatch.isValid()){
        if(currentPatchFile.getFileName().size() > 22){
            patchName->setLabel(currentPatchFile.getFileName().substr(0,21)+"...");
        }else{
            patchName->setLabel(currentPatchFile.getFileName());
        }
        watcher.removeAllPaths();
        watcher.addPath(filepath);

        ofLog(OF_LOG_NOTICE,"[verbose] PD patch: %s loaded & running!",filepath.c_str());
    }

}

//--------------------------------------------------------------
void PDPatch::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == loadButton){
            ofFileDialogResult openFileResult= ofSystemLoadDialog("Select a PD patch");
            if (openFileResult.bSuccess){
                ofFile file (openFileResult.getPath());
                if (file.exists()){
                    string fileExtension = ofToUpper(file.getExtension());
                    if(fileExtension == "PD") {
                        filepath = file.getAbsolutePath();
                        loadPatch(filepath);
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------
void PDPatch::pathChanged(const PathWatcher::Event &event) {
    switch(event.change) {
        case PathWatcher::CREATED:
            //ofLogVerbose(PACKAGE) << "path created " << event.path;
            break;
        case PathWatcher::MODIFIED:
            //ofLogVerbose(PACKAGE) << "path modified " << event.path;
            filepath = event.path;
            loadPatch(filepath);
            break;
        case PathWatcher::DELETED:
            //ofLogVerbose(PACKAGE) << "path deleted " << event.path;
            return;
        default: // NONE
            return;
    }

}

//--------------------------------------------------------------
void PDPatch::print(const std::string& message) {
    ofLog(OF_LOG_NOTICE,"PD print: %s", message.c_str());
}
