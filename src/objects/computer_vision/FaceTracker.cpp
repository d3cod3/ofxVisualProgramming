/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2019 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "FaceTracker.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
FaceTracker::FaceTracker() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 2;

    _inletParams[0] = new ofTexture();  // input

    _outletParams[0] = new ofTexture(); // output texture
    _outletParams[1] = new vector<float>(); // face tracker data

    this->initInletsState();

    posX = posY = drawW = drawH = 0.0f;

    pix                 = new ofPixels();
    outputFBO           = new ofFbo();

    isFBOAllocated      = false;

}

//--------------------------------------------------------------
void FaceTracker::newObject(){
    this->setName("face tracker");
    this->addInlet(VP_LINK_TEXTURE,"input");
    this->addOutlet(VP_LINK_TEXTURE,"output");
    this->addOutlet(VP_LINK_ARRAY,"faceData");

}

//--------------------------------------------------------------
void FaceTracker::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    tracker.setRescale(1.0);
    tracker.setIterations(5);
    tracker.setClamp(3.0);
    tracker.setTolerance(0.01);
    tracker.setAttempts(1);
    tracker.setup();
}

//--------------------------------------------------------------
void FaceTracker::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        
        if(!isFBOAllocated){
            isFBOAllocated = true;
            pix             = new ofPixels();
            outputFBO->allocate(static_cast<ofTexture *>(_inletParams[0])->getWidth(),static_cast<ofTexture *>(_inletParams[0])->getHeight(),GL_RGB,1);
        }

        static_cast<ofTexture *>(_inletParams[0])->readToPixels(*pix);

        tracker.update(toCv(*pix));

        if(outputFBO->isAllocated()){
            *static_cast<ofTexture *>(_outletParams[0]) = outputFBO->getTexture();

            static_cast<vector<float> *>(_outletParams[1])->clear();

            if(tracker.getFound()) {
                static_cast<vector<float> *>(_outletParams[1])->push_back(tracker.getPosition().x);
                static_cast<vector<float> *>(_outletParams[1])->push_back(tracker.getPosition().y);
                static_cast<vector<float> *>(_outletParams[1])->push_back(tracker.getScale());
                static_cast<vector<float> *>(_outletParams[1])->push_back(tracker.getOrientation().x);
                static_cast<vector<float> *>(_outletParams[1])->push_back(tracker.getOrientation().y);
                static_cast<vector<float> *>(_outletParams[1])->push_back(tracker.getOrientation().z);

                static_cast<vector<float> *>(_outletParams[1])->push_back(tracker.getGesture(ofxFaceTracker::MOUTH_WIDTH));
                static_cast<vector<float> *>(_outletParams[1])->push_back(tracker.getGesture(ofxFaceTracker::MOUTH_HEIGHT));
                static_cast<vector<float> *>(_outletParams[1])->push_back(tracker.getGesture(ofxFaceTracker::LEFT_EYEBROW_HEIGHT));
                static_cast<vector<float> *>(_outletParams[1])->push_back(tracker.getGesture(ofxFaceTracker::RIGHT_EYEBROW_HEIGHT));
                static_cast<vector<float> *>(_outletParams[1])->push_back(tracker.getGesture(ofxFaceTracker::LEFT_EYE_OPENNESS));
                static_cast<vector<float> *>(_outletParams[1])->push_back(tracker.getGesture(ofxFaceTracker::RIGHT_EYE_OPENNESS));
                static_cast<vector<float> *>(_outletParams[1])->push_back(tracker.getGesture(ofxFaceTracker::JAW_OPENNESS));
                static_cast<vector<float> *>(_outletParams[1])->push_back(tracker.getGesture(ofxFaceTracker::NOSTRIL_FLARE));

                static_cast<vector<float> *>(_outletParams[1])->push_back(tracker.getImagePoints().size());

                for(ofVec2f p : tracker.getImagePoints()) {
                    static_cast<vector<float> *>(_outletParams[1])->push_back(p.x);
                    static_cast<vector<float> *>(_outletParams[1])->push_back(p.y);
                }
            }
        }

    }else{
        isFBOAllocated = false;
    }

}

//--------------------------------------------------------------
void FaceTracker::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(this->inletsConnected[0] && outputFBO->isAllocated() && static_cast<ofTexture *>(_outletParams[0])->isAllocated()){

        outputFBO->begin();
        ofClear(0,0,0,255);

        ofSetColor(255);
        static_cast<ofTexture *>(_inletParams[0])->draw(0,0);

        if(tracker.getFound()) {
            ofSetLineWidth(1);
            tracker.getImageMesh().drawWireframe();
        }else{
            ofSetColor(0,255,0);
            ofDrawBitmapString("searching for face...", 20, this->height);
        }

        outputFBO->end();

        if(static_cast<ofTexture *>(_outletParams[0])->getWidth()/static_cast<ofTexture *>(_outletParams[0])->getHeight() >= this->width/this->height){
            if(static_cast<ofTexture *>(_outletParams[0])->getWidth() > static_cast<ofTexture *>(_outletParams[0])->getHeight()){   // horizontal texture
                drawW           = this->width;
                drawH           = (this->width/static_cast<ofTexture *>(_outletParams[0])->getWidth())*static_cast<ofTexture *>(_outletParams[0])->getHeight();
                posX            = 0;
                posY            = (this->height-drawH)/2.0f;
            }else{ // vertical texture
                drawW           = (static_cast<ofTexture *>(_outletParams[0])->getWidth()*this->height)/static_cast<ofTexture *>(_outletParams[0])->getHeight();
                drawH           = this->height;
                posX            = (this->width-drawW)/2.0f;
                posY            = 0;
            }
        }else{ // always considered vertical texture
            drawW           = (static_cast<ofTexture *>(_outletParams[0])->getWidth()*this->height)/static_cast<ofTexture *>(_outletParams[0])->getHeight();
            drawH           = this->height;
            posX            = (this->width-drawW)/2.0f;
            posY            = 0;
        }
        static_cast<ofTexture *>(_outletParams[0])->draw(posX,posY,drawW,drawH);
    }
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void FaceTracker::removeObjectContent(bool removeFileFromData){
    tracker.stopThread();
    tracker.waitForThread();
}
