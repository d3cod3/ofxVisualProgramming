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

#include "AudioAnalyzer.h"

//--------------------------------------------------------------
AudioAnalyzer::AudioAnalyzer() : PatchObject("audio analyzer"){

    this->numInlets  = 3;
    this->numOutlets = 2;

    _inletParams[0] = new ofSoundBuffer();  // Audio stream

    _inletParams[1] = new float();  // level
    _inletParams[2] = new float();  // smooth
    *(float *)&_inletParams[1] = 1.0f;
    *(float *)&_inletParams[2] = 0.0f;

    _outletParams[0] = new vector<float>();  // Analysis Data

    this->initInletsState();

    isAudioINObject                 = true;

    smoothingValue                  = 0.0f;
    audioInputLevel                 = 1.0f;

    startTime                       = ofGetElapsedTimeMillis();
    waitTime                        = 500;
    isConnected                     = false;

    newConnection                   = false;

    isLoaded                        = false;

    this->width     *= 1.3f;
    this->height    *= 1.8f;
}

//--------------------------------------------------------------
void AudioAnalyzer::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"level");
    this->addInlet(VP_LINK_NUMERIC,"smooth");

    this->addOutlet(VP_LINK_ARRAY,"analysisData");

    this->setCustomVar(static_cast<float>(audioInputLevel),"INPUT_LEVEL");
    this->setCustomVar(static_cast<float>(smoothingValue),"SMOOTHING");
}

//--------------------------------------------------------------
void AudioAnalyzer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();
}

//--------------------------------------------------------------
void AudioAnalyzer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

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


            index += lastBuffer.getNumFrames();
            // SPECTRUM
            for(int i=0;i<static_cast<int>(spectrum.size());i++){
                // inv log100 scale
                static_cast<vector<float> *>(_outletParams[0])->at(i+index) = (pow(100,ofMap(spectrum[i], DB_MIN, DB_MAX, 0.000001f, 1.0f,true))-1.0f)/99.0f;
            }
            index += spectrum.size();
            // MELBANDS
            for(int i=0;i<static_cast<int>(melBands.size());i++){
                // inv log100 scale
                static_cast<vector<float> *>(_outletParams[0])->at(i+index) = (pow(100,ofMap(melBands[i], DB_MIN, DB_MAX, 0.000001f, 1.0f, true))-1.0f)/99.0f;
            }
            index += melBands.size();
            // MFCC
            for(int i=0;i<static_cast<int>(mfcc.size());i++){
                static_cast<vector<float> *>(_outletParams[0])->at(i+index) = ofMap(mfcc[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0f, 1.0f, true);
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

    // LEVEL
    if(this->inletsConnected[1]){
        audioInputLevel = ofClamp(*(float *)&_inletParams[1],0.0f,1.0f);
    }

    // SMOOTH
    if(this->inletsConnected[2]){
        smoothingValue = ofClamp(*(float *)&_inletParams[2],0.0f,1.0f);
    }

    if(!isLoaded){
        isLoaded = true;
        audioInputLevel = this->getCustomVar("INPUT_LEVEL");
        smoothingValue = this->getCustomVar("SMOOTHING");
    }

}

//--------------------------------------------------------------
void AudioAnalyzer::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
}

//--------------------------------------------------------------
void AudioAnalyzer::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){

        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            drawObjectNodeConfig(); this->configMenuWidth = ImGui::GetWindowWidth();



            ImGui::EndMenu();
        }

        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        // draw waveform
        ImGuiEx::drawWaveform(_nodeCanvas.getNodeDrawList(), ImVec2(ImGui::GetWindowSize().x,ImGui::GetWindowSize().y*0.5f), plot_data, 1024, 1.3f, IM_COL32(255,255,120,255), this->scaleFactor);

        // draw signal RMS amplitude
        _nodeCanvas.getNodeDrawList()->AddRectFilled(ImGui::GetWindowPos()+ImVec2(0,ImGui::GetWindowSize().y*0.5f),ImGui::GetWindowPos()+ImVec2(ImGui::GetWindowSize().x,ImGui::GetWindowSize().y *0.5f * (1.0f - ofClamp(static_cast<ofSoundBuffer *>(_inletParams[0])->getRMSAmplitude()*audioInputLevel,0.0,1.0))),IM_COL32(255,255,120,12));

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/7, IM_COL32(255,255,120,255), "level", &audioInputLevel, 0.0f, 1.0f, 100.0f)){
            this->setCustomVar(static_cast<float>(audioInputLevel),"INPUT_LEVEL");
        }
        ImGui::SameLine();
        if(ImGuiEx::KnobFloat(_nodeCanvas.getNodeDrawList(), (ImGui::GetWindowSize().x-(46*scaleFactor))/7, IM_COL32(255,255,120,255), "smooth", &smoothingValue, 0.0f, 1.0f, 100.0f)){
            this->setCustomVar(static_cast<float>(smoothingValue),"SMOOTHING");
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void AudioAnalyzer::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "This object is an audio analysis station which transmits a vector with all the analyzed data. Each type of audio data is available in the different extractor objects inside the same category.",
                "https://mosaic.d3cod3.org/reference.php?r=audio-analyzer", scaleFactor);
}

//--------------------------------------------------------------
void AudioAnalyzer::removeObjectContent(bool removeFileFromData){
    audioAnalyzer.exit();
}

//--------------------------------------------------------------
void AudioAnalyzer::audioInObject(ofSoundBuffer &inputBuffer){
    if(this->inletsConnected[0] && isConnected && ofGetElapsedTimeMillis()-startTime > waitTime){

        lastBuffer = *static_cast<ofSoundBuffer *>(_inletParams[0]);

        lastBuffer *= audioInputLevel;

        for(size_t i = 0; i < lastBuffer.getNumFrames(); i++) {
            //float sample = lastBuffer.getSample(i,0);
            plot_data[i] = hardClip(lastBuffer.getSample(i,0));

            // SIGNAL BUFFER
            static_cast<vector<float> *>(_outletParams[0])->at(i) = lastBuffer.getSample(i,0);
        }

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
            plot_data[i] = 0.0f;
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

OBJECT_REGISTER( AudioAnalyzer , "audio analyzer", OFXVP_OBJECT_CAT_AUDIOANALYSIS)

#endif
