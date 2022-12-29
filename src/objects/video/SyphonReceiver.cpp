/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2021 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#if defined(TARGET_WIN32) || defined(TARGET_LINUX)
    // Unavailable on windows and linux
#elif !defined(OFXVP_BUILD_WITH_MINIMAL_OBJECTS)

#include "SyphonReceiver.h"

//--------------------------------------------------------------
SyphonReceiver::SyphonReceiver() : PatchObject("syphon receiver"){

    this->numInlets  = 0;
    this->numOutlets = 1;

    _outletParams[0] = new ofTexture(); // input

    this->initInletsState();

    posX = posY = drawW = drawH = 0.0f;

    needToGrab = true;

    this->setIsTextureObj(true);

}

//--------------------------------------------------------------
void SyphonReceiver::newObject(){
    PatchObject::setName( this->objectName );

    this->addOutlet(VP_LINK_TEXTURE,"textureReceived");
}

//--------------------------------------------------------------
void SyphonReceiver::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    this->unusedArgs(mainWindow);
  
    //setup our directory
    dir.setup();
    //setup our client
    syphonClient.setup();

    //register for our directory's callbacks
    ofAddListener(dir.events.serverAnnounced, this, &SyphonReceiver::serverAnnounced);
    ofAddListener(dir.events.serverRetired, this, &SyphonReceiver::serverRetired);

    dirIdx = -1;

}

//--------------------------------------------------------------
void SyphonReceiver::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    syphonClient.bind();
    syphonTexture = syphonClient.getTexture();
    syphonClient.unbind();

    if(needToGrab && dir.isValidIndex(dirIdx) && dir.size() > 0 && syphonTexture.isAllocated() && syphonTexture.getWidth() > 0 && syphonTexture.getHeight() > 0){
        needToGrab = false;
        static_cast<ofTexture *>(_outletParams[0])->allocate(syphonTexture.getWidth(), syphonTexture.getHeight(), GL_RGB );
    }

    if(static_cast<ofTexture *>(_outletParams[0])->isAllocated() && dir.isValidIndex(dirIdx)){
        *static_cast<ofTexture *>(_outletParams[0]) = syphonTexture;
    }

}

//--------------------------------------------------------------
void SyphonReceiver::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){

    ofSetColor(255);
    if(static_cast<ofTexture *>(_outletParams[0])->isAllocated()){
        // draw node texture preview with OF
        if(scaledObjW*canvasZoom > 90.0f){
            drawNodeOFTexture(*static_cast<ofTexture *>(_outletParams[0]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
        }
    }else{
        // background
        if(scaledObjW*canvasZoom > 90.0f){
            ofSetColor(34,34,34);
            ofDrawRectangle(objOriginX - (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/canvasZoom), objOriginY-(IMGUI_EX_NODE_HEADER_HEIGHT*this->scaleFactor/canvasZoom),scaledObjW + (IMGUI_EX_NODE_PINS_WIDTH_NORMAL*this->scaleFactor/canvasZoom),scaledObjH + (((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*this->scaleFactor)/canvasZoom) );
        }
    }
}

//--------------------------------------------------------------
void SyphonReceiver::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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
void SyphonReceiver::drawObjectNodeConfig(){
    ImGuiEx::ObjectInfo(
                "Receive (local network) video from the syphon. It will automatically receive the last syphon server published.",
                "https://mosaic.d3cod3.org/reference.php?r=syphon-receiver", scaleFactor);
}

//--------------------------------------------------------------
void SyphonReceiver::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void SyphonReceiver::serverAnnounced(ofxSyphonServerDirectoryEventArgs &arg){
    for( auto& dir : arg.servers ){
        ofLogNotice("ofxSyphonServerDirectory Server Announced")<<" Server Name: "<<dir.serverName <<" | App Name: "<<dir.appName;
        syphonClient.setServerName(dir.serverName);
        syphonClient.setApplicationName(dir.appName);
    }
    dirIdx = 0;
}

//--------------------------------------------------------------
void SyphonReceiver::serverRetired(ofxSyphonServerDirectoryEventArgs &arg){
    for( auto& dir : arg.servers ){
        ofLogNotice("ofxSyphonServerDirectory Server Retired")<<" Server Name: "<<dir.serverName <<" | App Name: "<<dir.appName;
    }
    dirIdx = 0;
}

OBJECT_REGISTER( SyphonReceiver, "syphon receiver", OFXVP_OBJECT_CAT_TEXTURE)

#endif
