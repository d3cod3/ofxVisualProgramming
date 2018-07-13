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

    this->numInlets  = 0;
    this->numOutlets = 2;

    _inletParams[0] = new float();  // channel
    *(float *)&_inletParams[0] = 0;

    _outletParams[0] = new vector<float>();  // Analysis Data
    _outletParams[1] = new ofSoundBuffer();  // Audio Stream

    for(int i=0;i<this->numInlets;i++){
        this->inletsConnected.push_back(false);
    }

    isAudioINObject = true;
    isGUIObject     = true;
    isOverGui       = true;

    actualChannel   = 0;
    numINChannels   = 0;

    window_actual_width = AUDIO_ANALYZER_WINDOW_WIDTH;
    window_actual_height = AUDIO_ANALYZER_WINDOW_HEIGHT;

    smoothingValue  = 0.0f;
    audioInputLevel = 1.0f;

    windowHeader                    = new ofRectangle();
    windowHeaderHeight              = 24;

    windowFont                      = new ofxFontStash();
    windowHeaderIcon                = new ofImage();
    freqDomainBG                    = new ofImage();

    start_dragging_mouseX           = 0;
    start_dragging_mouseY           = 0;
    start_dragging_mouseXinScreen   = 0;
    start_dragging_mouseYinScreen   = 0;
    shouldResetDrag                 = true;
}

//--------------------------------------------------------------
void AudioAnalyzer::newObject(){
    this->setName("audio analyzer");
    this->addInlet(VP_LINK_NUMERIC,"channel");
    this->addOutlet(VP_LINK_ARRAY);
    this->addOutlet(VP_LINK_AUDIO);

    this->setCustomVar(static_cast<float>(actualChannel),"CHANNEL");
    this->setCustomVar(static_cast<float>(audioInputLevel),"INPUT_LEVEL");
    this->setCustomVar(static_cast<float>(smoothingValue),"SMOOTHING");
}

//--------------------------------------------------------------
void AudioAnalyzer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();

    ofGLFWWindowSettings settings;
    settings.setGLVersion(2,1);
    settings.shareContextWith = mainWindow;
    settings.decorated = false;
    settings.resizable = false;
    settings.stencilBits = 0;
    // RETINA FIX
    if(ofGetScreenWidth() >= RETINA_MIN_WIDTH && ofGetScreenHeight() >= RETINA_MIN_HEIGHT){
        this->isRetina  =true;
        this->retinaScale = 2.0f;
        window_actual_width     = AUDIO_ANALYZER_WINDOW_WIDTH*2;
        window_actual_height    = AUDIO_ANALYZER_WINDOW_HEIGHT*2;
        windowHeaderHeight      *= 2;
        settings.setPosition(ofDefaultVec2(500,200));
        windowHeader->set(0,0,window_actual_width,windowHeaderHeight);
    }else{
        settings.setPosition(ofDefaultVec2(mainWindow->getScreenSize().x-(window_actual_width+250),100));
        windowHeader->set(0,0,window_actual_width,windowHeaderHeight);
    }
    settings.setSize(window_actual_width, window_actual_height);


    window = dynamic_pointer_cast<ofAppGLFWWindow>(ofCreateWindow(settings));
    window->setVerticalSync(true);
    windowHeaderIcon->load("images/logo_48.png");

    ofAddListener(window->events().update,this,&AudioAnalyzer::updateInWindow);
    ofAddListener(window->events().draw,this,&AudioAnalyzer::drawInWindow);
    ofAddListener(window->events().mouseMoved ,this,&AudioAnalyzer::mouseMoved);
    ofAddListener(window->events().mouseDragged ,this,&AudioAnalyzer::mouseDragged);
    ofAddListener(window->events().mousePressed ,this,&AudioAnalyzer::mousePressed);
    ofAddListener(window->events().mouseReleased ,this,&AudioAnalyzer::mouseReleased);
    ofAddListener(window->events().mouseScrolled ,this,&AudioAnalyzer::mouseScrolled);

    // WINDOW GUI
    windowGuiThemeRetina = new ofxDatGuiThemeRetina();
    windowGui = new ofxDatGui( ofxDatGuiAnchor::TOP_LEFT );
    windowGui->setAutoDraw(false);
    windowGui->setPosition(0,windowHeaderHeight);
    rmsLabel = windowGui->addLabel("RMS");
    rmsPlotter = windowGui->addValuePlotter("RMS",0.0f,1.0f);
    rmsPlotter->setDrawMode(ofxDatGuiGraph::LINES);
    rmsPlotter->setSpeed(1);
    powerLabel = windowGui->addLabel("POWER");
    powerPlotter = windowGui->addValuePlotter("POWER",0.0f,1.0f);
    powerPlotter->setDrawMode(ofxDatGuiGraph::LINES);
    powerPlotter->setSpeed(1);
    pitchFreqLabel = windowGui->addLabel("PITCH");
    pitchFreqPlotter = windowGui->addValuePlotter("PITCH",0.0f,4186.0f);
    pitchFreqPlotter->setDrawMode(ofxDatGuiGraph::LINES);
    pitchFreqPlotter->setSpeed(1);
    hfcLabel = windowGui->addLabel("HFC");
    hfcPlotter = windowGui->addValuePlotter("HFC",0.0f,1.0f);
    hfcPlotter->setDrawMode(ofxDatGuiGraph::LINES);
    hfcPlotter->setSpeed(1);
    centroidLabel = windowGui->addLabel("CENTROID");
    centroidPlotter = windowGui->addValuePlotter("CENTROID",0.0f,1.0f);
    centroidPlotter->setDrawMode(ofxDatGuiGraph::LINES);
    centroidPlotter->setSpeed(1);
    inharmonicityLabel = windowGui->addLabel("INHARMONICITY");
    inharmonicityPlotter = windowGui->addValuePlotter("INHARMONICITY",0.0f,1.0f);
    inharmonicityPlotter->setDrawMode(ofxDatGuiGraph::LINES);
    inharmonicityPlotter->setSpeed(1);
    dissonanceLabel = windowGui->addLabel("DISSONANCE");
    dissonancePlotter = windowGui->addValuePlotter("DISSONANCE",0.0f,1.0f);
    dissonancePlotter->setDrawMode(ofxDatGuiGraph::LINES);
    dissonancePlotter->setSpeed(1);
    rollOffLabel = windowGui->addLabel("ROLL-OFF");
    rollOffPlotter = windowGui->addValuePlotter("ROLL-OFF",0.0f,1.0f);
    rollOffPlotter->setDrawMode(ofxDatGuiGraph::LINES);
    rollOffPlotter->setSpeed(1);
    if(this->isRetina){
        windowGui->setTheme(windowGuiThemeRetina);
    }

    bpmPlot = new ofxHistoryPlot(NULL, "BPM", rmsPlotter->getWidth(), false);
    bpmPlot->setRange(0,200);
    bpmPlot->setColor(ofColor(255,255,255));
    bpmPlot->setRespectBorders(true);
    bpmPlot->setShowNumericalInfo(false);
    bpmPlot->setDrawTitle(false);
    bpmPlot->setLineWidth(1);
    bpmPlot->setBackgroundColor(ofColor(50,50,50,220));
    freqDomainBG->load("images/frequencyDomain.png");

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
    vector<string> channelsVector;
    for(int i=0;i<numINChannels;i++){
        channelsVector.push_back("CHANNEL "+ofToString(i));
    }
    channelSelector = gui->addDropdown("CHANNEL",channelsVector);
    channelSelector->setUseCustomMouse(true);
    channelSelector->select(actualChannel);
    for(size_t i=0;i<channelSelector->size();i++){
        channelSelector->children[i]->setUseCustomMouse(true);
    }

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

}

//--------------------------------------------------------------
void AudioAnalyzer::updateObjectContent(map<int,PatchObject*> &patchObjects){

    gui->update();
    header->update();
    inputLevel->update();
    smoothing->update();
    channelSelector->update();
    for(size_t i=0;i<channelSelector->size();i++){
        channelSelector->children[i]->update();
    }

    window->setWindowTitle("AudioAnalyzer on channel "+ofToString(actualChannel));

    unique_lock<mutex> lock(audioMutex);

    if(numINChannels > 0){
        int receivingChannel = static_cast<int>(floor(*(float *)&_inletParams[0]));
        if(this->inletsConnected[0] && receivingChannel >= 0 && receivingChannel < numINChannels){
            actualChannel = receivingChannel;
            channelSelector->select(actualChannel);
            this->setCustomVar(static_cast<float>(actualChannel),"CHANNEL");
        }

        int index = 0;

        waveform.clear();
        for(size_t i = 0; i < lastBuffer.getNumFrames(); i++) {
            float sample = lastBuffer.getSample(i,actualChannel);
            float x = ofMap(i, 0, lastBuffer.getNumFrames(), 0, this->width);
            float y = ofMap(hardClip(sample), -1, 1, headerHeight, this->height);
            waveform.addVertex(x, y);

            // SIGNAL BUFFER
            static_cast<vector<float> *>(_outletParams[0])->at(i+index) = sample;
        }

        index += lastBuffer.getNumFrames();
        // SPECTRUM
        for(int i=0;i<spectrum.size();i++){
            static_cast<vector<float> *>(_outletParams[0])->at(i+index) = ofMap(spectrum[i],DB_MIN,DB_MAX,0.0,1.0,true);
        }
        index += spectrum.size();
        // MELBANDS
        for(int i=0;i<melBands.size();i++){
            static_cast<vector<float> *>(_outletParams[0])->at(i+index) = ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
        }
        index += melBands.size();
        // MFCC
        for(int i=0;i<mfcc.size();i++){
            static_cast<vector<float> *>(_outletParams[0])->at(i+index) = ofMap(mfcc[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);
        }
        index += mfcc.size();
        // HPCP
        for(int i=0;i<hpcp.size();i++){
            static_cast<vector<float> *>(_outletParams[0])->at(i+index) = hpcp[i];
        }
        index += hpcp.size();
        // TRISTIMULUS
        for(int i=0;i<tristimulus.size();i++){
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

        // Outlet with audio stream
        *static_cast<ofSoundBuffer *>(_outletParams[1]) = lastBuffer;
    }

}

//--------------------------------------------------------------
void AudioAnalyzer::drawObjectContent(ofxFontStash *font){
    windowFont = font;
    ofSetColor(255);
    ofEnableAlphaBlending();
    waveform.draw();
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void AudioAnalyzer::removeObjectContent(){
    audioAnalyzer.exit();
    window->setWindowShouldClose();
}

//--------------------------------------------------------------
void AudioAnalyzer::audioInObject(ofSoundBuffer &inputBuffer){
    if(numINChannels > 0){

        for(size_t i = 0; i < inputBuffer.getNumFrames(); i++) {
            inputBuffer.getSample(i,actualChannel) *= audioInputLevel;
        }

        // ESSENTIA Analyze Audio
        inputBuffer.copyTo(monoBuffer, inputBuffer.getNumFrames(), 1, 0);
        audioAnalyzer.analyze(monoBuffer);

        // BTrack
        beatTrack->audioIn(&monoBuffer.getBuffer()[0], bufferSize, 1);

        unique_lock<mutex> lock(audioMutex);
        lastBuffer = monoBuffer;

    }
}

//--------------------------------------------------------------
void AudioAnalyzer::updateInWindow(ofEventArgs &e){

    windowGui->update();

    // Get analysis data
    rms = audioAnalyzer.getValue(RMS, 0, smoothingValue);
    rmsPlotter->setLabel(ofToString(rms,3));
    rmsPlotter->setValue(rms);
    power   = audioAnalyzer.getValue(POWER, 0, smoothingValue);
    powerPlotter->setLabel(ofToString(power,3));
    powerPlotter->setValue(power);
    pitchFreq = audioAnalyzer.getValue(PITCH_FREQ, 0, smoothingValue);
    if(pitchFreq > 4186){
        pitchFreq = 0;
    }
    pitchFreqPlotter->setLabel(ofToString(pitchFreq,0));
    pitchFreqPlotter->setValue(pitchFreq);
    hfc = audioAnalyzer.getValue(HFC, 0, smoothingValue);
    hfcPlotter->setLabel(ofToString(hfc,2));
    hfcPlotter->setValue(hfc);
    centroid = audioAnalyzer.getValue(CENTROID, 0, smoothingValue);
    centroidNorm = audioAnalyzer.getValue(CENTROID, 0, smoothingValue, TRUE);
    centroidPlotter->setLabel(ofToString(centroid,2));
    centroidPlotter->setValue(centroidNorm);
    inharmonicity   = audioAnalyzer.getValue(INHARMONICITY, 0, smoothingValue);
    inharmonicityPlotter->setLabel(ofToString(inharmonicity,2));
    inharmonicityPlotter->setValue(inharmonicity);
    dissonance = audioAnalyzer.getValue(DISSONANCE, 0, smoothingValue);
    dissonancePlotter->setLabel(ofToString(dissonance,2));
    dissonancePlotter->setValue(dissonance);
    rollOff = audioAnalyzer.getValue(ROLL_OFF, 0, smoothingValue);
    rollOffNorm  = audioAnalyzer.getValue(ROLL_OFF, 0, smoothingValue, TRUE);
    rollOffPlotter->setLabel(ofToString(rollOff,0));
    rollOffPlotter->setValue(rollOffNorm);

    spectrum = audioAnalyzer.getValues(SPECTRUM, 0, smoothingValue);
    melBands = audioAnalyzer.getValues(MEL_BANDS, 0, smoothingValue);
    mfcc = audioAnalyzer.getValues(MFCC, 0, smoothingValue);
    hpcp = audioAnalyzer.getValues(HPCP, 0, smoothingValue);
    tristimulus = audioAnalyzer.getValues(TRISTIMULUS, 0, smoothingValue);

    isOnset = audioAnalyzer.getOnsetValue(0);

    bpm     = beatTrack->getEstimatedBPM();
    beat    = beatTrack->hasBeat();

    bpmPlot->update(bpm);
}

//--------------------------------------------------------------
void AudioAnalyzer::drawInWindow(ofEventArgs &e){
    ofBackground(20);
    ofSetColor(255);
    ofPushStyle();
    ofEnableAlphaBlending();
    drawWindowHeader();
    windowGui->draw();

    int mw = rmsPlotter->getWidth();
    int ypos = 0;

    ofPushMatrix();
    ofTranslate(mw + 20*this->retinaScale, windowHeaderHeight);


    int graphH = rmsPlotter->getHeight() + (9*this->retinaScale);
    int yoffset = graphH + 19*this->retinaScale;
    ypos = 18*this->retinaScale;
    ofSetColor(255,255,255,30);
    freqDomainBG->draw(0,ypos+(9*this->retinaScale),mw,graphH-(9*this->retinaScale));
    ofSetColor(255);
    ofFill();
    windowFont->draw("SPECTRUM",this->fontSize*this->retinaScale,0,ypos);
    ofSetColor(255,220,110,20);
    ofNoFill();
    float bin_w = (float) mw / spectrum.size();
    for (int i = 0; i < spectrum.size(); i++){
        float clampedBin = ofMap(spectrum[i],DB_MIN,DB_MAX,0.0,1.0,true);
        float bin_h = -1 * (clampedBin * graphH);
        ofDrawLine(i*bin_w, graphH+ypos, i*bin_w, bin_h + graphH+ypos);
    }
    ofSetColor(255,220,110,240);
    bin_w = (float) mw / melBands.size();
    for (int i = 0; i < melBands.size(); i++){
        float scaledValue = ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
        float bin_h = -1 * (scaledValue * graphH);
        ofDrawRectangle(i*bin_w, graphH+ypos, bin_w, bin_h);
    }

    ypos += yoffset;
    ofFill();
    ofSetColor(255,255,255,30);
    freqDomainBG->draw(0,ypos+(9*this->retinaScale),mw,graphH-(9*this->retinaScale));
    ofSetColor(255);
    windowFont->draw("MFCC",this->fontSize*this->retinaScale,0,ypos);
    ofNoFill();
    ofSetColor(255,220,110,240);
    bin_w = (float) mw / mfcc.size();
    for (int i = 0; i < mfcc.size(); i++){
        ofDrawRectangle(i*bin_w, graphH + ypos, bin_w, -1 * (ofMap(mfcc[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true) * graphH));
    }

    ypos += yoffset;
    ofFill();
    ofSetColor(255,255,255,30);
    freqDomainBG->draw(0,ypos+(9*this->retinaScale),mw,graphH-(9*this->retinaScale));
    ofSetColor(255);
    windowFont->draw("HPCP",this->fontSize*this->retinaScale,0,ypos);
    ofNoFill();
    ofSetColor(255,220,110,240);
    bin_w = (float) mw / hpcp.size();
    for (int i = 0; i < hpcp.size(); i++){
        ofDrawRectangle(i*bin_w, graphH+ypos, bin_w, -1 * (hpcp[i] * graphH));
    }

    ypos += yoffset;
    ofFill();
    ofSetColor(255,255,255,30);
    freqDomainBG->draw(0,ypos+(9*this->retinaScale),mw,graphH-(9*this->retinaScale));
    ofSetColor(255);
    windowFont->draw("TRISTIMULUS",this->fontSize*this->retinaScale,0,ypos);
    ofNoFill();
    ofSetColor(255,220,110,240);
    bin_w = (float) mw / tristimulus.size();
    for (int i = 0; i < tristimulus.size(); i++){
        ofDrawRectangle(i*bin_w, graphH+ypos, bin_w, -1 * (tristimulus[i] * graphH));
    }

    ypos += yoffset;
    ofSetColor(255);
    ofFill();
    windowFont->draw("ONSETS",this->fontSize*this->retinaScale,0,ypos);
    ofSetColor(255,220,110,240);
    ofDrawRectangle(0,ypos+(9*this->retinaScale), isOnset * mw, graphH-(9*this->retinaScale));

    ypos += yoffset;
    ofSetColor(255);
    windowFont->draw("BPM: "+ofToString(bpm),this->fontSize*this->retinaScale,0,ypos);
    bpmPlot->draw(0,ypos+(9*this->retinaScale),mw, graphH-(9*this->retinaScale));

    ypos += yoffset;
    ofSetColor(255);
    windowFont->draw("BEAT TRACKING",this->fontSize*this->retinaScale,0,ypos);
    if(beat){
        ofSetColor(255,220,110,240);
        ofDrawRectangle(140*this->retinaScale,(-9*this->retinaScale)+ypos,10*this->retinaScale,10*this->retinaScale);
    }

    ofPopMatrix();

    ofDisableAlphaBlending();
    ofPopStyle();
}

//--------------------------------------------------------------
void AudioAnalyzer::drawWindowHeader(){
    ofSetColor(210);
    ofDrawRectangle(*windowHeader);

    string tempStr = ofToUpper(this->name)+" (channel "+ofToString(actualChannel)+")";

    if(this->isRetina){
        ofSetColor(255);
        windowHeaderIcon->draw(10,windowHeaderHeight/6,36,36);
        ofSetColor(20);
        windowFont->draw(tempStr,this->fontSize*2.4f,60,windowHeaderHeight/2 + this->fontSize/1.62);
        ofSetColor(10);
        ofDrawRectangle(windowHeader->getBottomRight().x - 36,windowHeaderHeight/2 - 2,16,4);
    }else{
        ofSetColor(255);
        windowHeaderIcon->draw(10,windowHeaderHeight/6,18,18);
        ofSetColor(20);
        windowFont->draw(tempStr,this->fontSize*1.2f,30,windowHeaderHeight/2 + this->fontSize/1.92);
        ofSetColor(10);
        ofDrawRectangle(windowHeader->getBottomRight().x - 18,windowHeaderHeight/2 - 1,8,2);
    }

}

//--------------------------------------------------------------
void AudioAnalyzer::loadAudioSettings(){
    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){
        if (XML.pushTag("settings")){
            numINChannels   = XML.getValue("input_channels",0);
            sampleRate = XML.getValue("sample_rate_in",0);
            bufferSize = XML.getValue("buffer_size",0);

            beatTrack = new ofxBTrack();
            beatTrack->setup(bufferSize);
            beatTrack->setConfidentThreshold(0.35);

            XML.popTag();
        }

        if(numINChannels < 1){
            ofLog(OF_LOG_ERROR,"%s: The selected Audio Device has no input capabilities!",this->name.c_str());
            ofLog(OF_LOG_ERROR,"%s: Input Channel Numbers: %i",this->name.c_str(),numINChannels);
        }else{
            // Audio Analysis
            audioAnalyzer.setup(sampleRate, bufferSize, 1);

            actualChannel = static_cast<int>(floor(this->getCustomVar("CHANNEL")));
            audioInputLevel = this->getCustomVar("INPUT_LEVEL");
            smoothingValue = this->getCustomVar("SMOOTHING");

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
}

//--------------------------------------------------------------
void AudioAnalyzer::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    smoothing->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    inputLevel->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    channelSelector->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    for(size_t i=0;i<channelSelector->size();i++){
        channelSelector->children[i]->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));;
    }

    isOverGui = smoothing->hitTest(_m-this->getPos()) || inputLevel->hitTest(_m-this->getPos());
}

//--------------------------------------------------------------
void AudioAnalyzer::dragGUIObject(ofVec3f _m){
    if(isOverGui){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        smoothing->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        inputLevel->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        channelSelector->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }else{
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
void AudioAnalyzer::mouseMoved(ofMouseEventArgs &e){

}

//--------------------------------------------------------------
void AudioAnalyzer::mouseDragged(ofMouseEventArgs &e){
    if(shouldResetDrag){
        start_dragging_mouseX = window->events().getMouseX();
        start_dragging_mouseY = window->events().getMouseY();
        start_dragging_mouseXinScreen = window->getWindowPosition().x + window->events().getMouseX();
        start_dragging_mouseYinScreen = window->getWindowPosition().y + window->events().getMouseY();

        shouldResetDrag = false;
    }

    int newOriginX = window->getWindowPosition().x + window->events().getMouseX() - start_dragging_mouseX;
    int newOriginY = window->getWindowPosition().y + window->events().getMouseY() - start_dragging_mouseY;

    //if(windowHeader->inside(window->events().getMouseX(),window->events().getMouseY())){
        window->setWindowPosition(newOriginX,newOriginY);
    //}
}

//--------------------------------------------------------------
void AudioAnalyzer::mousePressed(ofMouseEventArgs &e){
    if(this->isRetina){
        if(windowHeader->inside(window->events().getMouseX(),window->events().getMouseY()) && window->events().getMouseX() > window_actual_width-36){
            window->iconify(true);
        }
    }else{
        if(windowHeader->inside(window->events().getMouseX(),window->events().getMouseY()) && window->events().getMouseX() > window_actual_width-18){
            window->iconify(true);
        }
    }
}

//--------------------------------------------------------------
void AudioAnalyzer::mouseReleased(ofMouseEventArgs &e){
    shouldResetDrag = true;
}

//--------------------------------------------------------------
void AudioAnalyzer::mouseScrolled(ofMouseEventArgs &e){
    //gui->setCustomMousePos(window->events().getMouseX(),window->events().getMouseY());
    //smoothing->setCustomMousePos(window->events().getMouseX(),window->events().getMouseY());
}

//--------------------------------------------------------------
void AudioAnalyzer::onSliderEvent(ofxDatGuiSliderEvent e){
    if(e.target == smoothing){
        smoothingValue = static_cast<float>(smoothing->getValue());
        this->setCustomVar(static_cast<float>(smoothingValue),"SMOOTHING");
    }else if(e.target == inputLevel){
        audioInputLevel = static_cast<float>(inputLevel->getValue());
        this->setCustomVar(static_cast<float>(audioInputLevel),"INPUT_LEVEL");
    }
}
