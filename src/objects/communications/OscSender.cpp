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

#include "OscSender.h"

//--------------------------------------------------------------
OscSender::OscSender() : PatchObject("osc sender"){

    this->numInlets  = 0;
    this->numOutlets = 0;

    this->initInletsState();

    osc_port            = 12345;
    osc_port_string     = ofToString(osc_port);
    osc_host            = "localhost";

    _tempPixels         = new ofPixels();
    _tempImage          = new ofImage();
    _tempBuffer         = new ofBuffer();

    loaded              = false;
}

//--------------------------------------------------------------
void OscSender::newObject(){
    PatchObject::setName( this->objectName );

    this->setCustomVar(static_cast<float>(osc_port),"PORT");
    this->setCustomVar(0.0f,"@"+osc_host);
}

//--------------------------------------------------------------
void OscSender::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    this->unusedArgs(mainWindow);

    initInlets();

}

//--------------------------------------------------------------
void OscSender::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(loaded){
        for(int i=0;i<this->getNumInlets();i++){
            if(this->inletsConnected[i]){
                ofxOscMessage m;
                bool messageOK = false;
                m.setAddress(osc_labels.at(i));
                if(this->getInletType(i) == VP_LINK_NUMERIC){
                    m.addFloatArg(*(float *)&_inletParams[i]);
                    messageOK = true;
                }else if(this->getInletType(i) == VP_LINK_STRING){
                    m.addStringArg(*static_cast<string *>(_inletParams[i]));
                    messageOK = true;
                }else if(this->getInletType(i) == VP_LINK_ARRAY){
                    for(size_t s=0;s<static_cast<size_t>(static_cast<vector<float> *>(_inletParams[i])->size());s++){
                        m.addFloatArg(static_cast<vector<float> *>(_inletParams[i])->at(s));
                    }
                    messageOK = true;
                }else if(this->getInletType(i) == VP_LINK_TEXTURE && static_cast<ofTexture *>(_inletParams[i])->isAllocated()){
                    // note: the size of the image depends greatly on your network buffer sizes,
                    // if an image is too big the message won't come through
                    int depth = 1;
                    if(static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_LUMINANCE || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_LUMINANCE8 || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_LUMINANCE16 || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_LUMINANCE32F_ARB){
                        depth = 1;
                    }else if(static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGB || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGB8 || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGB16 || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGB32F || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGB32F_ARB){
                        depth = 3;
                    }else if(static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGBA ||static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGBA16 || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGBA32F_ARB){
                        depth = 4;
                    }
                    if(static_cast<ofTexture *>(_inletParams[i])->getWidth()*static_cast<ofTexture *>(_inletParams[i])->getHeight()*depth < 922000){ // 327680
                        static_cast<ofTexture *>(_inletParams[i])->readToPixels(*_tempPixels);
                        m.addFloatArg(static_cast<ofTexture *>(_inletParams[i])->getWidth());
                        m.addFloatArg(static_cast<ofTexture *>(_inletParams[i])->getHeight());
                        if(static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_LUMINANCE || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_LUMINANCE8 || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_LUMINANCE16 || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_LUMINANCE32F_ARB){
                            _tempImage->setFromPixels(_tempPixels->getData(),static_cast<ofTexture *>(_inletParams[i])->getWidth(),static_cast<ofTexture *>(_inletParams[i])->getHeight(),OF_IMAGE_GRAYSCALE);
                            m.addInt32Arg(OF_IMAGE_GRAYSCALE);
                        }else if(static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGB || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGB8 || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGB16 || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGB32F || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGB32F_ARB){
                            _tempImage->setFromPixels(_tempPixels->getData(),static_cast<ofTexture *>(_inletParams[i])->getWidth(),static_cast<ofTexture *>(_inletParams[i])->getHeight(),OF_IMAGE_COLOR);
                            m.addInt32Arg(OF_IMAGE_COLOR);
                        }else if(static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGBA ||static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGBA16 || static_cast<ofTexture *>(_inletParams[i])->getTextureData().glInternalFormat == GL_RGBA32F_ARB){
                            _tempImage->setFromPixels(_tempPixels->getData(),static_cast<ofTexture *>(_inletParams[i])->getWidth(),static_cast<ofTexture *>(_inletParams[i])->getHeight(),OF_IMAGE_COLOR_ALPHA);
                            m.addInt32Arg(OF_IMAGE_COLOR_ALPHA);
                        }
                        _tempImage->save(*_tempBuffer);
                        m.addBlobArg(*_tempBuffer);
                        _tempBuffer->clear();
                        messageOK = true;
                    }else{
                        ofLog(OF_LOG_ERROR,"The image you're trying to send via OSC is too big! Please choose an image below 1280x720 GRAYSCALE, or 640x480 RGB, or 640x360 RGBA");
                    }
                }
                if(messageOK){
                    osc_sender.sendMessage(m,false);
                }
            }
        }
    }

    if(!loaded){
        loaded = true;

        osc_host = getHostFromConfig();
        osc_port = static_cast<int>(floor(this->getCustomVar("PORT")));
        osc_port_string     = ofToString(osc_port);
        osc_sender.setup(osc_host.c_str(),osc_port);
    }

}

//--------------------------------------------------------------
void OscSender::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void OscSender::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            if(loaded){
                drawObjectNodeConfig(); this->configMenuWidth = ImGui::GetWindowWidth();
            }


            ImGui::EndMenu();
        }
        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*scaleFactor));

        ImGui::PushItemWidth(80*scaleFactor);
        if(ImGui::InputText("HOST",&osc_host)){
            filepath = osc_host;
            this->saveConfig(false);
        }
        ImGui::Spacing();
        if(ImGui::InputText("PORT",&osc_port_string)){
            if(isInteger(osc_port_string)){
                osc_port = ofToInt(osc_port_string);
                this->setCustomVar(static_cast<float>(osc_port),"PORT");
            }
        }
        ImGui::Spacing();
        if(ImGui::Button("APPLY",ImVec2(-1,26*scaleFactor))){
            osc_sender.setup(osc_host.c_str(),osc_port);
        }
        ImGui::PopItemWidth();

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void OscSender::drawObjectNodeConfig(){
    ImGui::Spacing();
    ImGui::Text("Sending OSC data @ %s:%s", osc_host.c_str(), osc_port_string.c_str());

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();

    if(ImGui::Button("ADD OSC NUMBER",ImVec2(224*scaleFactor,26*scaleFactor))){
        _inletParams[this->numInlets] = new float();
        *(float *)&_inletParams[this->numInlets] = 0.0f;
        this->addInlet(VP_LINK_NUMERIC,"number");
        this->inletsConnected.push_back(false);

        osc_labels.push_back("/numberlabel");
        osc_labels_type.push_back(VP_LINK_NUMERIC);
        this->setCustomVar(0.0f,"/numberlabel");

        this->numInlets++;
        this->saveConfig(false);
    }
    ImGui::Spacing();
    if(ImGui::Button("ADD OSC TEXT",ImVec2(224*scaleFactor,26*scaleFactor))){
        _inletParams[this->numInlets] = new string();  // control
        *static_cast<string *>(_inletParams[this->numInlets]) = "";
        this->addInlet(VP_LINK_STRING,"text");
        this->inletsConnected.push_back(false);

        osc_labels.push_back("/textlabel");
        osc_labels_type.push_back(VP_LINK_STRING);
        this->setCustomVar(0.0f,"/textlabel");

        this->numInlets++;
        this->saveConfig(false);
    }
    ImGui::Spacing();
    if(ImGui::Button("ADD OSC VECTOR",ImVec2(224*scaleFactor,26*scaleFactor))){
        _inletParams[this->numInlets] = new vector<float>();
        this->addInlet(VP_LINK_ARRAY,"vector");
        this->inletsConnected.push_back(false);

        osc_labels.push_back("/vectorlabel");
        osc_labels_type.push_back(VP_LINK_ARRAY);
        this->setCustomVar(0.0f,"/vectorlabel");

        this->numInlets++;
        this->saveConfig(false);
    }
    ImGui::Spacing();
    if(ImGui::Button("ADD OSC TEXTURE",ImVec2(224*scaleFactor,26*scaleFactor))){
        _inletParams[this->numInlets] = new ofTexture();
        this->addInlet(VP_LINK_TEXTURE,"texture");
        this->inletsConnected.push_back(false);

        osc_labels.push_back("/texturelabel");
        osc_labels_type.push_back(VP_LINK_TEXTURE);
        this->setCustomVar(0.0f,"/texturelabel");

        this->numInlets++;
        this->saveConfig(false);
    }
    ImGui::SameLine(); ImGuiEx::HelpMarker("UNSTABLE (limited by the small OSC max. send buffer size) Use it at your own risk!");

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();

    prev_osc_labels = osc_labels;

    for(int i=0;i<osc_labels.size();i++){
        ImGui::PushID(i);
        if(osc_labels_type.at(i) == VP_LINK_STRING){
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(200,180,255,30));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(200,180,255,60));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(200,180,255,60));
        }else if(osc_labels_type.at(i) == VP_LINK_ARRAY){
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(120,255,120,30));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(120,255,120,60));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(120,255,120,60));
        }else if(osc_labels_type.at(i) == VP_LINK_TEXTURE){
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(120,255,255,30));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(120,255,255,60));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(120,255,255,60));
        }else{
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered]);
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive]);
        }
        if(ImGui::InputText("###label",&osc_labels.at(i))){
            this->substituteCustomVar(prev_osc_labels.at(i),osc_labels.at(i));
            this->saveConfig(false);
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();
    }

    ImGuiEx::ObjectInfo(
                "Send data via OSC protocol as numeric type (float), text (string), vector<float> or texture. Texture send is limited by the maximum buffer size permitted, so you have max 1280x720 for grayscale texture, 640x480 for RGB and 640x360 for RGBA",
                "https://mosaic.d3cod3.org/reference.php?r=osc-sender", scaleFactor);
}

//--------------------------------------------------------------
void OscSender::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void OscSender::initInlets(){
    ofxXmlSettings XML;
    if(XML.loadFile(this->patchFile)){
        int totalObjects = XML.getNumTags("object");

        // Get object inlets config
        for(int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                if(XML.getValue("id", -1) == this->nId){
                    vector<int> tempTypes;
                    if(XML.pushTag("inlets")){
                        for (int t=0;t<XML.getNumTags("link");t++){
                            if(XML.pushTag("link",t)){
                                int type = XML.getValue("type", 0);
                                tempTypes.push_back(type);
                                XML.popTag();
                            }
                        }
                        XML.popTag();
                    }
                    if (XML.pushTag("vars")){
                        int totalOutlets = XML.getNumTags("var");
                        this->numInlets = totalOutlets-2;

                        int tempCounter = 0;
                        for (int t=0;t<totalOutlets;t++){
                            if(XML.pushTag("var",t)){
                                bool isreceiverIP = false;
                                if (XML.getValue("name","").find('@') != std::string::npos){
                                    isreceiverIP = true;
                                }else{
                                    isreceiverIP = false;
                                }
                                if(XML.getValue("name","") != "PORT" && !isreceiverIP){
                                    if(tempTypes.at(tempCounter) == 0){ // float
                                        _inletParams[tempCounter] = new float();
                                        *(float *)&_inletParams[tempCounter] = 0.0f;
                                        osc_labels.push_back(XML.getValue("name",""));
                                        osc_labels_type.push_back(VP_LINK_NUMERIC);
                                        tempCounter++;
                                    }else if(tempTypes.at(tempCounter) == 1){ // string
                                        _inletParams[tempCounter] = new string();  // control
                                        *static_cast<string *>(_inletParams[tempCounter]) = "";
                                        osc_labels.push_back(XML.getValue("name",""));
                                        osc_labels_type.push_back(VP_LINK_STRING);
                                        tempCounter++;
                                    }else if(tempTypes.at(tempCounter) == 2){ // vector<float>
                                        _inletParams[tempCounter] = new vector<float>();
                                        osc_labels.push_back(XML.getValue("name",""));
                                        osc_labels_type.push_back(VP_LINK_ARRAY);
                                        tempCounter++;
                                    }else if(tempTypes.at(tempCounter) == 3){ // ofTexture
                                        _inletParams[tempCounter] = new ofTexture();
                                        osc_labels.push_back(XML.getValue("name",""));
                                        osc_labels_type.push_back(VP_LINK_TEXTURE);
                                        tempCounter++;
                                    }
                                }
                                XML.popTag();
                            }
                        }
                        XML.popTag();
                    }
                }
                XML.popTag();
            }
        }
    }

    this->initInletsState();
}

//--------------------------------------------------------------
string OscSender::getHostFromConfig(){

    ofxXmlSettings XML;
    if(XML.loadFile(this->patchFile)){
        int totalObjects = XML.getNumTags("object");

        // Get object inlets config
        for(int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                if(XML.getValue("id", -1) == this->nId){
                    string temp = XML.getValue("filepath","none");

                    size_t found = temp.find_last_of("/");
                    if(found != string::npos){
                        return temp.substr(found+1);
                    }else{
                        if(temp == "none"){
                            return "localhost";
                        }else{
                            return temp;
                        }
                    }
                }
                XML.popTag();
            }
        }
    }

    return "localhost";
}


OBJECT_REGISTER( OscSender, "osc sender", OFXVP_OBJECT_CAT_COMMUNICATIONS)

#endif
