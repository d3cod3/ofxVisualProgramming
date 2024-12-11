
# ofxVisualProgramming - A visual-programming patching addon for OF

> A collection of visual interactive objects to create/develop in a dataflow+live-coding patching environment. Embedded with Lua scripting interpreter, plus live compile of GLSL Shaders from version 150 to 410, and live editing/execute Bash scripts(macOS & linux) capabilities.

![Mosaic 0.6.9](https://github.com/d3cod3/Mosaic/raw/master/process/img/31_sintax03.jpg)
Screenshot from project [Mosaic](http://mosaic.d3cod3.org/), embedding ofxVisualProgramming


Table of Contents
=================

   * [OF_COMPATIBLE_RELEASE](#of_compatible_release)
   * [REFERENCE](#reference)
   * [DESCRIPTION](#description)
   * [DEPENDENCIES](#dependencies)
   * [COMPILING](#compiling)
   * [INSTALLING](#installing)
   * [USAGE](#usage)
   * [CONTRIBUTING](#contributing)
   * [OBJECTS LIST](#objects_list)
   * [LICENSE](#license)
   * [CREDITS](#credits)


# OF COMPATIBLE RELEASE

## 0.12.0 STABLE (official download from [OF site](https://openframeworks.cc/))
> Compiled/tested with QTCreator on osx/linux

If you want to build ofxVisualProgramming, just download OF0.12.0 for your OS (osx, linux, windows) and follow the setup guide for [qtcreator](https://www.qt.io/) IDE.

# REFERENCE

ofxVisualProgramming came directly from the idea behind the ofxComposer addon by [patriciogonzalezvivo](https://github.com/patriciogonzalezvivo/ofxComposer) and [James George](http://www.jamesgeorge.org/), and obviously from the various commercial and no-commercial existing visual programming softwares, the open source option [Pure Data](http://puredata.info/), the commercial options [Max/Msp](https://cycling74.com/products/max) and [TouchDesigner](https://www.derivative.ca/), etc..., to talk of the latest ones, and without forgetting probably the first one from 1968,

<a href="http://www.youtube.com/watch?feature=player_embedded&v=QQhVQ1UG6aM" target="_blank"><img src="http://img.youtube.com/vi/QQhVQ1UG6aM/0.jpg"
alt="GRAIL" width="240" height="180" border="0" /></a>

GRAIL [RM-599-ARPA](https://www.rand.org/content/dam/rand/pubs/research_memoranda/2005/RM5999.pdf) from **the [RAND](https://www.rand.org) Corporation**.

So special thanks to all the precursors of this ideas, and more thanks to the [ofxComposer](https://github.com/patriciogonzalezvivo/ofxComposer) developers for their code, it has been a great reference for start working on this ofxaddon.

# DESCRIPTION

This [addon](https://github.com/d3cod3/ofxVisualProgramming) is the core code of the project [Mosaic](https://github.com/d3cod3/Mosaic)![Mosaic logo](https://github.com/d3cod3/Mosaic/raw/master/process/logo/logo_150.png), maintained isolated in order to obtain a better modularized code structure, encourage contributions, simplify bug fixing and enhance code quality.

This project deals with the idea of integrate/amplify man-machine communication, offering a real-time flowchart based visual interface for high level creative coding. As live-coding scripting languages offer a high level coding environment, ofxVisualProgramming and the Mosaic Project as his parent layer container, aim at a high level visual-programming environment, with embedded multi scripting languages availability (Lua, GLSL, and BASH(macOS & linux) ).

As this project is based on openFrameworks, one of the goals is to offer as more objects as possible, using the pre-defined OF classes for trans-media manipulation (audio, text, image, video, electronics, computer vision), plus all the gigantic ofxaddons ecosystem actually available (machine learning, protocols, web, hardware interface, among a lot more).

While the described characteristics could potentially offer an extremely high complex result (OF and OFXADDONS ecosystem is really huge, and the possibility of multiple scripting languages could lead every unexperienced user to confusion), the idea behind the interface design aim at avoiding the "high complex" situation, embodying a direct and natural drag&drop connect/disconnect interface (mouse/trackpad) on the most basic level of interaction, adding text editing (keyboard) on a intermediate level of interaction (script editing), following most advanced level of interaction for experienced users (external devices communication, automated interaction, etc...)


#### KEYWORDS
mosaic, ofxVisualProgramming, openframeworks, linux, macOS, windows, creative-coding, live-coding, cyber-coding, physical-computing, visual-computing, scripting, transmedia, programming, visual-programming, cyber-programming

# DEPENDENCIES

In order to build ofxVisualProgramming, you'll need this addons:


#### [ofxAudioFile](https://github.com/npisanti/ofxAudioFile)

#### [ofxBTrack](https://github.com/d3cod3/ofxBTrack)

#### [ofxCv](https://github.com/kylemcdonald/ofxCv)

#### [ofxEasing](https://github.com/arturoc/ofxEasing)

#### [ofxFFmpegRecorder](https://github.com/d3cod3/ofxFFmpegRecorder)

#### [ofxFft](https://github.com/kylemcdonald/ofxFft)

#### [ofxJSON](https://github.com/jeffcrouse/ofxJSON)

#### [ofxImGui](https://github.com/d3cod3/ofxImGui)

#### [ofxInfiniteCanvas](https://github.com/d3cod3/ofxInfiniteCanvas)

#### [ofxLua](https://github.com/danomatika/ofxLua)

#### [ofxMidi](https://github.com/danomatika/ofxMidi)

#### [ofxMtlMapping2D](https://github.com/d3cod3/ofxMtlMapping2D)

#### [ofxNDI](https://github.com/d3cod3/ofxNDI)

#### [ofxOpenDHT](https://github.com/d3cod3/ofxOpenDHT)

#### [ofxPd](https://github.com/danomatika/ofxPd)

#### [ofxPDSP](https://github.com/d3cod3/ofxPDSP)

#### [ofxSyphon](https://github.com/d3cod3/ofxSyphon)

#### [ofxTimeline](https://github.com/d3cod3/ofxTimeline)

#### [ofxWarp](https://github.com/d3cod3/ofxWarp)

>Some addons are forks of the original, due to some mods, compatibility with OF0.12.0 and the intention of cross-platform compiling (osx,linux)

# COMPILING

**MACOS/LINUX compiling with qtcreator 4.6.1**

# INSTALLING

Clone [this addon repository](https://github.com/d3cod3/ofxVisualProgramming) into your `<your_openframeworks_release_folder>/addons` together with all the others addons listed:

```bash
cd <your_openframeworks_release_folder>/addons

git clone https://github.com/npisanti/ofxAudioFile
git clone https://github.com/d3cod3/ofxBTrack
git clone https://github.com/kylemcdonald/ofxCv
git clone https://github.com/arturoc/ofxEasing
git clone https://github.com/d3cod3/ofxFFmpegRecorder
git clone https://github.com/kylemcdonald/ofxFft
git clone https://github.com/jeffcrouse/ofxJSON
git clone https://github.com/d3cod3/ofxImGui
git clone https://github.com/d3cod3/ofxInfiniteCanvas
git clone https://github.com/danomatika/ofxLua
git clone https://github.com/danomatika/ofxMidi
git clone https://github.com/d3cod3/ofxMtlMapping2D
git clone --branch=NDI5 https://github.com/d3cod3/ofxNDI
git clone https://github.com/d3cod3/ofxOpenDHT
git clone https://github.com/danomatika/ofxPd
git clone https://github.com/d3cod3/ofxPDSP
git clone https://github.com/d3cod3/ofxSyphon
git clone https://github.com/d3cod3/ofxTimeline
git clone https://github.com/d3cod3/ofxWarp
git clone https://github.com/d3cod3/ofxVisualProgramming
```

# USAGE

In ofApp.h include the ofxVisualProgramming addon:

```c

#include "ofMain.h"

#include "ofxVisualProgramming.h"

class ofApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();
    // ....
    // ....

    ofxVisualProgramming    *visualProgramming; /**/

};

```

then in ofApp.cpp:

```c

#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetWindowTitle("ofxVisualProgramming Example");

    visualProgramming = new ofxVisualProgramming();
    visualProgramming->setup();
}

//--------------------------------------------------------------
void ofApp::update(){
    visualProgramming->update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(20);
    visualProgramming->draw();
}

```

# CONTRIBUTING

Contributing to the project adding new objects is relatively easy, as ofxVisualProgramming is at the core of [Mosaic](https://github.com/d3cod3/Mosaic) software, i've implemented a plugin mechanism based on [Pugg](http://pugg.sourceforge.net/), in order to facilitate the extension and the creation of new objects/nodes to add at the ofxVisualProgramming/Mosaic default objects/nodes library. (see objects/nodes list below)

You can find the repo of the plugin template here: [ofxMosaicPlugin](https://github.com/d3cod3/ofxMosaicPlugin), with some generic objects templates and more detailed info in the readme.


# OBJECTS LIST

Audio Analysis | Ready
---------- | ----------
audio analyzer | X |
bpm extractor | X |
centroid extractor | X |
dissonance extractor | X |
fft extractor | X |
hfc extractor | X |
hpcp extractor | X |
inharmonicity extractor | X |
mel bands extractor | X |
mfcc extractor | X |
onset extractor | X |
pitch extractor | X |
power extractor | X |
rms extractor | X |
rolloff extractor | X |
tristimulus extractor | X |

Communications | Ready
---------- | ----------
arduino serial | X  |
key pressed | X  |
key released | X  |
midi key | X  |
midi knob | X  |
midi pad | X  |
midi receiver | X  |
midi sender | X  |
osc receiver | X  |
osc sender | X  |

Computer Vision | Ready
---------- | ----------
background subtraction | X  |
chroma key | X  |
color tracking | X  |
contour tracking | X  |
haar tracking | X |
motion detection | X |
optical flow | X |

Data | Ready
---------- | ----------
bang multiplexer | X  |
bang to float | X  |
color palette | X  |
data to texture | X  |
file to data | X  |
float multiplexer | X  |
floats to vector | X  |
texture to data | X  |
vector at | X  |
vector concat | X  |
vector extract | X  |
vector gate | X  |
vector operator | X  |

GUI | Ready
---------- | ----------
2D pad | X |
bang | X |
comment | X |
data viewer | X |
message | X |
player controls | X |
signal viewer | X |
slider | X |
sonogram | X |
timeline | X |
trigger | X |
value plotter | X |
video viewer | X |
vu meter | X |

Logic | Ready
---------- | ----------
boolean operator | X |
conditional operator | X |
counter | X |
delay bang | X |
delay float | X |
gate | X |
inverter | X |
loadbang | X |
spigot | X |
timed semaphore | X |

Math | Ready
---------- | ----------
1D noise | X |
clamp | X |
constant | X |
cosine generator | X |
map | X |
metronome | X |
operator | X |
simple random | X |
sine generator | X |
smooth | X |

Scripting | Ready
---------- | ----------
bash script | X |
glsl shader | X |
lua script | X |
python script | X |
scheme script | |


Sound | Ready
---------- | ----------
ADSR envelope | X |
AHR envelope | X |
amplifier | X |
audio exporter | X |
bit cruncher | X |
bit noise | X |
comb filter | X |
compressor | X |
crossfader | X |
data oscillator | X |
decimator | X |
delay | X |
dimension chorus | X |
high pass | X |
kick | X |
lfo | X |
low pass | X |
mixer | X |
note to frequency | X |
oscillator | X |
panner | X |
parametric EQ | | X
pd patch | X |
quad panner | X |
quantizer | | X
resonant filter | X |
reverb | X |
sample and hold | | X
saturator | | X
sequencer | X |
sidechain compressor | X |
signal gate | X |
signal operator | X |
signal trigger | X |
soundfile player | X |

Texture | Ready
---------- | ----------
image exporter | X |
image loader | X |
kinect grabber | X |
pixels to texture | X |
texture crop | X |
texture information | X |
texture mixer | X |
texture to pixels | X |
texture transform | X |
to grayscale texture | X |
syphon sender | X |
syphon receiver | X |
video exporter | X |
video feedback | X |
video gate | X |
video grabber | X |
video player | X |
video receiver | X |
video sender | X |
video streaming | X |
video timedelay | X |

Windowing | Ready
---------- | ----------
live patching | X |
output window | X |
projection mapping | X |

# LICENSE

[![license](https://img.shields.io/github/license/mashape/apistatus.svg)](LICENSE)

All contributions are made under the [MIT License](https://opensource.org/licenses/MIT). See [LICENSE](https://github.com/d3cod3/ofxVisualProgramming/blob/master/LICENSE.md).

# CREDITS

ofxAudioFile, ofxPDSP original addons by [Nicola Pisanti](https://github.com/npisanti)

ofxBTrack original addon by [Nao Tokui](https://github.com/naotokui)

ofxCv, ofxFft original addons by [Kyle McDonald](https://github.com/kylemcdonald)

ofxEasing original addon by [Arturo Castro](https://github.com/arturoc)

ofxFFmpegRecorder original addon by [Furkan Üzümcü](https://github.com/Furkanzmc)

ofxJSON original addon by [Jeff Crouse](https://github.com/jeffcrouse/)

ofxImGui original addon by [Jason Van Cleave](https://github.com/jvcleave)

ofxInfiniteCanvas original addon by [Roy Macdonald](https://github.com/roymacdonald)

ofxLua, ofxMidi, ofxPd original addons by [Dan Wilcox](https://github.com/danomatika)

ofxMtlMapping2D original addon by [morethanlogic](https://github.com/morethanlogic)

ofxNDI original addon by [Thomas Geissl](https://github.com/thomasgeissl)

ofxSyphon original addon by [Anthony Stellato](https://github.com/astellato)

ofxTimeline original addon by [James George and YCAM Interlab](https://github.com/YCAMInterlab/ofxTimeline)

ofxWarp original addon by [Elie Zananiri](https://github.com/prisonerjohn/ofxWarp)
