/*==============================================================================
    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2022 Daan de Lange

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

#include "TextureInformation.h"

//--------------------------------------------------------------
TextureInformation::TextureInformation() : PatchObject("texture information"){

    this->numInlets  = 1;
    this->numOutlets = 6;

    this->texWidth = this->texHeight = this->texChannels = this->texID = this->texBytesPerChannel = 0.f;

    _inletParams[0]     = new ofTexture(); // texture
    _outletParams[0]    = new float();
    _outletParams[1]    = new float();
    _outletParams[2]    = new float();
    _outletParams[3]    = new float();
    _outletParams[4]    = new float();
    _outletParams[5]    = new float();

    this->initInletsState();

    loaded                  = false;

}

//--------------------------------------------------------------
void TextureInformation::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"texture");
    this->addOutlet(VP_LINK_NUMERIC,"width");
    this->addOutlet(VP_LINK_NUMERIC,"height");
    this->addOutlet(VP_LINK_NUMERIC,"allocated");
    this->addOutlet(VP_LINK_NUMERIC,"texID");
    this->addOutlet(VP_LINK_NUMERIC,"channels");
    this->addOutlet(VP_LINK_NUMERIC,"channel bytes");

}

//--------------------------------------------------------------
void TextureInformation::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    this->loaded = true;
}

//--------------------------------------------------------------
void TextureInformation::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    if( this->inletsConnected[0] ){
        ofTexture* inTex = static_cast< ofTexture* >(_inletParams[0]);
        if( inTex && inTex->isAllocated() ){
            this->texIsAllocated = true;
            this->texWidth = inTex->getWidth();
            this->texHeight = inTex->getHeight();
            this->texID = inTex->getTextureData().textureID;
            this->texChannels = ofGetNumChannelsFromGLFormat(ofGetGLFormatFromInternal(inTex->getTextureData().glInternalFormat));
            this->texBytesPerChannel = ofGetBytesPerChannelFromGLType(ofGetGLTypeFromInternal(inTex->getTextureData().glInternalFormat));
        }
        else this->texIsAllocated = false;
    }
    else this->texIsAllocated = false;

    // Ensure to reset values on unallocated textures
    if( !this->texIsAllocated ){
        this->texWidth = this->texHeight = this->texChannels = this->texID = this->texBytesPerChannel = 0.f;
    }

    // Sync pointers with pointers
    *(float *)&_outletParams[0] = this->texWidth;
    *(float *)&_outletParams[1] = this->texHeight;
    *(float *)&_outletParams[2] = this->texIsAllocated;
    *(float *)&_outletParams[3] = this->texID;
    *(float *)&_outletParams[4] = this->texChannels;
    *(float *)&_outletParams[5] = this->texBytesPerChannel;

}

//--------------------------------------------------------------
void TextureInformation::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);
}

//--------------------------------------------------------------
void TextureInformation::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGui::Dummy(ImVec2(10,10));

        ImGui::TextDisabled( "Size: %.0fx%.0f", this->texWidth, this->texHeight );
        ImGui::TextDisabled( "Allocated: %d", (this->texIsAllocated == 1.f)?1:0 );
        ImGui::TextDisabled( "TextureID: %.0f", this->texID );
        ImGui::TextDisabled( "Color channels: %.0f", this->texChannels );
        ImGui::TextDisabled( "Bytes per channel: %.0f", this->texBytesPerChannel );

        _nodeCanvas.EndNodeContent();
    }



}

//--------------------------------------------------------------
void TextureInformation::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Extracts some texture information.",
                "https://mosaic.d3cod3.org/reference.php?r=texture-information");
}

//--------------------------------------------------------------
void TextureInformation::removeObjectContent(bool removeFileFromData){

}

OBJECT_REGISTER( TextureInformation, "texture information", OFXVP_OBJECT_CAT_TEXTURE)

#endif
