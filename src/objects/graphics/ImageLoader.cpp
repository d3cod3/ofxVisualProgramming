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

#include "ImageLoader.h"

//--------------------------------------------------------------
ImageLoader::ImageLoader() : PatchObject("image loader"){

    this->numInlets  = 0;
    this->numOutlets = 1;

    _outletParams[0] = new ofTexture(); // output

    this->initInletsState();

    img = new ofImage();

    isNewObject         = false;
    isFileLoaded        = false;
    
    loadImgFlag         = false;
    isImageLoaded       = false;

    posX = posY = drawW = drawH = 0.0f;

    imgName = "";
    imgPath = "";

    this->setIsTextureObj(true);

}

//--------------------------------------------------------------
void ImageLoader::newObject(){
    PatchObject::setName( this->objectName );

    this->addOutlet(VP_LINK_TEXTURE,"image");
}

//--------------------------------------------------------------
void ImageLoader::autoloadFile(string _fp){
    filepath = copyFileToPatchFolder(this->patchFolderPath,_fp);
    isImageLoaded= true;
    isFileLoaded = false;
}

//--------------------------------------------------------------
void ImageLoader::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    this->unusedArgs(mainWindow);

    fileDialog.setIsRetina(this->isRetina);

    if(filepath == "none"){
        isNewObject = true;
    }else{
        loadImageFile();
    }

}

//--------------------------------------------------------------
void ImageLoader::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(!isFileLoaded && img->isAllocated()){
        isFileLoaded = true;
        static_cast<ofTexture *>(_outletParams[0])->allocate(img->getPixels());
        ofLog(OF_LOG_NOTICE,"[verbose] image file loaded: %s",filepath.c_str());
    }

    if(img->isAllocated()){
        *static_cast<ofTexture *>(_outletParams[0]) = img->getTexture();
    }

    if(isImageLoaded){
        isImageLoaded = false;
        loadImageFile();
    }
    
}

//--------------------------------------------------------------
void ImageLoader::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending(); // make sure is enabled here for transparent png
    // draw node texture preview with OF
    if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
        if(scaledObjW*canvasZoom > 90.0f){
            drawNodeOFTexture(*static_cast<ofTexture *>(_outletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
        }
    }else{
        if(scaledObjW*canvasZoom > 90.0f){
            ofSetColor(34,34,34);
            ofDrawRectangle(objOriginX - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/canvasZoom), objOriginY-(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor/canvasZoom),scaledObjW + (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/canvasZoom),scaledObjH + (((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor)/canvasZoom) );
        }
    }
}

//--------------------------------------------------------------
void ImageLoader::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
    loadImgFlag = false;

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

        // get imgui node translated/scaled position/dimension for drawing textures in OF
        objOriginX = (ImGui::GetWindowPos().x + ((IMGUI_EX_NODE_PINS_WIDTH_NORMAL - 1)*this->scaleFactor) - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale();
        objOriginY = (ImGui::GetWindowPos().y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale();
        scaledObjW = this->width - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/_nodeCanvas.GetCanvasScale());
        scaledObjH = this->height - ((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor/_nodeCanvas.GetCanvasScale());

        _nodeCanvas.EndNodeContent();
    }

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, loadImgFlag, "Select image", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".jpg,.jpeg,.gif,.png,.tif,.tiff", "", scaleFactor)){
        ofFile file (fileDialog.selected_path);
        if (file.exists()){
            filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
            isImageLoaded= true;
            isFileLoaded = false;
        }
    }

}

//--------------------------------------------------------------
void ImageLoader::drawObjectNodeConfig(){
    loadImgFlag = false;

    ImGui::Spacing();
    ImGui::Text("Loaded File:");
    if(imgName == ""){
        ImGui::Text("none");
    }else{
        ImGui::Text("%s",imgName.c_str());
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",imgPath.c_str());
    }
    ImGui::Spacing();
    ImGui::Text("Resolution: %s",imgRes.c_str());
    ImGui::Spacing();
    if(ImGui::Button(ICON_FA_FILE,ImVec2(224*scaleFactor,26*scaleFactor))){
        loadImgFlag = true;
    }

    ImGuiEx::ObjectInfo(
                "Simple object for loading image files. Compatible formats are jpg, png, gif and tif.",
                "https://mosaic.d3cod3.org/reference.php?r=image-loader", scaleFactor);

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, loadImgFlag, "Select image", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".jpg,.jpeg,.gif,.png,.tif,.tiff", "", scaleFactor)){
        ofFile file (fileDialog.selected_path);
        if (file.exists()){
            filepath = copyFileToPatchFolder(this->patchFolderPath,file.getAbsolutePath());
            isImageLoaded= true;
            isFileLoaded = false;
        }
    }
}

//--------------------------------------------------------------
void ImageLoader::removeObjectContent(bool removeFileFromData){
    if(removeFileFromData){
        //removeFile(filepath);
    }
}

//--------------------------------------------------------------
void ImageLoader::loadImageFile(){
    if(filepath != "none"){
        filepath = forceCheckMosaicDataPath(filepath);
        isNewObject = false;
        img = new ofImage();
        img->load(filepath);

        ofFile tempFile(filepath);

        imgName = tempFile.getFileName();
        imgPath = tempFile.getAbsolutePath();

        imgRes = ofToString(img->getWidth())+"x"+ofToString(img->getHeight());

        this->saveConfig(false);
    }
}

OBJECT_REGISTER( ImageLoader, "image loader", OFXVP_OBJECT_CAT_TEXTURE)

#endif
