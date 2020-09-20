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

#include "moComment.h"

//--------------------------------------------------------------
moComment::moComment() : PatchObject("comment"){

    this->numInlets  = 2;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;
    _inletParams[1] = new string();  // comment
    *static_cast<string *>(_inletParams[1]) = "";

    _outletParams[0] = new string(); // output string
    *static_cast<string *>(_outletParams[0]) = "";

    this->initInletsState();

    bang                = false;
    nextFrame           = true;

    this->width             *= 2;

    this->setIsResizable(true);

    prevW                   = this->width;
    prevH                   = this->height;

    loaded                  = false;
}

//--------------------------------------------------------------
void moComment::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_STRING,"text");
    this->addOutlet(VP_LINK_STRING,"text");

    this->setCustomVar(static_cast<float>(prevW),"WIDTH");
    this->setCustomVar(static_cast<float>(prevH),"HEIGHT");
}

//--------------------------------------------------------------
void moComment::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){

    actualComment = "Comment your patches and share!";
    loadCommentSetting();

}

//--------------------------------------------------------------
void moComment::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] < 1.0){
            bang = false;
        }else{
            bang = true;
        }
    }

    if(this->inletsConnected[1]){
        actualComment = "";
        actualComment.append(*static_cast<string *>(_inletParams[1]));
    }

    if(!nextFrame){
        nextFrame = true;
        *static_cast<string *>(_outletParams[0]) = actualComment;
    }

    if(bang){
        nextFrame = false;
        *static_cast<string *>(_outletParams[0]) = actualComment+" ";
    }


    if(!loaded){
        loaded = true;
        prevW = this->getCustomVar("WIDTH");
        prevH = this->getCustomVar("HEIGHT");
        this->width             = prevW;
        this->height            = prevH;
    }

}

//--------------------------------------------------------------
void moComment::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void moComment::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){

        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            drawObjectNodeConfig();

            ImGui::EndMenu();
        }

        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        if(ImGui::InputTextMultiline("##source", &actualComment, ImVec2(ImGui::GetWindowSize().x-30, ImGui::GetWindowSize().y-24), ImGuiInputTextFlags_AllowTabInput)){
            saveCommentSetting();
        }


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
void moComment::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "A simple comment object.",
                "https://mosaic.d3cod3.org/reference.php?r=comment", scaleFactor);
}

//--------------------------------------------------------------
void moComment::removeObjectContent(bool removeFileFromData){
    
}

//--------------------------------------------------------------
void moComment::loadCommentSetting(){
    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){
        int totalObjects = XML.getNumTags("object");
        for(int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                if(XML.getValue("id", -1) == this->nId){
                    actualComment = XML.getValue("text","none");
                }
                XML.popTag();
            }
        }
    }
}

//--------------------------------------------------------------
void moComment::saveCommentSetting(){
    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){
        int totalObjects = XML.getNumTags("object");
        for(int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                if(XML.getValue("id", -1) == this->nId){
                    XML.setValue("text",actualComment);
                }
                XML.popTag();
            }
        }
        XML.saveFile();
    }
}

OBJECT_REGISTER( moComment, "comment", OFXVP_OBJECT_CAT_GUI)

#endif
