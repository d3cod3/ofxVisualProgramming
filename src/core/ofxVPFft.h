/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2025 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

    Original Fft code from ofxFft by Kyle McDonald <https://github.com/kylemcdonald/ofxFft>
    Simplified, stripped and reduced for ofxVP

==============================================================================*/

#pragma once

#include "ofMain.h"
#include "kiss_fftr.h"

namespace ofxVP {

class Fft {
public:

    void setup(int signalSize){
        this->signalSize = signalSize;
        this->binSize = (signalSize / 2) + 1;

        signalNormalized = true;
        signal = new float[signalSize];

        cartesianUpdated = true;
        cartesianNormalized = true;
        real = new float[binSize];
        imag = new float[binSize];

        polarUpdated = true;
        polarNormalized = true;
        amplitude = new float[binSize];
        phase = new float[binSize];

        clear();

        window = new float[signalSize];
        inverseWindow = new float[signalSize];

        // Hamming window type
        for(int i = 0; i < signalSize; i++)
            window[i] = .54 - .46 * cos((TWO_PI * i) / (signalSize - 1));

        windowSum = 0;
        for(int i = 0; i < signalSize; i++)
            windowSum += window[i];

        for(int i = 0; i < signalSize; i++)
            inverseWindow[i] = 1. / window[i];

        fftCfg = kiss_fftr_alloc(signalSize, 0, NULL, NULL);
        ifftCfg = kiss_fftr_alloc(signalSize, 1, NULL, NULL);
        windowedSignal = new float[signalSize];
        cx_out = new kiss_fft_cpx[binSize];
        cx_in = new kiss_fft_cpx[binSize];
    }
    ~Fft(){
        delete [] signal;
        delete [] real;
        delete [] imag;
        delete [] amplitude;
        delete [] phase;
        delete [] window;
        delete [] inverseWindow;
        kiss_fftr_free(fftCfg);
        kiss_fftr_free(ifftCfg);
        delete [] windowedSignal;
        delete [] cx_out;
        delete [] cx_in;
    }

    void setSignal(const vector<float>& signal){
        setSignal(&signal[0]);
    }
    void setSignal(const float* signal){
        clearUpdates();
        copySignal(signal);
        signalUpdated = true;
        signalNormalized = true;
    }
    void setCartesian(float* real, float* imag = NULL){
        clearUpdates();
        copyReal(real);
        copyImaginary(imag);
        cartesianUpdated = true;
        cartesianNormalized = true;
    }
    void setPolar(float* amplitude, float* phase = NULL){
        clearUpdates();
        copyAmplitude(amplitude);
        copyPhase(phase);
        polarUpdated = true;
        polarNormalized = true;
    }

    int getSignalSize(){
        return signalSize;
    }
    float* getSignal(){
        prepareSignal();
        return signal;
    }
    void clampSignal(){
        prepareSignal();
        for(int i = 0; i < signalSize; i++) {
            if(signal[i] > 1)
                signal[i] = 1;
            else if(signal[i] < -1)
                signal[i] = -1;
        }
    }

    int getBinSize() {
        return binSize;
    }
    float* getReal(){
        prepareCartesian();
        return real;
    }
    float* getImaginary(){
        prepareCartesian();
        return imag;
    }
    float* getAmplitude(){
        preparePolar();
        return amplitude;
    }
    float* getPhase(){
        preparePolar();
        return phase;
    }

    float getAmplitudeAtBin(float bin){
        float* amplitude = getAmplitude();
        int lowBin = ofClamp(floorf(bin), 0, binSize - 1);
        int highBin = ofClamp(ceilf(bin), 0, binSize - 1);
        return ofMap(bin, lowBin, highBin, amplitude[lowBin], amplitude[highBin]);
    }
    float getBinFromFrequency(float frequency, float sampleRate = 44100){
        return frequency * binSize / (sampleRate / 2);
    }
    float getAmplitudeAtFrequency(float frequency, float sampleRate = 44100){
        return getAmplitudeAtBin(getBinFromFrequency(frequency, sampleRate));
    }

private:
    kiss_fftr_cfg fftCfg, ifftCfg;
    float* windowedSignal;
    kiss_fft_cpx* cx_out;
    kiss_fft_cpx* cx_in;

protected:

    void executeFft(){
        memcpy(windowedSignal, signal, sizeof(float) * signalSize);
        runWindow(windowedSignal);
        kiss_fftr(fftCfg, windowedSignal, cx_out);

        for(int i = 0; i < binSize; i++) {
            real[i] = cx_out[i].r;
            imag[i] = cx_out[i].i;
        }
        cartesianUpdated = true;
    }
    void executeIfft(){
        for(int i = 0; i < binSize; i++) {
            cx_in[i].r = real[i];
            cx_in[i].i = imag[i];
        }for(int i = 0; i < binSize; i++) {
            cx_in[i].r = real[i];
            cx_in[i].i = imag[i];
        }
        kiss_fftri(ifftCfg, cx_in, signal);
        runInverseWindow(signal);

        signalUpdated = true;
        kiss_fftri(ifftCfg, cx_in, signal);
        runInverseWindow(signal);

        signalUpdated = true;
    }

    void clear(){
        memset(signal, 0, sizeof(float) * signalSize);
        memset(real, 0, sizeof(float) * binSize);
        memset(imag, 0, sizeof(float) * binSize);
        memset(amplitude, 0, sizeof(float) * binSize);
        memset(phase, 0, sizeof(float) * binSize);
    }

    // time domain data and methods
    float windowSum;
    float *window, *inverseWindow;

    inline void runWindow(float* signal) {
        for(int i = 0; i < signalSize; i++)
            signal[i] *= window[i];

    }

    inline void runInverseWindow(float* signal) {
        for(int i = 0; i < signalSize; i++)
            signal[i] *= inverseWindow[i];

    }

    float *signal;
    bool signalUpdated, signalNormalized;
    void prepareSignal(){
        if(!signalUpdated)
            updateSignal();
        if(!signalNormalized)
            normalizeSignal();
    }
    void updateSignal(){
        prepareCartesian();
        executeIfft();
        signalUpdated = true;
        signalNormalized = false;
    }
    void normalizeSignal(){
        float normalizer = (float) windowSum / (2 * signalSize);
        for (int i = 0; i < signalSize; i++)
            signal[i] *= normalizer;
        signalNormalized = true;
    }
    void copySignal(const float* signal){
        memcpy(this->signal, signal, sizeof(float) * signalSize);
    }

    // frequency domain data and methods
    int signalSize, binSize;

    float *real, *imag;
    bool cartesianUpdated, cartesianNormalized;
    void prepareCartesian(){
        if(!cartesianUpdated) {
            if(!polarUpdated)
                executeFft();
            else
                updateCartesian();
        }
        if(!cartesianNormalized)
            normalizeCartesian();
    }
    void updateCartesian(){
        for(int i = 0; i < binSize; i++) {
            real[i] = cosf(phase[i]) * amplitude[i];
            imag[i] = sinf(phase[i]) * amplitude[i];
        }
        cartesianUpdated = true;
        cartesianNormalized = polarNormalized;
    }
    void normalizeCartesian(){
        float normalizer = 2. / windowSum;
        for(int i = 0; i < binSize; i++) {
            real[i] *= normalizer;
            imag[i] *= normalizer;
        }
        cartesianNormalized = true;
    }
    void copyReal(float* real){
        memcpy(this->real, real, sizeof(float) * binSize);
    }
    void copyImaginary(float* imag){
        if(imag == NULL)
            memset(this->imag, 0, sizeof(float) * binSize);
        else
            memcpy(this->imag, imag, sizeof(float) * binSize);
    }

    float *amplitude, *phase;
    bool polarUpdated, polarNormalized;
    void preparePolar(){
        if(!polarUpdated)
            updatePolar();
        if(!polarNormalized)
            normalizePolar();
    }
    void updatePolar(){
        prepareCartesian();
        for(int i = 0; i < binSize; i++) {
            amplitude[i] = cartesianToAmplitude(real[i], imag[i]);
            phase[i] = cartesianToPhase(real[i], imag[i]);
        }
        polarUpdated = true;
        polarNormalized = cartesianNormalized;
    }
    void normalizePolar(){
        float normalizer = 2. / windowSum;
        for(int i = 0; i < binSize; i++)
            amplitude[i] *= normalizer;
        polarNormalized = true;
    }
    void copyAmplitude(float* amplitude){
        memcpy(this->amplitude, amplitude, sizeof(float) * binSize);
    }
    void copyPhase(float* phase){
        if(phase == NULL)
            memset(this->phase, 0, sizeof(float) * binSize);
        else
            memcpy(this->phase, phase, sizeof(float) * binSize);
    }

    void clearUpdates(){
        cartesianUpdated = false;
        polarUpdated = false;
        cartesianNormalized = false;
        polarNormalized = false;
        signalUpdated = false;
        signalNormalized = false;
    }

    inline float cartesianToAmplitude(float x, float y) {
        return sqrtf(x * x + y * y);
    }

    inline float cartesianToPhase(float x, float y) {
        return atan2f(y, x);
    }
};

}// End namespace ofxVP
