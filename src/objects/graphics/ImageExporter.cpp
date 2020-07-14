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
ImageExporter::ImageExporter() : PatchObject("image exporter"){

    this->numInlets  = 2;
    this->numOutlets = 0;

    _inletParams[0] = new ofTexture(); // input
    _inletParams[1] = new float();      // bang
    *(float *)&_inletParams[1] = 0.0f;

    this->initInletsState();

    img = unique_ptr<ofImage>(new ofImage());

    isNewObject         = false;
    
    saveImgFlag         = false;
    isImageSaved        = false;

    posX = posY = drawW = drawH = 0.0f;

    lastImageFile       = "";
    imageSequenceCounter= 0;

    this->setIsTextureObj(true);

}

//--------------------------------------------------------------
void ImageExporter::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"input");
    this->addInlet(VP_LINK_NUMERIC,"bang");
}

//--------------------------------------------------------------
void ImageExporter::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void ImageExporter::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(isImageSaved){
        isImageSaved = false;
        if(this->inletsConnected[0]){
            saveImageFile();
        }else{
            // blank image
            img = unique_ptr<ofImage>(new ofImage());
            img->allocate(1920,1080,OF_IMAGE_GRAYSCALE);
            img->save(lastImageFile);
        }

    }

    if(this->inletsConnected[1] && *(float *)&_inletParams[1] == 1.0){
        saveImageFile();
    }else if(!this->inletsConnected[1]){
        imageSequenceCounter = 0;
    }
    
}

//--------------------------------------------------------------
void ImageExporter::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    // draw node texture preview with OF
    drawNodeOFTexture(*static_cast<ofTexture *>(_inletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, IMGUI_EX_NODE_FOOTER_HEIGHT);
}

//--------------------------------------------------------------
void ImageExporter::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    ofFile tempFilename(lastImageFile);

    saveImgFlag = false;

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){

        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {
            ImGui::Spacing();
            ImGui::Text("Export to File:");
            if(lastImageFile == ""){
                ImGui::Text("none");
            }else{
                ImGui::Text("%s",tempFilename.getFileName().c_str());
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",tempFilename.getAbsolutePath().c_str());
            }
            ImGui::Spacing();
            if(ImGui::Button(ICON_FA_FILE_UPLOAD,ImVec2(84,26))){
                saveImgFlag = true;
            }
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, VHS_RED);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, VHS_RED_OVER);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, VHS_RED_OVER);
            if(ImGui::Button(ICON_FA_CIRCLE " EXPORT",ImVec2(84,26))){
                if(!this->inletsConnected[0]){
                    ofLog(OF_LOG_WARNING,"There is no ofTexture connected to the object inlet, connect something if you want to export it as image!");
                }else if(lastImageFile == ""){
                    ofLog(OF_LOG_WARNING,"No file selected. Please select one before recording!");
                }else{
                    saveImageFile();
                }
            }
            ImGui::PopStyleColor(3);

            ImGuiEx::ObjectInfo(
                        "Export an image or image sequence (using ### for auto-numeration) from every texture cable (blue ones).",
                        "https://mosaic.d3cod3.org/reference.php?r=image-exporter");

            ImGui::EndMenu();
        }

        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        // get imgui node translated/scaled position/dimension for drawing textures in OF
        objOriginX = (ImGui::GetWindowPos().x + IMGUI_EX_NODE_PINS_WIDTH_NORMAL - 1 - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale();
        objOriginY = (ImGui::GetWindowPos().y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale();
        scaledObjW = this->width - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL/_nodeCanvas.GetCanvasScale());
        scaledObjH = this->height - ((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)/_nodeCanvas.GetCanvasScale());

        _nodeCanvas.EndNodeContent();
    }

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, saveImgFlag, "Save image", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ".jpg,.jpeg,.gif,.png,.tif,.tiff")){
        lastImageFile = fileDialog.selected_path;
        filepath = lastImageFile;
        isImageSaved= true;
    }


}

//--------------------------------------------------------------
void ImageExporter::removeObjectContent(bool removeFileFromData){
    
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

OBJECT_REGISTER( ImageExporter, "image exporter", OFXVP_OBJECT_CAT_GRAPHICS)

#endif
