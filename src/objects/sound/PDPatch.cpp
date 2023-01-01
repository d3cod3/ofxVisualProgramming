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

#include "PDPatch.h"

//--------------------------------------------------------------
PDPatch::PDPatch() : PatchObject("pd patch"){

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

    pdIcon              = new ofImage();
    posX = posY = drawW = drawH = 0.0f;

    isNewObject         = false;

    isAudioINObject     = true;
    isAudioOUTObject    = true;

    lastLoadedPatch     = "";
    loadPatchFlag       = false;
    savePatchFlag       = false;
    patchLoaded         = false;
    patchSaved          = false;
    loading             = true;

    isPDSPPatchableObject   = true;

    loaded              = false;

    this->setIsTextureObj(true);

}

//--------------------------------------------------------------
void PDPatch::newObject(){
    PatchObject::setName( this->objectName );
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
    unusedArgs(mainWindow);

    // init PD engine
    loadAudioSettings();

    // GUI
    pdIcon->load("images/pd.png");

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

}

//--------------------------------------------------------------
void PDPatch::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(patchLoaded){
        patchLoaded = false;
        ofFile file (lastLoadedPatch);
        if (file.exists()){
            string fileExtension = ofToUpper(file.getExtension());
            if(fileExtension == "PD") {
                filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
                //filepath = file.getAbsolutePath();
                loadPatch(filepath);
                this->saveConfig(false);
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
        this->saveConfig(false);
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

    if(!loaded){
        loaded = true;
    }

}

//--------------------------------------------------------------
void PDPatch::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    // draw node texture preview with OF
    if(scaledObjW*canvasZoom > 90.0f){
        drawNodeOFTexture(pdIcon->getTexture(), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
    }

}

//--------------------------------------------------------------
void PDPatch::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
    loadPatchFlag   = false;
    savePatchFlag   = false;

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){

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
    string newFileName = "pdPatch_"+ofGetTimestampString("%y%m%d")+".pd";
    if(ImGuiEx::getFileDialog(fileDialog, savePatchFlag, "Save new PD patch as", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ".pd", newFileName, scaleFactor)){
        lastLoadedPatch = fileDialog.selected_path;
        patchSaved = true;
    }

    if(ImGuiEx::getFileDialog(fileDialog, loadPatchFlag, "Select a PD patch", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".pd", "", scaleFactor)){
        lastLoadedPatch = fileDialog.selected_path;
        patchLoaded = true;
    }

}

//--------------------------------------------------------------
void PDPatch::drawObjectNodeConfig(){
    ofFile tempFilename(filepath);

    loadPatchFlag   = false;
    savePatchFlag   = false;

    ImGui::Spacing();
    ImGui::Text("Loaded PD Patch:");
    if(filepath == "none"){
        ImGui::Text("%s",filepath.c_str());
    }else{
        ImGui::Text("%s",tempFilename.getFileName().c_str());
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",tempFilename.getAbsolutePath().c_str());
    }

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();

    if(ImGui::Button("New",ImVec2(224*scaleFactor,26*scaleFactor))){
        savePatchFlag = true;
    }
    ImGui::Spacing();
    if(ImGui::Button("Open",ImVec2(224*scaleFactor,26*scaleFactor))){
        loadPatchFlag = true;
    }

    ImGuiEx::ObjectInfo(
                "Pure Data ( Pd-Vanilla ) patch container with inlets and outlets. As for live coding, with this object you can live patching, passing in real time and in both directions audio and data cables.",
                "https://mosaic.d3cod3.org/reference.php?r=pd-patch", scaleFactor);

    // file dialog
    string newFileName = "pdPatch_"+ofGetTimestampString("%y%m%d")+".pd";
    if(ImGuiEx::getFileDialog(fileDialog, savePatchFlag, "Save new PD patch as", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ".pd", newFileName, scaleFactor)){
        lastLoadedPatch = fileDialog.selected_path;
        patchSaved = true;
    }

    if(ImGuiEx::getFileDialog(fileDialog, loadPatchFlag, "Select a PD patch", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".pd", "", scaleFactor)){
        lastLoadedPatch = fileDialog.selected_path;
        patchLoaded = true;
    }

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
        //removeFile(filepath);
    }
}

//--------------------------------------------------------------
void PDPatch::audioInObject(ofSoundBuffer &inputBuffer){

}

//--------------------------------------------------------------
void PDPatch::audioOutObject(ofSoundBuffer &outputBuffer){

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

    currentPatch = pd.openPatch(currentPatchFile.getAbsolutePath());

    pd.addToSearchPath(currentPatchFile.getEnclosingDirectory());

    pd.start();

    if(currentPatch.isValid()){
        watcher.removeAllPaths();
        watcher.addPath(filepath);

        ofLog(OF_LOG_NOTICE,"[verbose] PD patch: %s loaded & running!",filepath.c_str());
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
