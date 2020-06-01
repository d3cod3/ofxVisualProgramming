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

#include "ExampleObject.h"

//--------------------------------------------------------------
ExampleObject::ExampleObject() :
    // Extend the classes you need
    PatchObject("example object"),// ofThread(),

    // define default values
    intParam(0, "My integer param"),
    floatParam(0.f, "My float param"),
    myEnumParam(-1, "My Enum Param", {"Option 1", "Option 2"}),
    myColorParam( ofFloatColor(0), "Color"),
    myStringParam("Mosaic is great", "String"),
    myBoolParam(true, "ToggleMe")
{

    // default values
    intParam = (unsigned int) 1;
    floatParam = 0.f;
    myEnumParam = (int)-1;

    this->numInlets  = 1;
    this->numOutlets = 1;

    //*(float *)&_inletParams[0] = intParam.get();
    *(unsigned int *)&_inletParams[0] = intParam.get();
    *(float *)&_outletParams[0] = floatParam.get(); // output

    this->initInletsState();

    isGUIObject             = false;
    this->isOverGUI         = false;

    this->setIsResizable(true);

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

}

//--------------------------------------------------------------
void ExampleObject::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

}

//--------------------------------------------------------------
void ExampleObject::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
//    if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
//        static_cast<ofTexture *>(_outletParams[0])->draw(0,0,this->width,this->height);
//    }
    //cout << getUID() << endl;

    //ofDrawEllipse(glm::vec2(this->getPos().x, this->getPos().y), this->getObjectWidth(), this->getObjectHeight());

    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void ExampleObject::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){
    // Menu
    if(_nodeCanvas.BeginNodeMenu()){
        if (ImGui::BeginMenu("PARAMS"))
        {
            intParam.drawGui();
            floatParam.drawGui();
            myEnumParam.drawGui();
            myColorParam.drawGui();
            myStringParam.drawGui();
            myBoolParam.drawGui();

            ImGui::EndMenu();
        }

        _nodeCanvas.EndNodeMenu();
    }

    // Info view
    /*if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Info) ){
        ImGui::Button("Node Button", ImVec2(-1,20));
        ImGui::TextUnformatted("Hello World!");

        intParam.drawGui();
        floatParam.drawGui();
        myEnumParam.drawGui();
        myColorParam.drawGui();
        myStringParam.drawGui();
        myBoolParam.drawGui();

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
            if (ImGui::CollapsingHeader("CONFIG")){
                ImGui::Button("Node Button", ImVec2(-1,20));
                ImGui::TextUnformatted("Hello World!");

                intParam.drawGui();
                floatParam.drawGui();
                myEnumParam.drawGui();
                myColorParam.drawGui();
                myStringParam.drawGui();
                myBoolParam.drawGui();

                ImGui::TextUnformatted( ofToString(ImGui::GetCurrentWindow()->Pos).c_str() );
                ImGui::TextWrapped("Hovered:     %d", ImGui::IsWindowHovered() ? 1 : 0);
                ImGui::TextWrapped("PrevItemSize: %f, %f", ImGui::GetItemRectSize().x, ImGui::GetItemRectSize().y);//
                ImGui::TextWrapped("WindowSize: %f, %f", ImGui::GetCurrentWindow()->Rect().GetSize().x, ImGui::GetCurrentWindow()->Rect().GetSize().y);
                //ImGui::TextWrapped("WidgetSize: %f, %f", imSize.x, imSize.y);
                ImGui::TextWrapped("AvailableCRWidth: %f", ImGui::GetContentRegionAvailWidth());
            }

        }
        _nodeCanvas.EndNodeContent();
    }*/

    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){
        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING)); // Padding top
        ImGui::Button("Node Button", ImVec2(-1,20));
        ImGui::TextUnformatted("Hello World!");

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void ExampleObject::removeObjectContent(bool removeFileFromData){

}

// REGISTER OBJECT ON COMPILATION TIME
OBJECT_REGISTER( ExampleObject, "example object", OFXVP_OBJECT_CAT_GRAPHICS)
