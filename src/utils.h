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

#include "ofMain.h"

#include <math.h>
#include <string>
#include <iostream>
#include <fstream>

//--------------------------------------------------------------
static inline float hardClip(float x){
    float x1 = fabsf(x + 1.0f);
    float x2 = fabsf(x - 1.0f);

    return 0.5f * (x1 - x2);
}

//--------------------------------------------------------------
static inline uint64_t devURandom(){
    uint64_t r = 0;
    size_t size = sizeof(r);
    ifstream urandom("/dev/urandom",ios::in|ios::binary);
    if(urandom){
        urandom.read(reinterpret_cast<char*>(&r),size);
        if(urandom){
            return r;
        }else{
            return static_cast<uint64_t>(ofRandom(1000000));
        }
    }else{
        return static_cast<uint64_t>(ofRandom(1000000));
    }
}

//--------------------------------------------------------------
static inline float gaussianRandom(){
    float v1, v2, s;
    do{
        v1 = 2 * ofRandomf() - 1;
        v2 = 2 * ofRandomf() - 1;
        s = v1 * v1 + v2 * v2;
    }while (s >= 1 || s == 0);

    if(ofRandomf() < 0.0f){
        return v1 * sqrt(-2 * log(s)/s);
    }else{
        return v2 * sqrt(-2 * log(s)/s);
    }

}

//--------------------------------------------------------------
inline bool isInteger(const string & s){
   if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false ;

   char * p ;
   strtol(s.c_str(), &p, 10) ;

   return (*p == 0) ;
}

//--------------------------------------------------------------
inline bool isFloat(const string & s){
    string::const_iterator it = s.begin();
    bool decimalPoint = false;
    int minSize = 0;
    if(s.size()>0 && (s[0] == '-' || s[0] == '+')){
        it++;
        minSize++;
    }
    while(it != s.end()){
        if(*it == '.'){
            if(!decimalPoint) decimalPoint = true;
            else break;
        }else if(!isdigit(*it) && ((*it!='f') || it+1 != s.end() || !decimalPoint)){
            break;
        }
        ++it;
    }
    return s.size()>minSize && it == s.end() && decimalPoint;
}

//--------------------------------------------------------------
inline std::string execCmd(const char* cmd){
    char buffer[128];
    string result = "";
#ifdef TARGET_LINUX
    FILE* pipe = popen(cmd, "r");
#elif defined(TARGET_OSX)
    FILE* pipe = popen(cmd, "r");
#elif defined(TARGET_WIN32)
    FILE* pipe = _popen(cmd, "r");
#endif

    if (!pipe) throw runtime_error("popen() failed!");
    try {
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
    } catch (...) {
#ifdef TARGET_LINUX
        pclose(pipe);
#elif defined(TARGET_OSX)
        pclose(pipe);
#elif defined(TARGET_WIN32)
        _pclose(pipe);
#endif
        throw;
    }
#ifdef TARGET_LINUX
    pclose(pipe);
#elif defined(TARGET_OSX)
    pclose(pipe);
#elif defined(TARGET_WIN32)
    _pclose(pipe);
#endif
    return result;
}
