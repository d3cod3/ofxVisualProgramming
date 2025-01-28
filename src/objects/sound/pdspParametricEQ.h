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

#pragma once

#include "PatchObject.h"

#include "ofxFft.h"

#include "imgui_plot.h"
#include "imgui-knobs.h"

class pdspParametricEQ : public PatchObject{

public:

    pdspParametricEQ();

    void            newObject() override;
    void            setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow) override;
    void            setupAudioOutObjectContent(pdsp::Engine &engine) override;
    void            updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects) override;

    void            drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer) override;
    void            drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ) override;
    void            drawObjectNodeConfig() override;

    void            removeObjectContent(bool removeFileFromData=false) override;

    void            audioInObject(ofSoundBuffer &inputBuffer) override;
    void            audioOutObject(ofSoundBuffer &outputBuffer) override;


    void            loadAudioSettings();


    ofxFft                  *fft;
    float                   *spectrum;

    pdsp::Scope             scope;

    pdsp::LowShelfEQ        l1;
    pdsp::PeakEQ            m1, m2;
    pdsp::HighShelfEQ       h1;

    pdsp::ValueControl      l1_freq, l1_Q, l1_gain;
    pdsp::ValueControl      m1_freq, m1_Q, m1_gain;
    pdsp::ValueControl      m2_freq, m2_Q, m2_gain;
    pdsp::ValueControl      h1_freq, h1_Q, h1_gain;

    float                   float_l1freq, float_l1Q, float_l1gain;
    float                   float_m1freq, float_m1Q, float_m1gain;
    float                   float_m2freq, float_m2Q, float_m2gain;
    float                   float_h1freq, float_h1Q, float_h1gain;

    std::vector<float>      *l1Filter;
    std::vector<float>      *m1Filter;
    std::vector<float>      *m2Filter;
    std::vector<float>      *h1Filter;
    std::vector<float>      *parametricFilter;

    int                     bufferSize;
    int                     sampleRate;

    bool                    loaded;

protected:


private:

    OBJECT_FACTORY_PROPS

};

#endif
