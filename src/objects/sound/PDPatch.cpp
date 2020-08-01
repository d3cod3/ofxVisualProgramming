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

#include "PDPatch.h"

//--------------------------------------------------------------
PDPatch::PDPatch() : PatchObject(){

    this->numInlets  = 5;
    this->numOutlets = 5;

    _inletParams[0] = new ofSoundBuffer(); // Audio stream IN 1
    _inletParams[1] = new ofSoundBuffer(); // Audio stream IN 2
    _inletParams[2] = new ofSoundBuffer(); // Audio stream IN 3
    _inletParams[3] = new ofSoundBuffer(); // Audio stream IN 4
    _inletParams[4] = new vector<float>(); // Data to PD

    _outletParams[0] = new ofSoundBuffer();  // Audio stream OUT 1
    _outletParams[1] = new ofSoundBuffer();  // Audio stream OUT 2
    _outletParams[2] = new ofSoundBuffer();  // Audio stream OUT 3
    _outletParams[3] = new ofSoundBuffer();  // Audio stream OUT 4
    _outletParams[4] = new vector<float>();  // Data to Mosaic

    this->initInletsState();

    pdIcon            = new ofImage();

    isNewObject         = false;

    isGUIObject         = true;
    this->isOverGUI     = true;

    isAudioINObject     = true;
    isAudioOUTObject    = true;

    lastLoadedPatch     = "";
    prevExternalsFolder = "/path_to_pd_externals";
    lastExternalsFolder = "";
    loadPatchFlag       = false;
    savePatchFlag       = false;
    setExternalFlag     = false;
    patchLoaded         = false;
    patchSaved          = false;
    externalPathSaved   = false;
    loading             = true;

    isPDSPPatchableObject   = true;

}

//--------------------------------------------------------------
void PDPatch::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_AUDIO,"audio in 1");
    this->addInlet(VP_LINK_AUDIO,"audio in 2");
    this->addInlet(VP_LINK_AUDIO,"audio in 3");
    this->addInlet(VP_LINK_AUDIO,"audio in 4");
    this->addInlet(VP_LINK_ARRAY,"data");
    this->addOutlet(VP_LINK_AUDIO,"audioOut1");
    this->addOutlet(VP_LINK_AUDIO,"audioOut2");
    this->addOutlet(VP_LINK_AUDIO,"audioOut3");
    this->addOutlet(VP_LINK_AUDIO,"audioOut4");
    this->addOutlet(VP_LINK_ARRAY,"data");

    this->setCustomVar(0.0f,"/path_to_pd_externals");
}

//--------------------------------------------------------------
void PDPatch::autoloadFile(string _fp){
    lastLoadedPatch = _fp;
    //lastLoadedPatch = copyFileToPatchFolder(this->patchFolderPath,_fp);
    loading = false;
    patchLoaded = true;
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

    newButton = gui->addButton("NEW");
    newButton->setUseCustomMouse(true);

    loadButton = gui->addButton("OPEN");
    loadButton->setUseCustomMouse(true);

    gui->addBreak();
    setExternalPath = gui->addButton("SET EXTERNALS PATH");
    setExternalPath->setUseCustomMouse(true);

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
void PDPatch::setupAudioOutObjectContent(pdsp::Engine &engine){

    ch1IN.out_signal() >> this->pdspIn[0] >> mixIN;
    ch2IN.out_signal() >> this->pdspIn[1] >> mixIN;
    ch3IN.out_signal() >> this->pdspIn[2] >> mixIN;
    ch4IN.out_signal() >> this->pdspIn[3] >> mixIN;

    ch1OUT.out_signal() >> this->pdspOut[0] >> mixOUT;
    ch2OUT.out_signal() >> this->pdspOut[1] >> mixOUT;
    ch3OUT.out_signal() >> this->pdspOut[2] >> mixOUT;
    ch4OUT.out_signal() >> this->pdspOut[3] >> mixOUT;

    mixIN >> scopeIN >> engine.blackhole();
    mixOUT >> scopeOUT >> engine.blackhole();
}

//--------------------------------------------------------------
void PDPatch::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    // GUI
    gui->update();
    header->update();
    newButton->update();
    loadButton->update();
    setExternalPath->update();

    if(loadPatchFlag){
        loadPatchFlag = false;
        //fd.openFile("load pd patch"+ofToString(this->getId()),"Select a PD patch");
    }

    if(savePatchFlag){
        savePatchFlag = false;
        string newFileName = "pdPatch_"+ofGetTimestampString("%y%m%d")+".pd";
        //fd.saveFile("save pd patch"+ofToString(this->getId()),"Save new PD patch as",newFileName);
    }

    if(patchLoaded){
        patchLoaded = false;
        ofFile file (lastLoadedPatch);
        if (file.exists()){
            string fileExtension = ofToUpper(file.getExtension());
            if(fileExtension == "PD") {
                filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
                //filepath = file.getAbsolutePath();
                loadPatch(filepath);
            }
        }
    }

    if(patchSaved){
        patchSaved = false;
        ofFile fileToRead(ofToDataPath("scripts/empty.pd"));
        ofFile newPDFile (lastLoadedPatch);
        ofFile::copyFromTo(fileToRead.getAbsolutePath(),checkFileExtension(newPDFile.getAbsolutePath(), ofToUpper(newPDFile.getExtension()), "PD"),true,true);
        //filepath = copyFileToPatchFolder(this->patchFolderPath,newPDFile.getAbsolutePath());
        filepath = checkFileExtension(newPDFile.getAbsolutePath(), ofToUpper(newPDFile.getExtension()), "PD");
        filepath = copyFileToPatchFolder(this->patchFolderPath,filepath);
        loadPatch(filepath);
    }

    if(setExternalFlag){
        setExternalFlag = false;
        //fd.openFolder("load pd external folder"+ofToString(this->getId()),"Select your PD external folder");
    }

    if(externalPathSaved){
        externalPathSaved = false;
        if(lastExternalsFolder != ""){
            ofFile tempfile (lastExternalsFolder);
            if(tempfile.exists() && tempfile.isDirectory()){
                string temp = tempfile.getAbsolutePath().substr(0, tempfile.getAbsolutePath().size()-1);
                pd.addToSearchPath(temp);
                this->substituteCustomVar(prevExternalsFolder,temp.c_str());
                prevExternalsFolder = lastExternalsFolder;
                ofLog(OF_LOG_NOTICE,"PD External set to: %s",temp.c_str());
                this->saveConfig(false);
            }
        }
    }

    if(pd.isInited() && pd.isComputingAudio() && currentPatch.isValid()){
        pd.startMessage();
        for(size_t s=0;s<static_cast<size_t>(static_cast<vector<float> *>(_inletParams[4])->size());s++){
            pd.addFloat(static_cast<vector<float> *>(_inletParams[4])->at(s));
        }
        pd.finishList("fromMosaic");
    }

    while(watcher.waitingEvents()) {
        pathChanged(watcher.nextEvent());
    }

    // update waveforms
    waveformIN.clear();
    for(size_t i = 0; i < scopeIN.getBuffer().size(); i++) {
        float sample = scopeIN.getBuffer().at(i);
        float x = ofMap(i, 0, scopeIN.getBuffer().size(), 0, this->width);
        float y = ofMap(hardClip(sample), -1, 1, headerHeight, this->height/2);
        waveformIN.addVertex(x, y);
    }

    waveformOUT.clear();
    for(size_t i = 0; i < scopeOUT.getBuffer().size(); i++) {
        float sample = scopeOUT.getBuffer().at(i);
        float x = ofMap(i, 0, scopeOUT.getBuffer().size(), 0, this->width);
        float y = ofMap(hardClip(sample), -1, 1, this->height/2, this->height);
        waveformOUT.addVertex(x, y);
    }

}

//--------------------------------------------------------------
void PDPatch::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    ofSetColor(255,100);
    pdIcon->draw(this->width/2,this->headerHeight,((this->height/2.2f)/pdIcon->getHeight())*pdIcon->getWidth(),this->height/2.2f);
    // Scopes
    ofSetColor(255);
    waveformIN.draw();
    waveformOUT.draw();
    // GUI
    ofSetColor(255);
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void PDPatch::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }

    pd.clear();

    if(removeFileFromData){
        removeFile(filepath);
    }
}

//--------------------------------------------------------------
/*void PDPatch::fileDialogResponse(ofxThreadedFileDialogResponse &response){
    if(response.id == "load pd patch"+ofToString(this->getId())){
        lastLoadedPatch = response.filepath;
        patchLoaded = true;
    }else if(response.id == "save pd patch"+ofToString(this->getId())){
        lastLoadedPatch = response.filepath;
        patchSaved = true;
    }else if(response.id == "load pd external folder"+ofToString(this->getId())){
        lastExternalsFolder = response.filepath;
        externalPathSaved = true;
    }
}*/

//--------------------------------------------------------------
void PDPatch::audioInObject(ofSoundBuffer &inputBuffer){
    if(pd.isInited() && pd.isComputingAudio() && currentPatch.isValid()){
        if(this->inletsConnected[0]){
            lastInputBuffer1 = *static_cast<ofSoundBuffer *>(_inletParams[0]);
        }else{
            lastInputBuffer1 *= 0.0f;
        }
        if(this->inletsConnected[1]){
            lastInputBuffer2 = *static_cast<ofSoundBuffer *>(_inletParams[1]);
        }else{
            lastInputBuffer2 *= 0.0f;
        }
        if(this->inletsConnected[2]){
            lastInputBuffer3 = *static_cast<ofSoundBuffer *>(_inletParams[2]);
        }else{
            lastInputBuffer3 *= 0.0f;
        }
        if(this->inletsConnected[3]){
            lastInputBuffer4 = *static_cast<ofSoundBuffer *>(_inletParams[3]);
        }else{
            lastInputBuffer4 *= 0.0f;
        }

        lastInputBuffer.setChannel(lastInputBuffer1,0);
        lastInputBuffer.setChannel(lastInputBuffer2,1);
        lastInputBuffer.setChannel(lastInputBuffer3,2);
        lastInputBuffer.setChannel(lastInputBuffer4,3);

        ch1IN.copyInput(lastInputBuffer1.getBuffer().data(),lastInputBuffer1.getNumFrames());
        ch2IN.copyInput(lastInputBuffer2.getBuffer().data(),lastInputBuffer2.getNumFrames());
        ch3IN.copyInput(lastInputBuffer3.getBuffer().data(),lastInputBuffer3.getNumFrames());
        ch4IN.copyInput(lastInputBuffer4.getBuffer().data(),lastInputBuffer4.getNumFrames());

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

    ch1OUT.copyInput(lastOutputBuffer1.getBuffer().data(),lastOutputBuffer1.getNumFrames());
    ch2OUT.copyInput(lastOutputBuffer2.getBuffer().data(),lastOutputBuffer2.getNumFrames());
    ch3OUT.copyInput(lastOutputBuffer3.getBuffer().data(),lastOutputBuffer3.getNumFrames());
    ch4OUT.copyInput(lastOutputBuffer4.getBuffer().data(),lastOutputBuffer4.getNumFrames());

    *static_cast<ofSoundBuffer *>(_outletParams[0]) = lastOutputBuffer1;
    *static_cast<ofSoundBuffer *>(_outletParams[1]) = lastOutputBuffer2;
    *static_cast<ofSoundBuffer *>(_outletParams[2]) = lastOutputBuffer3;
    *static_cast<ofSoundBuffer *>(_outletParams[3]) = lastOutputBuffer4;
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
        int totalObjects = XML.getNumTags("object");

        // Load object outlet config
        for(int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                if(XML.getValue("id", -1) == this->nId){
                    if (XML.pushTag("vars")){
                        int totalVars = XML.getNumTags("var");

                        for (int t=0;t<totalVars;t++){
                            if(XML.pushTag("var",t)){
                                prevExternalsFolder = XML.getValue("name","");
                                //ofLog(OF_LOG_NOTICE,"%s",prevExternalsFolder.c_str());
                                XML.popTag();
                            }
                        }

                        XML.popTag();
                    }
                }
                XML.popTag();
            }
        }
    }

    lastInputBuffer.allocate(bufferSize,4);
    lastInputBuffer1.allocate(bufferSize,1);
    lastInputBuffer2.allocate(bufferSize,1);
    lastInputBuffer3.allocate(bufferSize,1);
    lastInputBuffer4.allocate(bufferSize,1);

    lastOutputBuffer.allocate(bufferSize,4);
    lastOutputBuffer1.allocate(bufferSize,1);
    lastOutputBuffer2.allocate(bufferSize,1);
    lastOutputBuffer3.allocate(bufferSize,1);
    lastOutputBuffer4.allocate(bufferSize,1);

    pd.init(4,4,sampleRate,bufferSize/ofxPd::blockSize(),false);
    // load externals
#if defined(TARGET_LINUX) || defined(TARGET_OSX)
    cyclone_setup();
    zexy_setup();
#endif


    pd.subscribe("toMosaic");

    pd.addReceiver(*this);
    pd.addMidiReceiver(*this);

    short *shortBuffer = new short[bufferSize];
    for (int i = 0; i < bufferSize; i++){
        shortBuffer[i] = 0;
    }

    _inletParams[0] = new ofSoundBuffer(shortBuffer,static_cast<size_t>(bufferSize),1,static_cast<unsigned int>(sampleRate));
    _inletParams[1] = new ofSoundBuffer(shortBuffer,static_cast<size_t>(bufferSize),1,static_cast<unsigned int>(sampleRate));
    _inletParams[2] = new ofSoundBuffer(shortBuffer,static_cast<size_t>(bufferSize),1,static_cast<unsigned int>(sampleRate));
    _inletParams[3] = new ofSoundBuffer(shortBuffer,static_cast<size_t>(bufferSize),1,static_cast<unsigned int>(sampleRate));

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

    filepath = scriptFile;

    currentPatchFile.open(filepath);

    if(prevExternalsFolder != "" && prevExternalsFolder != "/path_to_pd_externals"){
        ofFile tempfile (prevExternalsFolder);
        if(tempfile.exists() && tempfile.isDirectory()){
            pd.addToSearchPath(tempfile.getAbsolutePath());
        }
    }

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
        if(e.target == newButton){
            savePatchFlag = true;
        }else if(e.target == loadButton){
            loadPatchFlag = true;
        }else if(e.target == setExternalPath){
            setExternalFlag = true;
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
    //ofLog(OF_LOG_NOTICE,"Mosaic: bang %s", dest.c_str());
}

//--------------------------------------------------------------
void PDPatch::receiveFloat(const std::string& dest, float value) {
    //ofLog(OF_LOG_NOTICE,"Mosaic: float %s: %f", dest.c_str(), value);
}

//--------------------------------------------------------------
void PDPatch::receiveSymbol(const std::string& dest, const std::string& symbol) {
    //ofLog(OF_LOG_NOTICE,"Mosaic: symbol %s: %s", dest.c_str(), symbol.c_str());
}

//--------------------------------------------------------------
void PDPatch::receiveList(const std::string& dest, const List& list) {
    //ofLog(OF_LOG_NOTICE,"Mosaic: list %s: ", dest.c_str());

    static_cast<vector<float> *>(_outletParams[4])->clear();
    static_cast<vector<float> *>(_outletParams[4])->assign(list.len(),0.0f);

    for(int i = 0; i < list.len(); ++i) {
        if(list.isFloat(i)){
            static_cast<vector<float> *>(_outletParams[4])->at(i) = list.getFloat(i);
        }
    }
}

//--------------------------------------------------------------
void PDPatch::receiveMessage(const std::string& dest, const std::string& msg, const List& list) {
    //ofLog(OF_LOG_NOTICE,"Mosaic: message %s: %s %s %s", dest.c_str(), msg.c_str(), list.toString().c_str(), list.types().c_str());
}

//--------------------------------------------------------------
void PDPatch::receiveNoteOn(const int channel, const int pitch, const int velocity) {
    //ofLog(OF_LOG_NOTICE,"Mosaic MIDI: note on: %i %i %i", channel, pitch, velocity);
}

//--------------------------------------------------------------
void PDPatch::receiveControlChange(const int channel, const int controller, const int value) {
    //ofLog(OF_LOG_NOTICE,"Mosaic MIDI: control change: %i %i %i", channel, controller, value);
}

//--------------------------------------------------------------
// note: pgm nums are 1-128 to match pd
void PDPatch::receiveProgramChange(const int channel, const int value) {
    //ofLog(OF_LOG_NOTICE,"Mosaic MIDI: program change: %i %i", channel, value);
}

//--------------------------------------------------------------
void PDPatch::receivePitchBend(const int channel, const int value) {
    //ofLog(OF_LOG_NOTICE,"Mosaic MIDI: pitch bend: %i %i", channel, value);
}

//--------------------------------------------------------------
void PDPatch::receiveAftertouch(const int channel, const int value) {
    //ofLog(OF_LOG_NOTICE,"Mosaic MIDI: aftertouch: %i %i", channel, value);
}

//--------------------------------------------------------------
void PDPatch::receivePolyAftertouch(const int channel, const int pitch, const int value) {
    //ofLog(OF_LOG_NOTICE,"Mosaic MIDI: poly aftertouch: %i %i %i", channel, pitch, value);
}

//--------------------------------------------------------------
// note: pd adds +2 to the port num, so sending to port 3 in pd to [midiout], shows up at port 1 in ofxPd
void PDPatch::receiveMidiByte(const int port, const int byte) {
    //ofLog(OF_LOG_NOTICE,"Mosaic MIDI: midi byte: %i %i", port, byte);
}

OBJECT_REGISTER( PDPatch, "pd patch", OFXVP_OBJECT_CAT_SOUND)

#endif
