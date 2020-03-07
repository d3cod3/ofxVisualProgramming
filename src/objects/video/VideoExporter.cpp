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

#include "VideoExporter.h"

//--------------------------------------------------------------
VideoExporter::VideoExporter() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 0;

    _inletParams[0] = new ofTexture(); // input

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    isNewObject         = false;

    posX = posY = drawW = drawH = 0.0f;

    needToGrab          = false;
    exportVideoFlag     = false;
    videoSaved          = false;
}

//--------------------------------------------------------------
void VideoExporter::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_TEXTURE,"input");
}

//--------------------------------------------------------------
void VideoExporter::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    recButton = gui->addToggle("REC");
    recButton->setUseCustomMouse(true);
    recButton->setLabelAlignment(ofxDatGuiAlignment::CENTER);
    gui->addBreak();
    codecsList = {"hevc","libx264","jpeg2000","mjpeg","mpeg4"};
    codecs = gui->addDropdown("Codec",codecsList);
    codecs->onDropdownEvent(this,&VideoExporter::onDropdownEvent);
    codecs->setUseCustomMouse(true);
    codecs->select(4);
    for(int i=0;i<codecs->children.size();i++){
        codecs->getChildAt(i)->setUseCustomMouse(true);
    }

    gui->onToggleEvent(this, &VideoExporter::onToggleEvent);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    captureFbo.allocate( STANDARD_TEXTURE_WIDTH, STANDARD_TEXTURE_HEIGHT, GL_RGB );
    recorder.setup(true, false, glm::vec2(STANDARD_TEXTURE_WIDTH, STANDARD_TEXTURE_HEIGHT)); // record video only for now
    recorder.setOverWrite(true);

#if defined(TARGET_OSX)
    recorder.setFFmpegPath(ofToDataPath("ffmpeg/osx/ffmpeg",true));
#elif defined(TARGET_WIN32)
    recorder.setFFmpegPath(ofToDataPath("ffmpeg/win/ffmpeg.exe",true));
#endif
    
}

//--------------------------------------------------------------
void VideoExporter::updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    if(!header->getIsCollapsed()){
        recButton->update();
        codecs->update();
        for(int i=0;i<codecs->children.size();i++){
            codecs->getChildAt(i)->update();
        }
    }

    if(exportVideoFlag){
        exportVideoFlag = false;
        #if defined(TARGET_WIN32)
        fd.saveFile("export videofile"+ofToString(this->getId()),"Export new video file as","export.avi");
        #else
        fd.saveFile("export videofile"+ofToString(this->getId()),"Export new video file as","export.mp4");
        #endif
    }

    if(videoSaved){
        videoSaved = false;
        recorder.setOutputPath(filepath);
        recorder.setBitRate(20000);
        recorder.startCustomRecord();
    }

}

//--------------------------------------------------------------
void VideoExporter::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofSetCircleResolution(50);
    ofEnableAlphaBlending();

    if(this->inletsConnected[0]){
        if(static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
            if(!needToGrab){
                needToGrab = true;
                captureFbo.allocate(static_cast<ofTexture *>(_inletParams[0])->getWidth(), static_cast<ofTexture *>(_inletParams[0])->getHeight(), GL_RGB );
                recorder.setup(true, false, glm::vec2(static_cast<ofTexture *>(_inletParams[0])->getWidth(), static_cast<ofTexture *>(_inletParams[0])->getHeight())); // record video only for now
                recorder.setOverWrite(true);
            }

            captureFbo.begin();
            ofClear(0,0,0,255);
            ofSetColor(255);
            static_cast<ofTexture *>(_inletParams[0])->draw(0,0,static_cast<ofTexture *>(_inletParams[0])->getWidth(),static_cast<ofTexture *>(_inletParams[0])->getHeight());
            captureFbo.end();

            if(recorder.isRecording()) {
                reader.readToPixels(captureFbo, capturePix,OF_IMAGE_COLOR); // ofxFastFboReader
                //captureFbo.readToPixels(capturePix); // standard
                if(capturePix.getWidth() > 0 && capturePix.getHeight() > 0) {
                    recorder.addFrame(capturePix);
                }
            }

        }
    }else{
        captureFbo.begin();
        ofClear(0,0,0,255);
        captureFbo.end();

        needToGrab = false;
    }

    if(static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(static_cast<ofTexture *>(_inletParams[0])->getWidth() >= static_cast<ofTexture *>(_inletParams[0])->getHeight()){   // horizontal texture
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
        captureFbo.getTexture().draw(posX,posY,drawW,drawH);
        if (recorder.isPaused() && recorder.isRecording()){
            ofSetColor(ofColor::yellow);
        }else if (recorder.isRecording()){
            ofSetColor(ofColor::red);
        }else{
            ofSetColor(ofColor::green);
        }
        ofDrawCircle(ofPoint(this->width-20, 30), 10);
    }
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void VideoExporter::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void VideoExporter::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    recButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    codecs->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    for(int i=0;i<codecs->children.size();i++){
        codecs->getChildAt(i)->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || recButton->hitTest(_m-this->getPos()) || codecs->hitTest(_m-this->getPos());

        for(int i=0;i<codecs->children.size();i++){
            this->isOverGUI = codecs->getChildAt(i)->hitTest(_m-this->getPos());
        }

    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void VideoExporter::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        recButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        codecs->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        for(int i=0;i<codecs->children.size();i++){
            codecs->getChildAt(i)->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        }
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
void VideoExporter::fileDialogResponse(ofxThreadedFileDialogResponse &response){
    if(response.id == "export videofile"+ofToString(this->getId())){
        filepath = response.filepath;
        videoSaved = true;
    }
}

//--------------------------------------------------------------
void VideoExporter::onToggleEvent(ofxDatGuiToggleEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == recButton){
            if(e.checked){
                if(!recorder.isRecording()){
                    exportVideoFlag = true;
                }
                ofLog(OF_LOG_NOTICE,"START EXPORTING VIDEO");
            }else{
                if(recorder.isRecording()){
                    recorder.stop();
                }
                ofLog(OF_LOG_NOTICE,"FINISHED EXPORTING VIDEO");
            }
        }
    }
}

//--------------------------------------------------------------
void VideoExporter::onDropdownEvent(ofxDatGuiDropdownEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == codecs){
            recorder.setVideoCodec(codecsList.at(e.child));
            e.target->expand();
        }
    }
}

OBJECT_REGISTER( VideoExporter, "video exporter", OFXVP_OBJECT_CAT_VIDEO);
