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

#ifndef OFXVP_BUILD_WITH_MINIMAL_OBJECTS

#include "ColorTracking.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
ColorTracking::ColorTracking() : PatchObject("color tracking"){

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


    posX = posY = drawW = drawH = 0.0f;

    targetColor         = ofFloatColor(0.0f, 0.694117f, 0.250980f, 1.0f); // chroma green by default
    threshold           = 128.0f;
    minAreaRadius       = 10.0f;
    maxAreaRadius       = 200.0f;

    isFBOAllocated      = false;

    loaded              = false;

    this->setIsTextureObj(true);

}

//--------------------------------------------------------------
void ColorTracking::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"input");
    this->addOutlet(VP_LINK_TEXTURE,"output");
    this->addOutlet(VP_LINK_ARRAY,"blobsData");
    this->addOutlet(VP_LINK_ARRAY,"contourData");
    this->addOutlet(VP_LINK_ARRAY,"convexHullData");

    this->setCustomVar(threshold,"THRESHOLD");
    this->setCustomVar(minAreaRadius,"MIN_AREA_RADIUS");
    this->setCustomVar(maxAreaRadius,"MAX_AREA_RADIUS");
    this->setCustomVar(targetColor.r,"RED");
    this->setCustomVar(targetColor.g,"GREEN");
    this->setCustomVar(targetColor.b,"BLUE");
}

//--------------------------------------------------------------
void ColorTracking::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){


}

//--------------------------------------------------------------
void ColorTracking::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(!loaded){
        loaded = true;

        threshold = this->getCustomVar("THRESHOLD");
        minAreaRadius = this->getCustomVar("MIN_AREA_RADIUS");
        maxAreaRadius = this->getCustomVar("MAX_AREA_RADIUS");

        contourFinder->setMinAreaRadius(minAreaRadius);
        contourFinder->setMaxAreaRadius(maxAreaRadius);
        contourFinder->setThreshold(threshold);
        contourFinder->setFindHoles(false);
        // set color
        contourFinder->setTargetColor(targetColor, TRACK_COLOR_HS);
        // wait for half a second before forgetting something
        contourFinder->getTracker().setPersistence(60);
        // an object can move up to 32 pixels per frame
        contourFinder->getTracker().setMaximumDistance(64);
    }
    
}

//--------------------------------------------------------------
void ColorTracking::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){

        // UPDATE STUFF
        contourFinder->setMinAreaRadius(minAreaRadius);
        contourFinder->setMaxAreaRadius(maxAreaRadius);
        contourFinder->setThreshold(threshold);

        contourFinder->setTargetColor(targetColor, TRACK_COLOR_HS);

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

            outputFBO->begin();

            ofClear(0,0,0,255);

            ofSetColor(255);
            static_cast<ofTexture *>(_inletParams[0])->draw(0,0);

            ofSetLineWidth(2);
            ofSetColor(ofColor::aquamarine);
            contourFinder->draw();

            for(int i = 0; i < contourFinder->size(); i++) {
                ofNoFill();

                // convex hull of the contour
                ofSetColor(ofColor::yellowGreen);
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
                ofSetColor(255,255,255);
                font->draw(msg,fontSize,0,0);
                ofPopMatrix();
            }

            outputFBO->end();

        }

        // DRAW STUFF
        ofSetColor(255);
        // draw node texture preview with OF
        if(scaledObjW*canvasZoom > 90.0f){
            drawNodeOFTexture(*static_cast<ofTexture *>(_outletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
        }

    }else{
        if(scaledObjW*canvasZoom > 90.0f){
            ofSetColor(34,34,34);
            ofDrawRectangle(objOriginX - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/canvasZoom), objOriginY-(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor/canvasZoom),scaledObjW + (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/canvasZoom),scaledObjH + (((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor)/canvasZoom) );
        }

        isFBOAllocated = false;
    }

}

//--------------------------------------------------------------
void ColorTracking::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){

        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {
            ImGui::Spacing();
            ImVec4 color = ImVec4(targetColor.r,targetColor.g,targetColor.b,1.0f);
            if(ImGui::ColorEdit4( "Target Color", (float*)&color )){
                this->setCustomVar(color.x,"RED");
                this->setCustomVar(color.y,"GREEN");
                this->setCustomVar(color.z,"BLUE");

                targetColor.set(color.x,color.y,color.z,1.0f);
            }

            ImGui::Spacing();
            if(ImGui::SliderFloat("threshold",&threshold,0.0f,255.0f)){
                this->setCustomVar(threshold,"THRESHOLD");
            }
            ImGui::Spacing();
            if(ImGui::SliderFloat("min. area radius",&minAreaRadius,4.0f,99.0f)){
                this->setCustomVar(minAreaRadius,"MIN_AREA_RADIUS");
            }
            ImGui::Spacing();
            if(ImGui::SliderFloat("max aera radius",&maxAreaRadius,100.0f,500.0f)){
                this->setCustomVar(maxAreaRadius,"MAX_AREA_RADIUS");
            }


            ImGuiEx::ObjectInfo(
                        "Contour tracking over selected color. Extract blobs, contours and convex hulls.",
                        "https://mosaic.d3cod3.org/reference.php?r=contour-tracking", scaleFactor);


            ImGui::EndMenu();
        }

        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        // get imgui node translated/scaled position/dimension for drawing textures in OF
        objOriginX = (ImGui::GetWindowPos().x + ((IMGUI_EX_NODE_PINS_WIDTH_NORMAL - 1)*this->scaleFactor) - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale();
        objOriginY = (ImGui::GetWindowPos().y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale();
        scaledObjW = this->width - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/_nodeCanvas.GetCanvasScale());
        scaledObjH = this->height - ((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor/_nodeCanvas.GetCanvasScale());

        _nodeCanvas.EndNodeContent();
    }

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();

}

//--------------------------------------------------------------
void ColorTracking::removeObjectContent(bool removeFileFromData){
    
}


OBJECT_REGISTER( ColorTracking, "color tracking", OFXVP_OBJECT_CAT_CV)

#endif
