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

#include "HaarTracking.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
HaarTracking::HaarTracking() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 2;

    _inletParams[0] = new ofTexture();  // input texture
    _outletParams[0] = new ofTexture(); // output texture (for visualization)
    _outletParams[1] = new vector<float>();  // haar blobs vector

    this->initInletsState();

    haarFinder      = new ofxCv::ObjectFinder();
    pix             = new ofPixels();
    outputFBO       = new ofFbo();

    isGUIObject         = true;
    this->isOverGUI     = true;

    posX = posY = drawW = drawH = 0.0f;

    isFBOAllocated      = false;

    haarToLoad          = "";
    loadHaarConfigFlag  = false;
    haarConfigLoaded    = false;

}

//--------------------------------------------------------------
void HaarTracking::newObject(){
    this->setName("haar tracking");
    this->addInlet(VP_LINK_TEXTURE,"input");
    this->addOutlet(VP_LINK_TEXTURE,"output");
    this->addOutlet(VP_LINK_ARRAY,"haarBlobsData");
}

//--------------------------------------------------------------
void HaarTracking::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    haarFileName = gui->addLabel("frontalface");
    gui->addBreak();
    loadButton = gui->addButton("OPEN");
    loadButton->setUseCustomMouse(true);
    
    gui->onButtonEvent(this, &HaarTracking::onButtonEvent);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    if(filepath == "none"){
        ofFile tempHC("haarcascades/haarcascade_frontalface_alt.xml");
        filepath = tempHC.getAbsolutePath();
    }else{
        filepath = forceCheckMosaicDataPath(filepath);
    }
    haarFinder->setup(filepath);
    haarFinder->setPreset(ObjectFinder::Fast);
    haarFinder->getTracker().setSmoothingRate(.1);

}

//--------------------------------------------------------------
void HaarTracking::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){
    // Object GUI
    gui->update();
    header->update();
    if(!header->getIsCollapsed()){
        haarFileName->update();
        loadButton->update();
    }

    // file dialogs
    if(loadHaarConfigFlag){
        loadHaarConfigFlag = false;
        fd.openFile("load haar"+ofToString(this->getId()),"Select a haarcascade xml file");
    }

    if(haarConfigLoaded){
        haarConfigLoaded = false;
        if(haarToLoad != ""){
            ofFile file(haarToLoad);
            if (file.exists()){
                string fileExtension = ofToUpper(file.getExtension());
                if(fileExtension == "XML") {
                    filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
                    //filepath = file.getAbsolutePath();
                    haarFinder->setup(filepath);

                    size_t start = file.getFileName().find_first_of("_");
                    string tempName = file.getFileName().substr(start+1,file.getFileName().size()-start-5);

                    haarFileName->setLabel(tempName);
                }
            }
        }
    }

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
void HaarTracking::drawObjectContent(ofxFontStash *font){
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
void HaarTracking::removeObjectContent(bool removeFileFromData){
    if(removeFileFromData){
        removeFile(filepath);
    }
}

//--------------------------------------------------------------
void HaarTracking::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    haarFileName->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    loadButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || haarFileName->hitTest(_m-this->getPos()) || loadButton->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void HaarTracking::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        haarFileName->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        loadButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void HaarTracking::fileDialogResponse(ofxThreadedFileDialogResponse &response){
    if(response.id == "load haar"+ofToString(this->getId())){
        haarToLoad = response.filepath;
        haarConfigLoaded = true;
    }
}

//--------------------------------------------------------------
void HaarTracking::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if (e.target == loadButton){
            loadHaarConfigFlag = true;
        }
    }
}
