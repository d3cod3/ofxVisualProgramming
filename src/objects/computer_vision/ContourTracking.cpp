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

#include "ContourTracking.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
ContourTracking::ContourTracking() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 4;

    _inletParams[0] = new ofTexture();  // input texture
    _outletParams[0] = new ofTexture(); // output texture (for visualization)
    _outletParams[1] = new vector<float>();  // blobs vector
    _outletParams[2] = new vector<float>();  // contour vector
    _outletParams[3] = new vector<float>();  // convex hull vector

    this->initInletsState();

    contourFinder   = new ofxCv::ContourFinder();
    pix             = new ofPixels();
    outputFBO       = new ofFbo();

    isGUIObject         = true;
    this->isOverGUI     = true;

    posX = posY = drawW = drawH = 0.0f;

    isFBOAllocated      = false;

}

//--------------------------------------------------------------
void ContourTracking::newObject(){
    this->setName("contour tracking");
    this->addInlet(VP_LINK_TEXTURE,"input");
    this->addOutlet(VP_LINK_TEXTURE,"output");
    this->addOutlet(VP_LINK_ARRAY,"blobsData");
    this->addOutlet(VP_LINK_ARRAY),"contourData";
    this->addOutlet(VP_LINK_ARRAY,"convexHullData");

    this->setCustomVar(static_cast<float>(0.0),"INVERT_BW");
    this->setCustomVar(static_cast<float>(128.0),"THRESHOLD");
    this->setCustomVar(static_cast<float>(10.0),"MIN_AREA_RADIUS");
    this->setCustomVar(static_cast<float>(200.0),"MAX_AREA_RADIUS");
}

//--------------------------------------------------------------
void ContourTracking::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    invertBW = gui->addToggle("INVERT",static_cast<int>(floor(this->getCustomVar("INVERT_BW"))));
    invertBW->setUseCustomMouse(true);
    thresholdValue = gui->addSlider("THRESH",0,255);
    thresholdValue->setUseCustomMouse(true);
    thresholdValue->setValue(static_cast<double>(this->getCustomVar("THRESHOLD")));
    minAreaRadius = gui->addSlider("MIN.AREA",4,100);
    minAreaRadius->setUseCustomMouse(true);
    minAreaRadius->setValue(static_cast<double>(this->getCustomVar("MIN_AREA_RADIUS")));
    maxAreaRadius = gui->addSlider("MAX.AREA",101,500);
    maxAreaRadius->setUseCustomMouse(true);
    maxAreaRadius->setValue(static_cast<double>(this->getCustomVar("MAX_AREA_RADIUS")));

    gui->onToggleEvent(this, &ContourTracking::onToggleEvent);
    gui->onSliderEvent(this, &ContourTracking::onSliderEvent);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    contourFinder->setMinAreaRadius(minAreaRadius->getValue());
    contourFinder->setMaxAreaRadius(maxAreaRadius->getValue());
    contourFinder->setThreshold(thresholdValue->getValue());
    contourFinder->setFindHoles(false);
    // wait for half a second before forgetting something
    contourFinder->getTracker().setPersistence(15);
    // an object can move up to 32 pixels per frame
    contourFinder->getTracker().setMaximumDistance(32);

}

//--------------------------------------------------------------
void ContourTracking::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){
    gui->update();
    header->update();
    if(!header->getIsCollapsed()){
        invertBW->update();
        thresholdValue->update();
        minAreaRadius->update();
        maxAreaRadius->update();
    }

    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){

        if(!invertBW->getChecked()){
            contourFinder->setInvert(false);
        }else{
            contourFinder->setInvert(true);
        }

        contourFinder->setMinAreaRadius(minAreaRadius->getValue());
        contourFinder->setMaxAreaRadius(maxAreaRadius->getValue());
        contourFinder->setThreshold(thresholdValue->getValue());

        if(!isFBOAllocated){
            isFBOAllocated = true;
            pix             = new ofPixels();
            outputFBO->allocate(static_cast<ofTexture *>(_inletParams[0])->getWidth(),static_cast<ofTexture *>(_inletParams[0])->getHeight(),GL_RGB,1);
        }

        static_cast<ofTexture *>(_inletParams[0])->readToPixels(*pix);

        blur(*pix, 10);
        contourFinder->findContours(*pix);

        if(outputFBO->isAllocated()){
            *static_cast<ofTexture *>(_outletParams[0]) = outputFBO->getTexture();

            static_cast<vector<float> *>(_outletParams[1])->clear();
            static_cast<vector<float> *>(_outletParams[2])->clear();
            static_cast<vector<float> *>(_outletParams[3])->clear();

            static_cast<vector<float> *>(_outletParams[1])->push_back(contourFinder->size());
            static_cast<vector<float> *>(_outletParams[2])->push_back(contourFinder->size());
            static_cast<vector<float> *>(_outletParams[3])->push_back(contourFinder->size());

            for(int i = 0; i < contourFinder->size(); i++) {
                // blob id
                int label = contourFinder->getLabel(i);

                // some different styles of contour centers
                ofVec2f centroid = toOf(contourFinder->getCentroid(i));
                ofVec2f average = toOf(contourFinder->getAverage(i));
                ofVec2f center = toOf(contourFinder->getCenter(i));

                // velocity
                ofVec2f velocity = toOf(contourFinder->getVelocity(i));

                // area and perimeter
                double area = contourFinder->getContourArea(i);
                double perimeter = contourFinder->getArcLength(i);

                // bounding rect
                cv::Rect boundingRect = contourFinder->getBoundingRect(i);

                // contour
                ofPolyline contour = toOf(contourFinder->getContour(i));
                ofPolyline convexHull = toOf(contourFinder->getConvexHull(i));

                // 2
                static_cast<vector<float> *>(_outletParams[1])->push_back(static_cast<float>(label));
                static_cast<vector<float> *>(_outletParams[1])->push_back(contourFinder->getTracker().getAge(label));

                // 6
                static_cast<vector<float> *>(_outletParams[1])->push_back(centroid.x);
                static_cast<vector<float> *>(_outletParams[1])->push_back(centroid.y);
                static_cast<vector<float> *>(_outletParams[1])->push_back(average.x);
                static_cast<vector<float> *>(_outletParams[1])->push_back(average.y);
                static_cast<vector<float> *>(_outletParams[1])->push_back(center.x);
                static_cast<vector<float> *>(_outletParams[1])->push_back(center.y);

                // 2
                static_cast<vector<float> *>(_outletParams[1])->push_back(velocity.x);
                static_cast<vector<float> *>(_outletParams[1])->push_back(velocity.y);

                // 2
                static_cast<vector<float> *>(_outletParams[1])->push_back(area);
                static_cast<vector<float> *>(_outletParams[1])->push_back(perimeter);

                // 4
                static_cast<vector<float> *>(_outletParams[1])->push_back(boundingRect.x);
                static_cast<vector<float> *>(_outletParams[1])->push_back(boundingRect.y);
                static_cast<vector<float> *>(_outletParams[1])->push_back(boundingRect.width);
                static_cast<vector<float> *>(_outletParams[1])->push_back(boundingRect.height);

                // 1
                static_cast<vector<float> *>(_outletParams[2])->push_back(contour.getVertices().size());

                // 2
                static_cast<vector<float> *>(_outletParams[2])->push_back(static_cast<float>(label));
                static_cast<vector<float> *>(_outletParams[2])->push_back(contourFinder->getTracker().getAge(label));

                // contour.getVertices().size() * 2
                for(int c=0;c<contour.getVertices().size();c++){
                    static_cast<vector<float> *>(_outletParams[2])->push_back(contour.getVertices().at(c).x);
                    static_cast<vector<float> *>(_outletParams[2])->push_back(contour.getVertices().at(c).y);
                }

                // 1
                static_cast<vector<float> *>(_outletParams[3])->push_back(convexHull.getVertices().size());

                // 2
                static_cast<vector<float> *>(_outletParams[3])->push_back(static_cast<float>(label));
                static_cast<vector<float> *>(_outletParams[3])->push_back(contourFinder->getTracker().getAge(label));

                // convexHull.getVertices().size() * 2
                for(int c=0;c<convexHull.getVertices().size();c++){
                    static_cast<vector<float> *>(_outletParams[3])->push_back(convexHull.getVertices().at(c).x);
                    static_cast<vector<float> *>(_outletParams[3])->push_back(convexHull.getVertices().at(c).y);
                }

            }

        }

    }else{
        isFBOAllocated = false;
    }
    
}

//--------------------------------------------------------------
void ContourTracking::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(this->inletsConnected[0] && outputFBO->isAllocated() && static_cast<ofTexture *>(_outletParams[0])->isAllocated()){

        outputFBO->begin();

        ofClear(0,0,0,255);

        ofSetColor(255);
        static_cast<ofTexture *>(_inletParams[0])->draw(0,0);

        ofSetLineWidth(2);
        contourFinder->draw();

        for(int i = 0; i < contourFinder->size(); i++) {
            ofNoFill();

            // convex hull of the contour
            ofSetColor(yellowPrint);
            ofPolyline convexHull = toOf(contourFinder->getConvexHull(i));
            convexHull.draw();

            // blobs labels
            ofSetLineWidth(1);
            ofFill();
            ofPoint center = toOf(contourFinder->getCenter(i));
            ofPushMatrix();
            ofTranslate(center.x, center.y);
            int label = contourFinder->getLabel(i);
            string msg = ofToString(label) + ":" + ofToString(contourFinder->getTracker().getAge(label));
            if(!invertBW->getChecked()){
                ofSetColor(0,0,0);
            }else{
                ofSetColor(255,255,255);
            }
            font->draw(msg,fontSize,0,0);
            ofPopMatrix();
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
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void ContourTracking::removeObjectContent(){
    
}

//--------------------------------------------------------------
void ContourTracking::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    thresholdValue->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    invertBW->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    minAreaRadius->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    maxAreaRadius->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || invertBW->hitTest(_m-this->getPos()) || thresholdValue->hitTest(_m-this->getPos()) || minAreaRadius->hitTest(_m-this->getPos()) || maxAreaRadius->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void ContourTracking::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        thresholdValue->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        invertBW->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        minAreaRadius->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        maxAreaRadius->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }else{
        ofNotifyEvent(dragEvent, nId);

        box->setFromCenter(_m.x, _m.y,box->getWidth(),box->getHeight());
        headerBox->set(box->getPosition().x,box->getPosition().y,box->getWidth(),headerHeight);

        x = box->getPosition().x;
        y = box->getPosition().y;

        for(int j=0;j<static_cast<int>(outPut.size());j++){
            outPut[j]->linkVertices[0].move(outPut[j]->posFrom.x,outPut[j]->posFrom.y);
            outPut[j]->linkVertices[1].move(outPut[j]->posFrom.x+20,outPut[j]->posFrom.y);
        }
    }
}

//--------------------------------------------------------------
void ContourTracking::onToggleEvent(ofxDatGuiToggleEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == invertBW){
            this->setCustomVar(static_cast<float>(e.checked),"INVERT_BW");
        }
    }
}

//--------------------------------------------------------------
void ContourTracking::onSliderEvent(ofxDatGuiSliderEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == thresholdValue){
            this->setCustomVar(static_cast<float>(e.value),"THRESHOLD");
        }else if(e.target == minAreaRadius){
            this->setCustomVar(static_cast<float>(e.value),"MIN_AREA_RADIUS");
        }else if(e.target == maxAreaRadius){
            this->setCustomVar(static_cast<float>(e.value),"MAX_AREA_RADIUS");
        }
    }

}
