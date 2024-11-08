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

#include "AudioDevice.h"

//--------------------------------------------------------------
AudioDevice::AudioDevice() : PatchObject("audio device"){

    this->numInlets     = 0;
    this->numOutlets    = 0;

    in_channels         = 0;
    out_channels        = 0;

    sampleRateIN        = 0;
    sampleRateOUT       = 0;
    bufferSize          = 256;

    isSystemObject      = true;

    isAudioINObject     = true;
    isAudioOUTObject    = true;

    deviceLoaded        = false;

    bg                  = new ofImage();
    posX = posY = drawW = drawH = 0.0f;

    this->setIsTextureObj(true);
    this->setIsHardwareObj(true);
    
}

//--------------------------------------------------------------
void AudioDevice::newObject(){
    PatchObject::setName( this->objectName );
}

//--------------------------------------------------------------
void AudioDevice::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    loadDeviceInfo();

    ofDisableArbTex();
    bg->load("images/audioDevice_bg.jpg");
    ofEnableArbTex();

}

//--------------------------------------------------------------
void AudioDevice::setupAudioOutObjectContent(pdsp::Engine &engine){
    if(deviceLoaded && out_channels>0){
        for(size_t c=0;c<static_cast<size_t>(out_channels);c++){
            OUT_CH[c].out_signal() >> engine.audio_out(c);
        }
    }
    if(deviceLoaded && in_channels>0){
        for(size_t c=0;c<static_cast<size_t>(in_channels);c++){

            LF_ctrl >> LC_IN_CH[c].in_freq();
            HF_ctrl >> HC_IN_CH[c].in_freq();

            PN_IN_CH[c].out_signal() >> HC_IN_CH[c].in_signal();
            HC_IN_CH[c].out_signal() >> LC_IN_CH[c].in_signal();

            LC_IN_CH[c].out_signal() >> this->pdspOut[c];
            LC_IN_CH[c].out_signal() >> IN_SCOPE[c] >> engine.blackhole();
        }
        LF_ctrl.set(5.0f);
        HF_ctrl.set(20000.0f);
    }
}

//--------------------------------------------------------------
void AudioDevice::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);
}

//--------------------------------------------------------------
void AudioDevice::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

    if(this->height < OBJECT_HEIGHT*scaleFactor){
        this->height = OBJECT_HEIGHT*scaleFactor;
    }
}

//--------------------------------------------------------------
void AudioDevice::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

    // CONFIG GUI inside Menu
    if(_nodeCanvas.BeginNodeMenu()){

        if (ImGui::BeginMenu("CONFIG"))
        {

            drawObjectNodeConfig(); this->configMenuWidth = ImGui::GetWindowWidth();

            ImGui::EndMenu();
        }
        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        ImGui::SetCursorPos(ImVec2(posX+IMGUI_EX_NODE_PINS_WIDTH_NORMAL, posY+IMGUI_EX_NODE_HEADER_HEIGHT));
        ImGui::Image(bg->getTexture().getTextureData().textureID, ImVec2(this->width-IMGUI_EX_NODE_PINS_WIDTH_NORMAL-IMGUI_EX_NODE_PINS_WIDTH_SMALL,this->height-IMGUI_EX_NODE_HEADER_HEIGHT-IMGUI_EX_NODE_FOOTER_HEIGHT));

        // get imgui node translated/scaled position/dimension for drawing textures in OF
        //objOriginX = (ImGui::GetWindowPos().x + ((IMGUI_EX_NODE_PINS_WIDTH_NORMAL - 1)*this->scaleFactor) - _nodeCanvas.GetCanvasTranslation().x)/_nodeCanvas.GetCanvasScale();
        //objOriginY = (ImGui::GetWindowPos().y - _nodeCanvas.GetCanvasTranslation().y)/_nodeCanvas.GetCanvasScale();
        scaledObjW = this->width - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/_nodeCanvas.GetCanvasScale());
        scaledObjH = this->height - ((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor/_nodeCanvas.GetCanvasScale());

        _nodeCanvas.EndNodeContent();
    }

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();

}

//--------------------------------------------------------------
void AudioDevice::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Mosaic system object, which means that it cannot be added/deleted. The audio device object is a virtual direct connection to the audio hardware.",
                "https://mosaic.d3cod3.org/reference.php?r=audio-device", scaleFactor);
}

//--------------------------------------------------------------
void AudioDevice::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);

    for(size_t c=0;c<static_cast<size_t>(out_channels);c++){
        OUT_CH[c].disconnectOut();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void AudioDevice::audioInObject(ofSoundBuffer &inputBuffer){
    if(deviceLoaded && in_channels>0){
        if(in_channels == 1){
            inputBuffer.copyTo(IN_CH.at(0), inputBuffer.getNumFrames(), 1, 0);
            PN_IN_CH.at(0).copyInput(IN_CH.at(0).getBuffer().data(),IN_CH.at(0).getNumFrames());
            static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(IN_SCOPE[0].getBuffer(),1,inputBuffer.getNumFrames());
        }else{
            for(size_t c=0;c<static_cast<size_t>(in_channels);c++){
                inputBuffer.getChannel(IN_CH.at(c),c);
                PN_IN_CH.at(c).copyInput(IN_CH.at(c).getBuffer().data(),IN_CH.at(c).getNumFrames());
                static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(IN_SCOPE[c].getBuffer(),1,inputBuffer.getNumFrames());
            }
        }
    }
}

//--------------------------------------------------------------
void AudioDevice::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);

    if(deviceLoaded && out_channels>0){
        for(size_t c=0;c<static_cast<size_t>(out_channels);c++){
            if(this->inletsConnected[c] && !static_cast<ofSoundBuffer *>(_inletParams[c])->getBuffer().empty()){
                OUT_CH.at(c).copyInput(static_cast<ofSoundBuffer *>(_inletParams[c])->getBuffer().data(),static_cast<ofSoundBuffer *>(_inletParams[c])->getNumFrames());
            }
        }
    }
}

//--------------------------------------------------------------
void AudioDevice::resetSystemObject(){

    vector<bool> tempInletsConn;
    for(int i=0;i<this->numInlets;i++){
        if(this->inletsConnected[i]){
            tempInletsConn.push_back(true);
        }else{
            tempInletsConn.push_back(false);
        }
    }

    ofxXmlSettings XML;

    deviceLoaded      = false;

#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
    if (XML.loadFile(patchFile)){
#else
    if (XML.load(patchFile)){
#endif
        int totalObjects = XML.getNumTags("object");

        if (XML.pushTag("settings")){
            in_channels  = XML.getValue("input_channels",0);
            out_channels = XML.getValue("output_channels",0);
            sampleRateIN = XML.getValue("sample_rate_in",0);
            sampleRateOUT= XML.getValue("sample_rate_out",0);
            bufferSize   = XML.getValue("buffer_size",0);
            XML.popTag();
        }

        this->numInlets  = out_channels;
        this->numOutlets = in_channels;

        IN_CH.clear();
        PN_IN_CH.resize(in_channels);
        LC_IN_CH.resize(in_channels);
        HC_IN_CH.resize(in_channels);
        IN_SCOPE.resize(in_channels);
        OUT_CH.resize(out_channels);

        shortBuffer = new short[bufferSize];
        for (int i = 0; i < bufferSize; i++){
            shortBuffer[i] = 0;
        }

        for( int i = 0; i < out_channels; i++){
            _inletParams[i] = new ofSoundBuffer(shortBuffer,static_cast<size_t>(bufferSize),1,static_cast<unsigned int>(sampleRateOUT));
        }

        for( unsigned int i = 0; i < this->pdspOut.size(); i++){
            this->pdspOut[i].disconnectOut();
        }
        this->pdspOut.clear();

        for( int i = 0; i < in_channels; i++){
            _outletParams[i] = new ofSoundBuffer();
            ofSoundBuffer temp;
            IN_CH.push_back(temp);

            pdsp::PatchNode *tempPN = new pdsp::PatchNode();
            this->pdspOut[i] = *tempPN;
        }

        this->inletsType.clear();
        this->inletsNames.clear();
        this->inletsIDs.clear();
        this->inletsWirelessReceive.clear();

        for( int i = 0; i < out_channels; i++){
            this->addInlet(VP_LINK_AUDIO,"OUT CHANNEL "+ofToString(i+1));
        }

        this->outletsType.clear();
        this->outletsIDs.clear();
        this->outletsWirelessSend.clear();
        for( int i = 0; i < in_channels; i++){
            this->addOutlet(VP_LINK_AUDIO,"IN CHANNEL "+ofToString(i+1));
        }

        this->inletsConnected.clear();
        this->initInletsState();

        for(int i=0;i<this->numInlets;i++){
            if(i<static_cast<int>(tempInletsConn.size())){
                if(tempInletsConn.at(i)){
                    this->inletsConnected.push_back(true);
                }else{
                    this->inletsConnected.push_back(false);
                }
            }else{
                this->inletsConnected.push_back(false);
            }
        }

        this->height      = OBJECT_HEIGHT;

        if(this->numInlets > 6 || this->numOutlets > 6){
            this->height          *= 2;
        }

        if(this->numInlets > 12 || this->numOutlets > 12){
            this->height          *= 2;
        }

        // Save new object config
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
                }else{
                    // remove links to this object if exceed new inlets number
                    if(XML.pushTag("outlets")){
                        int totalLinks = XML.getNumTags("link");
                        for(int l=0;l<totalLinks;l++){
                            if(XML.pushTag("link",l)){
                                int totalTo = XML.getNumTags("to");
                                vector<bool> delLinks;
                                for(int t=0;t<totalTo;t++){
                                    if(XML.pushTag("to",t)){
                                        if(XML.getValue("id", -1) == this->nId && XML.getValue("inlet", -1) > this->getNumInlets()-1){
                                            delLinks.push_back(true);
                                        }else{
                                            delLinks.push_back(false);
                                        }
                                        XML.popTag();
                                    }
                                }
                                for(int d=delLinks.size()-1;d>=0;d--){
                                    if(delLinks.at(d)){
                                        XML.removeTag("to",d);
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

#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
            XML.saveFile();
#else
            XML.save();
#endif

        deviceLoaded      = true;
    }

    this->saveConfig(false);
}

//--------------------------------------------------------------
void AudioDevice::loadDeviceInfo(){

    vector<bool> tempInletsConn;
    for(int i=0;i<this->numInlets;i++){
        if(this->inletsConnected[i]){
            tempInletsConn.push_back(true);
        }else{
            tempInletsConn.push_back(false);
        }
    }

    ofxXmlSettings XML;

#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
    if (XML.loadFile(patchFile)){
#else
    if (XML.load(patchFile)){
#endif
        if (XML.pushTag("settings")){
            in_channels  = XML.getValue("input_channels",0);
            out_channels = XML.getValue("output_channels",0);
            sampleRateIN = XML.getValue("sample_rate_in",0);
            sampleRateOUT= XML.getValue("sample_rate_out",0);
            bufferSize   = XML.getValue("buffer_size",0);
            XML.popTag();
        }

        bool isNewObject = true;

        int totalObjects = XML.getNumTags("object");
        for (int i=0;i<totalObjects;i++){
            if(XML.pushTag("object", i)){
                if(XML.getValue("id",-1) == this->nId){
                    isNewObject = false;
                }
                XML.popTag();
            }
        }

        this->numInlets  = out_channels;
        this->numOutlets = in_channels;

        IN_CH.clear();
        PN_IN_CH.resize(in_channels);
        LC_IN_CH.resize(in_channels);
        HC_IN_CH.resize(in_channels);
        IN_SCOPE.resize(in_channels);
        OUT_CH.resize(out_channels);

        shortBuffer = new short[bufferSize];
        for (int i = 0; i < bufferSize; i++){
            shortBuffer[i] = 0;
        }

        for( int i = 0; i < out_channels; i++){
            _inletParams[i] = new ofSoundBuffer(shortBuffer,static_cast<size_t>(bufferSize),1,static_cast<unsigned int>(sampleRateOUT));
        }

        for( int i = 0; i < (int)this->pdspOut.size(); i++){
            this->pdspOut[i].disconnectOut();
        }
        this->pdspOut.clear();

        for( int i = 0; i < in_channels; i++){
            _outletParams[i] = new ofSoundBuffer();
            ofSoundBuffer temp;
            IN_CH.push_back(temp);

            pdsp::PatchNode *tempPN = new pdsp::PatchNode();
            this->pdspOut[i] = *tempPN;
        }

        this->inletsType.clear();
        this->inletsNames.clear();

        for( int i = 0; i < out_channels; i++){
            this->addInlet(VP_LINK_AUDIO,"OUT CHANNEL "+ofToString(i+1));
        }

        if(isNewObject){
            this->outletsType.clear();
            for( int i = 0; i < in_channels; i++){
                this->addOutlet(VP_LINK_AUDIO,"IN CHANNEL "+ofToString(i+1));
            }
        }

        this->inletsConnected.clear();
        this->initInletsState();

        for(int i=0;i<this->numInlets;i++){
            if(i<static_cast<int>(tempInletsConn.size())){
                if(tempInletsConn.at(i)){
                    this->inletsConnected.push_back(true);
                }else{
                    this->inletsConnected.push_back(false);
                }
            }else{
                this->inletsConnected.push_back(false);
            }
        }

        this->height      = OBJECT_HEIGHT;

        if(this->numInlets > 6 || this->numOutlets > 6){
            this->height          *= 2;
        }

        if(this->numInlets > 12 || this->numOutlets > 12){
            this->height          *= 2;
        }

        deviceLoaded      = true;
    }
}

OBJECT_REGISTER( AudioDevice, "audio device", OFXVP_OBJECT_CAT_SOUND)

#endif
