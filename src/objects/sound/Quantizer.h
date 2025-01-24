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

class ScaleQuantizer
{

    // ScaleQuantizer Class
    // Copyright 2024 Craig Corvin
    //
    // Author: Craig Corvin
    // github: https://github.com/craigcorvin
    //
    // Permission is hereby granted, free of charge, to any person obtaining a copy
    // of this software and associated documentation files (the "Software"), to deal
    // in the Software without restriction, including without limitation the rights
    // to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    // copies of the Software, and to permit persons to whom the Software is
    // furnished to do so, subject to the following conditions:
    //
    // The above copyright notice and this permission notice shall be included in
    // all copies or substantial portions of the Software.
    //
    // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    // IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    // FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    // AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    // LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    // THE SOFTWARE.
    //
    // See http://creativecommons.org/licenses/MIT/ for more information.

public:
        ScaleQuantizer() {}
        ~ScaleQuantizer() {}
    void Init()
    {
        // initialize as a Major scale
        // {C, C#, D, D#, E, F, F#, G, G#, A, A#, B}
        bool initScale[12] = {1,0,1,0,1,1,0,1,0,1,0,1};
        // update scale to
        UpdateScale(initScale);
        // update base frequency to C4
        basefrequency_ = 261.625580f;
        // standard 12 notes per octave
        // can be set to any EDO (Equal Divisions of the Octave)
        // initScale array can then be used to create multi-octave poly chords, et cetera
        semitonesperoctave_ = 12.0f;
        // set to true to not repeat notes in scale until all others have been used
        uniquenotes_ = 0;
        // octave range of quantization
        // range = 0 will only return frequencies in base octave
        octaverange_ = 0;
        // shift up octaves from basefequency
        // octave modifier set to 0
        octavemodifier_ = 0;
        // the scale ends with octave of first note
        scalewithoctave_ = 0;
    }
    float Process(float f)
    {

        // f is the frequency to be quantized to the scale set in UpdateScale

        int scaleLength = scaleSize;

        // 1 less than scaleLength, done for clarity in reading the code
        int scaleLengthSub = scaleLength - 1;

        // convert to chromatic
        // starting at 0
        int fincrement = round((logf(f / basefrequency_) / logf(2.0f)) * semitonesperoctave_);

        int increment = fincrement;

        // octave modifier
        int octmod = octavemodifier_;

        if (increment > 0)
        {
            // while (increment > 11) for semitonesperoctave_ of 12
            while (increment > (scaleLength - 1))
            {
                // increment = increment - 12 for semitonesperoctave_ of 12
                increment = increment - scaleLength;
                if (octmod < octaverange_)
                {
                    ++octmod;
                }
            }
        } else {

            while (increment < 0)
            {
                // increment = increment + 12 for semitonesperoctave_ of 12
                increment = increment + scaleLength;
                if (octmod > (octaverange_ * -1))
                {
                    --octmod;
                }
            }

        }

        int octincrement;

        // octave incremnet
        // int octincrement = octmod * 12 for semitonesperoctave_ of 12
        if (scalewithoctave_)
        {
            octincrement = octmod * scaleLengthSub;
        }
        else
        {
            octincrement = octmod * scaleLength;
        }

        // that's a good bingo, return increment frequency
        if (scale[increment]) {
            LimitScalePlayback(increment);
            // increment = 0 will return 1
            return powf(2.0f, (octincrement + increment) / semitonesperoctave_) * basefrequency_;
        }

        // flip increment if less than zero
        if (increment < 0) {
            // increment = increment + 12 for semitonesperoctave_ of 12
            increment = increment + scaleLength;
        }

        // find lower match
        int subincrement = increment;

        while (!scale[subincrement])
        {
            if (subincrement > 0)
            {
                --subincrement;
            }
            else
            {
                // subincrement = 11 for semitonesperoctave_ of 12, done for clarity
                subincrement = scaleLengthSub;
            }
        }

        // superincrement = 0 will return 1
        float subpitch = powf(2.0f, (octincrement + subincrement) / semitonesperoctave_) * basefrequency_;

        float subdifference = f - subpitch;

        if (subdifference < 0)
        {
            subdifference = subdifference * -1;
        }

        // find upper
        int superincrement = increment;

        while (!scale[superincrement])
        {
            // if (superincrement < 11) for semitonesperoctave_ of 12
            if (superincrement < scaleLengthSub)
            {
                ++superincrement;
            }
            else
            {
                superincrement = 0;
            }
        }

        // superincrement = 0 will return 1
        float superpitch =  powf(2.0f, (octincrement + superincrement) / semitonesperoctave_) * basefrequency_;

        // this will be a negative number if we are below the basefrequency
        float superdifference = f - superpitch;

        if (superdifference < 0)
        {
            superdifference = superdifference * -1;
        }

        if (subdifference < superdifference) {
            // does a return powf(2.0f, (octincrement + subincrement) / 12.0f) * basefrequency;
            LimitScalePlayback(subincrement);
            return subpitch;
        } else {
            // does a return powf(2.0f, (octincrement + superincrement) / 12.0f) * basefrequency;
            LimitScalePlayback(superincrement);
            return superpitch;
        }

    }
    template<size_t N>
    void UpdateScale(bool (&newscale)[N])
    {
        // size of scale
        scaleSize = sizeof(newscale);

        // clear previous scale
        scale.clear();
        scaleReference.clear();

        scaleNoteCounter = 0;

        // does what a for loop scale.push_back would do, but we need more
        // scale.insert(scale.begin(), newscale, newscale + sizeof(newscale));
        for (int x = 0; x < scaleSize; ++x)
        {
            scale.push_back(newscale[x]);
            scaleReference.push_back(newscale[x]);
            if (newscale[x])
            {
                ++scaleNoteCounter;
            }
        }

        scaleNoteCounterReference = scaleNoteCounter;

        // could add a check to see if the last element is an octave of the first?

    }
    void UpdateScale(vector<bool> newscale){
        // size of scale
        scaleSize = newscale.size();

        // clear previous scale
        scale.clear();
        scaleReference.clear();

        scaleNoteCounter = 0;

        // does what a for loop scale.push_back would do, but we need more
        // scale.insert(scale.begin(), newscale, newscale + sizeof(newscale));
        for (int x = 0; x < scaleSize; ++x)
        {
            scale.push_back(newscale[x]);
            scaleReference.push_back(newscale[x]);
            if (newscale[x])
            {
                ++scaleNoteCounter;
            }
        }

        scaleNoteCounterReference = scaleNoteCounter;
    }
    void LimitScalePlayback(int current_increment)
    {
        if (uniquenotes_)
        {
            // record that a note has been used
            scale.at(current_increment) = 0;
            --scaleNoteCounter;

            if (scaleNoteCounter < 1)
            {
                // reset scaleNoteCounter
                scaleNoteCounter = scaleNoteCounterReference;

                // reset
                scale.clear();
                bool current_value = 0;

                for (int x = 0; x < scaleSize; ++x)
                {
                    current_value = scaleReference[x];

                    // prevent same to note
                    if (x == current_increment)
                    {
                        current_value = 0;
                        // update scaleNoteCounter for missing current_value
                        --scaleNoteCounter;
                    }

                    // reset
                    scale.push_back(current_value);
                }

            }
        }
    }
    // basefrequency_
    inline void SetBaseFrequency(float basefrequency) { basefrequency_ = basefrequency; }
    inline float GetBaseFrequency() { return basefrequency_; }
    // semitonesperoctave_
    inline void SetSemitonesPerOctave(float semitonesperoctave) { semitonesperoctave_ = semitonesperoctave; }
    inline float GetSemitonesPerOctave() { return semitonesperoctave_; }
    // uniquenotes_
    inline void OnUniqueNotes() { uniquenotes_ = 1; }
    inline void OffUniqueNotes() { uniquenotes_ = 0; }
    // octaverange_
    inline void SetOctaveRange(int octaverange) { octaverange_ = octaverange; }
    inline int GetOctaveRange() { return octaverange_; }
    // octavemodifier_
    inline void SetOctaveModifier(int octavemodifier) { octavemodifier_ = octavemodifier; }
    inline int GetOctaveModifier() { return octavemodifier_; }
    // scalewithoctave_
    inline void OnScaleEndsWithOctave() { scalewithoctave_ = 1; }
    inline void OffScaleEndsWithOctave() { scalewithoctave_ = 0; }
    private:
        // set within UpdateScale, should these be accessible or modify-able outside class?
        std::vector<bool> scale;
        std::vector<bool> scaleReference;
        int scaleSize;
        int scaleNoteCounter;
        int scaleNoteCounterReference;
        // Set and Get methods for the following
        float basefrequency_;
        float semitonesperoctave_;
        bool uniquenotes_;
        int octaverange_;
        int octavemodifier_;
        bool scalewithoctave_;
};

class Quantizer : public PatchObject {

public:

    Quantizer();

    void            newObject() override;
    void            setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow) override;
    void            updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects) override;

    void            drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer) override;
    void            drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ) override;
    void            drawObjectNodeConfig() override;

    void            removeObjectContent(bool removeFileFromData=false) override;

    void            updateScale(string scalename);

    bool                    loaded;


    ScaleQuantizer          quant;
    vector<bool>            quantScale;

    vector<string>          scalesOptions;
    int                     selectedScale;
    int                     octave;


    ImU32 Black = IM_COL32(0,0,0,255);
    ImU32 Gray = IM_COL32(80,80,80,255);
    ImU32 Yellow = IM_COL32(190,134,60,255);

protected:


private:

    OBJECT_FACTORY_PROPS

};

#endif
