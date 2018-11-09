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
// simplifyDP():
//  This is the Douglas-Peucker recursive simplification routine
//  It just marks vertices that are part of the simplified polyline
//  for approximating the polyline subchain v[j] to v[k].
//    Input:  tol = approximation tolerance
//            v[] = polyline array of vertex points
//            j,k = indices for the subchain v[j] to v[k]
//    Output: mk[] = array of markers matching vertex array v[]
static void simplifyDP( float tol, ofVec3f* v, int j, int k, int* mk ){
    if (k <= j+1) // there is nothing to simplify
        return;

    // check for adequate approximation by segment S from v[j] to v[k]
    int     maxi = j;          // index of vertex farthest from S
    float   maxd2 = 0;         // distance squared of farthest vertex
    float   tol2 = tol * tol;  // tolerance squared
    Segment_s S = {v[j], v[k]};  // segment from v[j] to v[k]
    ofVec3f  u;
    u = S.P1 - S.P0;   // segment direction vector
    double  cu = dot_mu(u,u);     // segment length squared

    // test each vertex v[i] for max distance from S
    // compute using the Feb 2001 Algorithm's dist_ofVec3f_to_Segment()
    // Note: this works in any dimension (2D, 3D, ...)
    ofVec3f  w;
    ofVec3f   Pb;                // base of perpendicular from v[i] to S
    double  b, cw, dv2;        // dv2 = distance v[i] to S squared

    for (int i=j+1; i<k; i++)
    {
        // compute distance squared
        w = v[i] - S.P0;
        cw = dot_mu(w,u);
        if ( cw <= 0 )
            dv2 = d2_mu(v[i], S.P0);
        else if ( cu <= cw )
            dv2 = d2_mu(v[i], S.P1);
        else {
            b = cw / cu;
            Pb = S.P0 + b * u;
            dv2 = d2_mu(v[i], Pb);
        }
        // test with current max distance squared
        if (dv2 <= maxd2)
            continue;
        // v[i] is a new max vertex
        maxi = i;
        maxd2 = dv2;
    }
    if (maxd2 > tol2)        // error is worse than the tolerance
    {
        // split the polyline at the farthest vertex from S
        mk[maxi] = 1;      // mark v[maxi] for the simplified polyline
        // recursively simplify the two subpolylines at v[maxi]
        simplifyDP( tol, v, j, maxi, mk );  // polyline v[j] to v[maxi]
        simplifyDP( tol, v, maxi, k, mk );  // polyline v[maxi] to v[k]
    }
    // else the approximation is OK, so ignore intermediate vertices
    return;
}

//===================================================================


// Copyright 2002, softSurfer (www.softsurfer.com)
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.

// Assume that classes are already given for the objects:
//    ofVec3f and Vector with
//        coordinates {float x, y, z;}    // as many as are needed
//        operators for:
//            == to test equality
//            != to test inequality
//            (Vector)0 = (0,0,0)         (null vector)
//            ofVec3f  = ofVec3f ± Vector
//            Vector = ofVec3f - ofVec3f
//            Vector = Vector ± Vector
//            Vector = Scalar * Vector    (scalar product)
//            Vector = Vector * Vector    (cross product)
//    Segment with defining endpoints {ofVec3f P0, P1;}
//===================================================================


// poly_simplify():
//    Input:  tol = approximation tolerance
//            V[] = polyline array of vertex points
//            n   = the number of points in V[]
//    Output: sV[]= simplified polyline vertices (max is n)
//    Return: m   = the number of points in sV[]
static int poly_simplify( float tol, ofVec3f* V, int n, ofVec3f* sV ){
    int    i, k, m, pv;            // misc counters
    float  tol2 = tol * tol;       // tolerance squared
    ofVec3f* vt = new ofVec3f[n];      // vertex buffer
    int			*mk = new int[n];  // marker buffer
    memset(mk, 0, sizeof(int) * n );

    // STAGE 1.  Vertex Reduction within tolerance of prior vertex cluster
    vt[0] = V[0];              // start at the beginning
    for (i=k=1, pv=0; i<n; i++) {
        if (d2_mu(V[i], V[pv]) < tol2)
            continue;
        vt[k++] = V[i];
        pv = i;
    }
    if (pv < n-1)
        vt[k++] = V[n-1];      // finish at the end

    // STAGE 2.  Douglas-Peucker polyline simplification
    mk[0] = mk[k-1] = 1;       // mark the first and last vertices
    simplifyDP( tol, vt, 0, k-1, mk );

    // copy marked vertices to the output simplified polyline
    for (i=m=0; i<k; i++) {
        if (mk[i])
            sV[m++] = vt[i];
    }
    delete[] vt;
    delete[] mk;
    return m;         // m vertices in simplified polyline
}

//--------------------------------------------------------------
static void smoothContour(vector <ofVec2f> &contourIn, vector <ofVec2f> &contourOut, float smoothPct){
    int length = MIN(contourIn.size(), contourOut.size());

    float invPct = 1.0 - smoothPct;

    //first copy the data
    for(int i = 0; i < length; i++){
        contourOut[i] = contourIn[i];
    }

    //then smooth the contour
    //we start at 1 as we take a small pct of the prev value
    for(int i = 1; i < length; i++){
        contourOut[i] = contourOut[i] * smoothPct   +   contourOut[i-1] * invPct;
    }
}

//--------------------------------------------------------------
static void simplifyContour(vector <ofVec2f> &contourIn, vector <ofVec2f> &contourOut, float tolerance){
    int length = contourIn.size();

    //the polyLine simplify class needs data as a vector of ofVec3fs
    ofVec3f		*polyLineIn = new ofVec3f[length];
    ofVec3f		*polyLineOut = new ofVec3f[length];

    //first we copy over the data to a 3d point array
    for(int i = 0; i < length; i++){
        polyLineIn[i].x = contourIn[i].x;
        polyLineIn[i].y = contourIn[i].y;
        polyLineIn[i].z = 0.0f;
        polyLineOut[i].x = 0.0f;
        polyLineOut[i].y = 0.0f;
        polyLineOut[i].z = 0.0f;
    }

    int numPoints = poly_simplify(tolerance, polyLineIn, length, polyLineOut);
    contourOut.clear();
    contourOut.assign(numPoints, ofVec2f());

    //copy the data to our contourOut vector
    for(int i = 0; i < numPoints; i++){
        contourOut[i].x = polyLineOut[i].x;
        contourOut[i].y = polyLineOut[i].y;
    }

    delete[] polyLineIn;
    delete[] polyLineOut;

}

//--------------------------------------------------------------
static void makeContourNormals(vector <ofVec2f> &contourIn, vector <ofVec2f> &normalsOut, int direction = 1){
    int length = contourIn.size();

    normalsOut.clear();
    normalsOut.assign(length, ofVec2f() );

    ofVec2f normal;
    ofVec2f delta;
    for(int i = 1; i < length; i++){
        delta = contourIn[i] - contourIn[i-1];
        delta.normalize();

        if(direction){
            normal.set(delta.y, -delta.x);
        }else{
            normal.set(-delta.y, delta.x);
        }

        normalsOut[i] = normal;
    }

}
