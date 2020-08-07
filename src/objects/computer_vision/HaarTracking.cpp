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

#include "HaarTracking.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
HaarTracking::HaarTracking() : PatchObject("haar tracking"){

    this->numInlets  = 1;
    this->numOutlets = 2;

    _inletParams[0] = new ofTexture();  // input texture
    _outletParams[0] = new ofTexture(); // output texture (for visualization)
    _outletParams[1] = new vector<float>();  // haar blobs vector

    this->initInletsState();

    haarFinder      = new ofxCv::ObjectFinder();
    pix             = new ofPixels();
    outputFBO       = new ofFbo();
    haarfileName    = "";

    posX = posY = drawW = drawH = 0.0f;

    isFBOAllocated      = false;

    loadHaarConfigFlag  = false;

    this->setIsTextureObj(true);

}

//--------------------------------------------------------------
void HaarTracking::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"input");

    this->addOutlet(VP_LINK_TEXTURE,"output");
    this->addOutlet(VP_LINK_ARRAY,"haarBlobsData");
}

//--------------------------------------------------------------
void HaarTracking::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    fileDialog.setIsRetina(this->isRetina);

    if(filepath == "none"){
        ofFile tempHC("haarcascades/haarcascade_frontalface_alt.xml");
        //filepath = tempHC.getAbsolutePath();
        filepath = copyFileToPatchFolder(this->patchFolderPath,tempHC.getAbsolutePath());
    }else{
        //filepath = forceCheckMosaicDataPath(filepath);
        filepath = copyFileToPatchFolder(this->patchFolderPath,filepath);
    }
    haarFinder->setup(filepath);
    haarFinder->setPreset(ObjectFinder::Fast);
    haarFinder->getTracker().setSmoothingRate(.1);

    ofFile file(filepath);
    size_t start = file.getFileName().find_first_of("_");
    haarfileName = file.getFileName().substr(start+1,file.getFileName().size()-start-5);

}

//--------------------------------------------------------------
void HaarTracking::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    // HAAR Tracking
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){

        if(!isFBOAllocated){
            isFBOAllocated  = true;
            pix             = new ofPixels();
            outputFBO->allocate(static_cast<ofTexture *>(_inletParams[0])->getWidth(),static_cast<ofTexture *>(_inletParams[0])->getHeight(),GL_RGB,1);
        }

        static_cast<ofTexture *>(_inletParams[0])->readToPixels(*pix);

        haarFinder->update(*pix);

        if(outputFBO->isAllocated()){
            *static_cast<ofTexture *>(_outletParams[0]) = outputFBO->getTexture();

            static_cast<vector<float> *>(_outletParams[1])->clear();
            static_cast<vector<float> *>(_outletParams[1])->push_back(haarFinder->size());

            for(int i = 0; i < haarFinder->size(); i++) {
                
                // blob id
                int label = haarFinder->getLabel(i);

                // bounding rect
                ofRectangle boundingRect = haarFinder->getObjectSmoothed(i);

                // 2
                static_cast<vector<float> *>(_outletParams[1])->push_back(static_cast<float>(label));
                static_cast<vector<float> *>(_outletParams[1])->push_back(haarFinder->getTracker().getAge(label));

                // 2
                static_cast<vector<float> *>(_outletParams[1])->push_back(boundingRect.getCenter().x);
                static_cast<vector<float> *>(_outletParams[1])->push_back(boundingRect.getCenter().y);

                // 4
                static_cast<vector<float> *>(_outletParams[1])->push_back(boundingRect.x);
                static_cast<vector<float> *>(_outletParams[1])->push_back(boundingRect.y);
                static_cast<vector<float> *>(_outletParams[1])->push_back(boundingRect.width);
                static_cast<vector<float> *>(_outletParams[1])->push_back(boundingRect.height);
            }

        }

    }else{
        isFBOAllocated = false;
    }
    
}

//--------------------------------------------------------------
void HaarTracking::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(this->inletsConnected[0] && outputFBO->isAllocated() && static_cast<ofTexture *>(_outletParams[0])->isAllocated()){

        outputFBO->begin();

        ofClear(0,0,0,255);

        ofSetColor(255);
        static_cast<ofTexture *>(_inletParams[0])->draw(0,0);

        for(int i = 0; i < haarFinder->size(); i++) {
            ofNoFill();

            // haar blobs 
            ofRectangle object = haarFinder->getObjectSmoothed(i);
            ofSetLineWidth(2);
            ofDrawRectangle(object);

            // haar blobs labels
            ofSetLineWidth(1);
            ofFill();
            ofPoint center = object.getCenter();
            ofPushMatrix();
            ofTranslate(center.x, center.y);
            int label = haarFinder->getLabel(i);
            string msg = ofToString(label) + ":" + ofToString(haarFinder->getTracker().getAge(label));
            font->draw(msg,fontSize,0,0);
            ofPopMatrix();
        }

        outputFBO->end();

    }
    ofDisableAlphaBlending();

    // draw node texture preview with OF
    if(scaledObjW*canvasZoom > 90.0f){
        drawNodeOFTexture(*static_cast<ofTexture *>(_outletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
    }
}

//--------------------------------------------------------------
void HaarTracking::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
    ofFile tempFilename(filepath);

    loadHaarConfigFlag = false;

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){

        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {
            ImGui::Spacing();
            ImGui::Text("Loaded File:");
            if(filepath == "none"){
                ImGui::Text("%s",filepath.c_str());
            }else{
                ImGui::Text("%s",tempFilename.getFileName().c_str());
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",tempFilename.getAbsolutePath().c_str());
            }
            if(ImGui::Button("OPEN",ImVec2(180,20))){
                loadHaarConfigFlag = true;
            }

            ImGuiEx::ObjectInfo(
                        "Detects shapes with specific characteristics or structures within images or video frames.",
                        "https://mosaic.d3cod3.org/reference.php?r=haar-tracking", scaleFactor);


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

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, loadHaarConfigFlag, "Select haarcascade xml file", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".xml", "", scaleFactor)){
        ofFile file (fileDialog.selected_path);
        if (file.exists()){
            filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
            haarFinder->setup(filepath);

            size_t start = file.getFileName().find_first_of("_");
            haarfileName = file.getFileName().substr(start+1,file.getFileName().size()-start-5);
        }
    }

}

//--------------------------------------------------------------
void HaarTracking::removeObjectContent(bool removeFileFromData){
    if(removeFileFromData){
        removeFile(filepath);
    }
}

OBJECT_REGISTER( HaarTracking, "haar tracking", OFXVP_OBJECT_CAT_CV)

#endif
