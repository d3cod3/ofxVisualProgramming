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

//#ifndef OFXVP_BUILD_WITH_MINIMAL_OBJECTS

#include "ExampleObject.h"

//--------------------------------------------------------------
ExampleObject::ExampleObject() :
    // Extend the classes you need
    PatchObject("Example Object"),// ofThread(),

    // define default values
    intParam(0, "My integer param"),
    floatParam(0.f, "My float param")
{

    // default values
    intParam = (unsigned int) 1;
    floatParam = 0.f;

    this->numInlets  = 1;
    this->numOutlets = 1;

    //*(float *)&_inletParams[0] = intParam.get();
    *(unsigned int *)&_inletParams[0] = intParam.get();
    *(float *)&_outletParams[0] = floatParam.get(); // output

    this->initInletsState();

    isGUIObject             = true;
    this->isOverGUI         = true;

}

//--------------------------------------------------------------
void ExampleObject::newObject(){
    //this->setName(this->objectName);
    PatchObject::setName( this->objectName );
    //ofxVisualProgrammingObjects::factory::RegistryEntry<ExampleObject>;
    //ofxVisualProgrammingObjects::factory::objectName;
    this->addInlet(VP_LINK_NUMERIC,"input");
    //this->addInlet(VP_LINK_NUMERIC,"jpeg quality");
    //this->addInlet(VP_LINK_NUMERIC,"jpeg quality variation");
    //this->addInlet(VP_LINK_NUMERIC,"jpeg subsampling");
    this->addOutlet(VP_LINK_NUMERIC,"output");

    //this->setCustomVar(jpegQuality,"QUALITY");
    //this->setCustomVar(jpegQualityVariation,"QUALITYVARIATION");
    //this->setCustomVar(jpegSubSampling,"SUBSAMPLING");
}

//--------------------------------------------------------------
void ExampleObject::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onSliderEvent(this, &ExampleObject::onSliderEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    gui->addBreak();
    gui->addBreak();
//    qualitySlider = gui->addSlider("Quality", 0.0, 1.0, jpegQuality);
//    qualitySlider->setUseCustomMouse(true);
//    qualityVariationSlider = gui->addSlider("Q Variation", 0.0, 1.0, jpegQualityVariation);
//    qualityVariationSlider->setUseCustomMouse(true);
//    subSamplingSlider = gui->addSlider("SubSampling", 0, 6, jpegSubSampling);
//    subSamplingSlider->setUseCustomMouse(true);
//    threadedParams = tjcParams(jpegQuality, jpegQualityVariation, jpegSubSampling);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);
}

//--------------------------------------------------------------
void ExampleObject::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    header->update();

    //qualitySlider->update();
    //qualityVariationSlider->update();
    //subSamplingSlider->update();

//    if(this->inletsConnected[1] && jpegQuality!=ofClamp(*(float *)&_inletParams[1],0.0f,1.0f)){
//        jpegQuality = ofClamp(*(float *)&_inletParams[1],0.0f,1.0f);
//        qualitySlider->setValue(jpegQuality);
//        bChanged = true;
//    }
//    if(this->inletsConnected[2] && jpegQualityVariation != ofClamp(*(float *)&_inletParams[2],0.0f,1.0f)){
//        jpegQualityVariation = ofClamp(*(float *)&_inletParams[2],0.0f,1.0f);
//        qualitySlider->setValue(jpegQualityVariation);
//        bChanged = true;
//    }
//    if(this->inletsConnected[3] && jpegSubSampling !=  ofClamp(*(int *)&_inletParams[3],0,10) ){
//        jpegSubSampling = ofClamp(*(int *)&_inletParams[3],0,10);
//        subSamplingSlider->setValue(jpegSubSampling);
//        bChanged = true;
//    }

//    if(this->inletsConnected[3] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
//        sliderX->setValue(ofClamp(*(int *)&_inletParams[3],0.0f,static_cast<ofTexture *>(_inletParams[0])->getWidth()));
//    }

}

//--------------------------------------------------------------
void ExampleObject::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
//    if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
//        static_cast<ofTexture *>(_outletParams[0])->draw(0,0,this->width,this->height);
//    }
    gui->draw();
    //cout << getUID() << endl;

    ofDrawEllipse(glm::vec2(this->getPos().x, this->getPos().y), this->getObjectWidth(), this->getObjectHeight());

    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void ExampleObject::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
    // Menu
    if(_nodeCanvas.BeginNodeMenu()){
        ImGui::MenuItem("Menu From User code !");
        _nodeCanvas.EndNodeMenu();
    }

    // Info view
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Info) ){
        ImGui::Button("Node Button", ImVec2(-1,20));
        ImGui::TextUnformatted("Hello World!");
        ImGui::TextUnformatted( ofToString(ImGui::GetCurrentWindow()->Pos).c_str() );
        ImGui::TextWrapped("Hovered:     %d", ImGui::IsWindowHovered() ? 1 : 0);
        ImGui::TextWrapped("PrevItemSize: %f, %f", ImGui::GetItemRectSize().x, ImGui::GetItemRectSize().y);//
        ImGui::TextWrapped("WindowSize: %f, %f", ImGui::GetCurrentWindow()->Rect().GetSize().x, ImGui::GetCurrentWindow()->Rect().GetSize().y);
        //ImGui::TextWrapped("WidgetSize: %f, %f", imSize.x, imSize.y);
        ImGui::TextWrapped("AvailableCRWidth: %f", ImGui::GetContentRegionAvailWidth());
        _nodeCanvas.EndNodeContent();
    }

    // Any other view
    else if( _nodeCanvas.BeginNodeContent() ){
        if(_nodeCanvas.GetNodeData().viewName == ImGuiExNodeView_Params){
            ImGui::Text("Cur View :Parameters");
        }
        else {
            ImGui::Text("Unknown View : %d", _nodeCanvas.GetNodeData().viewName );
        }
        _nodeCanvas.EndNodeContent();
    }
}

//--------------------------------------------------------------
void ExampleObject::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void ExampleObject::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    //qualitySlider->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    //qualityVariationSlider->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    //subSamplingSlider->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

//    if(!header->getIsCollapsed()){
//        this->isOverGUI = header->hitTest(_m-this->getPos());
//    }else{
//        this->isOverGUI = header->hitTest(_m-this->getPos());
//    }
    this->isOverGUI = header->hitTest(_m-this->getPos());
}

//--------------------------------------------------------------
void ExampleObject::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        header->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
//        qualitySlider->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
//        qualityVariationSlider->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
//        subSamplingSlider->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }else{
        ofNotifyEvent(dragEvent, nId);

        box->setFromCenter( _m.x, _m.y, box->getWidth(), box->getHeight() );
        headerBox->set( box->getPosition().x, box->getPosition().y, box->getWidth(), headerHeight );

        x = box->getPosition().x;
        y = box->getPosition().y;

        for(int j=0;j<static_cast<int>(outPut.size());j++){
            outPut[j]->linkVertices[0].move(outPut[j]->posFrom.x,outPut[j]->posFrom.y);
            outPut[j]->linkVertices[1].move(outPut[j]->posFrom.x+20,outPut[j]->posFrom.y);
        }
    }
}

//--------------------------------------------------------------
void ExampleObject::onSliderEvent(ofxDatGuiSliderEvent e){

}

// REGISTER OBJECT ON COMPILATION TIME
OBJECT_REGISTER( ExampleObject, "Example Object", OFXVP_OBJECT_CAT_GRAPHICS);

//#endif
