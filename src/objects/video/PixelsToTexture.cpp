/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2020 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "PixelsToTexture.h"

//--------------------------------------------------------------
PixelsToTexture::PixelsToTexture() : PatchObject("pixels to texture"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new ofPixels();  // pixels

    _outletParams[0] = new ofTexture(); // texture

    this->initInletsState();

    this->height     /= 2;

}

//--------------------------------------------------------------
void PixelsToTexture::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_PIXELS,"pixels");

    this->addOutlet(VP_LINK_TEXTURE,"texture");
}

//--------------------------------------------------------------
void PixelsToTexture::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);
}

//--------------------------------------------------------------
void PixelsToTexture::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(this->inletsConnected[0]){
        if(static_cast<ofPixels *>(_inletParams[0])->getWidth() > 0 && static_cast<ofPixels *>(_inletParams[0])->getHeight() > 0){
            static_cast<ofTexture *>(_outletParams[0])->loadData(*static_cast<ofPixels *>(_inletParams[0]));
        }
    }
}

//--------------------------------------------------------------
void PixelsToTexture::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

}

//--------------------------------------------------------------
void PixelsToTexture::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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



        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void PixelsToTexture::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Transforms the signal from a pixels cable (emerald green) into a texture signal (blue cable). Texture data is processed on GPU while pixel data on CPU",
                "https://mosaic.d3cod3.org/reference.php?r=pixel-to-texture", scaleFactor);
}

//--------------------------------------------------------------
void PixelsToTexture::removeObjectContent(bool removeFileFromData){

}

OBJECT_REGISTER( PixelsToTexture, "pixels to texture", OFXVP_OBJECT_CAT_TEXTURE)

#endif
