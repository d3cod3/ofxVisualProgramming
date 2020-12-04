/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2018 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "ChromaKey.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
ChromaKey::ChromaKey() : PatchObject("chroma key"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new ofTexture();  // input
    _inletParams[1] = new ofTexture();  // mask
    _outletParams[0] = new ofTexture(); // output

    this->initInletsState();

    posX = posY = drawW = drawH = 0.0f;

    isInputConnected    = false;

    chromaBgColor       = ofFloatColor(0.0f, 0.694117f, 0.250980f, 1.0f); // green screen by default
    baseMaskStrength    = 0.5f;
    chromaMaskStrength  = 0.487f;
    greenSpillStrength  = 0.3857f;
    chromaBlur          = 1408.0f;
    multiplyFilterHue   = 0.2625f;

    loaded              = false;

    this->setIsTextureObj(true);

}

//--------------------------------------------------------------
void ChromaKey::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_TEXTURE,"input");
    this->addInlet(VP_LINK_TEXTURE,"background");

    this->addOutlet(VP_LINK_TEXTURE,"output");

    this->setCustomVar(chromaBgColor.r,"RED");
    this->setCustomVar(chromaBgColor.g,"GREEN");
    this->setCustomVar(chromaBgColor.b,"BLUE");
    this->setCustomVar(baseMaskStrength,"BASE_MASK_STRENGTH");
    this->setCustomVar(chromaMaskStrength,"CHROMA_MASK_STRENGTH");
    this->setCustomVar(greenSpillStrength,"GREEN_SPILL_STRENGTH");
    this->setCustomVar(chromaBlur,"CHROMA_BLUR");
    this->setCustomVar(multiplyFilterHue,"MULT_FILTER_HUE");
}

//--------------------------------------------------------------
void ChromaKey::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){


}

//--------------------------------------------------------------
void ChromaKey::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(!loaded){
        loaded = true;
        chromaBgColor.set(this->getCustomVar("RED"),this->getCustomVar("GREEN"),this->getCustomVar("BLUE"));
        baseMaskStrength    = this->getCustomVar("BASE_MASK_STRENGTH");
        chromaMaskStrength  = this->getCustomVar("CHROMA_MASK_STRENGTH");
        greenSpillStrength  = this->getCustomVar("GREEN_SPILL_STRENGTH");
        chromaBlur          = this->getCustomVar("CHROMA_BLUR");
        multiplyFilterHue   = this->getCustomVar("MULT_FILTER_HUE");
        if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
            chromakey->setbgColor(chromaBgColor);
        }
    }
    
}

//--------------------------------------------------------------
void ChromaKey::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){

    // UPDATE SHADER
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(!isInputConnected){
            isInputConnected = true;
            chromakey = new ofxChromaKeyShader(static_cast<ofTexture *>(_inletParams[0])->getWidth(),static_cast<ofTexture *>(_inletParams[0])->getHeight());
            updateChromaVars();
        }

        if(this->inletsConnected[1] && static_cast<ofTexture *>(_inletParams[1])->isAllocated()){
            chromakey->updateChromakeyMask(*static_cast<ofTexture *>(_inletParams[0]),*static_cast<ofTexture *>(_inletParams[1]));
            *static_cast<ofTexture *>(_outletParams[0]) = chromakey->fbo_final.getTexture();
        }else{
            *static_cast<ofTexture *>(_outletParams[0]) = *static_cast<ofTexture *>(_inletParams[0]);
        }
    }else{
        isInputConnected = false;
    }

    ofSetColor(255);
    // draw node texture preview with OF
    if(scaledObjW*canvasZoom > 90.0f){
        drawNodeOFTexture(*static_cast<ofTexture *>(_outletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
    }
}

//--------------------------------------------------------------
void ChromaKey::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

}

//--------------------------------------------------------------
void ChromaKey::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated() && this->inletsConnected[1] && static_cast<ofTexture *>(_inletParams[1])->isAllocated()){
        if(static_cast<ofTexture *>(_inletParams[0])->getWidth() != static_cast<ofTexture *>(_inletParams[1])->getWidth() || static_cast<ofTexture *>(_inletParams[0])->getHeight() != static_cast<ofTexture *>(_inletParams[1])->getHeight()){
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,0,0,255));
            ImGui::Text("Input inlet texture Resolution: %.0fx%.0f",static_cast<ofTexture *>(_inletParams[0])->getWidth(),static_cast<ofTexture *>(_inletParams[0])->getHeight());
            ImGui::Text("Background inlet texture Resolution: %.0fx%.0f",static_cast<ofTexture *>(_inletParams[1])->getWidth(),static_cast<ofTexture *>(_inletParams[1])->getHeight());
            ImGui::PopStyleColor(1);
            ImGui::SameLine(); ImGuiEx::HelpMarker("Inlet texture resolutions MUST be the same!");
        }
    }

    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        ImGui::Spacing();
        ImVec4 color = ImVec4(chromaBgColor.r,chromaBgColor.g,chromaBgColor.b,1.0f);
        if(ImGui::ColorEdit4( "Chroma BG Color", (float*)&color )){
            this->setCustomVar(color.x,"RED");
            this->setCustomVar(color.y,"GREEN");
            this->setCustomVar(color.z,"BLUE");

            chromaBgColor.set(color.x,color.y,color.z,1.0f);
            chromakey->setbgColor(chromaBgColor);
        }

        ImGui::Spacing();
        if(ImGui::SliderFloat("base mask strength",&baseMaskStrength,0.0f,1.0f)){
            this->setCustomVar(baseMaskStrength,"BASE_MASK_STRENGTH");
            chromakey->setbaseMaskStrength(baseMaskStrength);
        }
        ImGui::Spacing();
        if(ImGui::SliderFloat("chroma mask strength",&chromaMaskStrength,0.0f,1.0f)){
            this->setCustomVar(chromaMaskStrength,"CHROMA_MASK_STRENGTH");
            chromakey->setchromaMaskStrength(chromaMaskStrength);
        }
        if(ImGui::SliderFloat("green spill strength",&greenSpillStrength,0.0f,1.0f)){
            this->setCustomVar(greenSpillStrength,"GREEN_SPILL_STRENGTH");
            chromakey->setgreenSpillStrength(greenSpillStrength);
        }
        if(ImGui::SliderFloat("chroma blur",&chromaBlur,0.0f,4096.0f)){
            this->setCustomVar(chromaBlur,"CHROMA_BLUR");
            chromakey->setblurValue(chromaBlur);
        }
        if(ImGui::SliderFloat("multiply filter hue",&multiplyFilterHue,0.0f,1.0f)){
            this->setCustomVar(multiplyFilterHue,"MULT_FILTER_HUE");
            chromakey->setmultiplyFilterHueOffset(multiplyFilterHue);
        }
    }


    ImGuiEx::ObjectInfo(
                "Standard chromakey effect with source and background mask.",
                "https://mosaic.d3cod3.org/reference.php?r=chroma-key", scaleFactor);
}

//--------------------------------------------------------------
void ChromaKey::removeObjectContent(bool removeFileFromData){
    
}

//--------------------------------------------------------------
void ChromaKey::updateChromaVars(){
    chromakey->setbgColor(chromaBgColor);
    chromakey->setbaseMaskStrength(baseMaskStrength);
    chromakey->setchromaMaskStrength(chromaMaskStrength);
    chromakey->setgreenSpillStrength(greenSpillStrength);
    chromakey->setblurValue(chromaBlur);
    chromakey->setmultiplyFilterHueOffset(multiplyFilterHue);
}



OBJECT_REGISTER( ChromaKey, "chroma key", OFXVP_OBJECT_CAT_CV)

#endif
