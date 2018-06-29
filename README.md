
[ofxVisualProgramming](https://github.com/d3cod3/ofxVisualProgramming)
======================

Linux, OSX [![Build status](https://travis-ci.org/d3cod3/ofxVisualProgramming.svg?branch=master)](https://travis-ci.org/d3cod3/ofxVisualProgramming)

Windows [![Build status](https://ci.appveyor.com/api/projects/status/65sk40q6y8bqfunw/branch/master?svg=true)](https://ci.appveyor.com/project/d3cod3/ofxvisualprogramming/branch/master)


### A visual programming patching environment for OF

Table of Contents
=================

   * [OF_COMPATIBLE_RELEASE](#of_compatible_release)
   * [REFERENCE](#reference)
   * [DESCRIPTION](#description)
   * [DEPENDENCIES](#dependencies)
   * [COMPILING](#compiling)
   * [INSTALLING](#installing)
   * [USAGE](#usage)
   * [EXAMPLE](#example)
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

#### [ofxBTrack](https://github.com/d3cod3/ofxBTrack)

#### [ofxDatGui](https://github.com/d3cod3/ofxDatGui)

#### [ofxFontStash](https://github.com/armadillu/ofxFontStash)

#### [ofxGLError](https://github.com/armadillu/ofxGLError)

#### [ofxHistoryPlot](https://github.com/armadillu/ofxHistoryPlot)

#### [ofxInfiniteCanvas](https://github.com/d3cod3/ofxInfiniteCanvas)

#### [ofxLua](https://github.com/d3cod3/ofxLua)

#### [ofxTimeMeasurements](https://github.com/armadillu/ofxTimeMeasurements)

#### ofxXmlSettings --> Core OF Addon

Some addons are forks of the original, due to some mods, compatibility with OF0.10 and the intention of cross-platform compiling (osx,linux,win)

# COMPILING

**OSX/LINUX Compiling with no problems.**

**Windows is compiling with qtcreator 4.6.1 only, for now.**

**Right now Appveyor build is failing on MSYS2 due to
_execvp: /mingw32/bin/g++: Argument list too long_**

**Visual Studio 2017, soon...**

# INSTALLING

Clone [this addon repository](https://github.com/d3cod3/ofxVisualProgramming) into your `<your_openframeworks_release_folder>/addons` together all the others addons listed:

```bash
cd <your_openframeworks_release_folder>/addons

git clone https://github.com/d3cod3/ofxAudioAnalyzer
git clone https://github.com/d3cod3/ofxBTrack
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
    ofSetDrawBitmapMode(OF_BITMAPMODE_SIMPLE);

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

# EXAMPLE

See the example_ofxVisualProgramming included

# CONTRIBUTING

Contributing to the project adding new objects is relatively easy, i'm going to show here a basic object template class, everyone is invited to fork ofxVisualProgramming addon, create new objects, and make a pull request. ONLY ONE PULL REQUEST FOR EACH NEW OBJECT WILL BE ACCEPTED, please do not make a pull request with thousands lines of new code!!!

So, the template, as this is a work in progress right now, i'm going to show a basic object template only.

The header of your new object (_SuperAmazingObject_, for instance) will be SuperAmazingObject.h :

```c

/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2018 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

    See https://github.com/d3cod3/ofxVisualProgramming for documentation

    SuperAmazingObject:
    developed by _object author_
    _author github_
    _author www_

==============================================================================*/

#pragma once

#include "PatchObject.h"

/*
  This is a simplified version of the SimpleRandom object already included in
  the ofxVisualProgramming addon:
  src/objects/math/SimpleRandom.h
  srcobjects/math/SimpleRandom.cpp
*/


class SuperAmazingObject : public PatchObject {

public:

    // constructor
    SuperAmazingObject();

    // inherit virtual methods from base class PatchObject
    // this are all methods available, you can use it all
    // or just the one you need.
    // for this simple object i'm going to leave commented
    // the ones i don't need
    void  newObject();
    void  setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow);
    void  updateObjectContent();
    void  drawObjectContent(ofxFontStash *font);
    void  removeObjectContent();
    //void  mouseMovedObjectContent(ofVec3f _m);
    //void  dragGUIObject(ofVec3f _m);
    //void  audioInObject(ofSoundBuffer &inputBuffer);
    //void  audioOutObject(ofSoundBuffer &outBuffer);

    // my SuperAmazingObject public variables (if any)
    // .............

};

```

And the source SuperAmazingObject.cpp :

```c

//--------------------------------------------------------------
SuperAmazingObject::SuperAmazingObject() : PatchObject(){

    // first, declare how many inlet/outlet the object have
    this->numInlets  = 2;
    this->numOutlets = 1;

    // then init the pointers
    // i have used here void* pointers
    // and established 5 fixed type of data to cast
    // and create proper connections

    // VP_LINK_NUMERIC --> float
    // VP_LINK_STRING  --> string
    // VP_LINK_ARRAY   --> vector<float>
    // VP_LINK_TEXTURE --> ofTexture
    // VP_LINK_AUDIO   --> ofSoundBuffer

    // These are the ONLY available connections
    // so we are creating here a random number generator
    // a really basic one will have two inlets, _min_ and _max_
    // for the random range
    // and one outlet, the random number

    // we create/cast the two inlets then as float
    _inletParams[0] = new float();  // min
    _inletParams[1] = new float();  // max
    // and init them with some start value
    *(float *)&_inletParams[0] = 0.0f;
    *(float *)&_inletParams[1] = 1.0f;

    // then we create/cast the outlet as float too
    _outletParams[0] = new float(); // output
    *(float *)&_outletParams[0] = 0.0f;

    // we call this to init some stuff
    // soon this will be DEPRECATED, a function call will
    // substitute this --- TODO
    for(int i=0;i<this->numInlets;i++){
        this->inletsConnected.push_back(false);
    }

}

//--------------------------------------------------------------
void SimpleRandom::newObject(){

    // first set the name of the object
    // (choose a cool one, no strange chars please)
    this->setName("super amazing object");

    // and then the inlets/outlets names/types
    this->addInlet(VP_LINK_NUMERIC,"min");
    this->addInlet(VP_LINK_NUMERIC,"max");
    this->addOutlet(VP_LINK_NUMERIC);
}

//--------------------------------------------------------------
void SimpleRandom::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    // setup your object here, as any ofApp example
    ofSeedRandom(ofGetElapsedTimeMillis());
}

//--------------------------------------------------------------
void SimpleRandom::updateObjectContent(){
    // update your object here, as any ofApp example

    // in this case we just need to update the outlet value
    // using the ofRandom function
    // (why not, at the end is just a random number generator object)
    *(float *)&_outletParams[0] = ofRandom(*(float *)&_inletParams[0],*(float *)&_inletParams[1]);
}

//--------------------------------------------------------------
void SimpleRandom::drawObjectContent(ofxFontStash *font){
    // draw your object content here, as any ofApp example
    // and if you need to write some text, just use the font
    // available with the method,
    ofSetColor(255);
    ofEnableAlphaBlending();
    font->draw(ofToString(*(float *)&_outletParams[0]),this->fontSize,this->width/2,this->headerHeight*2.3);
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void SimpleRandom::removeObjectContent(){
  // anything you need to remove or stop when deleting the object
}

```

And thats it! This is all for now, for every question just open an issue here on github, or send me an email.

More soon...
