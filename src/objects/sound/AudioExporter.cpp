/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2019 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "AudioExporter.h"

//--------------------------------------------------------------
AudioExporter::AudioExporter() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 0;

    _inletParams[0] = new ofSoundBuffer(); // input

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    isAudioINObject     = true;

    exportAudioFlag     = false;
    audioSaved          = false;

    audioFPS            = 0.0f;
    audioCounter        = 0;
    lastAudioTimeReset  = ofGetElapsedTimeMillis();
}

//--------------------------------------------------------------
void AudioExporter::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_AUDIO,"input");
}

//--------------------------------------------------------------
void AudioExporter::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    recButton = gui->addToggle("REC");
    recButton->setUseCustomMouse(true);
    recButton->setLabelAlignment(ofxDatGuiAlignment::CENTER);

    gui->onToggleEvent(this, &AudioExporter::onToggleEvent);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    recorder.setup(true, false, glm::vec2(1280, 720));
    recorder.setAudioConfig(bufferSize,sampleRate);
    recorder.setOverWrite(true);

#if defined(TARGET_OSX)
    recorder.setFFmpegPath(ofToDataPath("ffmpeg/osx/ffmpeg",true));
#elif defined(TARGET_WIN32)
    recorder.setFFmpegPath(ofToDataPath("ffmpeg/win/ffmpeg.exe",true));
#endif

}

//--------------------------------------------------------------
void AudioExporter::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    if(!header->getIsCollapsed()){
        recButton->update();
    }

    if(exportAudioFlag){
        exportAudioFlag = false;
        fd.saveFile("export audiofile"+ofToString(this->getId()),"Export new mp3 320kb audio file as","export.mp3");
    }

    if(audioSaved){
        audioSaved = false;
        recorder.setOutputPath(filepath);
        recorder.startCustomAudioRecord();
    }

}

//--------------------------------------------------------------
void AudioExporter::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofSetCircleResolution(50);
    ofEnableAlphaBlending();
    if(this->inletsConnected[0]){
        waveform.draw();
    }
    if (recorder.isPaused() && recorder.isRecording()){
        ofSetColor(ofColor::yellow);
    }else if (recorder.isRecording()){
        ofSetColor(ofColor::red);
    }else{
        ofSetColor(ofColor::green);
    }
    ofDrawCircle(ofPoint(this->width-20, 30), 10);
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void AudioExporter::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void AudioExporter::loadAudioSettings(){
    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){
        if (XML.pushTag("settings")){
            sampleRate = XML.getValue("sample_rate_in",0);
            bufferSize = XML.getValue("buffer_size",0);

            XML.popTag();
        }
    }
}

//--------------------------------------------------------------
void AudioExporter::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    recButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || recButton->hitTest(_m-this->getPos());

    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void AudioExporter::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        recButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

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
void AudioExporter::audioInObject(ofSoundBuffer &inputBuffer){
    if(ofGetElapsedTimeMillis()-lastAudioTimeReset >= 1000){
        lastAudioTimeReset = ofGetElapsedTimeMillis();
        audioFPS = audioCounter;
        audioCounter = 0;
    }else{
        audioCounter++;
    }

    if(this->inletsConnected[0]){
        if(recorder.isRecording()){
            recorder.addBuffer(*static_cast<ofSoundBuffer *>(_inletParams[0]),audioFPS);
        }

        waveform.clear();
        for(size_t i = 0; i < static_cast<ofSoundBuffer *>(_inletParams[0])->getNumFrames(); i++) {
            float sample = static_cast<ofSoundBuffer *>(_inletParams[0])->getSample(i,0);
            float x = ofMap(i, 0, static_cast<ofSoundBuffer *>(_inletParams[0])->getNumFrames(), 0, this->width);
            float y = ofMap(hardClip(sample), -1, 1, headerHeight, this->height);
            waveform.addVertex(x, y);
        }
    }
}

//--------------------------------------------------------------
void AudioExporter::fileDialogResponse(ofxThreadedFileDialogResponse &response){
    if(response.id == "export audiofile"+ofToString(this->getId())){
        filepath = response.filepath;
        audioSaved = true;
    }
}

//--------------------------------------------------------------
void AudioExporter::onToggleEvent(ofxDatGuiToggleEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == recButton){
            if(e.checked){
                if(!recorder.isRecording()){
                    exportAudioFlag = true;
                }
                ofLog(OF_LOG_NOTICE,"START EXPORTING AUDIO");
            }else{
                if(recorder.isRecording()){
                    recorder.stop();
                }
                ofLog(OF_LOG_NOTICE,"FINISHED EXPORTING AUDIO");
            }
        }
    }
}

OBJECT_REGISTER( AudioExporter, "audio exporter", OFXVP_OBJECT_CAT_SOUND)

#endif