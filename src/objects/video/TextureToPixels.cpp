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

#include "TextureToPixels.h"

//--------------------------------------------------------------
TextureToPixels::TextureToPixels() : PatchObject("texture to pixels"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();  // texture

    _outletParams[0] = new ofPixels(); // pixels

    this->initInletsState();

    this->height     /= 2;

}

//--------------------------------------------------------------
void TextureToPixels::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"texture");

    this->addOutlet(VP_LINK_PIXELS,"pixels");
}

//--------------------------------------------------------------
void TextureToPixels::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

}

//--------------------------------------------------------------
void TextureToPixels::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        static_cast<ofTexture *>(_inletParams[0])->readToPixels(*static_cast<ofPixels *>(_outletParams[0]));
    }

}

//--------------------------------------------------------------
void TextureToPixels::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void TextureToPixels::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            ImGuiEx::ObjectInfo(
                        "Transforms the signal from a texture cable (blue) into a pixel signal (emerald green). Texture data is processed on GPU while pixel data on CPU",
                        "https://mosaic.d3cod3.org/reference.php?r=texture-to-pixel", scaleFactor);

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
void TextureToPixels::removeObjectContent(bool removeFileFromData){

}

OBJECT_REGISTER( TextureToPixels, "texture to pixels", OFXVP_OBJECT_CAT_VIDEO)

#endif
