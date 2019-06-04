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

#include "SoundfilePlayer.h"

//--------------------------------------------------------------
SoundfilePlayer::SoundfilePlayer() : PatchObject(){

    this->numInlets  = 5;
    this->numOutlets = 2;

    _inletParams[0] = new string();  // control
    *static_cast<string *>(_inletParams[0]) = "";
    _inletParams[1] = new float();  // playhead
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();  // speed
    *(float *)&_inletParams[2] = 0.0f;
    _inletParams[3] = new float();  // volume
    *(float *)&_inletParams[3] = 0.0f;
    _inletParams[4] = new float();  // trigger
    *(float *)&_inletParams[4] = 0.0f;

    _outletParams[0] = new ofSoundBuffer();  // signal
    _outletParams[1] = new vector<float>(); // audio buffer

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    isAudioOUTObject    = true;

    isNewObject         = false;
    isFileLoaded        = false;
    isPlaying           = false;
    audioWasPlaying     = false;
    lastMessage         = "";

    loop                = false;
    volume              = 1.0f;
    speed               = 1.0;
    sampleRate          = 44100.0;
    bufferSize          = 256;

    lastSoundfile       = "";
    loadSoundfileFlag   = false;
    soundfileLoaded     = false;

    isPDSPPatchableObject   = true;

    this->width         *= 2;
    
}

//--------------------------------------------------------------
void SoundfilePlayer::newObject(){
    this->setName("soundfile player");
    this->addInlet(VP_LINK_STRING,"control");
    this->addInlet(VP_LINK_NUMERIC,"playhead");
    this->addInlet(VP_LINK_NUMERIC,"speed");
    this->addInlet(VP_LINK_NUMERIC,"volume");
    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addOutlet(VP_LINK_AUDIO,"audioFileSignal");
    this->addOutlet(VP_LINK_ARRAY,"dataBuffer");
}

//--------------------------------------------------------------
void SoundfilePlayer::autoloadFile(string _fp){
    lastSoundfile = _fp;
    soundfileLoaded = true;
    startTime = ofGetElapsedTimeMillis();
}

//--------------------------------------------------------------
void SoundfilePlayer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    loadSettings();

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onButtonEvent(this, &SoundfilePlayer::onButtonEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    soundfileName = gui->addLabel("NONE");
    gui->addBreak();
    loadButton = gui->addButton("OPEN");
    loadButton->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    loading = true;
}

//--------------------------------------------------------------
void SoundfilePlayer::setupAudioOutObjectContent(pdsp::Engine &engine){

    fileOUT.out_signal() >> this->pdspOut[0];
    fileOUT.out_signal() >> scope >> engine.blackhole();

    startTime   = ofGetElapsedTimeMillis();
}

//--------------------------------------------------------------
void SoundfilePlayer::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){
    gui->update();
    header->update();
    loadButton->update();

    if(loadSoundfileFlag){
        loadSoundfileFlag = false;
        fd.openFile("load soundfile"+ofToString(this->getId()),"Select an audio file");
    }

    if(soundfileLoaded && ofGetElapsedTimeMillis()-startTime > 100){
        soundfileLoaded = false;
        ofFile file (lastSoundfile);
        if (file.exists()){
            string fileExtension = ofToUpper(file.getExtension());
            if(fileExtension == "WAV" || fileExtension == "OGG" || fileExtension == "MP3" || fileExtension == "FLAC") {
                loadAudioFile(file.getAbsolutePath());
            }
        }
    }

    if(loading && ofGetElapsedTimeMillis()-startTime > 500){
        loading = false;
        if(filepath != "none"){
            loadAudioFile(filepath);
        }else{
            isNewObject = true;
        }
    }

    if(!isFileLoaded && audiofile.loaded() && audiofile.samplerate() > 100){
        isFileLoaded = true;
        ofLog(OF_LOG_NOTICE,"[verbose] sound file loaded: %s, Sample Rate: %s, Audiofile length: %s",filepath.c_str(), ofToString(audiofile.samplerate()).c_str(), ofToString(audiofile.length()).c_str());
    }

    if(isFileLoaded && audiofile.loaded()){
        // listen to message control (_inletParams[0])
        if(this->inletsConnected[0]){
            if(lastMessage != *static_cast<string *>(_inletParams[0])){
                lastMessage = *static_cast<string *>(_inletParams[0]);

                if(lastMessage == "play"){
                    isPlaying = true;
                    playhead = 0.0;
                    audioWasPlaying = true;
                }else if(lastMessage == "pause"){
                    isPlaying = false;
                }else if(lastMessage == "unpause"){
                    if(audioWasPlaying){
                        isPlaying = true;
                    }
                }else if(lastMessage == "stop"){
                    isPlaying = false;
                    playhead = 0.0;
                    audioWasPlaying = false;
                }else if(lastMessage == "loop_normal"){
                    loop = true;
                }else if(lastMessage == "loop_none"){
                    loop = false;
                }
            }
        }
        // playhead
        if(this->inletsConnected[1] && *(float *)&_inletParams[1] != -1.0f){
            playhead = static_cast<double>(*(float *)&_inletParams[1]) * audiofile.length();
        }
        // speed
        if(this->inletsConnected[2]){
            speed = static_cast<double>(ofClamp(*(float *)&_inletParams[2],-10.0f,10.0f));
        }else{
            speed = 1.0;
        }
        // volume
        if(this->inletsConnected[3]){
            volume = ofClamp(*(float *)&_inletParams[3],0.0f,1.0f);
        }else{
            volume = 1.0f;
        }

        // trigger
        if(this->inletsConnected[4]){
            if(ofClamp(*(float *)&_inletParams[4],0.0f,1.0f) == 1.0f){
                playhead = 0.0;
                isPlaying = true;
            }
        }
    }

}

//--------------------------------------------------------------
void SoundfilePlayer::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(isFileLoaded && audiofile.loaded()){
        posX = 0;
        posY = this->headerHeight;
        drawW = this->width;
        drawH = this->height - this->headerHeight - header->getHeight();

        ofSetColor(0);
        ofDrawRectangle(posX, posY, drawW, drawH);
        ofSetColor(10,10,10);
        ofDrawRectangle(posX+1, posY+1, drawW-2, drawH-2);

        ofSetColor(255,255,120,255);
        ofSetLineWidth(1);
        for( int x=0; x<drawW; ++x){
            int n = ofMap( x, 0, drawW, 0, audiofile.length(), true );
            float val = audiofile.sample( n, 0 );
            ofDrawLine(x+posX, (drawH*0.5)+posY - (val*(drawH*0.5)),x+posX, ((drawH*0.5)+posY) + (val*(drawH*0.5)));
        }

        // draw player state
        ofSetColor(255,60);
        if(isPlaying){ // play
            ofBeginShape();
            ofVertex(this->width - 30,this->height - 50);
            ofVertex(this->width - 30,this->height - 30);
            ofVertex(this->width - 10,this->height - 40);
            ofEndShape();
        }else if(!isPlaying && playhead > 0.0){ // pause
            ofDrawRectangle(this->width - 30, this->height - 50,8,20);
            ofDrawRectangle(this->width - 18, this->height - 50,8,20);
        }else if(!isPlaying && playhead == 0.0){ // stop
            ofDrawRectangle(this->width - 30, this->height - 50,20,20);
        }

        ofSetColor(255);
        ofSetLineWidth(2);
        float phx = ofMap( playhead, 0, audiofile.length(), 0, drawW );
        ofDrawLine( phx, posY+2, phx, drawH+posY);
    }else if(!isNewObject && !loading){
        ofSetColor(255,0,0);
        ofDrawRectangle(0,0,this->width,this->height);
        ofSetColor(255);
        font->draw("FILE NOT FOUND!",this->fontSize,this->width/3 + 4,this->headerHeight*2.3);
    }
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void SoundfilePlayer::removeObjectContent(){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void SoundfilePlayer::fileDialogResponse(ofxThreadedFileDialogResponse &response){
    if(response.id == "load soundfile"+ofToString(this->getId())){
        lastSoundfile = response.filepath;
        soundfileLoaded = true;
        startTime = ofGetElapsedTimeMillis();
    }
}

//--------------------------------------------------------------
void SoundfilePlayer::mouseMovedObjectContent(ofVec3f _m){
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
void SoundfilePlayer::dragGUIObject(ofVec3f _m){
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
void SoundfilePlayer::audioOutObject(ofSoundBuffer &outputBuffer){
    if(isFileLoaded && audiofile.loaded() && isPlaying){
        for(size_t i = 0; i < monoBuffer.getNumFrames(); i++) {
            int n = static_cast<int>(floor(playhead));

            if(n < audiofile.length()-1){
                float fract = static_cast<float>(playhead - n);
                float isample = audiofile.sample(n, 0)*(1.0f-fract) + audiofile.sample(n+1, 0)*fract; // linear interpolation
                monoBuffer.getSample(i,0) = isample * volume;

                playhead += (step*speed);

            }else{
                monoBuffer.getSample(i,0) = 0.0f;
                if(loop){
                    // backword
                    if(speed < 0.0){
                        playhead = audiofile.length()-2;
                    }else if(speed > 0.0){
                        playhead = 0.0;
                    }

                }
            }
        }
        lastBuffer = monoBuffer;
    }else{
        lastBuffer = monoBuffer * 0.0f;
    }

    fileOUT.copyInput(lastBuffer.getBuffer().data(),lastBuffer.getNumFrames());
    *static_cast<ofSoundBuffer *>(_outletParams[0]) = lastBuffer;
    *static_cast<vector<float> *>(_outletParams[1]) = scope.getBuffer();

}

//--------------------------------------------------------------
void SoundfilePlayer::loadSettings(){
    ofxXmlSettings XML;

    if(XML.loadFile(patchFile)){
        if(XML.pushTag("settings")){
            sampleRate = static_cast<double>(XML.getValue("sample_rate_out",0));
            bufferSize = XML.getValue("buffer_size",0);
            XML.popTag();
        }
    }

    for(int i=0;i<bufferSize;i++){
        static_cast<vector<float> *>(_outletParams[1])->push_back(0.0f);
    }

    shortBuffer = new short[bufferSize];
    for (int i = 0; i < bufferSize; i++){
        shortBuffer[i] = 0;
    }

    ofSoundBuffer tmpBuffer(shortBuffer,static_cast<size_t>(bufferSize),1,static_cast<unsigned int>(sampleRate));
    monoBuffer = tmpBuffer;
}

//--------------------------------------------------------------
void SoundfilePlayer::loadAudioFile(string audiofilepath){

    filepath = forceCheckMosaicDataPath(audiofilepath);

    audiofile.free();
    audiofile.load(filepath);
    playhead = std::numeric_limits<int>::max();
    step = audiofile.samplerate() / sampleRate;

    ofSoundBuffer tmpBuffer(shortBuffer,static_cast<size_t>(bufferSize),1,static_cast<unsigned int>(sampleRate));
    monoBuffer.clear();
    monoBuffer = tmpBuffer;

    ofFile tempFile(filepath);
    if(tempFile.getFileName().size() > 44){
        soundfileName->setLabel(tempFile.getFileName().substr(0,41)+"...");
    }else{
        soundfileName->setLabel(tempFile.getFileName());
    }

    playhead = 0.0;

    this->saveConfig(false,this->nId);

    isFileLoaded = false;

}

//--------------------------------------------------------------
void SoundfilePlayer::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if (e.target == loadButton){
            loadSoundfileFlag = true;
        }
    }
}
