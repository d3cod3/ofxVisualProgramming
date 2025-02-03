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

    recButtonLabel      = "REC";

}

//--------------------------------------------------------------
void DataToFile::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"input");
    this->addInlet(VP_LINK_NUMERIC,"bang");
}

//--------------------------------------------------------------
void DataToFile::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    fileDialog.setIsRetina(this->isRetina);
}

//--------------------------------------------------------------
void DataToFile::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(this->inletsConnected[1]){
        if(*(float *)&_inletParams[1] < 1.0){
            bang = false;
        }else{
            bang = true;
        }
    }

    if(fileSaved && !recordData && bang){
        // start recording data to file
        recordData = true;
        ofLog(OF_LOG_NOTICE,"%s","START EXPORTING DATA");
    }else if(recordData && bang){
        recordData = false;
        ofLog(OF_LOG_NOTICE,"%s","FINISHED EXPORTING DATA");
    }

    if(this->inletsConnected[0] && !static_cast<vector<float> *>(_inletParams[0])->empty() && recordData){
        string temp = "";
        for(size_t i=0;i<static_cast<vector<float> *>(_inletParams[0])->size();i++){
            temp += ofToString(static_cast<vector<float> *>(_inletParams[0])->at(i));
            if(i<static_cast<vector<float> *>(_inletParams[0])->size()-1){
                temp += ",";
            }
        }
        appendLineToFile(filepath,temp);
    }

}

//--------------------------------------------------------------
void DataToFile::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

    ofSetColor(255);
}

//--------------------------------------------------------------
void DataToFile::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
    exportFileFlag = false;

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
        if(ImGui::Button(ICON_FA_FILE_UPLOAD,ImVec2(-1,26*scaleFactor))){
            exportFileFlag = true;
        }

        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 window_size = ImVec2(this->width*_nodeCanvas.GetCanvasScale(),this->height*_nodeCanvas.GetCanvasScale());
        ImVec2 pos = ImVec2(window_pos.x + window_size.x - (ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,10,60)*scaleFactor), window_pos.y + IMGUI_EX_NODE_HEADER_HEIGHT + (ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,1,40)*scaleFactor));
        float radius = ofMap(_nodeCanvas.GetCanvasScale(),CANVAS_MIN_SCALE,CANVAS_MAX_SCALE,1,20)*scaleFactor;
        if (recordData){
            _nodeCanvas.getNodeDrawList()->AddCircleFilled(pos, radius, IM_COL32(255, 0, 0, 255), 40);
        }else{
            _nodeCanvas.getNodeDrawList()->AddCircleFilled(pos, radius, IM_COL32(0, 255, 0, 255), 40);
        }

        _nodeCanvas.EndNodeContent();
    }

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, exportFileFlag, "Export new data file as", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ".txt", "data.txt", scaleFactor)){
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
void DataToFile::drawObjectNodeConfig(){
    ofFile tempFilename(filepath);

    exportFileFlag = false;

    ImGui::Spacing();
    ImGui::Text("Saving data to:");
    if(filepath == "none"){
        ImGui::Text("%s",filepath.c_str());
    }else{
        ImGui::Text("%s",tempFilename.getFileName().c_str());
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",tempFilename.getAbsolutePath().c_str());
    }
    ImGui::Spacing();
    if(ImGui::Button(ICON_FA_FILE_UPLOAD,ImVec2(108*scaleFactor,26*scaleFactor))){
        exportFileFlag = true;
    }
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, VHS_RED);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, VHS_RED_OVER);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, VHS_RED_OVER);
    char tmp[256];
    sprintf_s(tmp,"%s %s",ICON_FA_CIRCLE, recButtonLabel.c_str());
    if(ImGui::Button(tmp,ImVec2(108*scaleFactor,26*scaleFactor))){
        if(!this->inletsConnected[0]){
            ofLog(OF_LOG_WARNING,"%s","There is no data cable connected to the object inlet, connect something if you want to export it!");
        }else if(!fileSaved){
            ofLog(OF_LOG_WARNING,"%s","No file selected. Please select one before recording!");
        }else{
            if(fileSaved && !recordData){
                recButtonLabel = "STOP";
                // start recording data to file
                recordData = true;
                ofLog(OF_LOG_NOTICE,"%s","START EXPORTING DATA");
            }else if(recordData){
                recButtonLabel = "REC";
                recordData = false;
                ofLog(OF_LOG_NOTICE,"%s","FINISHED EXPORTING DATA");
            }
        }
    }
    ImGui::PopStyleColor(3);

    ImGuiEx::ObjectInfo(
                "Saves the vector data in a .txt file, line by line for each computing frame.",
                "https://mosaic.d3cod3.org/reference.php?r=data-to-file", scaleFactor);

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, exportFileFlag, "Export new data file as", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ".txt", "data.txt", scaleFactor)){
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
    unusedArgs(removeFileFromData);
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
