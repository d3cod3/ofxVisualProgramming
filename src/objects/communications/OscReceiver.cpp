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

#include "OscReceiver.h"

//--------------------------------------------------------------
OscReceiver::OscReceiver() : PatchObject("osc receiver"){

    this->numInlets  = 0;
    this->numOutlets = 0;

    this->initInletsState();

    osc_port            = 12345;
    osc_port_string     = ofToString(osc_port);
    local_ip            = "0.0.0.0";

    _tempImage          = new ofImage();

    loaded              = false;

}

//--------------------------------------------------------------
void OscReceiver::newObject(){
    PatchObject::setName( this->objectName );

    this->setCustomVar(static_cast<float>(osc_port),"PORT");
}

//--------------------------------------------------------------
void OscReceiver::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    local_ip = getLocalIP();

    initOutlets();
}

//--------------------------------------------------------------
void OscReceiver::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    if(osc_labels.size() > 0 && loaded && osc_receiver.isListening()){
        while(osc_receiver.hasWaitingMessages()){
            ofxOscMessage m;
            osc_receiver.getNextMessage(m);

            for(int i=0;i<this->getNumOutlets();i++){
                if(m.getAddress() == osc_labels.at(i)){
                    if(this->getOutletType(i) == VP_LINK_NUMERIC){
                        if(m.getNumArgs() == 1){
                            if(m.getArgType(0) == OFXOSC_TYPE_INT32 || m.getArgType(0) == OFXOSC_TYPE_FLOAT){
                                *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[i]) = static_cast<float>(m.getArgAsFloat(0));
                            }
                        }
                    }else if(this->getOutletType(i) == VP_LINK_STRING){
                        if(m.getNumArgs() == 1 && m.getArgType(0) == OFXOSC_TYPE_STRING){
                            *ofxVP_CAST_PIN_PTR<string>(_outletParams[i]) = m.getArgAsString(0);
                        }
                    }else if(this->getOutletType(i) == VP_LINK_ARRAY){
                        ofxVP_CAST_PIN_PTR<vector<float>>(_outletParams[i])->clear();
                        for(size_t a = 0; a < m.getNumArgs(); a++){
                            if(m.getArgType(a) == OFXOSC_TYPE_INT32 || m.getArgType(a) == OFXOSC_TYPE_FLOAT){
                                ofxVP_CAST_PIN_PTR<vector<float>>(_outletParams[i])->push_back(m.getArgAsFloat(a));
                            }
                        }
                    }else if(this->getOutletType(i) == VP_LINK_TEXTURE){
                        // note: the size of the image depends greatly on your network buffer sizes,
                        // if an image is too big the message won't come through
                        if(m.getNumArgs() == 4 && m.getArgType(2) == OFXOSC_TYPE_INT32 && m.getArgType(3) == OFXOSC_TYPE_BLOB){
                            _tempImage->load(m.getArgAsBlob(3));
                            if(m.getArgAsInt32(2) == OF_IMAGE_GRAYSCALE){
                                ofxVP_CAST_PIN_PTR<ofTexture>(_outletParams[i])->allocate(m.getArgAsInt32(0),m.getArgAsInt32(1),GL_LUMINANCE);
                                ofxVP_CAST_PIN_PTR<ofTexture>(_outletParams[i])->loadData(_tempImage->getPixels(),GL_LUMINANCE);
                            }else if(m.getArgAsInt32(2) == OF_IMAGE_COLOR){
                                ofxVP_CAST_PIN_PTR<ofTexture>(_outletParams[i])->allocate(m.getArgAsInt32(0),m.getArgAsInt32(1),GL_RGB);
                                ofxVP_CAST_PIN_PTR<ofTexture>(_outletParams[i])->loadData(_tempImage->getPixels(),GL_RGB);
                            }else if(m.getArgAsInt32(2) == OF_IMAGE_COLOR_ALPHA){
                                ofxVP_CAST_PIN_PTR<ofTexture>(_outletParams[i])->allocate(m.getArgAsInt32(0),m.getArgAsInt32(1),GL_RGBA);
                                ofxVP_CAST_PIN_PTR<ofTexture>(_outletParams[i])->loadData(_tempImage->getPixels(),GL_RGB);
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    if(!loaded){
        loaded = true;

        osc_port = static_cast<int>(floor(this->getCustomVar("PORT")));
        osc_port_string     = ofToString(osc_port);
        osc_receiver.setup(osc_port);
    }

}

//--------------------------------------------------------------
void OscReceiver::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);

}

//--------------------------------------------------------------
void OscReceiver::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::BeginMenu("CONFIG"))
        {

            if(loaded && osc_receiver.isListening()){
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
        ImGui::Text("%s", local_ip.c_str());
        ImGui::Spacing();
        if(ImGui::InputText("PORT",&osc_port_string)){
            if(isInteger(osc_port_string)){
                osc_port = ofToInt(osc_port_string);
                this->setCustomVar(static_cast<float>(osc_port),"PORT");
            }
        }
        ImGui::Spacing();
        if(ImGui::Button("APPLY",ImVec2(-1,26*scaleFactor))){
            osc_receiver.setup(osc_port);
        }
        ImGui::PopItemWidth();

        _nodeCanvas.EndNodeContent();
    }

}

//--------------------------------------------------------------
void OscReceiver::drawObjectNodeConfig(){
    ImGui::Spacing();
    ImGui::Text("Receiving OSC data @ port %s", osc_port_string.c_str());

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();

    if(ImGui::Button("ADD OSC NUMBER",ImVec2(224*scaleFactor,26*scaleFactor))){
        _outletParams[this->numOutlets] = new float();
        *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[this->numOutlets]) = 0.0f;
        this->addOutlet(VP_LINK_NUMERIC,"number");

        osc_labels.push_back("/numberlabel");
        osc_labels_type.push_back(VP_LINK_NUMERIC);
        this->setCustomVar(0.0f,"/numberlabel");

        this->numOutlets++;
        resetOutlets();
    }
    ImGui::Spacing();
    if(ImGui::Button("ADD OSC TEXT",ImVec2(224*scaleFactor,26*scaleFactor))){
        _outletParams[this->numOutlets] = new string();  // control
        *ofxVP_CAST_PIN_PTR<string>(_outletParams[this->numOutlets]) = "";
        this->addOutlet(VP_LINK_STRING,"text");

        osc_labels.push_back("/textlabel");
        osc_labels_type.push_back(VP_LINK_STRING);
        this->setCustomVar(0.0f,"/textlabel");

        this->numOutlets++;
        resetOutlets();
    }
    ImGui::Spacing();
    if(ImGui::Button("ADD OSC VECTOR",ImVec2(224*scaleFactor,26*scaleFactor))){
        _outletParams[this->numOutlets] = new vector<float>();
        this->addOutlet(VP_LINK_ARRAY,"vector");

        osc_labels.push_back("/vectorlabel");
        osc_labels_type.push_back(VP_LINK_ARRAY);
        this->setCustomVar(0.0f,"/vectorlabel");

        this->numOutlets++;
        resetOutlets();
    }
    ImGui::Spacing();
    if(ImGui::Button("ADD OSC TEXTURE",ImVec2(224*scaleFactor,26*scaleFactor))){
        _outletParams[this->numOutlets] = new ofTexture();
        this->addOutlet(VP_LINK_TEXTURE,"texture");

        osc_labels.push_back("/texturelabel");
        osc_labels_type.push_back(VP_LINK_TEXTURE);
        this->setCustomVar(0.0f,"/texturelabel");

        this->numOutlets++;
        resetOutlets();
    }

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
                "Receive data via OSC protocol as numeric type (float), text (string), vector<float> or texture. Texture receive is limited by the maximum buffer size permitted, so you have max 1280x720 for grayscale texture, 640x480 for RGB and 640x360 for RGBA",
                "https://mosaic.d3cod3.org/reference.php?r=osc-receiver", scaleFactor);
}

//--------------------------------------------------------------
void OscReceiver::removeObjectContent(bool removeFileFromData){
    if(osc_receiver.isListening()){
        osc_receiver.stop();
    }
}

//--------------------------------------------------------------
void OscReceiver::initOutlets(){
    ofxXmlSettings XML;
#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
    if (XML.loadFile(this->patchFile)){
#else
    if (XML.load(this->patchFile)){
#endif
        int totalObjects = XML.getNumTags("object");

        // Load object outlet config
        for(int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                if(XML.getValue("id", -1) == this->nId){
                    vector<int> tempTypes;
                    if(XML.pushTag("outlets")){
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
                        this->numOutlets = totalOutlets-1;

                        int tempCounter = 0;
                        for (int t=0;t<totalOutlets;t++){
                            if(XML.pushTag("var",t)){
                                if(XML.getValue("name","") != "PORT"){
                                    if(tempTypes.at(tempCounter) == 0){
                                        _outletParams[tempCounter] = new float();
                                        *ofxVP_CAST_PIN_PTR<float>(this->_outletParams[tempCounter]) = 0.0f;
                                        osc_labels.push_back(XML.getValue("name",""));
                                        osc_labels_type.push_back(VP_LINK_NUMERIC);
                                        tempCounter++;
                                    }else if(tempTypes.at(tempCounter) == 1){
                                        _outletParams[tempCounter] = new string();
                                        *ofxVP_CAST_PIN_PTR<string>(_outletParams[tempCounter]) = "";
                                        osc_labels.push_back(XML.getValue("name",""));
                                        osc_labels_type.push_back(VP_LINK_STRING);
                                        tempCounter++;
                                    }else if(tempTypes.at(tempCounter) == 2){
                                        _outletParams[tempCounter] = new vector<float>();
                                        osc_labels.push_back(XML.getValue("name",""));
                                        osc_labels_type.push_back(VP_LINK_ARRAY);
                                        tempCounter++;
                                    }else if(tempTypes.at(tempCounter) == 3){
                                        _outletParams[tempCounter] = new ofTexture();
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
}

//--------------------------------------------------------------
void OscReceiver::resetOutlets(){

    this->height      = OBJECT_HEIGHT;
    if(this->numOutlets > 6){
        this->height          *= 2;
    }
    if(this->numOutlets > 12){
        this->height          *= 2;
    }

    ofxXmlSettings XML;
#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
    if (XML.loadFile(this->patchFile)){
#else
    if (XML.load(this->patchFile)){
#endif
        int totalObjects = XML.getNumTags("object");

        // Save new object outlet config
        for(int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                if(XML.getValue("id", -1) == this->nId){
                    // Dynamic reloading outlets
                    XML.removeTag("outlets");
                    int newOutlets = XML.addTag("outlets");
                    if(XML.pushTag("outlets",newOutlets)){
                        for(int j=0;j<static_cast<int>(this->outletsType.size());j++){
                            int newLink = XML.addTag("link");
                            if(XML.pushTag("link",newLink)){
                                XML.setValue("type",this->outletsType.at(j));
                                XML.popTag();
                            }
                        }
                        XML.popTag();
                    }
                }
                XML.popTag();
            }
        }

#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
            XML.saveFile();
#else
            XML.save();
#endif
    }

    this->saveConfig(false);
}

//--------------------------------------------------------------
string OscReceiver::getLocalIP(){
    string IP = "";

    string cmd = "";
    FILE *execFile;
#ifdef TARGET_LINUX
    //cmd = "ifconfig | grep -w 'inet' | grep -v 127.0.0.1 | awk '{print $2}'";
    cmd = "ifconfig | grep -w 'flags=4163' -A 1 | grep -w 'inet' | grep -v 127.0.0.1 | awk '{print $2}'";
    execFile = popen(cmd.c_str(), "r");
#elif defined(TARGET_OSX)
    cmd = "ifconfig | grep -w 'inet' | grep -v 127.0.0.1 | awk '{print $2}'";
    execFile = popen(cmd.c_str(), "r");
#elif defined(TARGET_WIN32)
    cmd = "ipconfig | findstr v4";
    execFile = _popen(cmd.c_str(), "r");
#endif

    if (execFile){
        char buffer[128];
        while(!feof(execFile)){
            if(fgets(buffer, sizeof(buffer), execFile) != nullptr){
                char *s = buffer;
                std::string tempstr(s);
                #ifdef TARGET_WIN32
                    size_t found = tempstr.find_last_of(":");
                    IP.append(tempstr.substr(found+2));
                #else
                    IP.append(tempstr);
                #endif
            }
        }

#ifdef TARGET_LINUX
        pclose(execFile);
#elif defined(TARGET_OSX)
        pclose(execFile);
#elif defined(TARGET_WIN32)
        _pclose(execFile);
#endif

    }

    return IP;
}

OBJECT_REGISTER( OscReceiver, "osc receiver", OFXVP_OBJECT_CAT_COMMUNICATIONS)

#endif
