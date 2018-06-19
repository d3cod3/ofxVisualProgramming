#pragma once

#include "PatchObject.h"

#include "ofxAudioAnalyzer.h"

class AudioAnalyzer : public PatchObject {

public:

    AudioAnalyzer();

    void            newObject();
    void            setupObjectContent(shared_ptr<ofAppBaseWindow> &mainWindow);
    void            updateObjectContent();
    void            drawObjectContent(ofxFontStash *font);
    void            removeObjectContent();
    void            audioInObject(ofSoundBuffer &inputBuffer);

    bool            loadAudioSettings();


    ofxAudioAnalyzer        audioAnalyzer;
    ofSoundBuffer           lastBuffer;
    ofPolyline              waveform;
    std::mutex              audioMutex;

    int                     actualChannel;
    int                     bufferSize;
    int                     sampleRate;

};

static inline float hardClip(float x){
    float x1 = fabsf(x + 1.0f);
    float x2 = fabsf(x - 1.0f);

    return 0.5f * (x1 - x2);
}
