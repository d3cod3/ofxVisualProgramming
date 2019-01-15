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

#pragma once


// 3d objects

// audio_analysis objects
#include "objects/audio_analysis/AudioAnalyzer.h"
#include "objects/audio_analysis/BeatExtractor.h"
#include "objects/audio_analysis/BPMExtractor.h"
#include "objects/audio_analysis/CentroidExtractor.h"
#include "objects/audio_analysis/DissonanceExtractor.h"
#include "objects/audio_analysis/FftExtractor.h"
#include "objects/audio_analysis/HFCExtractor.h"
#include "objects/audio_analysis/HPCPExtractor.h"
#include "objects/audio_analysis/InharmonicityExtractor.h"
#include "objects/audio_analysis/MelBandsExtractor.h"
#include "objects/audio_analysis/MFCCExtractor.h"
#include "objects/audio_analysis/OnsetExtractor.h"
#include "objects/audio_analysis/PitchExtractor.h"
#include "objects/audio_analysis/PowerExtractor.h"
#include "objects/audio_analysis/RMSExtractor.h"
#include "objects/audio_analysis/RollOffExtractor.h"
#include "objects/audio_analysis/TristimulusExtractor.h"

// computer_vision objects
#include "objects/computer_vision/BackgroundSubtraction.h"
#include "objects/computer_vision/ChromaKey.h"
#include "objects/computer_vision/ColorTracking.h"
#include "objects/computer_vision/ContourTracking.h"
#include "objects/computer_vision/HaarTracking.h"
#include "objects/computer_vision/MotionDetection.h"
#include "objects/computer_vision/OpticalFlow.h"

// data objects
#include "objects/data/FloatsToVector.h"
#include "objects/data/VectorConcat.h"

// graphics objects

// gui objects
#include "objects/gui/moBang.h"
#include "objects/gui/moComment.h"
#include "objects/gui/moMessage.h"
#include "objects/gui/moPlayerControls.h"
#include "objects/gui/moSlider.h"
#include "objects/gui/moSignalViewer.h"
#include "objects/gui/moTimeline.h"
#include "objects/gui/moTrigger.h"
#include "objects/gui/moVideoViewer.h"

// input_output objects

// logic objects
#include "objects/logic/Counter.h"
#include "objects/logic/DelayBang.h"
#include "objects/logic/Gate.h"
#include "objects/logic/Inverter.h"
#include "objects/logic/LoadBang.h"
#include "objects/logic/Select.h"
#include "objects/logic/Spigot.h"

// machine_learning objects

// math objects
#include "objects/math/Add.h"
#include "objects/math/Clamp.h"
#include "objects/math/Constant.h"
#include "objects/math/Divide.h"
#include "objects/math/Metronome.h"
#include "objects/math/Multiply.h"
#include "objects/math/SimpleRandom.h"
#include "objects/math/SimpleNoise.h"
#include "objects/math/Smooth.h"
#include "objects/math/Subtract.h"

// midi objects

// osc objects

// physics objects

// scripting
#if defined(TARGET_LINUX) || defined(TARGET_OSX)
#include "objects/scripting/BashScript.h"
#include "objects/scripting/PythonScript.h"
#endif

#include "objects/scripting/LuaScript.h"
#include "objects/scripting/ShaderObject.h"

// sound objects
#include "objects/sound/AudioDevice.h"
#include "objects/sound/AudioGate.h"
#include "objects/sound/PDPatch.h"
#include "objects/sound/SoundfilePlayer.h"

// video objects
#include "objects/video/KinectGrabber.h"
#include "objects/video/VideoGate.h"
#include "objects/video/VideoGrabber.h"
#include "objects/video/VideoPlayer.h"

// web objects

// window objects
#include "objects/windowing/LivePatching.h"
#include "objects/windowing/OutputWindow.h"
