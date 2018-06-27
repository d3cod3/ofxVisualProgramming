
[ofxVisualProgramming](https://github.com/d3cod3/ofxVisualProgramming)
======================

Linux, OSX [![Build status](https://travis-ci.org/d3cod3/ofxVisualProgramming.svg?branch=master)](https://travis-ci.org/d3cod3/ofxVisualProgramming)

Windows [![Build status](https://ci.appveyor.com/api/projects/status/65sk40q6y8bqfunw/branch/master?svg=true)](https://ci.appveyor.com/project/d3cod3/ofxvisualprogramming/branch/master)


### A visual programming patching environment for OF

Table of Contents
=================

   * [OF_COMPATIBLE_RELEASE](#of_compatible_release)
   * [REFERENCE](#reference)
   * [DESCRIPTION](#reference)
   * [DEPENDENCIES](#reference)
   * [INSTALLING](#reference)
   * [USAGE](#reference)
   * [EXAMPLE](#reference)
   * [REFERENCE](#reference)
   * [CONTRIBUTING](#contributing)


# OF COMPATIBLE RELEASE

#### 0.10.0
Compiled/tested with QTCreator on osx/linux/windows

# REFERENCE

ofxVisualProgramming came directly from the idea behind the ofxComposer addon by [patriciogonzalezvivo](https://github.com/patriciogonzalezvivo/ofxComposer), and obviously from the various commercial and no-commercial existing visual programming softwares, Pure Data, Max/Msp, TouchDesigner, etc...
So special thanks to all the precursors of this ideas, and more thanks to the ofxComposer developer for their code, it has been a great reference to start working on this ofxaddon.

# DESCRIPTION

# DEPENDENCIES

#### [ofxAudioAnalyzer](https://github.com/d3cod3/ofxAudioAnalyzer)

#### [ofxBPMDetector](https://github.com/d3cod3/ofxBPMDetector)

#### [ofxDatGui](https://github.com/d3cod3/ofxDatGui)

#### [ofxFontStash](https://github.com/armadillu/ofxFontStash)

#### [ofxGLError](https://github.com/armadillu/ofxGLError)

#### [ofxHistoryPlot](https://github.com/armadillu/ofxHistoryPlot)

#### [ofxInfiniteCanvas](https://github.com/d3cod3/ofxInfiniteCanvas)

#### [ofxLua](https://github.com/d3cod3/ofxLua)

#### [ofxTimeMeasurements](https://github.com/armadillu/ofxTimeMeasurements)

#### ofxXmlSettings --> Core OF Addon

Some addons are forks of the original, due to some mods, compatibility with OF0.10 and the intention of cross-platform compiling (osx,linux,win)

# INSTALLING

Clone [this addon repository](https://github.com/d3cod3/ofxVisualProgramming) into your `<your_openframeworks_release_folder>/addons` together all the others addons listed:

```bash
cd <your_openframeworks_release_folder>/addons

git clone https://github.com/d3cod3/ofxAudioAnalyzer
git clone https://github.com/d3cod3/ofxBPMDetector
git clone https://github.com/d3cod3/ofxDatGui
git clone https://github.com/armadillu/ofxFontStash
git clone https://github.com/armadillu/ofxGLError
git clone https://github.com/armadillu/ofxHistoryPlot
git clone https://github.com/d3cod3/ofxInfiniteCanvas
git clone https://github.com/armadillu/ofxTimeMeasurements
git clone https://github.com/d3cod3/ofxVisualProgramming
```

Then, if on Linux/osx:

```bash
cd <your_openframeworks_release_folder>/addons

git clone --branch=of-0.10.0 https://github.com/d3cod3/ofxLua
```

or windows:

```bash
cd <your_openframeworks_release_folder>/addons

git clone --branch=windows https://github.com/d3cod3/ofxLua
```

# USAGE


# EXAMPLE

See the example_ofxVisualProgramming included

# CONTRIBUTING

Objects can be easily added, recipie soon...
