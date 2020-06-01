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

#include "DataToFile.h"

//--------------------------------------------------------------
DataToFile::DataToFile() :
            PatchObject("data to file")
{

    this->numInlets  = 2;
    this->numOutlets = 0;

    _inletParams[0] = new vector<float>(); // input

    _inletParams[1] = new float();  // bang
    *(float *)&_inletParams[1] = 0.0f;

    this->initInletsState();

    bang                = false;
    exportFileFlag      = false;
    fileSaved           = false;
    recordData          = false;

    tmpFileName         = "";
}

//--------------------------------------------------------------
void DataToFile::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"input");
    this->addInlet(VP_LINK_NUMERIC,"bang");
}

//--------------------------------------------------------------
void DataToFile::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void DataToFile::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

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

}

//--------------------------------------------------------------
void DataToFile::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();

    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void DataToFile::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    exportFileFlag = false;

    // Info view
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Info) ){
        ImGui::TextWrapped("Saves the vector data in a .txt file, line by line for each computing frame.");
        _nodeCanvas.EndNodeContent();
    }

    // Any other view
    else if( _nodeCanvas.BeginNodeContent() ){
        // parameters view
        if(_nodeCanvas.GetNodeData().viewName == ImGuiExNodeView_Params){
            ImGui::Text("Saving data to:");
            ImGui::Text("%s",tmpFileName.c_str());
            if(ImGui::Button("SELECT FILE",ImVec2(-1,20))){
                exportFileFlag = true;
            }
        }
        // visualize view
        else {
            ImVec2 window_pos = ImGui::GetWindowPos();
            ImVec2 window_size = ImGui::GetWindowSize();
            ImVec2 pos = ImVec2(window_pos.x + window_size.x - 20, window_pos.y + 40);
            if (recordData){ 
                ImGui::GetForegroundDrawList()->AddCircleFilled(pos, 10, IM_COL32(255, 0, 0, 255), 40);
            }else{
                ImGui::GetForegroundDrawList()->AddCircleFilled(pos, 10, IM_COL32(0, 255, 0, 255), 40);
            }
        }
        _nodeCanvas.EndNodeContent();
    }

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, exportFileFlag, "Export new data file as", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ".txt")){
        ofFile file (fileDialog.selected_path);
        if (!file.exists()){
            file.create();
        }
        filepath = checkFileExtension(file.getAbsolutePath(), ofToUpper(file.getExtension()), "TXT");
        tmpFileName = file.getFileName();
        fileSaved = true;
    }
}

//--------------------------------------------------------------
void DataToFile::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void DataToFile::appendLineToFile(string filepath, string line){
    std::ofstream file;
    file.open(filepath, std::ios::out | std::ios::app);
    file << line << std::endl;
    file.close();
}

OBJECT_REGISTER( DataToFile, "data to file", OFXVP_OBJECT_CAT_DATA)

#endif
