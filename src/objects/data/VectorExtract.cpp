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

#include "VectorExtract.h"

//--------------------------------------------------------------
VectorExtract::VectorExtract() : PatchObject("vector extract"){

    this->numInlets  = 3;
    this->numOutlets = 1;

    _inletParams[0] = new vector<float>(); // data vector
    _inletParams[1] = new float();  // start
    *(float *)&_inletParams[1] = 0.0f;
    _inletParams[2] = new float();  // end
    *(float *)&_inletParams[2] = 0.0f;

    _outletParams[0] = new vector<float>();  // final vector

    this->initInletsState();

    start           = 0;
    end             = 0;

    loaded          = false;

}

//--------------------------------------------------------------
void VectorExtract::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_ARRAY,"data");
    this->addInlet(VP_LINK_NUMERIC,"start");
    this->addInlet(VP_LINK_NUMERIC,"end");

    this->addOutlet(VP_LINK_ARRAY,"dataExtract");

    this->setCustomVar(static_cast<float>(start),"START");
    this->setCustomVar(static_cast<float>(end),"END");
}

//--------------------------------------------------------------
void VectorExtract::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);
}

//--------------------------------------------------------------
void VectorExtract::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    static_cast<vector<float> *>(_outletParams[0])->clear();

    if(this->inletsConnected[0] && !static_cast<vector<float> *>(_inletParams[0])->empty() && start < end){
        for(int s=start;s<end;s++){
            static_cast<vector<float> *>(_outletParams[0])->push_back(static_cast<vector<float> *>(_inletParams[0])->at(s));
        }
    }

    if(this->inletsConnected[1]){
        start = ofClamp(static_cast<int>(floor(*(float *)&_inletParams[1])),0,static_cast<vector<float> *>(_inletParams[0])->size()-2);
    }
    if(this->inletsConnected[2]){
        end = ofClamp(static_cast<int>(floor(*(float *)&_inletParams[2])),start+1,static_cast<vector<float> *>(_inletParams[0])->size()-1);
    }

    if(!loaded){
        loaded  = true;
        start   = static_cast<float>(this->getCustomVar("START"));
        end     = static_cast<float>(this->getCustomVar("END"));
    }
}

//--------------------------------------------------------------
void VectorExtract::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

    ofSetColor(255);

}

//--------------------------------------------------------------
void VectorExtract::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGui::SetCursorPos(ImVec2(IMGUI_EX_NODE_PINS_WIDTH_NORMAL+(4*scaleFactor), (this->height/2 *_nodeCanvas.GetCanvasScale()) - (6*scaleFactor)));
        if(static_cast<int>(static_cast<vector<float> *>(_outletParams[0])->size()) > 0){
            ImGui::Text("size\nrange");
            ImGui::SameLine();
            ImGui::Text("= %i\n= [0 - %i]",static_cast<int>(static_cast<vector<float> *>(_outletParams[0])->size()),static_cast<int>(static_cast<vector<float> *>(_outletParams[0])->size())-1);
        }


        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void VectorExtract::drawObjectNodeConfig(){
    ImGui::Spacing();
    if(ImGui::InputInt("Start",&start)){
        if(start < 0){
            start = 0;
        }
        if(start > static_cast<int>(static_cast<vector<float> *>(_inletParams[0])->size())-2){
            start = static_cast<vector<float> *>(_inletParams[0])->size()-2;
        }
        this->setCustomVar(static_cast<float>(start),"START");
    }
    ImGui::Spacing();
    int prevEnd = end;
    if(ImGui::InputInt("Size",&end)){
        if(end > start && end <= static_cast<int>(static_cast<vector<float> *>(_inletParams[0])->size())-1){
            this->setCustomVar(static_cast<float>(end),"END");
        }else{
            end = prevEnd;
        }

    }

    ImGuiEx::ObjectInfo(
                "Extract a subsection of a data vector.",
                "https://mosaic.d3cod3.org/reference.php?r=vector-extract", scaleFactor);
}

//--------------------------------------------------------------
void VectorExtract::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
}


OBJECT_REGISTER( VectorExtract, "vector extract", OFXVP_OBJECT_CAT_DATA)

#endif
