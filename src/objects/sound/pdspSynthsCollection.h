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


// A Synths class collection from ofxPDSP (amazing addon) examples synths from Nicola Pisanti:
// https://github.com/npisanti/ofxPDSP

#pragma once

#include "ofMain.h"
#include "ofxPDSP.h"


struct BassPattern : public pdsp::Sequence{
    
    // helper routine to add notes to the score
    // this routin also add a message for stopping the note
    // so we have to be careful that notes durations don't overlap
    void note(double step16, float gate, float pitch, float slew, double duration){    
        message( step16 ,           gate,  0  ); // adds a trigger on to the gate output
        message( step16,            pitch, 1  ); // adds a value to the pitch output
        message( step16 ,           slew,  2  ); // adds a trigger on to the gate output
        message( step16 + duration, 0.0f,  0 );  // adds a trigger off value to the gate output
    }
    
    // this routine shuffles the pitches inside of the sequence vector
    void shuffleSequence(){
        int seqLen = static_cast<int> (sequence.size());
        int index1 = pdsp::dice(seqLen);
        int index2 = index1 + pdsp::dice(seqLen-1);
        while(index2 >= seqLen){ index2 -= seqLen; }
        float temp = sequence[index1];
        sequence[index1] = sequence[index2];
        sequence[index2] = temp;
    }

    
    // this returns the pitches for the generative routine
    // returns the pitches from the sequence the first, second and third bar
    // on the fourth bar the second part of the returned pitches will be random values
    // counter() returns the value of an internal counter that measure how many time the sequence restarted
    float pfun(int index){
        if(index>4 && counter() == 3){
            float nextPitch = static_cast<float> (pdsp::dice(12) + 41.0f); 
            return nextPitch;            
        }else{
            return sequence[index];
        }
    }     
    

    //inits the pattern and set the pitches to use
    BassPattern(){

        sequence = { 29.0f, 31.f, 34.f, 36.f, 38.f, 41.f, 43.f, 29.f };
        
        code = [&] () noexcept {
            if (counter() == 4 ) resetCount(); 
            // pdsp::Sequence has an internal counter() 
            // to count how many times the sequence restarted 
            // that you can reset it with resetCount() 
            // this counter is automatically reset on Sequence change or launch

            shuffleSequence();            

            steplen = 1.0/16.0;
            
            begin();
         
            //   step   velo    pitch       slew%    duration
            note(0.0,   1.0f,   29.0f,      0.0f,    gate_long);
            note(2.0,   0.5f,   pfun(0),    0.0f,    gate_long);
            note(4.0,   0.5f,   pfun(1),    1.0f,    gate_short);
            note(6.0,   1.0f,   pfun(2),    0.0f,    gate_long);
            note(8.0,   0.5f,   pfun(3),    2.0f,    gate_long);
            note(10.0,  1.0f,   pfun(4),    0.0f,    gate_short);        
            note(11.0,  0.5f,   pfun(5),    0.0f,    gate_short);        
            note(12.0,  0.5f,   pfun(6),    0.0f,    gate_short);        
            note(13.0,  0.5f,   pfun(7),    0.0f,    gate_short);
           
            end();
                      
        };
        
    }

    const double gate_long =  0.95;  // a bit more than 1/16       
    const double gate_short = 0.4; // almost 1/32th
    std::vector<float> sequence;
    
};

// ----------------------------------------------------------------------------------------------------------------------------
class Zap : public pdsp::Patchable{
public:
    Zap(){ patch(); }
    Zap(const Zap & other){ patch(); }

    void patch(){
        addModuleInput("trig", triggers);
        addModuleInput("pitch", osc.in_pitch());
        addModuleInput("pitch_mod_amount", pModAmt.in_mod());
        addModuleInput("mod_env_time", pEnv.in_release());
        addModuleInput("mod_env_velo", pEnv.in_velocity());
        addModuleInput("amp_env_time", aEnv.in_hold());
        addModuleOutput("signal", amp );

        //patching
        triggers >> aEnv.set(3.0f, 40.0f, 20.0f)  >> amp.in_mod();
        osc >> amp;
        triggers >> pEnv.set(0.0f, 0.0f,  50.0f) >> pModAmt.set(48.0f) >> osc.in_pitch();
        pitchControl >> osc.in_pitch(); // overriden by ensemble

        pitchControl.set(36.0f);
    }

    void enableDB( float minValue=-18.0f ){
        aEnv.enableDBTriggering( minValue, 0.0f);
    }

    float meter_env() const {
        return aEnv.meter_output();
    }

    pdsp::Parameter     pitchControl;

private:
    pdsp::PatchNode     triggers;
    pdsp::FMOperator    osc;
    pdsp::AHR           aEnv;
    pdsp::AHR           pEnv;
    pdsp::Amp           amp;
    pdsp::Amp           pModAmt;
    pdsp::Amp           fbModAmt;

};


class Zaps {

public:
    void setup( int numVoices ){
        voices.resize(numVoices);

        parameters.setName("zaps UI");
        parameters.add(fader.set("gain", -12, -48, 12 ) );
        parameters.add(envAmtControl.set("env to pitch", 48, 0, 72 ) );
        parameters.add(envDecayControl.set("env decay", 50, 5, 100 ) );
        parameters.add(veloToEnv.set( "env dyn amt", 0.5f, 0.0f, 1.0f ) );

        float panWidth = 0.30f;

        for( size_t i=0; i<voices.size(); ++i){
            veloToEnv >> voices[i].in("mod_env_velo");
            envAmtControl >> voices[i].in("pitch_mod_amount");
            envDecayControl >> voices[i].in("mod_env_time");
            string label = "zap ";
            label += ofToString( i+1 );
            label += " pitch";
            parameters.add( voices[i].pitchControl.set(label, 46+i*7, 24, 84 ));

            float pan = pdsp::spread( i, voices.size(), panWidth );
            voices[i] * pdsp::panL(pan) >> fader.ch(0);
            voices[i] * pdsp::panR(pan) >> fader.ch(1);
        }
    }

    ofParameterGroup parameters;

    std::vector<Zap> voices;
    pdsp::ParameterGain fader;
private:
    pdsp::Parameter veloToEnv;
    pdsp::Parameter envAmtControl;
    pdsp::Parameter envDecayControl;

};

// ----------------------------------------------------------------------------------------------------------------------------
class SineBleep : public pdsp::Patchable{
public:
    SineBleep() { patch(); } 
    SineBleep(const SineBleep & other) { patch(); } 

    void patch() {
        //add inputs / outputs with these methods 
        addModuleInput("trig", env.in_trig()); // arguments are tag and the Unit in/out to link to that tag
        addModuleInput("pitch", osc.in_pitch());
        addModuleOutput("signal", amp ); // if in/out is not selected default in/out is used
        
        //patching
        env.set(0.0f, 50.0f, 1.0f, 350.0f) * 0.25f >> amp.in_mod();
        env * 0.10f           >> osc.in_fb() >> amp;
    }
private:
    pdsp::Amp amp;
    pdsp::FMOperator osc;
    pdsp::ADSR env;
    
};

// ----------------------------------------------------------------------------------------------------------------------------
class KickSynth : public pdsp::Patchable{
public:
    KickSynth() { patch(); }
    KickSynth( const KickSynth & other ) { patch(); }
    
    void patch (){
        //set inputs/outputs
        addModuleInput("trig", trigger_in);
        addModuleOutput("signal", amp );
        
        //patching
                                                  osc >> amp;
        trigger_in >> ampEnv.set(0.0f, 50.0f, 100.0f) >> amp.in_mod();        
        trigger_in >> modEnv.set(0.0f, 0.0f, 50.0f) * 48.0f >> osc.in_pitch();
                                                      48.0f >> osc.in_pitch();
        ampEnv.enableDBTriggering( -24.0f );
    }
private:
    pdsp::Amp           amp;
    pdsp::FMOperator    osc;
    pdsp::AHR           ampEnv;
    pdsp::AHR           modEnv;
    pdsp::PatchNode     trigger_in;
};

// ----------------------------------------------------------------------------------------------------------------------------
class BassSynth : public pdsp::Patchable{
public:
    BassSynth() { patch(); }
    BassSynth( const BassSynth & other ) { patch(); }
    
    void patch (){
        //set inputs/outputs
        addModuleInput("trig", trigger_in);
        addModuleInput("pitch", osc.in_pitch());
        addModuleOutput("signal", amp );
        
        //patching
        osc.out_saw() * 2.5f >> drive >> filter >> amp;
        
        trigger_in >> ampEnv.set(0.0f, 50.0f, 1.0f, 200.0f) * 0.7f >> amp.in_mod();        
        trigger_in >> filterEnv.set(0.0f, 80.0f, 0.0f, 200.0f) * 60.0f >> filter.in_cutoff();
                                                                 50.0f >> filter.in_cutoff();
                                                                 0.2f  >> filter.in_reso();
    }
private:

    pdsp::Amp           amp;
    pdsp::VAOscillator  osc;
    pdsp::VAFilter     filter;
    pdsp::ADSR          ampEnv;
    pdsp::ADSR          filterEnv;
    pdsp::Saturator1    drive;
    pdsp::PatchNode     trigger_in;
};

// ----------------------------------------------------------------------------------------------------------------------------
class SlideSynth : public pdsp::Patchable{
    
public:
    
    SlideSynth() { patch(); } // default constructor
    SlideSynth( const SlideSynth & other ) { patch(); } // you need this to use std::vector with your class, otherwise will not compile
    // remember that is a bad thing to copy construct in pdsp, 
    //      always just resize the vector and let the default constructor do the work
    //          resizing the vector will also disconnect everything, so do it just once before patching


    void patch (){
       
        //create inputs/outputs to be used with the in("tag") and out("tag") methods
        addModuleInput("pitch", osc.in_pitch()); // the first input added is the default input
        addModuleInput("amp", y_ctrl);
        addModuleOutput("signal", amp ); // the first output added is the default output
        
        
        // pdsp::PatchNode is a class that simply route its inputs to its output
        y_ctrl.enableBoundaries(0.0f, 1.0f); // you can clip the input of pdsp::PatchNode
        y_ctrl.set(0.0f); // and you can set its input default value
        
        //patching
        osc.out_saw() * 2.0f >> drive >> filter >> amp;
                                         y_ctrl >> amp.in_mod();        
                                         y_ctrl * 60.0f >> filter.in_cutoff();
                                                  48.0f >> filter.in_cutoff();
                                                  0.3f  >> filter.in_reso();
    }
    
    // those are optional
    pdsp::Patchable & in_pitch() {
        return in("pitch");
    }
    
    pdsp::Patchable & in_amp() {
        return in("amp");
    }
    
    pdsp::Patchable & out_signal() {
        return out("signal");
    }
    
private:

    pdsp::PatchNode     y_ctrl;
    pdsp::PatchNode     pitch_ctrl;
    pdsp::Amp           amp;
    pdsp::VAOscillator  osc;
    pdsp::Saturator1    drive; // distort the signal
    pdsp::VAFilter      filter; // 24dB multimode filter

};

// ----------------------------------------------------------------------------------------------------------------------------
class MultiSampler : public pdsp::Patchable{
    
public:
    MultiSampler() { patch(); } 
    MultiSampler( const MultiSampler & other ) { patch(); } 
    
    ~MultiSampler(){ // deallocate elements
        for ( size_t i=0; i<samples.size(); ++i ) delete samples[i];
    }
    
    void patch (){
       
        addModuleInput("trig", triggers );
        addModuleInput("position", sampler.in_start() );
        addModuleInput("pitch", sampler.in_pitch()); 
        addModuleInput("select", sampler.in_select()); 
        addModuleOutput("signal", amp );
        
        env.enableDBTriggering(-24.0f, 0.0f);
        setAHR(0.0f, 20.0f, 100.0f); // all the samples used here are shorter than this
        
        triggers >> sampler >> amp;
        triggers >> env     >> amp.in_mod();
        
        samples.reserve( 32 );
    }
    
    void add( string path, bool setHoldTime = false ){

        samples.push_back( new pdsp::SampleBuffer() );
        samples.back()->load(path);
        sampler.addSample( samples.back() );

    }
    
    void gain(float dBvalue) {
        sampler * dB(dBvalue) >> amp;
    }
    
    float meter_env() const {
        return env.meter_output();
    }

    float meter_position() const {
        return sampler.meter_position();
    }
    
    void setAHR ( float attack, float hold, float release ) {
        env.set( attack, hold, release );
    }
    
private:
    pdsp::PatchNode     triggers;
    pdsp::Sampler       sampler;
    pdsp::AHR           env;
    pdsp::Amp           amp;

    std::vector<pdsp::SampleBuffer*> samples;
};


// ----------------------------------------------------------------------------------------------------------------------------
class Reese : public pdsp::Patchable{
    
public:
    Reese() { patch(); } 
    Reese( const Reese & other ) { patch(); } 
    
    void patch (){
       
        addModuleInput("trig", env.in_trig() );
        addModuleInput("pitch", pitchNode); 
        addModuleOutput("signal", amp );
        
        osc1.out_saw() * dB(12.0f) >> drive >> filter >> amp;
        osc2.out_saw() * dB(12.0f) >> drive;
      
        65.0f >> filter.in_cutoff();
        0.1f  >> filter.in_reso();

        env.enableDBTriggering(-24.0f, 0.0f);              
        env.set(40.0f, 100.0f, 1.0f, 200.0f) >> amp.in_mod();
        
        pitchNode >> osc1.in_pitch(); 
           -0.30f >> osc1.in_pitch(); 
            0.30f >> osc2.in_pitch(); 
        pitchNode >> osc2.in_pitch();         
    }
        
private:

    pdsp::PatchNode     pitchNode;
    pdsp::VAOscillator  osc1;
    pdsp::VAOscillator  osc2;
    pdsp::Saturator1    drive;
    pdsp::VAFilter      filter;
    pdsp::ADSR          env;
    pdsp::Amp           amp;
    
};

