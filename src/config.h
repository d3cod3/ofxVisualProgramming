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

#define PACKAGE_OFXVP       "ofxVisualProgramming"
#define	VERSION_OFXVP       "0.1.0"
#define DESCRIPTION_OFXVP   "A visual programming patching environment for OF"


// choose modifier key based on platform
#ifdef TARGET_OSX
    #define MOD_KEY OF_KEY_SUPER
#else
    #define MOD_KEY OF_KEY_CONTROL
#endif


#define RETINA_MIN_WIDTH        2560
#define RETINA_MIN_HEIGHT       1600

#define STANDARD_PROJECTOR_WINDOW_WIDTH     854
#define STANDARD_PROJECTOR_WINDOW_HEIGHT    480

#define AUDIO_ANALYZER_WINDOW_WIDTH         564
#define AUDIO_ANALYZER_WINDOW_HEIGHT        808

#define OBJECT_STANDARD_WIDTH   160
#define OBJECT_STANDARD_HEIGHT  120

#define OBJECT_WIDTH            OBJECT_STANDARD_WIDTH
#define OBJECT_HEIGHT           OBJECT_STANDARD_HEIGHT
#define HEADER_HEIGHT           16
#define MAX_INLETS              24
#define MAX_OUTLETS             24

#define COLOR_NUMERIC_LINK   ofColor(250,250,250,255)
#define COLOR_STRING_LINK    ofColor(230,210,255,255)
#define COLOR_ARRAY_LINK     ofColor(255,255,200,255)
#define COLOR_TEXTURE_LINK   ofColor(200,255,255,255)
#define COLOR_AUDIO_LINK     ofColor(255,255,200,255)
#define COLOR_SCRIPT_LINK    ofColor(255,180,180,255)

#define COLOR_NUMERIC        ofColor(210,210,210,255)
#define COLOR_STRING         ofColor(200,180,255,255)
#define COLOR_ARRAY          ofColor(200,255,200,255)
#define COLOR_TEXTURE        ofColor(120,255,255,255)
#define COLOR_AUDIO          ofColor(255,255,120,255)
#define COLOR_SCRIPT         ofColor(255,128,128,255)

#define MAIN_FONT            "ofxbraitsch/fonts/Verdana.ttf"
#define LIVECODING_FONT      "fonts/IBMPlexSans-Text.otf"

#define LUA_SYNTAX           "livecoding/luaSyntax.xml"
#define PYTHON_SYNTAX        "livecoding/pythonSyntax.xml"

#define LIVECODING_COLORS    "livecoding/colorScheme.xml"
