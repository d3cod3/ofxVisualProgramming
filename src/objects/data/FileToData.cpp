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
FileToData::FileToData() : PatchObject("file to data"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;
    _outletParams[0] = new vector<float>(); // output

    this->initInletsState();

    openFileFlag        = false;
    fileOpened          = false;
    readData            = false;

    actualIndex         = 0;

    tmpFileName         = "";
}

//--------------------------------------------------------------
void FileToData::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addOutlet(VP_LINK_ARRAY,"output");
}

//--------------------------------------------------------------
void FileToData::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    fileDialog.setIsRetina(this->isRetina);

    if(filepath != "none"){
        loadDataFile(filepath);
    }

}

//--------------------------------------------------------------
void FileToData::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(fileOpened){
        fileOpened = false;
        ofLog(OF_LOG_NOTICE,"%s","FILE DATA IMPORTED!");
        // start reading data from file
        readData = true;
    }

    if(readData){
        static_cast<vector<float> *>(_outletParams[0])->clear();
        *static_cast<vector<float> *>(_outletParams[0]) = dataMatrix.at(actualIndex);
    }

    if(this->inletsConnected[0] && readData){
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
void FileToData::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

    ofSetColor(255);

}

//--------------------------------------------------------------
void FileToData::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
    openFileFlag = false;

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){

        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {
            drawObjectNodeConfig(); this->configMenuWidth = ImGui::GetWindowWidth();


            ImGui::EndMenu();
        }

        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        ImGui::SetCursorPos(ImVec2(IMGUI_EX_NODE_PINS_WIDTH_NORMAL+(4*scaleFactor), (this->height/2 *_nodeCanvas.GetCanvasScale()) - (6*scaleFactor)));
        if(ImGui::Button(ICON_FA_FILE,ImVec2(-1,26*scaleFactor))){
            openFileFlag = true;
        }

        _nodeCanvas.EndNodeContent();
    }

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, openFileFlag, "Open a previously saved Mosaic data file", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".txt", "", scaleFactor)){
        ofLog(OF_LOG_NOTICE,"%s","START IMPORTING DATA");
        ofFile file (fileDialog.selected_path);
        tmpFileName = file.getFileName();
        filepath = file.getAbsolutePath();
        loadDataFile(filepath);
    }
}

//--------------------------------------------------------------
void FileToData::drawObjectNodeConfig(){
    ofFile tempFilename(filepath);

    openFileFlag = false;

    ImGui::Spacing();
    ImGui::Text("Reading data from:");
    if(filepath == "none"){
        ImGui::Text("%s",filepath.c_str());
    }else{
        ImGui::Text("%s",tempFilename.getFileName().c_str());
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",tempFilename.getAbsolutePath().c_str());
    }
    ImGui::Spacing();
    if(ImGui::Button(ICON_FA_FILE,ImVec2(224*scaleFactor,26*scaleFactor))){
        openFileFlag = true;
    }

    ImGuiEx::ObjectInfo(
                "Loads a txt file, previously saved by the 'data to file' object, and return the vector data, line by line, with reading synced by his bang inlet.",
                "https://mosaic.d3cod3.org/reference.php?r=file-to-data", scaleFactor);

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, openFileFlag, "Open a previously saved Mosaic data file", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".txt", "", scaleFactor)){
        ofLog(OF_LOG_NOTICE,"%s","START IMPORTING DATA");
        ofFile file (fileDialog.selected_path);
        tmpFileName = file.getFileName();
        filepath = file.getAbsolutePath();
        loadDataFile(filepath);
    }
}

//--------------------------------------------------------------
void FileToData::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
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

    ofLog(OF_LOG_NOTICE,"%s","FINISHED IMPORTING DATA");

    fileOpened = true;
}

OBJECT_REGISTER( FileToData, "file to data", OFXVP_OBJECT_CAT_DATA)

#endif
