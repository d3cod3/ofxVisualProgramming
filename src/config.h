#pragma once

#define PACKAGE	"ofxVisualProgramming"
#define	VERSION	"0.1.0"
#define DESCRIPTION "A visual programming patching environment for OF"


// choose modifier key based on platform
#ifdef TARGET_OSX
    #define MOD_KEY OF_KEY_SUPER
#else
    #define MOD_KEY OF_KEY_CONTROL
#endif


#define RETINA_MIN_WIDTH        2560
#define RETINA_MIN_HEIGHT       1600

#define OBJECT_STANDARD_WIDTH   160
#define OBJECT_STANDARD_HEIGHT  120

#define OBJECT_WIDTH            OBJECT_STANDARD_WIDTH
#define OBJECT_HEIGHT           OBJECT_STANDARD_HEIGHT
#define HEADER_HEIGHT           16
#define MAX_INLETS              6
#define MAX_OUTLETS             6

#define COLOR_NUMERIC_LINK   ofColor(250,250,250,255)
#define COLOR_STRING_LINK    ofColor(230,210,255,255)
#define COLOR_ARRAY_LINK     ofColor(255,255,200,255)
#define COLOR_TEXTURE_LINK   ofColor(200,255,255,255)
#define COLOR_AUDIO_LINK     ofColor(255,255,200,255)

#define COLOR_NUMERIC        ofColor(210,210,210,255)
#define COLOR_STRING         ofColor(200,180,255,255)
#define COLOR_ARRAY          ofColor(200,255,200,255)
#define COLOR_TEXTURE        ofColor(120,255,255,255)
#define COLOR_AUDIO          ofColor(255,255,120,255)

#define MAIN_FONT            "ofxbraitsch/fonts/Verdana.ttf"
