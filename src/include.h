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

// communications objects
#include "objects/communications/ArduinoSerial.h"
#include "objects/communications/KeyPressed.h"
#include "objects/communications/KeyReleased.h"
#include "objects/communications/MidiKey.h"
#include "objects/communications/MidiKnob.h"
#include "objects/communications/MidiPad.h"
#include "objects/communications/MidiScore.h"
#include "objects/communications/MidiReceiver.h"
#include "objects/communications/MidiSender.h"
#include "objects/communications/OscReceiver.h"
#include "objects/communications/OscSender.h"

// computer_vision objects
#include "objects/computer_vision/BackgroundSubtraction.h"
#include "objects/computer_vision/ChromaKey.h"
#include "objects/computer_vision/ColorTracking.h"
#include "objects/computer_vision/ContourTracking.h"
#if defined(TARGET_LINUX) || defined(TARGET_OSX)
#include "objects/computer_vision/FaceTracker.h"
#endif
#include "objects/computer_vision/HaarTracking.h"
#include "objects/computer_vision/MotionDetection.h"
#include "objects/computer_vision/OpticalFlow.h"

// data objects
#include "objects/data/BangMultiplexer.h"
#include "objects/data/DataToTexture.h"
#include "objects/data/FloatsToVector.h"
#include "objects/data/TextureToData.h"
#include "objects/data/VectorAt.h"
#include "objects/data/VectorConcat.h"
#include "objects/data/VectorGate.h"
#include "objects/data/VectorMultiply.h"

// graphics objects
#include "objects/graphics/ImageExporter.h"
#include "objects/graphics/ImageLoader.h"

// gui objects
#include "objects/gui/mo2DPad.h"
#include "objects/gui/moBang.h"
#include "objects/gui/moComment.h"
#include "objects/gui/moMessage.h"
#include "objects/gui/moPlayerControls.h"
#include "objects/gui/moSlider.h"
#include "objects/gui/moSignalViewer.h"
#include "objects/gui/moSonogram.h"
#include "objects/gui/moTimeline.h"
#include "objects/gui/moTrigger.h"
#include "objects/gui/moVideoViewer.h"
#include "objects/gui/moVUMeter.h"

// input_output objects

// logic objects
#include "objects/logic/BiggerThan.h"
#include "objects/logic/Counter.h"
#include "objects/logic/DelayBang.h"
#include "objects/logic/Equality.h"
#include "objects/logic/Gate.h"
#include "objects/logic/Inequality.h"
#include "objects/logic/Inverter.h"
#include "objects/logic/LoadBang.h"
#include "objects/logic/Select.h"
#include "objects/logic/SmallerThan.h"
#include "objects/logic/Spigot.h"

// machine_learning objects

// math objects
#include "objects/math/Add.h"
#include "objects/math/Clamp.h"
#include "objects/math/Constant.h"
#include "objects/math/Divide.h"
#include "objects/math/Metronome.h"
#include "objects/math/Multiply.h"
#include "objects/math/Range.h"
#include "objects/math/SimpleRandom.h"
#include "objects/math/SimpleNoise.h"
#include "objects/math/Smooth.h"
#include "objects/math/Subtract.h"

// physics objects

// scripting
#if defined(TARGET_LINUX) || defined(TARGET_OSX)
#include "objects/scripting/BashScript.h"
#endif

#include "objects/scripting/LuaScript.h"
#include "objects/scripting/ProcessingScript.h"
#include "objects/scripting/PythonScript.h"
#include "objects/scripting/ShaderObject.h"

// sound objects
#include "objects/sound/AudioDevice.h"
#include "objects/sound/AudioExporter.h"
#include "objects/sound/AudioGate.h"
#include "objects/sound/Crossfader.h"
#include "objects/sound/Mixer.h"
#include "objects/sound/NoteToFrequency.h"
#include "objects/sound/Oscillator.h"
#include "objects/sound/OscTriangle.h"
#include "objects/sound/OscSaw.h"
#include "objects/sound/OscPulse.h"
#include "objects/sound/Panner.h"
#include "objects/sound/PDPatch.h"
#include "objects/sound/QuadPanner.h"
#include "objects/sound/SigMult.h"
#include "objects/sound/SignalTrigger.h"
#include "objects/sound/SoundfilePlayer.h"

#include "objects/sound/pdspAHR.h"
#include "objects/sound/pdspADSR.h"
#include "objects/sound/pdspBitNoise.h"
#include "objects/sound/pdspChorusEffect.h"
#include "objects/sound/pdspCombFilter.h"
#include "objects/sound/pdspCompressor.h"
#include "objects/sound/pdspDataOscillator.h"
#include "objects/sound/pdspDecimator.h"
#include "objects/sound/pdspDelay.h"
#include "objects/sound/pdspDucker.h"
#include "objects/sound/pdspHiCut.h"
#include "objects/sound/pdspLFO.h"
#include "objects/sound/pdspLowCut.h"
#include "objects/sound/pdspReverb.h"
#include "objects/sound/pdspWhiteNoise.h"

// video objects
#include "objects/video/KinectGrabber.h"
#include "objects/video/VideoCrop.h"
#include "objects/video/VideoDelay.h"
#include "objects/video/VideoExporter.h"
#include "objects/video/VideoGate.h"
#include "objects/video/VideoGrabber.h"
#include "objects/video/VideoPlayer.h"
#include "objects/video/VideoScale.h"
#include "objects/video/VideoStreaming.h"
#include "objects/video/VideoTimelapse.h"

// web objects

// window objects
#include "objects/windowing/LivePatching.h"
#include "objects/windowing/OutputWindow.h"
#include "objects/windowing/ProjectionMapping.h"
