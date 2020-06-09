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
        if(img->isAllocated()){
            ofTextureData texData;
            texData.width = img->getWidth();
            texData.height = img->getHeight();
            texData.textureTarget = GL_TEXTURE_2D;
            texData.bFlipTexture = true;

            _outletParams[0] = new ofTexture();
            static_cast<ofTexture *>(_outletParams[0])->clear();
            static_cast<ofTexture *>(_outletParams[0])->allocate(texData);
            static_cast<ofTexture *>(_outletParams[0])->loadData(img->getPixels());
            ofLog(OF_LOG_NOTICE,"[verbose] image file loaded: %s",filepath.c_str());
        }else{
            if(!isNewObject){
                ofLog(OF_LOG_ERROR,"image file: %s NOT FOUND!",filepath.c_str());
            }
            filepath = "none";
        }
    }

    if(img->isAllocated()){
        static_cast<ofTexture *>(_outletParams[0])->loadData(img->getPixels());
    }

    if(isImageLoaded){
        isImageLoaded = false;
        loadImageFile();
    }
    
}

//--------------------------------------------------------------
void ImageLoader::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();

    ofDisableAlphaBlending();
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
            ImGui::Text("Loaded File:");
            ImGui::Text("%s",imgName.c_str());
            ImGui::Text("Resolution: %s",imgRes.c_str());
            if(ImGui::Button("OPEN",ImVec2(200,20))){
                loadImgFlag = true;
            }

            ImGui::Spacing();
            if (ImGui::CollapsingHeader("INFO", ImGuiTreeNodeFlags_None)){
                ImGui::TextWrapped("Simple object for loading image files. Compatible formats are jpg, png, gif and tif.");
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.5f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.129f, 0.0f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.5f, 1.0f, 1.0f));
                if(ImGui::Button("Reference")){
                    ofLaunchBrowser("https://mosaic.d3cod3.org/reference.php?r=image-loader");
                }
                ImGui::PopStyleColor(3);
            }

            ImGui::EndMenu();
        }
        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){
        if(isFileLoaded){
            float _tw = this->width*_nodeCanvas.GetCanvasScale();
            float _th = (this->height*_nodeCanvas.GetCanvasScale()) - (IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT);

            ImGuiEx::drawOFTexture(static_cast<ofTexture *>(_outletParams[0]),_tw,_th,posX,posY,drawW,drawH);

        }else if(!isNewObject){
            ImGui::Text("FILE NOT FOUND!");
        }
    }

    // file dialog
    if(ImGuiEx::getFileDialog(fileDialog, loadImgFlag, "Select image", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ".jpg,.jpeg,.gif,.png,.tif,.tiff")){
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
        removeFile(filepath);
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
        if(tempFile.getFileName().size() > 22){
            imgName = tempFile.getFileName().substr(0,21)+"...";
        }else{
            imgName = tempFile.getFileName();
        }

        imgRes = ofToString(img->getWidth())+"x"+ofToString(img->getHeight());

        this->saveConfig(false,this->nId);
    }
}

OBJECT_REGISTER( ImageLoader, "image loader", OFXVP_OBJECT_CAT_GRAPHICS)

#endif
