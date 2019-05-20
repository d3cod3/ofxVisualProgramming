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

#include "AudioAnalyzer.h"

//--------------------------------------------------------------
AudioAnalyzer::AudioAnalyzer() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 2;

    _inletParams[0] = new ofSoundBuffer();  // Audio stream

    _outletParams[0] = new vector<float>();  // Analysis Data

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    isAudioINObject     = true;

    smoothingValue                  = 0.0f;
    audioInputLevel                 = 1.0f;

    startTime                       = ofGetElapsedTimeMillis();
    waitTime                        = 500;
    isConnected                     = false;

    newConnection                   = false;
}

//--------------------------------------------------------------
void AudioAnalyzer::newObject(){
    this->setName("audio analyzer");
    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addOutlet(VP_LINK_ARRAY,"analysisData");

    this->setCustomVar(static_cast<float>(audioInputLevel),"INPUT_LEVEL");
    this->setCustomVar(static_cast<float>(smoothingValue),"SMOOTHING");
}

//--------------------------------------------------------------
void AudioAnalyzer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    // GUI
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onSliderEvent(this, &AudioAnalyzer::onSliderEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    gui->addBreak();

    inputLevel = gui->addSlider("Level",0.0f,1.0f,1.0f);
    inputLevel->setUseCustomMouse(true);
    inputLevel->setValue(audioInputLevel);
    smoothing = gui->addSlider("Smooth.",0.0f,0.8f,0.0f);
    smoothing->setUseCustomMouse(true);
    smoothing->setValue(smoothingValue);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

}

//--------------------------------------------------------------
void AudioAnalyzer::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){
    gui->update();
    header->update();
    inputLevel->update();
    smoothing->update();

    if(this->inletsConnected[0]){
        if(!isConnected){
            isConnected = true;
            startTime   = ofGetElapsedTimeMillis();
        }

        if(!newConnection){
            newConnection = true;
            loadAudioSettings();
        }

        if(isConnected && ofGetElapsedTimeMillis()-startTime > waitTime){
            // Get analysis data
            rms = audioAnalyzer.getValue(RMS, 0, smoothingValue);
            power   = audioAnalyzer.getValue(POWER, 0, smoothingValue);
            pitchFreq = audioAnalyzer.getValue(PITCH_FREQ, 0, smoothingValue);
            if(pitchFreq > 4186){
                pitchFreq = 0;
            }
            hfc = audioAnalyzer.getValue(HFC, 0, smoothingValue);
            centroid = audioAnalyzer.getValue(CENTROID, 0, smoothingValue);
            centroidNorm = audioAnalyzer.getValue(CENTROID, 0, smoothingValue, TRUE);
            inharmonicity   = audioAnalyzer.getValue(INHARMONICITY, 0, smoothingValue);
            dissonance = audioAnalyzer.getValue(DISSONANCE, 0, smoothingValue);
            rollOff = audioAnalyzer.getValue(ROLL_OFF, 0, smoothingValue);
            rollOffNorm  = audioAnalyzer.getValue(ROLL_OFF, 0, smoothingValue, TRUE);

            spectrum = audioAnalyzer.getValues(SPECTRUM, 0, smoothingValue);
            melBands = audioAnalyzer.getValues(MEL_BANDS, 0, smoothingValue);
            mfcc = audioAnalyzer.getValues(MFCC, 0, smoothingValue);
            hpcp = audioAnalyzer.getValues(HPCP, 0, smoothingValue);
            tristimulus = audioAnalyzer.getValues(TRISTIMULUS, 0, smoothingValue);

            isOnset = audioAnalyzer.getOnsetValue(0);

            bpm     = beatTrack->getEstimatedBPM();
            beat    = beatTrack->hasBeat();


            unique_lock<mutex> lock(audioMutex);

            int index = 0;

            waveform.clear();
            for(size_t i = 0; i < lastBuffer.getNumFrames(); i++) {
                float sample = lastBuffer.getSample(i,0);
                float x = ofMap(i, 0, lastBuffer.getNumFrames(), 0, this->width);
                float y = ofMap(hardClip(sample), -1, 1, headerHeight, this->height);
                waveform.addVertex(x, y);

                // SIGNAL BUFFER
                static_cast<vector<float> *>(_outletParams[0])->at(i+index) = sample;
            }

            index += lastBuffer.getNumFrames();
            // SPECTRUM
            for(int i=0;i<static_cast<int>(spectrum.size());i++){
                static_cast<vector<float> *>(_outletParams[0])->at(i+index) = ofMap(spectrum[i],DB_MIN,DB_MAX,0.0,1.0,true);
            }
            index += spectrum.size();
            // MELBANDS
            for(int i=0;i<static_cast<int>(melBands.size());i++){
                static_cast<vector<float> *>(_outletParams[0])->at(i+index) = ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
            }
            index += melBands.size();
            // MFCC
            for(int i=0;i<static_cast<int>(mfcc.size());i++){
                static_cast<vector<float> *>(_outletParams[0])->at(i+index) = ofMap(mfcc[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);
            }
            index += mfcc.size();
            // HPCP
            for(int i=0;i<static_cast<int>(hpcp.size());i++){
                static_cast<vector<float> *>(_outletParams[0])->at(i+index) = hpcp[i];
            }
            index += hpcp.size();
            // TRISTIMULUS
            for(int i=0;i<static_cast<int>(tristimulus.size());i++){
                static_cast<vector<float> *>(_outletParams[0])->at(i+index) = tristimulus[i];
            }
            index += tristimulus.size();
            // SINGLE VALUES (RMS, POWER, PITCH, HFC, CENTROID, INHARMONICITY, DISSONANCE, ROLLOFF, ONSET, BPM, BEAT)
            static_cast<vector<float> *>(_outletParams[0])->at(index) = rms;
            static_cast<vector<float> *>(_outletParams[0])->at(index+1) = power;
            static_cast<vector<float> *>(_outletParams[0])->at(index+2) = pitchFreq;
            static_cast<vector<float> *>(_outletParams[0])->at(index+3) = hfc;
            static_cast<vector<float> *>(_outletParams[0])->at(index+4) = centroidNorm;
            static_cast<vector<float> *>(_outletParams[0])->at(index+5) = inharmonicity;
            static_cast<vector<float> *>(_outletParams[0])->at(index+6) = dissonance;
            static_cast<vector<float> *>(_outletParams[0])->at(index+7) = rollOffNorm;
            static_cast<vector<float> *>(_outletParams[0])->at(index+8) = static_cast<float>(isOnset);
            static_cast<vector<float> *>(_outletParams[0])->at(index+9) = bpm;
            static_cast<vector<float> *>(_outletParams[0])->at(index+10) = static_cast<float>(beat);

        }
    }else{
        isConnected     = false;
    }

}

//--------------------------------------------------------------
void AudioAnalyzer::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    waveform.draw();
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void AudioAnalyzer::removeObjectContent(){
    audioAnalyzer.exit();
}

//--------------------------------------------------------------
void AudioAnalyzer::audioInObject(ofSoundBuffer &inputBuffer){
    if(this->inletsConnected[0] && isConnected && ofGetElapsedTimeMillis()-startTime > waitTime){

        lastBuffer = *static_cast<ofSoundBuffer *>(_inletParams[0]);

        lastBuffer *= audioInputLevel;

        // ESSENTIA Analyze Audio
        lastBuffer.copyTo(monoBuffer, lastBuffer.getNumFrames(), 1, 0);
        audioAnalyzer.analyze(monoBuffer);

        // BTrack
        beatTrack->audioIn(&monoBuffer.getBuffer()[0], bufferSize, 1);

        unique_lock<mutex> lock(audioMutex);
        lastBuffer = monoBuffer;
    }
}

//--------------------------------------------------------------
void AudioAnalyzer::loadAudioSettings(){
    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){
        if (XML.pushTag("settings")){
            sampleRate = XML.getValue("sample_rate_in",0);
            bufferSize = XML.getValue("buffer_size",0);
            XML.popTag();
        }

        // Beat Tracking
        beatTrack = new ofxBTrack();
        beatTrack->setup(bufferSize);
        beatTrack->setConfidentThreshold(0.35);

        // Audio Analysis
        audioAnalyzer.setup(sampleRate, bufferSize, 1);

        audioInputLevel = this->getCustomVar("INPUT_LEVEL");
        smoothingValue = this->getCustomVar("SMOOTHING");

        _outletParams[0] = new vector<float>();
        // SIGNAL BUFFER
        for(int i=0;i<bufferSize;i++){
            static_cast<vector<float> *>(_outletParams[0])->push_back(0.0f);
        }
        // SPECTRUM
        for(int i=0;i<(bufferSize/2)+1;i++){
            static_cast<vector<float> *>(_outletParams[0])->push_back(0.0f);
        }
        // MELBANDS
        for(int i=0;i<MELBANDS_BANDS_NUM;i++){
            static_cast<vector<float> *>(_outletParams[0])->push_back(0.0f);
        }
        // MFCC
        for(int i=0;i<DCT_COEFF_NUM;i++){
            static_cast<vector<float> *>(_outletParams[0])->push_back(0.0f);
        }
        // HPCP
        for(int i=0;i<HPCP_SIZE;i++){
            static_cast<vector<float> *>(_outletParams[0])->push_back(0.0f);
        }
        // TRISTIMULUS
        for(int i=0;i<TRISTIMULUS_BANDS_NUM;i++){
            static_cast<vector<float> *>(_outletParams[0])->push_back(0.0f);
        }
        // SINGLE VALUES (RMS, POWER, PITCH, HFC, CENTROID, INHARMONICITY, DISSONANCE, ROLLOFF, ONSET, BPM, BEAT)
        for(int i=0;i<11;i++){
            static_cast<vector<float> *>(_outletParams[0])->push_back(0.0f);
        }
    }
}

//--------------------------------------------------------------
void AudioAnalyzer::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    smoothing->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    inputLevel->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || smoothing->hitTest(_m-this->getPos()) || inputLevel->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void AudioAnalyzer::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        smoothing->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        inputLevel->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void AudioAnalyzer::onSliderEvent(ofxDatGuiSliderEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == smoothing){
            smoothingValue = static_cast<float>(smoothing->getValue());
            this->setCustomVar(static_cast<float>(smoothingValue),"SMOOTHING");
        }else if(e.target == inputLevel){
            audioInputLevel = static_cast<float>(inputLevel->getValue());
            this->setCustomVar(static_cast<float>(audioInputLevel),"INPUT_LEVEL");
        }
    }
}
