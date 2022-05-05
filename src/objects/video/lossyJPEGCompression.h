/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2022 Daan de Lange

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

// This Object compresses images using the JPEG algorithm
// Note: Based on code from KarmaMapper : https://github.com/Daandelange/karmaMapper/tree/master/src/effects/lossyImageEffect
// Note: The threaded stuff needs to be figured out, there's a draft.

#ifndef OFXVP_BUILD_WITH_MINIMAL_OBJECTS

#pragma once

#include "PatchObject.h"

#include "ofxTurboJpeg.h"

class lossyJPEGCompression : public PatchObject, public ofThread {

public:

    lossyJPEGCompression();

    void            newObject() override;
    void            setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow) override;
    void            updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects) override;

    void            drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer) override;
    void            drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ) override;
    void            drawObjectNodeConfig() override;

    void            removeObjectContent(bool removeFileFromData=false) override;

    void            threadedFunction() override;
    void            threadablePixelsFunction( ofPixels& pixels);
    void            stopTurboThread();
    void            startTurboThread(const bool restart=false);


    ofPixels                    *inPixels;
    ofPixels                    outPixels;

    bool                        bInitialised;

    float fJpegQuality;
    bool bIsThreaded;
private:
    bool bHasNewFrameThreaded;

	ofThreadChannel<ofPixels> toCompress;
	ofThreadChannel<ofPixels> receiveCompressed;

	tjhandle handleCompress;
	tjhandle handleDecompress;

    OBJECT_FACTORY_PROPS

};

#endif
