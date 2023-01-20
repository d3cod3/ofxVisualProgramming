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

int	melBandsEdges[MEL_SCALE_CRITICAL_BANDS] = {100,200,300,400,510,630,770,920,1080,1270,1480,1720,2000,2320,2700,3150,3700,4400,5300,6400,7700,9500,12000,20000};

//--------------------------------------------------------------
AudioAnalyzer::AudioAnalyzer() : PatchObject("audio analyzer"){

    this->numInlets  = 3;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();  // Audio stream

    _inletParams[1] = new float();  // level
    _inletParams[2] = new float();  // smooth
    *(float *)&_inletParams[1] = 1.0f;
    *(float *)&_inletParams[2] = 0.0f;

    _outletParams[0] = new vector<float>();  // Analysis Data

    this->initInletsState();

    isAudioOUTObject                = true;

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
    unusedArgs(mainWindow);

    loadAudioSettings();
}

//--------------------------------------------------------------
void AudioAnalyzer::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

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


            bpm     = beatTrack->getEstimatedBPM();
            beat    = beatTrack->hasBeat();


            unique_lock<mutex> lock(audioMutex);

            int index = 0;


            index += lastBuffer.getNumFrames();
            // SPECTRUM
            for(int i = 0; i < fftBinSize; i++){
                static_cast<vector<float> *>(_outletParams[0])->at(i+index) = _s_spectrum[i];
            }

            index += fftBinSize;
            // MEL BANDS
            for(int i=0;i<MEL_SCALE_CRITICAL_BANDS-1;i++){
                static_cast<vector<float> *>(_outletParams[0])->at(i+index) = _s_melBins[i];
            }

            index += MEL_SCALE_CRITICAL_BANDS-1;

            // SINGLE VALUES (RMS, PITCH, BPM, BEAT)
            static_cast<vector<float> *>(_outletParams[0])->at(index) = _s_rms;
            static_cast<vector<float> *>(_outletParams[0])->at(index+1) = _s_pitch;
            static_cast<vector<float> *>(_outletParams[0])->at(index+2) = bpm;
            static_cast<vector<float> *>(_outletParams[0])->at(index+3) = static_cast<float>(beat);

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
    unusedArgs(font,glRenderer);

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
        ImGuiEx::drawWaveform(_nodeCanvas.getNodeDrawList(), ImVec2(ImGui::GetWindowSize().x,ImGui::GetWindowSize().y*0.5f), plot_data, bufferSize, 1.3f, IM_COL32(255,255,120,255), this->scaleFactor);

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
    unusedArgs(removeFileFromData);
}

//--------------------------------------------------------------
void AudioAnalyzer::audioOutObject(ofSoundBuffer &inputBuffer){
    unusedArgs(inputBuffer);

    if(static_cast<ofSoundBuffer *>(_inletParams[0])->getBuffer().empty()) return;

    if(this->inletsConnected[0] && isConnected && ofGetElapsedTimeMillis()-startTime > waitTime){

        lastBuffer = *static_cast<ofSoundBuffer *>(_inletParams[0]);

        lastBuffer *= audioInputLevel;

        for(size_t i = 0; i < lastBuffer.getNumFrames(); i++) {
            //float sample = lastBuffer.getSample(i,0);
            plot_data[i] = hardClip(lastBuffer.getSample(i,0));

            // SIGNAL BUFFER
            static_cast<vector<float> *>(_outletParams[0])->at(i) = lastBuffer.getSample(i,0);
        }

        lastBuffer.copyTo(monoBuffer, lastBuffer.getNumFrames(), 1, 0);

        // autocorrelation + normalization
        doAutoCorrelation(monoBuffer.getBuffer().data());


        // get volume
        detectRMS();

        // get pitch
        detectPitch();

        // FFT Analyze Audio
        fft->setSignal(autoCorrelationNorm);
        memcpy(spectrum, fft->getAmplitude(), sizeof(float) * fftBinSize);

        fft_StrongestBinValue	= 0.0f;

        for(unsigned int j=0;j<MEL_SCALE_CRITICAL_BANDS-1;j++){
            melBins[j] = 0.0f;
        }

        for(int i = 0; i < fftBinSize; i++){
            // storing strongest bin for pitch detection
            if(spectrum[i] > fft_StrongestBinValue){
                fft_StrongestBinValue = spectrum[i];
                fft_StrongestBinIndex = i;
            }
            // calculate Mel scale bins from fft bins
            updateMelScale(i);
        }

        fft->setPolar(spectrum, fft->getPhase());
        fft->clampSignal();

        smoothingValues();

        // BTrack (BPM and Beat tracker)
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
        fft = ofxFft::create(bufferSize, OF_FFT_WINDOW_HAMMING);

        fftBinSize              = fft->getBinSize();
        fft_binSizeHz           = ((sampleRate/2)/(fftBinSize-1));
        fft_StrongestBinValue   = 0.0f;
        fft_StrongestBinIndex	= 0;
        fft_pitchBin			= 0;

        autoCorrelation			= new float[bufferSize];
        autoCorrelationNorm		= new float[bufferSize];
        spectrum                = new float[fftBinSize];
        binsToMel               = new int[fftBinSize];
        melBins                 = new float[MEL_SCALE_CRITICAL_BANDS];

        _s_spectrum            = new float[fftBinSize];
        _s_melBins             = new float[MEL_SCALE_CRITICAL_BANDS];


        rms                     = 0.0f;
        pitch                   = 0.0f;
        _s_rms                  = 0.0f;
        _s_pitch                = 0.0f;

        setupMelScale();

        audioInputLevel = this->getCustomVar("INPUT_LEVEL");
        smoothingValue  = this->getCustomVar("SMOOTHING");

        _outletParams[0] = new vector<float>();
        // SIGNAL BUFFER
        plot_data = new float[bufferSize];
        for(int i=0;i<bufferSize;i++){
            static_cast<vector<float> *>(_outletParams[0])->push_back(0.0f);
            plot_data[i] = 0.0f;
        }
        // SPECTRUM
        for(int i=0;i<fftBinSize;i++){
            static_cast<vector<float> *>(_outletParams[0])->push_back(0.0f);
        }

        // MEL BANDS
        for(int i=0;i<MEL_SCALE_CRITICAL_BANDS-1;i++){
            static_cast<vector<float> *>(_outletParams[0])->push_back(0.0f);
        }

        // SINGLE VALUES (RMS, PITCH, BPM, BEAT)
        for(int i=0;i<4;i++){
            static_cast<vector<float> *>(_outletParams[0])->push_back(0.0f);
        }
    }
}

//--------------------------------------------------------------
void AudioAnalyzer::doAutoCorrelation(float* signal){

    float sum;
    std::vector<float> autoCorrelationResults(bufferSize);

    for (int i = 0; i < bufferSize; i++) {
        sum = 0;
        for (int j = 0; j < bufferSize-i; j++) {
            sum += signal[j]*signal[j+i];
        }
        autoCorrelationResults[i]=sum;

    }

    memcpy(autoCorrelation, &autoCorrelationResults[0], bufferSize * sizeof(float));

    float maxValue = 0;

    for (int i=0;i<bufferSize;i++) {
        if (fabs(autoCorrelationResults[i]) > maxValue){
            maxValue = fabs(autoCorrelationResults[i]);
        }
    }

    if (maxValue > 0){
        for(int i=0;i<bufferSize;i++) {
            autoCorrelationResults[i] /= maxValue;
        }
    }
    memcpy(autoCorrelationNorm, &autoCorrelationResults[0], bufferSize * sizeof(float));
}

//--------------------------------------------------------------
void AudioAnalyzer::detectRMS(){
    for (int i = 0; i < bufferSize; i++) {
        rms += abs(autoCorrelation[i]);
    }
    rms /= bufferSize;
}

//--------------------------------------------------------------
void AudioAnalyzer::detectPitch(){
    pitch = (fft_StrongestBinIndex*fft_binSizeHz) + (fft_binSizeHz/2.0f);

    fft_pitchBin = pitch * fftBinSize /(sampleRate / 2);
}

//--------------------------------------------------------------
void AudioAnalyzer::setupMelScale(){
    // setup Mel scale reduction (it depends on samplerate and buffersize)
    // first bin is the 0 Hz component (a constant DC offset to the signal), we do not consider it
    int tempFreq = 0;
    for(unsigned int j=0;j<MEL_SCALE_CRITICAL_BANDS-1;j++){
        melBins[j]	   = 0.0f;
        for(int i = 0; i < fftBinSize; i++){
            tempFreq = (int)((i*fft_binSizeHz) + (fft_binSizeHz/2.0f));
            if(j == 0){
                if(tempFreq <= melBandsEdges[j]){
                    binsToMel[i] = j;
                }else{
                    i = fftBinSize;
                }
            }else if(j > 0){
                if(tempFreq > melBandsEdges[j-1] && tempFreq <= melBandsEdges[j]){
                    binsToMel[i] = j;
                }
            }
        }
    }
    binsToMel[fft->getBinSize()-1] = 23;
}

//--------------------------------------------------------------
void AudioAnalyzer::updateMelScale(int i){
    for(int j=0;j<MEL_SCALE_CRITICAL_BANDS-1;j++){
        if(binsToMel[i] == j){
            melBins[j] += spectrum[i];
        }
    }
}

//--------------------------------------------------------------
void AudioAnalyzer::smoothingValues(){
    // volume detection
    _s_rms = _s_rms*smoothingValue + (1.0-smoothingValue)*rms;
    // pitch detection
    _s_pitch = _s_pitch*smoothingValue + (1.0-smoothingValue)*pitch;
    // fft spectrum
    for(int i=0;i<fftBinSize;i++){
        _s_spectrum[i] = _s_spectrum[i]*smoothingValue + (1.0-smoothingValue)*spectrum[i];
    }
    // bark scale bins
    for(int i=0;i<MEL_SCALE_CRITICAL_BANDS-1;i++){
        _s_melBins[i] = _s_melBins[i]*smoothingValue + (1.0-smoothingValue)*melBins[i];
    }
}


OBJECT_REGISTER( AudioAnalyzer , "audio analyzer", OFXVP_OBJECT_CAT_AUDIOANALYSIS)

#endif
