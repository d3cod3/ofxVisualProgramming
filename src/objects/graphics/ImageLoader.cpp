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

#include "ImageLoader.h"

//--------------------------------------------------------------
ImageLoader::ImageLoader() : PatchObject(){

    this->numInlets  = 0;
    this->numOutlets = 1;

    _outletParams[0] = new ofTexture(); // output

    this->initInletsState();

    img = new ofImage();

    isGUIObject         = true;
    this->isOverGUI     = true;

    isNewObject         = false;
    isFileLoaded        = false;
    
    loadImgFlag         = false;
    isImageLoaded       = false;

    posX = posY = drawW = drawH = 0.0f;

}

//--------------------------------------------------------------
void ImageLoader::newObject(){
    this->setName("image loader");
    this->addOutlet(VP_LINK_TEXTURE);
}

//--------------------------------------------------------------
void ImageLoader::autoloadFile(string _fp){
    filepath = _fp;
    isImageLoaded= true;
    isFileLoaded = false;
}

//--------------------------------------------------------------
void ImageLoader::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onButtonEvent(this, &ImageLoader::onButtonEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    imgName = gui->addLabel("NONE");
    imgRes = gui->addLabel("0x0");
    gui->addBreak();
    loadButton = gui->addButton("OPEN");
    loadButton->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    if(filepath == "none"){
        isNewObject = true;
    }else{
        loadImageFile();
    }

}

//--------------------------------------------------------------
void ImageLoader::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){

    if(!isFileLoaded && img->isAllocated()){
        isFileLoaded = true;
        if(img->isAllocated()){
            static_cast<ofTexture *>(_outletParams[0])->allocate(img->getPixels());
            ofLog(OF_LOG_NOTICE,"[verbose] image file loaded: %s",filepath.c_str());
        }else{
            if(!isNewObject){
                ofLog(OF_LOG_ERROR,"image file: %s NOT FOUND!",filepath.c_str());
            }
            filepath = "none";
        }
    }

    gui->update();
    header->update();
    imgName->update();
    imgRes->update();
    loadButton->update();

    if(img->isAllocated()){
        *static_cast<ofTexture *>(_outletParams[0]) = img->getTexture();
    }

    if(loadImgFlag){
        loadImgFlag = false;
        fd.openFile("load imagefile"+ofToString(this->getId()),"Select an image file");
    }

    if(isImageLoaded){
        isImageLoaded = false;
        loadImageFile();
    }
    
}

//--------------------------------------------------------------
void ImageLoader::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(isFileLoaded){
        if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
            imgRes->setLabel(ofToString(static_cast<ofTexture *>(_outletParams[0])->getWidth())+"x"+ofToString(static_cast<ofTexture *>(_outletParams[0])->getHeight()));
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
    }else if(!isNewObject){
        ofSetColor(255,0,0);
        ofDrawRectangle(0,0,this->width,this->height);
        ofSetColor(255);
        font->draw("FILE NOT FOUND!",this->fontSize,this->width/3 + 4,this->headerHeight*2.3);
    }
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void ImageLoader::removeObjectContent(){
    
}

//--------------------------------------------------------------
void ImageLoader::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    imgName->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    imgRes->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    loadButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || imgName->hitTest(_m-this->getPos()) || imgRes->hitTest(_m-this->getPos()) || loadButton->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void ImageLoader::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        imgName->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        imgRes->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void ImageLoader::fileDialogResponse(ofxThreadedFileDialogResponse &response){
    if(response.id == "load imagefile"+ofToString(this->getId())){
        ofFile file (response.filepath);
        if (file.exists()){
            string fileExtension = ofToUpper(file.getExtension());
            if(fileExtension == "PNG" || fileExtension == "GIF" || fileExtension == "JPG" || fileExtension == "JPEG" || fileExtension == "TIF" || fileExtension == "TIFF") {
                filepath = file.getAbsolutePath();
                isImageLoaded= true;
                isFileLoaded = false;
            }
        }
    }
}

//--------------------------------------------------------------
void ImageLoader::loadImageFile(){
    if(filepath != "none"){
        filepath = forceCheckMosaicDataPath(filepath);
        isNewObject = false;
        img->load(filepath);

        ofFile tempFile(filepath);
        if(tempFile.getFileName().size() > 22){
            imgName->setLabel(tempFile.getFileName().substr(0,21)+"...");
        }else{
            imgName->setLabel(tempFile.getFileName());
        }

        this->saveConfig(false,this->nId);
    }
}

//--------------------------------------------------------------
void ImageLoader::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if (e.target == loadButton){
            loadImgFlag = true;
        }
    }
}
