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

#include "FileToData.h"

//--------------------------------------------------------------
FileToData::FileToData() : PatchObject(){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;
    _outletParams[0] = new vector<float>(); // output

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    openFileFlag        = false;
    fileOpened          = false;
    readData            = false;

    actualIndex         = 0;
}

//--------------------------------------------------------------
void FileToData::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addOutlet(VP_LINK_ARRAY,"output");
}

//--------------------------------------------------------------
void FileToData::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    readButton = gui->addToggle("READ");
    readButton->setUseCustomMouse(true);
    readButton->setLabelAlignment(ofxDatGuiAlignment::CENTER);

    gui->onToggleEvent(this, &FileToData::onToggleEvent);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

    if(filepath != "none"){
        loadDataFile(filepath);
    }

}

//--------------------------------------------------------------
void FileToData::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    if(!header->getIsCollapsed()){
        readButton->update();
    }

    if(openFileFlag){
        openFileFlag = false;
        // open file
        fd.openFile("import datafile"+ofToString(this->getId()),"Select a previously saved Mosaic data file");
    }

    if(fileOpened){
        fileOpened = false;
        ofLog(OF_LOG_NOTICE,"FILE DATA IMPORTED!");
        // start reading data from file
        readData = true;
    }

    if(readData){
        static_cast<vector<float> *>(_outletParams[0])->clear();
        *static_cast<vector<float> *>(_outletParams[0]) = dataMatrix.at(actualIndex);
    }

    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] == 1.0){
            if(actualIndex < dataMatrix.size()-1){
                actualIndex++;
            }else{
                actualIndex = 0;
            }
        }
    }

}

//--------------------------------------------------------------
void FileToData::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofSetCircleResolution(50);
    ofEnableAlphaBlending();

    if (readButton->getChecked()){
        ofSetColor(ofColor::red);
    }else{
        ofSetColor(ofColor::green);
    }
    ofDrawCircle(ofPoint(this->width-20, 30), 10);

    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void FileToData::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void FileToData::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    readButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || readButton->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void FileToData::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        readButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void FileToData::fileDialogResponse(ofxThreadedFileDialogResponse &response){
    if(response.id == "import datafile"+ofToString(this->getId())){
        filepath = response.filepath;

        loadDataFile(filepath);

    }
}

//--------------------------------------------------------------
void FileToData::loadDataFile(string filepath){
    readData = false;

    fileBuffer = ofBufferFromFile(filepath);

    // fill data matrix
    if(fileBuffer.size()) {
        dataMatrix.clear();
        for (ofBuffer::Line it = fileBuffer.getLines().begin(), end = fileBuffer.getLines().end(); it != end; ++it) {
            string line = *it;
            // make sure its not a empty line
            if(line.empty() == false) {
                vector<float> temp;
                float temp_number;
                // read line into temp
                std::istringstream str_buf{line};
                while ( str_buf >> temp_number ) {
                    temp.push_back(temp_number);
                    // If the next char is a comma, extract it. std::ws discards whitespace
                    if ( ( str_buf >> std::ws).peek() == ',' ) {
                        str_buf.ignore();
                    }
                }
                dataMatrix.push_back(temp);
            }
        }
    }

    fileOpened = true;

    readButton->setChecked(true);
}

//--------------------------------------------------------------
void FileToData::onToggleEvent(ofxDatGuiToggleEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == readButton){
            if(e.checked){
                openFileFlag = true;
                ofLog(OF_LOG_NOTICE,"START IMPORTING DATA");
            }else{
                readData = false;
                ofLog(OF_LOG_NOTICE,"FINISHED IMPORTING DATA");
            }
        }
    }
}

OBJECT_REGISTER( FileToData, "file to data", OFXVP_OBJECT_CAT_DATA)

#endif