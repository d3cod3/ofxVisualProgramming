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

#include "ColorPalette.h"

//--------------------------------------------------------------
ColorPalette::ColorPalette() : PatchObject("color palette"){

    this->numInlets  = 1;
    this->numOutlets = 1;

    _inletParams[0] = new vector<float>(); // base color
    _outletParams[0] = new vector<float>(); // palette

    this->initInletsState();

    posX = posY = drawW = drawH = 0.0f;

    baseColor           = ofFloatColor(0.0f, 0.694117f, 0.250980f, 1.0f);
    numColors           = 4;
    selectedGeneration  = 0;
    selectedChannel     = 0;
    spread              = 0.2;

    this->setIsTextureObj(true);

}

//--------------------------------------------------------------
void ColorPalette::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"baseColor");

    this->addOutlet(VP_LINK_ARRAY,"palette");

    this->setCustomVar(baseColor.r,"RED");
    this->setCustomVar(baseColor.g,"GREEN");
    this->setCustomVar(baseColor.b,"BLUE");
}

//--------------------------------------------------------------
void ColorPalette::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    generationOptions.push_back("Random");
    generationOptions.push_back("Mono Chromatic");
    generationOptions.push_back("Complementary");
    generationOptions.push_back("Triad");
    generationOptions.push_back("Complementary Triad");
    generationOptions.push_back("Analogous");

    channelOptions.push_back("SATURATION");
    channelOptions.push_back("BRIGHTNESS");

    initPalette();

}

//--------------------------------------------------------------
void ColorPalette::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[0]){
        if(static_cast<vector<float> *>(_inletParams[0])->size() == 1){
            baseColor.set(static_cast<vector<float> *>(_inletParams[0])->at(0),static_cast<vector<float> *>(_inletParams[0])->at(0),static_cast<vector<float> *>(_inletParams[0])->at(0));
        }else if(static_cast<vector<float> *>(_inletParams[0])->size() == 3){
            baseColor.set(static_cast<vector<float> *>(_inletParams[0])->at(0),static_cast<vector<float> *>(_inletParams[0])->at(1),static_cast<vector<float> *>(_inletParams[0])->at(2));
        }
    }

    static_cast<vector<float> *>(_outletParams[0])->clear();
    for(int i=0;i<palette.size();i++){
        static_cast<vector<float> *>(_outletParams[0])->push_back(palette.at(i).r);
        static_cast<vector<float> *>(_outletParams[0])->push_back(palette.at(i).g);
        static_cast<vector<float> *>(_outletParams[0])->push_back(palette.at(i).b);
    }

}

//--------------------------------------------------------------
void ColorPalette::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    if(scaledObjW*canvasZoom > 90.0f){
        ofSetColor(34,34,34);
        ofDrawRectangle(objOriginX - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/canvasZoom), objOriginY-(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor/canvasZoom),scaledObjW + (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/canvasZoom),scaledObjH + (((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor)/canvasZoom) );

        for(int i=0;i<palette.size();i++){
            ofSetColor(palette.at(i));
            float tw = (scaledObjW - (IMGUI_EX_NODE_PINS_WIDTH_SMALL*this->scaleFactor/canvasZoom))/palette.size();
            float th = scaledObjH;
            ofDrawRectangle(objOriginX + (tw*i), objOriginY,tw, th);
        }
    }
}

//--------------------------------------------------------------
void ColorPalette::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            ImGui::Spacing();
            ImVec4 color = ImVec4(baseColor.r,baseColor.g,baseColor.b,1.0f);
            if(ImGui::ColorEdit4( "Base Color", (float*)&color )){
                this->setCustomVar(color.x,"RED");
                this->setCustomVar(color.y,"GREEN");
                this->setCustomVar(color.z,"BLUE");

                baseColor.set(color.x,color.y,color.z,1.0f);
            }

            ImGui::Spacing();
            if(ImGui::BeginCombo("Palette Type", generationOptions.at(selectedGeneration).c_str() )){
                for(int i=0; i < generationOptions.size(); ++i){
                    bool is_selected = (selectedGeneration == i );
                    if (ImGui::Selectable(generationOptions.at(i).c_str(), is_selected)){
                        selectedGeneration = i;
                        //this->setCustomVar(static_cast<float>(selectedGeneration),"SUBTRACTION_TECHNIQUE");
                    }
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            ImGui::Spacing();
            if(ImGui::BeginCombo("Color Channel", channelOptions.at(selectedChannel).c_str() )){
                for(int i=0; i < channelOptions.size(); ++i){
                    bool is_selected = (selectedChannel == i );
                    if (ImGui::Selectable(channelOptions.at(i).c_str(), is_selected)){
                        selectedChannel = i;
                        //this->setCustomVar(static_cast<float>(selectedGeneration),"SUBTRACTION_TECHNIQUE");
                    }
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }
            ImGui::SameLine();  ImGuiEx::HelpMarker("For MonoChromatic and Complementary palettes only!");

            ImGui::Spacing();
            if(ImGui::InputInt("Num Colors",&numColors)){
                if(numColors < 3){
                    numColors = 3;
                }
            }

            ImGui::Spacing();
            if(ImGui::SliderFloat("spread",&spread,0.0f,1.0f)){

            }
            ImGui::SameLine();  ImGuiEx::HelpMarker("For Analogous palettes only!");

            ImGui::Spacing();
            if(ImGui::Button("GENERATE",ImVec2(-1,26*scaleFactor))){
                if(selectedGeneration == 0){
                    generateRandom(numColors);
                }else if(selectedGeneration == 1){
                    generateMonoChromatic((ColorChannel)selectedChannel,numColors);
                }else if(selectedGeneration == 2){
                    generateComplementary((ColorChannel)selectedChannel,numColors);
                }else if(selectedGeneration == 3){
                    generateTriad();
                }else if(selectedGeneration == 4){
                    generateComplementaryTriad();
                }else if(selectedGeneration == 5){
                    generateAnalogous(numColors,spread);
                }
            }

            ImGuiEx::ObjectInfo(
                        "Color palette generator.",
                        "https://mosaic.d3cod3.org/reference.php?r=color-palette", scaleFactor);

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
void ColorPalette::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void ColorPalette::initPalette(){
    palette.clear();
    palette.push_back(baseColor);
}

//--------------------------------------------------------------
void ColorPalette::generateRandom(int numColors){
    initPalette();
    for(int i = 1; i < numColors; i++){
        palette.push_back(ofFloatColor(ofRandom(baseColor.limit()),ofRandom(baseColor.limit()),ofRandom(baseColor.limit())));
    }
}

//--------------------------------------------------------------
void ColorPalette::generateMonoChromatic(ColorChannel channel, int numColors){
    initPalette();

    float value;
    switch (channel) {
    case SATURATION:
        value = baseColor.getSaturation();
        break;
    case BRIGHTNESS:
        value = baseColor.getBrightness();
        break;
    default:
        break;
    }

    float steps = value / numColors;
    for (int i = 1; i < numColors; i++) {
        switch (channel) {
        case SATURATION:
            palette.push_back(ofFloatColor::fromHsb(baseColor.getHue(),value - i* steps,baseColor.getBrightness()));
            break;
        case BRIGHTNESS:
            palette.push_back(ofFloatColor::fromHsb(baseColor.getHue(),baseColor.getSaturation(),value - i* steps));
            break;
        default:
            break;
        }
    }
}

//--------------------------------------------------------------
void ColorPalette::generateComplementary(ColorChannel channel, int numColors){
    initPalette();

    //hack only even numbers allowed, otherwise the code will bloat out.
    if (numColors % 2 == 1) {
        numColors -= 1;
    }

    float value;
    switch (channel) {
    case SATURATION:
        value = baseColor.getSaturation();
        break;
    case BRIGHTNESS:
        value = baseColor.getBrightness();
        break;
    default:
        break;
    }

    // base is again brightest for now, second enum to define the "direction" {toBlack to White}?
    int numLeft = numColors/2;
    float stepSize = (2 * value) / numColors;

    for (int i = 1; i < numLeft; i++) {
        value -= stepSize;
        switch (channel) {
        case SATURATION:
            palette.push_back(ofFloatColor::fromHsb(baseColor.getHue(),value,baseColor.getBrightness()));
            break;
        case BRIGHTNESS:
            palette.push_back(ofFloatColor::fromHsb(baseColor.getHue(),baseColor.getSaturation(),value));
            break;
        default:
            break;
        }

    }

    ofFloatColor complement = baseColor.invert();
    for (int i = 1; i < numColors - numLeft; i++) {
        switch (channel) {
        case SATURATION:
            palette.push_back(ofFloatColor::fromHsb(complement.getHue(),value,complement.getBrightness()));
            break;
        case BRIGHTNESS:
            palette.push_back(ofFloatColor::fromHsb(complement.getHue(),complement.getSaturation(),value));
            break;
        default:
            break;
        }
        value += stepSize;

    }
    palette.push_back(complement);

}

//--------------------------------------------------------------
void ColorPalette::generateTriad(){
    initPalette();

    float hue = baseColor.getHue();
    float stepSize = ofFloatColor::limit()/3;
    palette.push_back(ofFloatColor::fromHsb(normalizeValue(hue + stepSize),baseColor.getSaturation(),baseColor.getBrightness()));
    palette.push_back(ofFloatColor::fromHsb(normalizeValue(hue +  2 * stepSize),baseColor.getSaturation(),baseColor.getBrightness()));
}

//--------------------------------------------------------------
void ColorPalette::generateComplementaryTriad(){
    initPalette();

    ofFloatColor complement = baseColor.invert();
    float hue = complement.getHue();
    float spread = 0.083f * ofFloatColor::limit();
    palette.push_back(ofFloatColor::fromHsb(normalizeValue(hue - spread),complement.getSaturation(),complement.getBrightness()));
    palette.push_back(ofFloatColor::fromHsb(normalizeValue(hue + spread),complement.getSaturation(),complement.getBrightness()));

}

//--------------------------------------------------------------
void ColorPalette::generateAnalogous(int numColors, float spread){
    initPalette();

    float hue = baseColor.getHue(); //between 0 and limit()
    float stepSize = spread / numColors;// distribute the colors accross range
    stepSize = stepSize * ofFloatColor::limit(); // scale to PixelType
    numColors -=1;

    int numLeft = numColors / 2;
    //fill in left colors
    float newHue;
    for (int i = 1; i <= numLeft ; i++) {
        newHue = normalizeValue(hue - i * stepSize);
        palette.push_back(ofFloatColor::fromHsb(newHue,baseColor.getSaturation(),baseColor.getBrightness()));
    }
    //fill in right colors
    for (int i = 1; i <= numColors- numLeft; i++) {
        newHue = normalizeValue(hue + i * stepSize);
        palette.push_back(ofFloatColor::fromHsb(newHue,baseColor.getSaturation(),baseColor.getBrightness()));
    }
}

//--------------------------------------------------------------
float ColorPalette::normalizeValue(float val){
    float limit = ofFloatColor::limit();
    if(val > limit)
    {
        return val - limit;
    } else if(val < 0)
    {
        return limit + val;
    } else
    {
        return val;
    }
}



OBJECT_REGISTER( ColorPalette, "color palette", OFXVP_OBJECT_CAT_DATA)

#endif
