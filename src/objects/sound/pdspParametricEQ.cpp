/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2024 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "pdspParametricEQ.h"

//--------------------------------------------------------------
static inline float lowShelfFn(float x, float amplitude, float center, float width){
    float base = (x - center) / width; // divide top by bottom
    base *= base * -.5; // square top and bottom, multiply by -1/2
    base = exp(base); // take pow(e, base)
    if(x < center){
        return amplitude;
    }else{
        return amplitude * base;
    }
}

//--------------------------------------------------------------
static inline float hiShelfFn(float x, float amplitude, float center, float width){
    float base = (x - center) / width; // divide top by bottom
    base *= base * -.5; // square top and bottom, multiply by -1/2
    base = exp(base); // take pow(e, base)
    if(x > center){
        return amplitude;
    }else{
        return amplitude * base;
    }
}

//--------------------------------------------------------------
pdspParametricEQ::pdspParametricEQ() : PatchObject("parametric EQ"){

    this->numInlets  = 1;
    this->numOutlets = 2;

    _inletParams[0] = new ofSoundBuffer(); // audio input

    _outletParams[0] = new ofSoundBuffer(); // audio output
    _outletParams[1] = new vector<float>();  // FFT Data

    this->initInletsState();

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    float_l1freq    = 100.0f;
    float_l1Q       = 1.0f;
    float_l1gain    = 0.0f;
    float_m1freq    = 350.0f;
    float_m1Q       = 3.0f;
    float_m1gain    = 0.0f;
    float_m2freq    = 1500.0f;
    float_m2Q       = 3.0f;
    float_m2gain    = 0.0f;
    float_h1freq    = 5000.0f;
    float_h1Q       = 1.0f;
    float_h1gain    = 0.0f;

    this->width     *= 2.7f;
    this->height    *= 3.2f;

    loaded           = false;

}

//--------------------------------------------------------------
void pdspParametricEQ::newObject(){
    PatchObject::setName( "parametric EQ" );

    this->addInlet(VP_LINK_AUDIO,"signal");

    this->addOutlet(VP_LINK_AUDIO,"equalizedSignal");
    this->addOutlet(VP_LINK_ARRAY,"spectrum data");

    // LF, LMF, HMF, HF
    this->setCustomVar(float_l1freq,"LF_FREQ");
    this->setCustomVar(float_l1Q,"LF_Q");
    this->setCustomVar(float_l1gain,"LF_GAIN");

    this->setCustomVar(float_m1freq,"LMF_FREQ");
    this->setCustomVar(float_m1Q,"LMF_Q");
    this->setCustomVar(float_m1gain,"LMF_GAIN");

    this->setCustomVar(float_m2freq,"HMF_FREQ");
    this->setCustomVar(float_m2Q,"HMF_Q");
    this->setCustomVar(float_m2gain,"HMF_GAIN");

    this->setCustomVar(float_h1freq,"HF_FREQ");
    this->setCustomVar(float_h1Q,"HF_Q");
    this->setCustomVar(float_h1gain,"HF_GAIN");
}

//--------------------------------------------------------------
void pdspParametricEQ::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadAudioSettings();

}

//--------------------------------------------------------------
void pdspParametricEQ::setupAudioOutObjectContent(pdsp::Engine &engine){

    // LF
    l1_freq >> l1.in_freq();
    l1_freq.set(float_l1freq);
    l1_freq.enableSmoothing(50.0f);
    l1_Q >> l1.in_Q();
    l1_Q.set(float_l1Q);
    l1_Q.enableSmoothing(50.0f);
    l1_gain >> l1.in_gain();
    l1_gain.set(float_l1gain);
    l1_gain.enableSmoothing(50.0f);

    // LMF
    m1_freq >> m1.in_freq();
    m1_freq.set(float_m1freq);
    m1_freq.enableSmoothing(50.0f);
    m1_Q >> m1.in_Q();
    m1_Q.set(float_m1Q);
    m1_Q.enableSmoothing(50.0f);
    m1_gain >> m1.in_gain();
    m1_gain.set(float_m1gain);
    m1_gain.enableSmoothing(50.0f);

    // HMF
    m2_freq >> m2.in_freq();
    m2_freq.set(float_m2freq);
    m2_freq.enableSmoothing(50.0f);
    m2_Q >> m2.in_Q();
    m2_Q.set(float_m2Q);
    m2_Q.enableSmoothing(50.0f);
    m2_gain >> m2.in_gain();
    m2_gain.set(float_m2gain);
    m2_gain.enableSmoothing(50.0f);

    // HF
    h1_freq >> h1.in_freq();
    h1_freq.set(float_h1freq);
    h1_freq.enableSmoothing(50.0f);
    h1_Q >> h1.in_Q();
    h1_Q.set(float_h1Q);
    h1_Q.enableSmoothing(50.0f);
    h1_gain >> h1.in_gain();
    h1_gain.set(float_h1gain);
    h1_gain.enableSmoothing(50.0f);

    this->pdspIn[0] >> l1 >> m1 >> m2 >> h1 >> this->pdspOut[0]; //
    this->pdspIn[0] >> l1 >> m1 >> m2 >> h1 >> scope >> engine.blackhole();

}

//--------------------------------------------------------------
void pdspParametricEQ::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    l1_freq.set(float_l1freq);
    l1_Q.set(float_l1Q);
    l1_gain.set(float_l1gain);

    m1_freq.set(float_m1freq);
    m1_Q.set(float_m1Q);
    m1_gain.set(float_m1gain);

    m2_freq.set(float_m2freq);
    m2_Q.set(float_m2Q);
    m2_gain.set(float_m2gain);

    h1_freq.set(float_h1freq);
    h1_Q.set(float_h1Q);
    h1_gain.set(float_h1gain);


    if(!loaded){
        loaded = true;
        float_l1freq=this->getCustomVar("LF_FREQ"), float_l1Q=this->getCustomVar("LF_Q"), float_l1gain=this->getCustomVar("LF_GAIN");
        float_m1freq=this->getCustomVar("LMF_FREQ"), float_m1Q=this->getCustomVar("LMF_Q"), float_m1gain=this->getCustomVar("LMF_GAIN");
        float_m2freq=this->getCustomVar("HMF_FREQ"), float_m2Q=this->getCustomVar("HMF_Q"), float_m2gain=this->getCustomVar("HMF_GAIN");
        float_h1freq=this->getCustomVar("HF_FREQ"), float_h1Q=this->getCustomVar("HF_Q"), float_h1gain=this->getCustomVar("HF_GAIN");
    }
}

//--------------------------------------------------------------
void pdspParametricEQ::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

}

//--------------------------------------------------------------
void pdspParametricEQ::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*2*scaleFactor));
        // Signal FFT
        ImGuiEx::PlotSpectrum(_nodeCanvas.getNodeDrawList(), 0, (ImGui::GetWindowSize().y/3) - 26*scaleFactor, static_cast<vector<float> *>(_outletParams[1]),1.0f,true,IM_COL32(90,90,80,235));
        // parametric eq points
        //ImGuiEx::PlotEQPoint(_nodeCanvas.getNodeDrawList(),ImVec2((float_l1freq/27000.0f)*this->width*scaleFactor*_nodeCanvas.GetCanvasScale(),float_l1gain/20.0f),0, (ImGui::GetWindowSize().y/3) - 26*scaleFactor,0.3,IM_COL32(118,57,52,255));
        //ImGuiEx::PlotEQPoint(_nodeCanvas.getNodeDrawList(),ImVec2((float_m1freq/27000.0f)*this->width*scaleFactor*_nodeCanvas.GetCanvasScale(),float_m1gain/20.0f),0, (ImGui::GetWindowSize().y/3) - 26*scaleFactor,0.3,IM_COL32(38,95,90,255));
        //ImGuiEx::PlotEQPoint(_nodeCanvas.getNodeDrawList(),ImVec2((float_m2freq/27000.0f)*this->width*scaleFactor*_nodeCanvas.GetCanvasScale(),float_m2gain/20.0f),0, (ImGui::GetWindowSize().y/3) - 26*scaleFactor,0.3,IM_COL32(60,80,100,255));
        //ImGuiEx::PlotEQPoint(_nodeCanvas.getNodeDrawList(),ImVec2((float_h1freq/27000.0f)*this->width*scaleFactor*_nodeCanvas.GetCanvasScale(),float_h1gain/20.0f),0, (ImGui::GetWindowSize().y/3) - 26*scaleFactor,0.3,IM_COL32(84,70,84,255));
        // parametric eq func
        ImGuiEx::PlotEQFilter(_nodeCanvas.getNodeDrawList(), 0, (ImGui::GetWindowSize().y/3) - 26*scaleFactor, l1Filter, 0.5f, false, IM_COL32(88,27,22,245));
        ImGuiEx::PlotEQFilter(_nodeCanvas.getNodeDrawList(), 0, (ImGui::GetWindowSize().y/3) - 26*scaleFactor, m1Filter, 0.5f, false, IM_COL32(8,65,60,245));
        ImGuiEx::PlotEQFilter(_nodeCanvas.getNodeDrawList(), 0, (ImGui::GetWindowSize().y/3) - 26*scaleFactor, m2Filter, 0.5f, false, IM_COL32(30,50,70,245));
        ImGuiEx::PlotEQFilter(_nodeCanvas.getNodeDrawList(), 0, (ImGui::GetWindowSize().y/3) - 26*scaleFactor, h1Filter, 0.5f, false, IM_COL32(54,40,54,245));
        ImGuiEx::PlotEQFilter(_nodeCanvas.getNodeDrawList(), 0, (ImGui::GetWindowSize().y/3) - 26*scaleFactor, parametricFilter, 1.5f, true, IM_COL32(190,190,190,215));

        ImGui::SetCursorPos(ImVec2(0, (ImGui::GetWindowSize().y/3)));

        // LF
        ImGui::Dummy(ImVec2(-1,14*scaleFactor));
        if(ImGuiKnobs::Knob("LF Freq", &float_l1freq, 20.0f, 500.0f, 1.0f, "%.2fHz", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(float_l1freq,"LF_FREQ");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("LF Q", &float_l1Q, 0.3f, 20.0f, 0.1f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(float_l1Q,"LF_Q");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("LF Gain", &float_l1gain, -20.0f, 20.0f, 0.1f, "%.2fdB", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(float_l1gain,"LF_GAIN");
        }

        // LMF
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("LMF Freq", &float_m1freq, 30.0f, 2000.0f, 10.0f, "%.2fHz", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(float_m1freq,"LMF_FREQ");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("LMF Q", &float_m1Q, 0.3f, 20.0f, 0.1f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(float_m1Q,"LMF_Q");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("LMF Gain", &float_m1gain, -20.0f, 20.0f, 0.1f, "%.2fdB", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(float_m1gain,"LMF_GAIN");
        }

        // HMF
        ImGui::Dummy(ImVec2(-1,14*scaleFactor));
        if(ImGuiKnobs::Knob("HMF Freq", &float_m2freq, 500.0f, 5000.0f, 10.0f, "%.2fHz", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(float_m2freq,"HMF_FREQ");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("HMF Q", &float_m2Q, 0.3f, 20.0f, 0.1f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(float_m2Q,"HMF_Q");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("HMF Gain", &float_m2gain, -20.0f, 20.0f, 0.1f, "%.2fdB", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(float_m2gain,"HMF_GAIN");
        }

        // HF
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("HF Freq", &float_h1freq, 1000.0f, 20000.0f, 10.0f, "%.2fHz", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(float_h1freq,"HF_FREQ");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("HF Q", &float_h1Q, 0.3f, 20.0f, 0.1f, "%.2f", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(float_h1Q,"HF_Q");
        }
        ImGui::SameLine();
        if(ImGuiKnobs::Knob("HF Gain", &float_h1gain, -20.0f, 20.0f, 0.1f, "%.2fdB", ImGuiKnobVariant_Wiper,ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,MIN_KNOB_SCALE,MAX_KNOB_SCALE)*this->scaleFactor)){
            this->setCustomVar(float_h1gain,"HF_GAIN");
        }

        _nodeCanvas.EndNodeContent();

    }

}

//--------------------------------------------------------------
void pdspParametricEQ::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Parametric Equalizer.",
                "https://mosaic.d3cod3.org/reference.php?r=parametric-eq", scaleFactor);
}

//--------------------------------------------------------------
void pdspParametricEQ::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);

    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void pdspParametricEQ::loadAudioSettings(){
    ofxXmlSettings XML;

#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
    if (XML.loadFile(patchFile)){
#else
    if (XML.load(patchFile)){
#endif
        if (XML.pushTag("settings")){
            sampleRate = XML.getValue("sample_rate_in",0);
            bufferSize = XML.getValue("buffer_size",0);

            XML.popTag();
        }

        fft = ofxFft::create(bufferSize, OF_FFT_WINDOW_HAMMING);
        spectrum = new float[fft->getBinSize()];
        l1Filter = new std::vector<float>;
        m1Filter = new std::vector<float>;
        m2Filter = new std::vector<float>;
        h1Filter = new std::vector<float>;
        parametricFilter = new std::vector<float>;

        for(int i=0;i<fft->getBinSize();i++){
            static_cast<vector<float> *>(_outletParams[1])->push_back(0.0f);

            l1Filter->push_back(0.0f);
            m1Filter->push_back(0.0f);
            m2Filter->push_back(0.0f);
            h1Filter->push_back(0.0f);
            parametricFilter->push_back(0.0f);
        }
    }
}

//--------------------------------------------------------------
void pdspParametricEQ::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);

    // SIGNAL BUFFER
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);

    fft->setSignal(scope.getBuffer().data());
    memcpy(spectrum, fft->getAmplitude(), sizeof(float) * fft->getBinSize());
    fft->setPolar(spectrum, fft->getPhase());
    fft->clampSignal();

    // SPECTRUM
    for(size_t i = 0; i < fft->getBinSize(); i++){
        size_t pos = static_cast<int>(floor((std::log(i+20 / 20.f) / std::log(512.f))*fft->getBinSize()));
        if(pos < fft->getBinSize()){
            static_cast<vector<float> *>(_outletParams[1])->at(pos) = spectrum[i];
        }
        l1Filter->at(i) = lowShelfFn(i, float_l1gain/20.0f, (std::log(float_l1freq / 20.f) / std::log(1000.f))*this->width, float_l1Q);
        m1Filter->at(i) = gaussianFn(i, float_m1gain/20.0f, (std::log(float_m1freq / 20.f) / std::log(1000.f))*this->width, float_m1Q);
        m2Filter->at(i) = gaussianFn(i, float_m2gain/20.0f, (std::log(float_m2freq / 20.f) / std::log(1000.f))*this->width, float_m2Q);
        h1Filter->at(i) = hiShelfFn(i, float_h1gain/20.0f, (std::log(float_h1freq / 20.f) / std::log(1000.f))*this->width, float_h1Q);
        parametricFilter->at(i)	= (l1Filter->at(i)+m1Filter->at(i)+m2Filter->at(i)+h1Filter->at(i))/4.0f;
    }

}

OBJECT_REGISTER( pdspParametricEQ, "parametric EQ", OFXVP_OBJECT_CAT_SOUND)

#endif
