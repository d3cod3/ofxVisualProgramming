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

#ifndef OFXVP_BUILD_WITH_MINIMAL_OBJECTS

#include "ImageExporter.h"

//--------------------------------------------------------------
ImageExporter::ImageExporter() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 0;

    _inletParams[0] = new ofTexture(); // input
    _inletParams[1] = new float();      // bang
    *(float *)&_inletParams[1] = 0.0f;

    this->initInletsState();

    img = unique_ptr<ofImage>(new ofImage());

    isGUIObject         = true;
    this->isOverGUI     = true;

    isNewObject         = false;
    
    saveImgFlag         = false;
    isImageSaved        = false;

    posX = posY = drawW = drawH = 0.0f;

    lastImageFile       = "";
    imageSequenceCounter= 0;
}

//--------------------------------------------------------------
void ImageExporter::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_TEXTURE,"input");
    this->addInlet(VP_LINK_NUMERIC,"bang");
}

//--------------------------------------------------------------
void ImageExporter::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onButtonEvent(this, &ImageExporter::onButtonEvent);
    gui->onTextInputEvent(this, &ImageExporter::onTextInputEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    imgName = gui->addTextInput("name","export.jpg");
    imgName->setUseCustomMouse(true);
    gui->addBreak();
    saveButton = gui->addButton("SAVE");
    saveButton->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

}

//--------------------------------------------------------------
void ImageExporter::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    imgName->update();
    saveButton->update();

    if(saveImgFlag){
        saveImgFlag = false;
        if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
            fd.saveFile("save imagefile"+ofToString(this->getId()),"Save an image file",imgName->getText());
        }else{
            ofLog(OF_LOG_NOTICE,"There is no ofTexture connected to the object inlet, connect something if you want to export it as image!");
        }
    }

    if(isImageSaved){
        isImageSaved = false;
        saveImageFile();
        ofFile tempFilename(lastImageFile);
        imgName->setText(tempFilename.getFileName());
    }

    if(this->inletsConnected[1] && *(float *)&_inletParams[1] == 1.0){
        saveImageFile();
    }else if(!this->inletsConnected[1]){
        imageSequenceCounter = 0;
    }
    
}

//--------------------------------------------------------------
void ImageExporter::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();

    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(static_cast<ofTexture *>(_inletParams[0])->getWidth()/static_cast<ofTexture *>(_inletParams[0])->getHeight() >= this->width/this->height){
            if(static_cast<ofTexture *>(_inletParams[0])->getWidth() > static_cast<ofTexture *>(_inletParams[0])->getHeight()){   // horizontal texture
                drawW           = this->width;
                drawH           = (this->width/static_cast<ofTexture *>(_inletParams[0])->getWidth())*static_cast<ofTexture *>(_inletParams[0])->getHeight();
                posX            = 0;
                posY            = (this->height-drawH)/2.0f;
            }else{ // vertical texture
                drawW           = (static_cast<ofTexture *>(_inletParams[0])->getWidth()*this->height)/static_cast<ofTexture *>(_inletParams[0])->getHeight();
                drawH           = this->height;
                posX            = (this->width-drawW)/2.0f;
                posY            = 0;
            }
        }else{ // always considered vertical texture
            drawW           = (static_cast<ofTexture *>(_inletParams[0])->getWidth()*this->height)/static_cast<ofTexture *>(_inletParams[0])->getHeight();
            drawH           = this->height;
            posX            = (this->width-drawW)/2.0f;
            posY            = 0;
        }
        static_cast<ofTexture *>(_inletParams[0])->draw(posX,posY,drawW,drawH);
    }

    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void ImageExporter::removeObjectContent(bool removeFileFromData){
    
}

//--------------------------------------------------------------
void ImageExporter::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    imgName->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    saveButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || imgName->hitTest(_m-this->getPos()) || saveButton->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void ImageExporter::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        imgName->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        saveButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void ImageExporter::fileDialogResponse(ofxThreadedFileDialogResponse &response){
    if(response.id == "save imagefile"+ofToString(this->getId())){
        lastImageFile = response.filepath;
        isImageSaved = true;
    }
}

//--------------------------------------------------------------
void ImageExporter::saveImageFile(){
    static_cast<ofTexture *>(_inletParams[0])->readToPixels(capturePix);

    img = unique_ptr<ofImage>(new ofImage());
    // OF_IMAGE_GRAYSCALE, OF_IMAGE_COLOR, or OF_IMAGE_COLOR_ALPHA
    // GL_LUMINANCE, GL_RGB, GL_RGBA
    if(static_cast<ofTexture *>(_inletParams[0])->getTextureData().glInternalFormat == GL_LUMINANCE){
        img->allocate(static_cast<ofTexture *>(_inletParams[0])->getWidth(),static_cast<ofTexture *>(_inletParams[0])->getHeight(),OF_IMAGE_GRAYSCALE);
    }else if(static_cast<ofTexture *>(_inletParams[0])->getTextureData().glInternalFormat == GL_RGB){
        img->allocate(static_cast<ofTexture *>(_inletParams[0])->getWidth(),static_cast<ofTexture *>(_inletParams[0])->getHeight(),OF_IMAGE_COLOR);
    }else if(static_cast<ofTexture *>(_inletParams[0])->getTextureData().glInternalFormat == GL_RGBA){
        img->allocate(static_cast<ofTexture *>(_inletParams[0])->getWidth(),static_cast<ofTexture *>(_inletParams[0])->getHeight(),OF_IMAGE_COLOR_ALPHA);
    }

    ofFile tempFilename(lastImageFile);
    string lastImageFileFolder = tempFilename.getEnclosingDirectory();
    string lastImageFileName = tempFilename.getFileName();

    string finalFileName = "";

    if (lastImageFileName.find('#') != std::string::npos){
        // found
        string sequencedFileName = tempFilename.getFileName().substr(0,lastImageFileName.find_last_of('.'));

        int count = std::count(sequencedFileName.begin(), sequencedFileName.end(), '#');
        string actualFrameStr = ofToString(imageSequenceCounter);

        string cleanSequencedFileName = sequencedFileName.substr(0,sequencedFileName.length()-count);

        cleanSequencedFileName += "_";
        for(unsigned int i=0;i<(count-actualFrameStr.length());i++){
            cleanSequencedFileName += "0";
        }
        cleanSequencedFileName += actualFrameStr;
        cleanSequencedFileName += "."+tempFilename.getExtension();

        finalFileName = lastImageFileFolder+cleanSequencedFileName;
        imageSequenceCounter++;
    }else{
        finalFileName = lastImageFile;
    }

    img->setFromPixels(capturePix);
    img->save(finalFileName);

}

//--------------------------------------------------------------
void ImageExporter::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if (e.target == saveButton){
            saveImgFlag = true;
        }
    }
}

//--------------------------------------------------------------
void ImageExporter::onTextInputEvent(ofxDatGuiTextInputEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == imgName){

        }
    }
}

OBJECT_REGISTER( ImageExporter, "image exporter", OFXVP_OBJECT_CAT_GRAPHICS)

#endif
