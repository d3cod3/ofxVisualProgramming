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

    this->numInlets  = 2;
    this->numOutlets = 5;

    _inletParams[0] = new ofSoundBuffer(); // Audio stream IN
    _inletParams[1] = new vector<float>(); // Data to PD

    _outletParams[0] = new ofSoundBuffer();  // Audio stream 1
    _outletParams[1] = new ofSoundBuffer();  // Audio stream 2
    _outletParams[2] = new ofSoundBuffer();  // Audio stream 3
    _outletParams[3] = new ofSoundBuffer();  // Audio stream 4
    _outletParams[4] = new vector<float>();  // Data to Mosaic

    this->initInletsState();

    pdIcon            = new ofImage();

    isNewObject         = false;

    isGUIObject         = true;
    this->isOverGUI     = true;

    isAudioINObject     = true;
    isAudioOUTObject    = true;

    lastLoadedPatch     = "";
    loadPatchFlag       = false;
    patchLoaded         = false;

}

//--------------------------------------------------------------
void PDPatch::newObject(){
    this->setName("pd patch");
    this->addInlet(VP_LINK_AUDIO,"audio in");
    this->addInlet(VP_LINK_ARRAY,"data");
    this->addOutlet(VP_LINK_AUDIO);
    this->addOutlet(VP_LINK_AUDIO);
    this->addOutlet(VP_LINK_AUDIO);
    this->addOutlet(VP_LINK_AUDIO);
    this->addOutlet(VP_LINK_ARRAY);
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
void PDPatch::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){

    // GUI
    gui->update();
    header->update();
    loadButton->update();

    if(loadPatchFlag){
        loadPatchFlag = false;
        fd.openFile("load pd patch","Select a PD patch");
    }

    if(patchLoaded){
        patchLoaded = false;
        ofFile file (lastLoadedPatch);
        if (file.exists()){
            string fileExtension = ofToUpper(file.getExtension());
            if(fileExtension == "PD") {
                filepath = file.getAbsolutePath();
                loadPatch(filepath);
            }
        }
    }

    while(watcher.waitingEvents()) {
        pathChanged(watcher.nextEvent());
    }

}

//--------------------------------------------------------------
void PDPatch::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    ofSetColor(255,150);
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
void PDPatch::fileDialogResponse(ofxThreadedFileDialogResponse &response){
    if(response.id == "load pd patch"){
        lastLoadedPatch = response.filepath;
        patchLoaded = true;
    }
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
        pd.audioOut(lastOutputBuffer.getBuffer().data(), lastOutputBuffer.getNumFrames(), 4);
    }else{
        lastOutputBuffer *= 0.0f;
    }

    // separate global sound buffer to Left & Right
    lastOutputBuffer.getChannel(lastOutputBuffer1,0);
    lastOutputBuffer.getChannel(lastOutputBuffer2,1);
    lastOutputBuffer.getChannel(lastOutputBuffer3,2);
    lastOutputBuffer.getChannel(lastOutputBuffer4,3);

    *static_cast<ofSoundBuffer *>(_outletParams[0]) = lastOutputBuffer1;
    *static_cast<ofSoundBuffer *>(_outletParams[1]) = lastOutputBuffer2;
    *static_cast<ofSoundBuffer *>(_outletParams[2]) = lastOutputBuffer3;
    *static_cast<ofSoundBuffer *>(_outletParams[3]) = lastOutputBuffer4;
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

    lastOutputBuffer.allocate(bufferSize,4);
    lastOutputBuffer1.allocate(bufferSize,1);
    lastOutputBuffer2.allocate(bufferSize,1);
    lastOutputBuffer3.allocate(bufferSize,1);
    lastOutputBuffer4.allocate(bufferSize,1);

    pd.init(4,1,sampleRate,4,false);

    pd.subscribe("toMosaic");

    pd.addReceiver(*this);
    pd.addMidiReceiver(*this);

    short *shortBuffer = new short[bufferSize];
    for (int i = 0; i < bufferSize; i++){
        shortBuffer[i] = 0;
    }

    _outletParams[0] = new ofSoundBuffer(shortBuffer,static_cast<size_t>(bufferSize),1,static_cast<unsigned int>(sampleRate));
    _outletParams[1] = new ofSoundBuffer(shortBuffer,static_cast<size_t>(bufferSize),1,static_cast<unsigned int>(sampleRate));
    _outletParams[2] = new ofSoundBuffer(shortBuffer,static_cast<size_t>(bufferSize),1,static_cast<unsigned int>(sampleRate));
    _outletParams[3] = new ofSoundBuffer(shortBuffer,static_cast<size_t>(bufferSize),1,static_cast<unsigned int>(sampleRate));

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
            loadPatchFlag = true;
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
    ofLog(OF_LOG_NOTICE,"PD: print %s", message.c_str());
}

//--------------------------------------------------------------
void PDPatch::receiveBang(const std::string& dest) {
    ofLog(OF_LOG_NOTICE,"Mosaic: bang %s", dest.c_str());
}

//--------------------------------------------------------------
void PDPatch::receiveFloat(const std::string& dest, float value) {
    ofLog(OF_LOG_NOTICE,"Mosaic: float %s: %f", dest.c_str(), value);
}

//--------------------------------------------------------------
void PDPatch::receiveSymbol(const std::string& dest, const std::string& symbol) {
    ofLog(OF_LOG_NOTICE,"Mosaic: symbol %s: %s", dest.c_str(), symbol.c_str());
}

//--------------------------------------------------------------
void PDPatch::receiveList(const std::string& dest, const List& list) {
    ofLog(OF_LOG_NOTICE,"Mosaic: list %s: ", dest.c_str());

    // step through the list
    for(int i = 0; i < list.len(); ++i) {
        if(list.isFloat(i))
            ofLog(OF_LOG_NOTICE,"%f", list.getFloat(i));
        else if(list.isSymbol(i))
            ofLog(OF_LOG_NOTICE,"%s", list.getSymbol(i).c_str());
    }

    // print an OSC-style type string
    ofLog(OF_LOG_NOTICE,"Mosaic: list %s: ", list.types().c_str());
}

//--------------------------------------------------------------
void PDPatch::receiveMessage(const std::string& dest, const std::string& msg, const List& list) {
    ofLog(OF_LOG_NOTICE,"Mosaic: message %s: %s %s %s", dest.c_str(), msg.c_str(), list.toString().c_str(), list.types().c_str());
}

//--------------------------------------------------------------
void PDPatch::receiveNoteOn(const int channel, const int pitch, const int velocity) {
    ofLog(OF_LOG_NOTICE,"Mosaic MIDI: note on: %i %i %i", channel, pitch, velocity);
}

//--------------------------------------------------------------
void PDPatch::receiveControlChange(const int channel, const int controller, const int value) {
    ofLog(OF_LOG_NOTICE,"Mosaic MIDI: control change: %i %i %i", channel, controller, value);
}

//--------------------------------------------------------------
// note: pgm nums are 1-128 to match pd
void PDPatch::receiveProgramChange(const int channel, const int value) {
    ofLog(OF_LOG_NOTICE,"Mosaic MIDI: program change: %i %i", channel, value);
}

//--------------------------------------------------------------
void PDPatch::receivePitchBend(const int channel, const int value) {
    ofLog(OF_LOG_NOTICE,"Mosaic MIDI: pitch bend: %i %i", channel, value);
}

//--------------------------------------------------------------
void PDPatch::receiveAftertouch(const int channel, const int value) {
    ofLog(OF_LOG_NOTICE,"Mosaic MIDI: aftertouch: %i %i", channel, value);
}

//--------------------------------------------------------------
void PDPatch::receivePolyAftertouch(const int channel, const int pitch, const int value) {
    ofLog(OF_LOG_NOTICE,"Mosaic MIDI: poly aftertouch: %i %i %i", channel, pitch, value);
}

//--------------------------------------------------------------
// note: pd adds +2 to the port num, so sending to port 3 in pd to [midiout], shows up at port 1 in ofxPd
void PDPatch::receiveMidiByte(const int port, const int byte) {
    ofLog(OF_LOG_NOTICE,"Mosaic MIDI: midi byte: %i %i", port, byte);
}
