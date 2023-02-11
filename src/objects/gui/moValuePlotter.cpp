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

#include "moValuePlotter.h"

//--------------------------------------------------------------
moValuePlotter::moValuePlotter() :
        PatchObject("value plotter"),

        // define default values
        lastMinRange(0.f,"min"),
        lastMaxRange(1.f,"max")

{

    this->numInlets  = 3;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // value
    *(float *)&_inletParams[0] = 0.0f;

    *(float *)&_inletParams[1] = lastMinRange.get();
    *(float *)&_inletParams[2] = lastMaxRange.get();

    _outletParams[0] = new float(); // value
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    name = "";
    color = ImVec4(1.0f,1.0f,1.0f,1.0f);

    this->setIsResizable(true);

    prevW                   = this->width;
    prevH                   = this->height;

    loaded                  = false;

}

//--------------------------------------------------------------
void moValuePlotter::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"value");
    this->addInlet(VP_LINK_NUMERIC,"min");

    this->addInlet(VP_LINK_NUMERIC,"max");
    this->addOutlet(VP_LINK_NUMERIC,"value");

    this->setCustomVar(lastMinRange.get(),"MIN");
    this->setCustomVar(lastMaxRange.get(),"MAX");
    this->setCustomVar(1.0f,"RED");
    this->setCustomVar(1.0f,"GREEN");
    this->setCustomVar(1.0f,"BLUE");
    this->setCustomVar(1.0f,"ALPHA");

    this->setCustomVar(static_cast<float>(prevW),"WIDTH");
    this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
}

//--------------------------------------------------------------
void moValuePlotter::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadVariableName();
}

//--------------------------------------------------------------
void moValuePlotter::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[1] || this->inletsConnected[2]){
        if(lastMinRange.get() != *(float *)&_inletParams[1] || lastMaxRange.get() != *(float *)&_inletParams[2]){
            lastMinRange.get()    = *(float *)&_inletParams[1];
            lastMaxRange.get()    = *(float *)&_inletParams[2];
        }
    }

    // bypass value
    *(float *)&_outletParams[0] = *(float *)&_inletParams[0];


    if(!loaded){
        loaded = true;
        prevW = this->getCustomVar("WIDTH");
        prevH = this->getCustomVar("HEIGHT");
        this->width             = prevW;
        this->height            = prevH;
        lastMinRange.set(this->getCustomVar("MIN"));
        lastMaxRange.set(this->getCustomVar("MAX"));
        color = ImVec4(this->getCustomVar("RED"),this->getCustomVar("GREEN"),this->getCustomVar("BLUE"),this->getCustomVar("ALPHA"));
    }

}

//--------------------------------------------------------------
void moValuePlotter::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void moValuePlotter::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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
        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 window_size = ImGui::GetWindowSize();

        ImGuiEx::plotValue(*(float *)&_outletParams[0], lastMinRange.get(), lastMaxRange.get(), IM_COL32(color.x*255,color.y*255,color.z*255,color.w*255), this->scaleFactor);

        _nodeCanvas.getNodeDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize()*_nodeCanvas.GetCanvasScale(), ImVec2(window_pos.x +(40*_nodeCanvas.GetCanvasScale()), window_pos.y+window_size.y-(36*_nodeCanvas.GetCanvasScale())), IM_COL32_WHITE, name.c_str(), NULL, 0.0f);

        if(this->width != prevW){
            prevW = this->width;
            this->setCustomVar(static_cast<float>(prevW),"WIDTH");
        }
        if(this->height != prevH){
            prevH = this->height;
            this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
        }

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void moValuePlotter::drawObjectNodeConfig(){
    ImGui::Spacing();
    ImGui::PushItemWidth(224*scaleFactor);
    if(ImGui::InputText("Label",&name)){
        saveVariableName();
    }
    ImGui::Spacing();
    if(ImGui::DragFloat("min", &lastMinRange.get())){
        this->setCustomVar(lastMinRange.get(),"MIN");
    }
    ImGui::Spacing();
    if(ImGui::DragFloat("max", &lastMaxRange.get())){
        this->setCustomVar(lastMaxRange.get(),"MAX");
    }
    ImGui::PopItemWidth();

    ImGui::Spacing();
    if(ImGui::ColorEdit4( "Color", (float*)&color )){
        this->setCustomVar(color.x,"RED");
        this->setCustomVar(color.y,"GREEN");
        this->setCustomVar(color.z,"BLUE");
        this->setCustomVar(color.w,"ALPHA");
    }

    ImGuiEx::ObjectInfo(
                "A customizable numeric value plotter.",
                "https://mosaic.d3cod3.org/reference.php?r=value-plotter", scaleFactor);
}

//--------------------------------------------------------------
void moValuePlotter::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void moValuePlotter::loadVariableName(){
    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){
        int totalObjects = XML.getNumTags("object");
        for(int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                if(XML.getValue("id", -1) == this->nId){
                    name = XML.getValue("varname","none");
                }
                XML.popTag();
            }
        }
    }
}

//--------------------------------------------------------------
void moValuePlotter::saveVariableName(){
    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){
        int totalObjects = XML.getNumTags("object");
        for(int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                if(XML.getValue("id", -1) == this->nId){
                    XML.setValue("varname",name);
                }
                XML.popTag();
            }
        }
        XML.saveFile();
    }
}

OBJECT_REGISTER( moValuePlotter, "value plotter", OFXVP_OBJECT_CAT_GUI)

#endif
