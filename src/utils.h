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

#include "ofxLua.h"
#if defined(TARGET_LINUX) || defined(TARGET_OSX)
#include <pwd.h>
#include "ofxPython.h"
#endif
#include "ofxEditor.h"

#include <math.h>
#include <string>
#include <iostream>
#include <fstream>

struct LiveCoding{
    ofxLua          lua;
    #if defined(TARGET_LINUX) || defined(TARGET_OSX)
    ofxPythonObject python;
    #endif
    ofxEditor       liveEditor;
    string          filepath;
    bool            hide;
};

struct Segment_s{
    ofVec3f P0;
    ofVec3f P1;
};

#define dot_mu(u,v)    ((u).x * (v).x + (u).y * (v).y + (u).z * (v).z)
#define norm2_mu(v)    dot_mu(v,v)        // norm2 = squared length of vector
#define norm_mu(v)     sqrt(norm2_mu(v))  // norm = length of vector
#define d2_mu(u,v)     norm2_mu(u-v)      // distance squared = norm2 of difference
#define d_mu(u,v)      norm_mu(u-v)       // distance = norm of difference

//--------------------------------------------------------------
static inline float hardClip(float x){
    float x1 = fabsf(x + 1.0f);
    float x2 = fabsf(x - 1.0f);

    return 0.5f * (x1 - x2);
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
    unsigned int minSize = 0;
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
static inline int* uniqueRandom(int dim) {
    int * keys = new int[dim];
    int * world = new int[dim];
    int magnitude = dim-1;
    int rr = (int)(ofRandomuf()*magnitude);

    for(int i=0;i<dim;i++){
        world[i] = i;
        keys[i] = 0;
    }

    for(int i=0;i<dim;i++){
        int pos = int(ofRandom(0,magnitude));
        keys[i] = world[pos];
        world[pos] = world[magnitude];
        magnitude--;
    }
    int buffer = keys[0];
    keys[0] = keys[rr];
    keys[rr] = buffer;
    return keys;
}

//--------------------------------------------------------------
static inline float gaussianFn(float x, float amplitude, float center, float width){
    float base = (x - center) / width; // divide top by bottom
    base *= base * -.5; // square top and bottom, multiply by -1/2
    base = exp(base); // take pow(e, base)
    return amplitude * base;
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

//--------------------------------------------------------------
inline bool checkFilenameError(string fn){
    #if defined(TARGET_LINUX) || defined(TARGET_OSX)
    if(fn.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_./") != string::npos){
        return true;
    }else{
        return false;
    }
    #else
    if(fn.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_.\/") != string::npos){
        return true;
    }else{
        return false;
    }
    #endif
}

//--------------------------------------------------------------
inline vector<string> recursiveScanDirectory(ofDirectory dir){
    size_t size;
    size = dir.listDir();
    dir.sort();

    vector<string> tempFileList;

    for (size_t i = 0; i < size; i++){
        if (dir.getFile(i).isDirectory()==1){
            ofDirectory newDir;
            newDir.listDir(dir.getFile(i).getAbsolutePath());
            recursiveScanDirectory( newDir );
        }else{
            tempFileList.push_back(dir.getPath(i));
        }
    }

    return tempFileList;
}

//--------------------------------------------------------------
inline string forceCheckMosaicDataPath(string filepath){
    ofFile file (filepath);

    if(file.exists()){
        return filepath;
    }else{
        if(filepath.find("Mosaic/data/") != string::npos || filepath.find("Mosaic/examples/") != string::npos) {
            size_t start = filepath.find("Mosaic/");
            string newPath = filepath.substr(start,filepath.size()-start);

            const char *homeDir = getenv("HOME");

            #if defined(TARGET_OSX)
            if(!homeDir){
                struct passwd* pwd;
                pwd = getpwuid(getuid());
                if (pwd){
                    homeDir = pwd->pw_dir;
                }
            }
            #endif

            string finalPath(homeDir);

            #if defined(TARGET_WIN32) || defined(TARGET_LINUX)
                finalPath = ofToDataPath("",true);
                finalPath = finalPath.substr(0,finalPath.size()-11); // cut "Mosaic/data/" at the end
                finalPath += newPath;
            #elif defined(TARGET_OSX)
                finalPath += "/Documents/"+newPath;
            #endif

            //ofLog(OF_LOG_NOTICE,"%s",finalPath.c_str());

            ofFile test(finalPath);

            if(test.exists()){
                return finalPath;
            }else{
                return filepath;
            }
        }else{
            return filepath;
        }
    }
}
