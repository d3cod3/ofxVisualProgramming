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

#include "DataToFile.h"

//--------------------------------------------------------------
DataToFile::DataToFile() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 0;

    _inletParams[0] = new vector<float>(); // input

    _inletParams[1] = new float();  // bang
    *(float *)&_inletParams[1] = 0.0f;

    this->initInletsState();

    isGUIObject         = true;
    this->isOverGUI     = true;

    bang                = false;
    exportFileFlag      = false;
    fileSaved           = false;
    recordData          = false;
}

//--------------------------------------------------------------
void DataToFile::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_ARRAY,"input");
    this->addInlet(VP_LINK_NUMERIC,"bang");
}

//--------------------------------------------------------------
void DataToFile::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);

    recButton = gui->addButton("SELECT FILE");
    recButton->setUseCustomMouse(true);
    recButton->setLabelAlignment(ofxDatGuiAlignment::CENTER);

    gui->onButtonEvent(this, &DataToFile::onButtonEvent);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);

}

//--------------------------------------------------------------
void DataToFile::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();
    if(!header->getIsCollapsed()){
        recButton->update();
    }

    if(exportFileFlag){
        exportFileFlag = false;
        fd.saveFile("export datafile"+ofToString(this->getId()),"Export new data file as","data.txt");
    }

    if(this->inletsConnected[1]){
        if(*(float *)&_inletParams[1] < 1.0){
            bang = false;
        }else{
            bang = true;
        }
    }

    if(fileSaved && !recordData && bang){
        fileSaved = false;
        // start recording data to file
        recordData = true;
        ofLog(OF_LOG_NOTICE,"START EXPORTING DATA");
    }else if(recordData && bang){
        recordData = false;
        ofLog(OF_LOG_NOTICE,"FINISHED EXPORTING DATA");
    }

}

//--------------------------------------------------------------
void DataToFile::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofSetCircleResolution(50);
    ofEnableAlphaBlending();

    if(this->inletsConnected[0] && recordData){
        string temp = "";
        for(int i=0;i<static_cast<vector<float> *>(_inletParams[0])->size();i++){
            temp += ofToString(static_cast<vector<float> *>(_inletParams[0])->at(i));
            if(i<static_cast<vector<float> *>(_inletParams[0])->size()-1){
                temp += ",";
            }
        }
        appendLineToFile(filepath,temp);
    }

    if (recordData){
        ofSetColor(ofColor::red);
    }else{
        ofSetColor(ofColor::green);
    }
    ofDrawCircle(ofPoint(this->width-20, 30), 10);

    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void DataToFile::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void DataToFile::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    recButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    if(!header->getIsCollapsed()){
        this->isOverGUI = header->hitTest(_m-this->getPos()) || recButton->hitTest(_m-this->getPos());
    }else{
        this->isOverGUI = header->hitTest(_m-this->getPos());
    }

}

//--------------------------------------------------------------
void DataToFile::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        recButton->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
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
void DataToFile::fileDialogResponse(ofxThreadedFileDialogResponse &response){
    if(response.id == "export datafile"+ofToString(this->getId())){
        ofFile temp(response.filepath);
        filepath = checkFileExtension(temp.getAbsolutePath(), ofToUpper(temp.getExtension()), "TXT");
        fileSaved = true;
    }
}

//--------------------------------------------------------------
void DataToFile::onButtonEvent(ofxDatGuiButtonEvent e){
    if(!header->getIsCollapsed()){
        if(e.target == recButton){
            exportFileFlag = true;
        }
    }
}

//--------------------------------------------------------------
void DataToFile::appendLineToFile(string filepath, string line){
    std::ofstream file;
    file.open(filepath, std::ios::out | std::ios::app);
    file << line << std::endl;
    file.close();
}

OBJECT_REGISTER( DataToFile, "data to file", OFXVP_OBJECT_CAT_DATA);
