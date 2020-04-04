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


#define RETINA_MIN_WIDTH                    2560
#define RETINA_MIN_HEIGHT                   1600

#define OUTPUT_TEX_MAX_WIDTH                4800
#define OUTPUT_TEX_MAX_HEIGHT               4800

#define STANDARD_PROJECTOR_WINDOW_WIDTH     854
#define STANDARD_PROJECTOR_WINDOW_HEIGHT    480

#define STANDARD_TEXTURE_WIDTH              1280
#define STANDARD_TEXTURE_HEIGHT             720

#define OBJECT_STANDARD_WIDTH   160
#define OBJECT_STANDARD_HEIGHT  120

#define OBJECT_WIDTH            OBJECT_STANDARD_WIDTH
#define OBJECT_HEIGHT           OBJECT_STANDARD_HEIGHT
#define HEADER_HEIGHT           16
#define MAX_INLETS              24
#define MAX_OUTLETS             24

#define COLOR_NUMERIC_LINK      ofColor(210,210,210,255)
#define COLOR_STRING_LINK       ofColor(200,180,255,255)
#define COLOR_ARRAY_LINK        ofColor(120,255,120,255)
#define COLOR_PIXELS_LINK       ofColor(0,180,140,255)
#define COLOR_TEXTURE_LINK      ofColor(120,255,255,255)
#define COLOR_AUDIO_LINK        ofColor(255,255,120,255)
#define COLOR_SCRIPT_LINK       ofColor(255,128,128,255)

#define COLOR_NUMERIC           ofColor(210,210,210,255)
#define COLOR_STRING            ofColor(200,180,255,255)
#define COLOR_ARRAY             ofColor(120,255,120,255)
#define COLOR_PIXELS            ofColor(0,180,140,255)
#define COLOR_TEXTURE           ofColor(120,255,255,255)
#define COLOR_AUDIO             ofColor(255,255,120,255)
#define COLOR_SCRIPT            ofColor(255,128,128,255)
#define COLOR_UNKNOWN           ofColor(255,126,000,255)

#define MAIN_FONT               "ofxbraitsch/fonts/Verdana.ttf"
#define LIVECODING_FONT         "fonts/IBMPlexMono-Medium.ttf"

#define PLUGINS_FOLDER          "plugins/"

#define LUA_SYNTAX              "livecoding/luaSyntax.xml"
#define PYTHON_SYNTAX           "livecoding/pythonSyntax.xml"

#define LIVECODING_COLORS       "livecoding/colorScheme.xml"

#define	NOTES                           128
#define MOSAIC_DEFAULT_BUFFER_SIZE     1024

#define OFXVP_OBJECT_CAT_AUDIOANALYSIS  "audio analysis"
#define OFXVP_OBJECT_CAT_COMMUNICATIONS "communications"
#define OFXVP_OBJECT_CAT_CV             "computer vision"
#define OFXVP_OBJECT_CAT_DATA           "data"
#define OFXVP_OBJECT_CAT_GRAPHICS       "graphics"
#define OFXVP_OBJECT_CAT_GUI            "gui"
#define OFXVP_OBJECT_CAT_LOGIC          "logic"
#define OFXVP_OBJECT_CAT_MATH           "math"
#define OFXVP_OBJECT_CAT_SCRIPTING      "scripting"
#define OFXVP_OBJECT_CAT_SOUND          "sound"
#define OFXVP_OBJECT_CAT_VIDEO          "video"
#define OFXVP_OBJECT_CAT_WINDOWING      "windowing"
