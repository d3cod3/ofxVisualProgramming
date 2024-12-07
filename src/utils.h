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

#if defined(TARGET_LINUX) || defined(TARGET_OSX)
#include <pwd.h>
#endif

#include <math.h>
#include <random>
#include <string>
#include <iostream>
#include <fstream>

#include "imgui_node_canvas.h"

// MACROS
#if defined(TARGET_LINUX) || defined(TARGET_OSX)
#define sprintf_s(buf, ...) snprintf((buf), sizeof(buf), __VA_ARGS__)
#endif

// this macro is used to silent unused variables warnings on virtual functions
template <typename... Ts> void unusedArgs(const Ts&...) {}


//--------------------------------------------------------------
inline std::string random_string( size_t length ){

    static auto& chrs = "0123456789"
                        "abcdefghijklmnopqrstuvwxyz"
                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                        "!?*+{}[]()=$-_";

    thread_local static std::mt19937 rg{std::random_device{}()};
    thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

    std::string s;

    s.reserve(length);

    while(length--)
        s += chrs[pick(rg)];

    return s;
}

//--------------------------------------------------------------
inline bool containString(string str1, string str2) {
    return (str1.find(str2) != string::npos);
}

//--------------------------------------------------------------
inline std::string& fix_newlines(std::string& s){
    size_t start_pos = 0;
    while((start_pos = s.find("\\n", start_pos)) != std::string::npos) {
         s.replace(start_pos, 2, "\n");
         start_pos += 1;
    }
    return s;
}

//--------------------------------------------------------------
inline void stringReplaceChar(std::string& s, std::string x, std::string y){
    size_t pos;
    while ((pos = s.find(x)) != std::string::npos) {
        s.replace(pos, 1, y);
    }
}

//--------------------------------------------------------------
inline std::string& sanitizeFilename(std::string& s){
    stringReplaceChar(s,std::string(" "), std::string("_"));
    return s;
}

//--------------------------------------------------------------
inline std::vector<std::string> splitStringWithDelimiter(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

//--------------------------------------------------------------
inline int nthOccurrence(const std::string& str, const std::string& findMe, int nth){
    size_t  pos = 0;
    int     cnt = 0;

    while( cnt != nth )
    {
        pos+=1;
        pos = str.find(findMe, pos);
        if ( pos == std::string::npos )
            return -1;
        cnt++;
    }
    return pos;
}

//--------------------------------------------------------------
static inline float hardClip(float x){
    float x1 = fabsf(x + 1.0f);
    float x2 = fabsf(x - 1.0f);

    return 0.5f * (x1 - x2);
}

//--------------------------------------------------------------
inline ofVec3f ofRandVec3f() {
    return ofVec3f(ofRandomf(),ofRandomf(),ofRandomf()).normalize().scale(ofRandomf());
}

//--------------------------------------------------------------
inline float fastSqrt(float x) {
    float xhalf = 0.5f * x;
    int i = *(int*)&x;            // store floating-point bits in integer
    i = 0x5f3759df - (i >> 1);    // initial guess for Newton's method
    x = *(float*)&i;              // convert new bits into float
    x = x*(1.5f - xhalf*x*x);     // One round of Newton's method
    return 1.0f / x;
}

//--------------------------------------------------------------
inline bool isInteger(const std::string & s){
   if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false ;

   char * p ;
   strtol(s.c_str(), &p, 10) ;

   return (*p == 0) ;
}

//--------------------------------------------------------------
inline bool isFloat(const std::string & s){
    std::string::const_iterator it = s.begin();
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
    std::string result = "";
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
inline std::string checkFileExtension(std::string filename, std::string ext, std::string extNeeded){
    std::string fileExtension = ofToUpper(ext);
    std::string newFilename = filename;
    if(fileExtension != extNeeded) {
        newFilename = filename+"."+ofToLower(extNeeded);
    }
    return newFilename;
}

//--------------------------------------------------------------
inline void removeFile(std::string filepath){
    ofFile fileToRemove(filepath);
    if(fileToRemove.exists()){
        if(fileToRemove.isDirectory()){
            ofDirectory temp;
            temp.removeDirectory(fileToRemove.getAbsolutePath(),true,true);
        }else{
            fileToRemove.remove();
        }
    }
}

//--------------------------------------------------------------
inline std::string copyFileToPatchFolder(std::string folderpath, std::string filepath){

    ofFile fileToRead(filepath);

    if(folderpath != fileToRead.getEnclosingDirectory()){
        ofFile patchDataFolder(folderpath);
        if(!patchDataFolder.exists()){
            patchDataFolder.create();
        }
        ofFile patchDataFolder2(folderpath);

        std::string fileToSave = patchDataFolder2.getAbsolutePath()+fileToRead.getFileName();
        ofFile newFile (fileToSave);

        //ofLog(OF_LOG_NOTICE,"%s",fileToSave.c_str());

        ofFile::copyFromTo(fileToRead.getAbsolutePath(),newFile.getAbsolutePath(),true,true);

        return folderpath+newFile.getFileName();
    }else{
        return filepath;
    }


}

//--------------------------------------------------------------
inline std::string forceCheckMosaicDataPath(std::string filepath){
    ofFile file (filepath);

    if(file.exists()){
        return filepath;
    }else{
        if(filepath.find("Mosaic/data/") != std::string::npos || filepath.find("Mosaic/examples/") != string::npos) {
            size_t start = filepath.find("Mosaic/");
            std::string newPath = filepath.substr(start,filepath.size()-start);

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

            std::string finalPath(homeDir);

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

//--------------------------------------------------------------
inline void drawNodeOFTexture(ofTexture &tex, float &px, float &py, float &w, float &h, float originX, float originY, float scaledW, float scaledH, float zoom, float retinaScale=1.0f, bool hasInlets=true){

    if(tex.isAllocated()){
        if(tex.getWidth()/tex.getHeight() >= scaledW/scaledH){
            if(tex.getWidth() > tex.getHeight()){   // horizontal texture
                w           = scaledW;
                h           = (scaledW/tex.getWidth())*tex.getHeight();
                px          = 0;
                py          = (scaledH-h)/2.0f;
            }else{ // vertical texture
                w           = (tex.getWidth()*scaledH)/tex.getHeight();
                h           = scaledH;
                px          = (scaledW-w)/2.0f;
                py          = 0;
            }
        }else{ // always considered vertical texture
            w               = (tex.getWidth()*scaledH)/tex.getHeight();
            h               = scaledH;
            px              = (scaledW-w)/2.0f;
            py              = 0;
        }

        // background
        ofSetColor(34,34,34);
        if(hasInlets){
            ofDrawRectangle(originX-(IMGUI_EX_NODE_PINS_WIDTH_NORMAL*retinaScale/zoom),originY-(IMGUI_EX_NODE_HEADER_HEIGHT*retinaScale/zoom),scaledW + (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*retinaScale/zoom),scaledH + ((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*retinaScale/zoom) );
        }else{
            ofDrawRectangle(originX,originY-(IMGUI_EX_NODE_HEADER_HEIGHT*retinaScale/zoom),scaledW,scaledH + ((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*retinaScale/zoom) );
        }

        // texture
        ofSetColor(255);
        tex.draw(px+originX,py+originY,w-(2*retinaScale),h);
    }else{
        // background
        ofSetColor(34,34,34);
        if(hasInlets){
            ofDrawRectangle(originX-(IMGUI_EX_NODE_PINS_WIDTH_NORMAL*retinaScale/zoom),originY-(IMGUI_EX_NODE_HEADER_HEIGHT*retinaScale/zoom),scaledW + (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*retinaScale/zoom),scaledH + ((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*retinaScale/zoom) );
        }else{
            ofDrawRectangle(originX,originY-(IMGUI_EX_NODE_HEADER_HEIGHT*retinaScale/zoom),scaledW,scaledH + ((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*retinaScale/zoom) );
        }
    }

}

//--------------------------------------------------------------
inline void calcTextureDims(ofTexture &tex, float &px, float &py, float &w, float &h, float originX, float originY, float scaledW, float scaledH, float zoom, float retinaScale=1.0f, bool hasInlets=true){

    if(tex.isAllocated()){
        if(tex.getWidth()/tex.getHeight() >= scaledW/scaledH){
            if(tex.getWidth() > tex.getHeight()){   // horizontal texture
                w           = scaledW;
                h           = (scaledW/tex.getWidth())*tex.getHeight();
                px          = 0;
                py          = (scaledH-h)/2.0f;
            }else{ // vertical texture
                w           = (tex.getWidth()*scaledH)/tex.getHeight();
                h           = scaledH;
                px          = (scaledW-w)/2.0f;
                py          = 0;
            }
        }else{ // always considered vertical texture
            w               = (tex.getWidth()*scaledH)/tex.getHeight();
            h               = scaledH;
            px              = (scaledW-w)/2.0f;
            py              = 0;
        }
    }

}
