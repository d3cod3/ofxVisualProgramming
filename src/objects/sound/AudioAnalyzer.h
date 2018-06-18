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
