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

#include "Constant.h"

//--------------------------------------------------------------
Constant::Constant() :
        // Extend the classes you need
        PatchObject("constant"),

        // define default values
        inputValueNew(0.f, "")
{

    this->numInlets  = 1;
    this->numOutlets = 2;

    // Bind Inlets to values
    *(float *)&_inletParams[0] = inputValueNew.get();

    // Bind outlets to values
    *(float *)&_outletParams[0] = inputValueNew.get(); // float output

     _outletParams[1] = new string(); // string output
     *static_cast<string *>(_outletParams[1]) = "";

    this->initInletsState();

    this->height        /= 2;

}

//--------------------------------------------------------------
void Constant::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"number");
    this->addOutlet(VP_LINK_NUMERIC,"number");
    this->addOutlet(VP_LINK_STRING,"numberString");

    this->setCustomVar(static_cast<float>(inputValueNew.get()),"NUMBER");
}

//--------------------------------------------------------------
void Constant::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    inputValueNew = this->getCustomVar("NUMBER");

}

//--------------------------------------------------------------
void Constant::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){
    if(this->inletsConnected[0]){
      inputValueNew = *(float *)&_inletParams[0];
    }

    *(float *)&_outletParams[0] = inputValueNew;

    *static_cast<string *>(_outletParams[1]) = ofToString( inputValueNew.get() );

}

//--------------------------------------------------------------
void Constant::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();

    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void Constant::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    // Menu
    if(_nodeCanvas.BeginNodeMenu()){
        ImGui::MenuItem("Menu From User code !");
        _nodeCanvas.EndNodeMenu();
    }

    // Info view
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Info) ){

        ImGui::Text("info View : %d", _nodeCanvas.GetNodeData().viewName );
        _nodeCanvas.EndNodeContent();
    }

    else if( _nodeCanvas.BeginNodeContent() ){
        // Parameters view
        if(_nodeCanvas.GetNodeData().viewName == ImGuiExNodeView_Params){
            ImGui::DragFloat("", &inputValueNew.get()); // inputValueNew.getDisplayName().c_str()
            //inputValueNew.drawGui();
        }
        // visualize view
        else {
            ImGui::DragFloat("", &inputValueNew.get());
        }
        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void Constant::removeObjectContent(bool removeFileFromData){

}

OBJECT_REGISTER( Constant, "constant", OFXVP_OBJECT_CAT_MATH)
