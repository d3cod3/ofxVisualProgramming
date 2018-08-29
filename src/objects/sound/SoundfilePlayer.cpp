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

    this->numInlets  = 3;
    this->numOutlets = 1;

    _inletParams[0] = new string();  // control
    *(string *)&_inletParams[0] = "";
    _inletParams[1] = new float();  // playhead
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();  // speed
    *(float *)&_inletParams[2] = 0.0f;

    _outletParams[0] = new ofSoundBuffer();  // signal

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    isAudioOUTObject    = true;

    isFileLoaded        = false;

    sampleRate          = 44100;
    bufferSize          = 256;
    
}

//--------------------------------------------------------------
void SoundfilePlayer::newObject(){
    this->setName("soundfile player");
    this->addInlet(VP_LINK_STRING,"control");
    this->addInlet(VP_LINK_NUMERIC,"playhead");
    this->addInlet(VP_LINK_NUMERIC,"speed");
    this->addOutlet(VP_LINK_AUDIO);
}

//--------------------------------------------------------------
void SoundfilePlayer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
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

    if(filepath != "none"){
        loadAudioFile(filepath);
    }
}

//--------------------------------------------------------------
void SoundfilePlayer::setupAudioOutObjectContent(pdsp::Engine &engine){

}

//--------------------------------------------------------------
void SoundfilePlayer::updateObjectContent(map<int,PatchObject*> &patchObjects){
    gui->update();
    header->update();
    loadButton->update();

}

//--------------------------------------------------------------
void SoundfilePlayer::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(isFileLoaded){
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

        ofSetColor(255);
        ofSetLineWidth(1.5);
        float phx = ofMap( playhead, 0, audiofile.length(), 0, drawW );
        ofDrawLine( phx, posY, phx, drawH+posY);
    }
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void SoundfilePlayer::removeObjectContent(){
    
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
    if(isFileLoaded){
        if( playheadControl >= 0.0 ){
            playhead = playheadControl;
            playheadControl = -1.0;
        }

        for(size_t i = 0; i < monoBuffer.getNumFrames(); i++) {
            int n = static_cast<int>(floor(playhead));

            if(n < audiofile.length()-1){
                float fract = static_cast<float>(playhead - n);
                float isample = audiofile.sample(n, 0)*(1.0f-fract) + audiofile.sample(n+1, 0)*fract; // linear interpolation
                monoBuffer.getSample(i,0) = isample;

                playhead += step;

            }else{
                monoBuffer.getSample(i,0) = 0.0f;
                //playhead = std::numeric_limits<int>::max();
            }
        }
        lastBuffer = monoBuffer;
    }else{
        lastBuffer *= 0.0f;
    }

    *static_cast<ofSoundBuffer *>(_outletParams[0]) = lastBuffer;

}

//--------------------------------------------------------------
void SoundfilePlayer::loadAudioFile(string audiofilepath){

    ofxXmlSettings XML;

    if(XML.loadFile(patchFile)){
        if(XML.pushTag("settings")){
            sampleRate = XML.getValue("sample_rate_out",0);
            bufferSize = XML.getValue("buffer_size",0);
            XML.popTag();
        }
    }

    short *shortBuffer = new short[bufferSize];
    for (int i = 0; i < bufferSize; i++){
        shortBuffer[i] = 0;
    }

    ofSoundBuffer tmpBuffer(shortBuffer,static_cast<size_t>(bufferSize),1,static_cast<unsigned int>(sampleRate));
    monoBuffer = tmpBuffer;

    filepath = audiofilepath;

    // TESTING
    audiofile.load(filepath);
    playhead = std::numeric_limits<int>::max();
    playheadControl = 0.0;
    step = audiofile.samplerate() / sampleRate;

    ofFile tempFile(filepath);
    if(tempFile.getFileName().size() > 22){
        soundfileName->setLabel(tempFile.getFileName().substr(0,21)+"...");
    }else{
        soundfileName->setLabel(tempFile.getFileName());
    }

    ofLog(OF_LOG_NOTICE,"[verbose] sound file loaded: %s, Sample Rate: %s",filepath.c_str(), ofToString(audiofile.samplerate()).c_str());

    isFileLoaded = true;

}

//--------------------------------------------------------------
void SoundfilePlayer::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if (e.target == loadButton){
            ofFileDialogResult openFileResult= ofSystemLoadDialog("Select an audio file");
            if (openFileResult.bSuccess){
                ofFile file (openFileResult.getPath());
                if (file.exists()){
                    string fileExtension = ofToUpper(file.getExtension());
                    if(fileExtension == "WAV" || fileExtension == "OGG" || fileExtension == "MP3" || fileExtension == "FLAC") {
                        isFileLoaded = false;
                        loadAudioFile(file.getAbsolutePath());
                    }
                }
            }
        }
    }
}
