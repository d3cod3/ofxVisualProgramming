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

==============================================================================*/

#ifndef OFXVP_BUILD_WITH_MINIMAL_OBJECTS

#pragma once

#include "PatchObject.h"

#include "imgui-knobs.h"

#define MAX_OSC_VOICES 16

class PolyphonicOscillator : public PatchObject{

public:

    PolyphonicOscillator();

    void            newObject() override;
    void            setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow) override;
    void            setupAudioOutObjectContent(pdsp::Engine &engine) override;
    void            updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects) override;

    void            drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer) override;
    void            drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ) override;
    void            drawObjectNodeConfig() override;

    void            removeObjectContent(bool removeFileFromData=false) override;

    void            audioOutObject(ofSoundBuffer &outputBuffer) override;

    void            loadAudioSettings();


    std::vector<pdsp::VAOscillator>      osc;
    std::vector<pdsp::Amp>               level;
    std::vector<pdsp::Amp>               voice_amp_sine;
    std::vector<pdsp::Amp>               voice_amp_triangle;
    std::vector<pdsp::Amp>               voice_amp_saw;
    std::vector<pdsp::Amp>               voice_amp_pulse;
    pdsp::Scope             scope;
    pdsp::Scope             sine_scope;
    pdsp::Scope             triangle_scope;
    pdsp::Scope             saw_scope;
    pdsp::Scope             pulse_scope;

    std::vector<pdsp::ValueControl>      level_ctrl;
    std::vector<pdsp::ValueControl>      pitch_ctrl;
    pdsp::ValueControl      detuneCoarse_ctrl;
    pdsp::ValueControl      detuneFine_ctrl;
    pdsp::ValueControl      pw_ctrl;

    std::vector<pdsp::ValueControl>      sine_ctrl;
    std::vector<pdsp::ValueControl>      triangle_ctrl;
    std::vector<pdsp::ValueControl>      saw_ctrl;
    std::vector<pdsp::ValueControl>      pulse_ctrl;
    std::vector<pdsp::ValueControl>      noise_ctrl;
    std::vector<pdsp::ValueControl>      voice_ctrl;

    std::vector<pdsp::Amp>               sineLevel;
    std::vector<pdsp::Amp>               triangleLevel;
    std::vector<pdsp::Amp>               sawLevel;
    std::vector<pdsp::Amp>               pulseLevel;

    float                   level_float;
    std::vector<float>                   voice_float;
    std::vector<float>                   pitch_float;
    float                   detune_float;
    float                   fine_float;
    float                   pw_float;

    float                   sine_float;
    float                   triangle_float;
    float                   saw_float;
    float                   pulse_float;

    float                   *plot_data;
    int                     bufferSize;
    int                     sampleRate;

    bool                    loaded;

protected:


private:

    OBJECT_FACTORY_PROPS

};

#endif
