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

#include "moSignalViewer.h"

//--------------------------------------------------------------
moSignalViewer::moSignalViewer() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();  // signal

    _outletParams[0] = new ofSoundBuffer();  // signal

    this->initInletsState();

    this->isBigGuiViewer    = true;
    this->width             *= 2;
}

//--------------------------------------------------------------
void moSignalViewer::newObject(){
    this->setName("signal viewer");
    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addOutlet(VP_LINK_AUDIO);
}

//--------------------------------------------------------------
void moSignalViewer::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    
}

//--------------------------------------------------------------
void moSignalViewer::updateObjectContent(map<int,PatchObject*> &patchObjects){
    if(this->inletsConnected[0]){
        waveform.clear();
        for(size_t i = 0; i < static_cast<ofSoundBuffer *>(_inletParams[0])->getNumFrames(); i++) {
            float sample = static_cast<ofSoundBuffer *>(_inletParams[0])->getSample(i,0);
            float x = ofMap(i, 0, static_cast<ofSoundBuffer *>(_inletParams[0])->getNumFrames(), 0, this->width);
            float y = ofMap(hardClip(sample), -1, 1, 0, this->height);
            waveform.addVertex(x, y);
        }
        *static_cast<ofSoundBuffer *>(_outletParams[0]) = *static_cast<ofSoundBuffer *>(_inletParams[0]);
    }

}

//--------------------------------------------------------------
void moSignalViewer::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    waveform.draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void moSignalViewer::removeObjectContent(){
    
}
