/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2022 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "FboParticles.h"

//--------------------------------------------------------------
FboParticles::FboParticles() : PatchObject("fbo particles"){

    this->numInlets  = 3;
    this->numOutlets = 2;

    // fbo inlet
    _inletParams[0] = new ofxPingPong();
    // effect control
    *(float *)&_inletParams[1] = 0.0f; // particle system origin X
    *(float *)&_inletParams[2] = 0.0f; // particle system otigin Y

    _outletParams[0] = new ofxPingPong();
    _outletParams[1] = new ofTexture();

    kuro    = new ofImage();

    posX = posY = drawW = drawH = 0.0f;

    _x = 0.5f;
    _y = 0.5f;

    output_width    = STANDARD_TEXTURE_WIDTH;
    output_height   = STANDARD_TEXTURE_HEIGHT;

    prevW   = this->width;
    prevH   = this->height;

    loaded  = false;

    this->setIsResizable(true);
    this->setIsTextureObj(true);

    this->initInletsState();

}

//--------------------------------------------------------------
void FboParticles::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_FBO,"fboIN");
    this->addInlet(VP_LINK_NUMERIC,"x");
    this->addInlet(VP_LINK_NUMERIC,"y");

    this->addOutlet(VP_LINK_FBO,"fboOUT");
    this->addOutlet(VP_LINK_TEXTURE,"texture");

    this->setCustomVar(static_cast<float>(output_width),"OUTPUT_WIDTH");
    this->setCustomVar(static_cast<float>(output_height),"OUTPUT_HEIGHT");

    this->setCustomVar(static_cast<float>(prevW),"WIDTH");
    this->setCustomVar(static_cast<float>(prevH),"HEIGHT");

    this->setCustomVar(static_cast<float>(_x),"XPOS");
    this->setCustomVar(static_cast<float>(_y),"YPOS");
}

//--------------------------------------------------------------
void FboParticles::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    initResolution();

    // load kuro
    kuro->load("images/kuro.jpg");

    initFBOEffect();
}

//--------------------------------------------------------------
void FboParticles::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    // particle system origin X
    if(this->inletsConnected[1]){
        _x = ofClamp(*(float *)&_inletParams[1],0.0f,1.0f);
    }

    // particle system origin Y
    if(this->inletsConnected[2]){
        _y = ofClamp(*(float *)&_inletParams[2],0.0f,1.0f);
    }

    fboW = static_cast<ofTexture *>(_outletParams[1])->getWidth();
    fboH = static_cast<ofTexture *>(_outletParams[1])->getHeight();
    fboRect.set(0,0,fboW,fboH);

    updateFBOEffect();

    if(!loaded){
        loaded = true;
        prevW = this->getCustomVar("WIDTH");
        prevH = this->getCustomVar("HEIGHT");
        this->width             = prevW;
        this->height            = prevH;
        _x = this->getCustomVar("XPOS");
        _y = this->getCustomVar("YPOS");
    }
}

//--------------------------------------------------------------
void FboParticles::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){

    ///////////////////////////////////////////
    // FBO DRAW
    if(this->inletsConnected[0] && this->getInletType(0) == VP_LINK_FBO && static_cast<ofxPingPong *>(_inletParams[0])->src->isAllocated()){

        // fbo outlet
        static_cast<ofxPingPong *>(_outletParams[0])->src = static_cast<ofxPingPong *>(_inletParams[0])->dst;

        static_cast<ofxPingPong *>(_outletParams[0])->dst->begin();
        ofPushView();
        ofPushStyle();
        ofPushMatrix();
        ofEnableAlphaBlending();
        // ping pong
        ofSetColor(255);
        static_cast<ofxPingPong *>(_outletParams[0])->src->draw(0,0);
        // draw effect
        drawFBOEffect();
        ofPopMatrix();
        ofPopStyle();
        ofPopView();
        static_cast<ofxPingPong *>(_outletParams[0])->dst->end();

        static_cast<ofxPingPong *>(_outletParams[0])->swap();

        // texture outlet
        *static_cast<ofTexture *>(_outletParams[1]) = static_cast<ofxPingPong *>(_outletParams[0])->dst->getTexture();
    }else{
        *static_cast<ofTexture *>(_outletParams[1]) = kuro->getTexture();
    }
    ///////////////////////////////////////////

    ofSetColor(255);
    if(static_cast<ofTexture *>(_outletParams[1])->isAllocated()){
        // draw node texture preview with OF
        if(scaledObjW*canvasZoom > 90.0f){
            drawNodeOFTexture(*static_cast<ofTexture *>(_outletParams[1]), posX, posY, drawW, drawH, objOriginX, objOriginY, scaledObjW, scaledObjH, canvasZoom, this->scaleFactor);
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
void FboParticles::initFBOEffect(){

    topEmitter.setVelocity(ofVec3f(10.0,0.0));
    topEmitter.posSpread = ofVec3f(10,10.0);
    topEmitter.velSpread = ofVec3f(10.0,10);
    topEmitter.life = 600;
    topEmitter.lifeSpread = 5.0;
    topEmitter.numPars = 3;
    topEmitter.color = ofColor(200,100,100);
    topEmitter.colorSpread = ofColor(50,50,50);
    topEmitter.size = 32;

    vectorField.allocate(128, 128, 3);

    rotAcc = 25;
    gravAcc = 80;
    drag = 0.96;
    fieldMult = 0.4;

    pTex.load("images/particle.png");
}

//--------------------------------------------------------------
void FboParticles::updateFBOEffect(){

    topEmitter.setPosition(ofVec3f(fboW*_x,fboH*_y,0));

    for(size_t y = 0; y < vectorField.getHeight(); y++)
        for(size_t x=0; x< vectorField.getWidth(); x++){
            float angle = ofNoise(x/(float)vectorField.getWidth()*4.0, y/(float)vectorField.getHeight()*4.0,ofGetElapsedTimef()*0.05)*TWO_PI*2.0;
            ofVec2f dir(cos(angle), sin(angle));
            dir.normalize().scale(ofNoise(x/(float)vectorField.getWidth()*4.0, y/(float)vectorField.getHeight()*4.0,ofGetElapsedTimef()*0.05+10.0));
            vectorField.setColor(x, y, ofColor_<float>(dir.x,dir.y, 0));
        }

    particleSystem.gravitateTo(ofPoint(fboW/2,fboH/2), gravAcc, 1, 10.0, false);
    particleSystem.rotateAround(ofPoint(fboW/2,fboH/2), rotAcc, 10.0, false);
    particleSystem.applyVectorField(vectorField.getData(), vectorField.getWidth(), vectorField.getHeight(), vectorField.getNumChannels(), fboRect, fieldMult);

    particleSystem.update((ofGetLastFrameTime() * 60), drag);

    particleSystem.addParticles(topEmitter);

}

//--------------------------------------------------------------
void FboParticles::drawFBOEffect(){

    particleSystem.draw(pTex.getTexture());

}

//--------------------------------------------------------------
void FboParticles::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

    // get imgui canvas zoom
    canvasZoom = _nodeCanvas.GetCanvasScale();

}

//--------------------------------------------------------------
void FboParticles::drawObjectNodeConfig(){

    ImGui::Spacing();
    ImGui::Text("Rendering at: %.0fx%.0f",static_cast<ofTexture *>(_outletParams[1])->getWidth(),static_cast<ofTexture *>(_outletParams[1])->getHeight());
    ImGui::Spacing();

    ImGui::Text("Particle System Origin");
    if(ImGuiEx::Pad2D( ImGui::GetContentRegionAvail().x,ImGui::GetContentRegionAvail().x/static_cast<ofTexture *>(_outletParams[1])->getWidth()*static_cast<ofTexture *>(_outletParams[1])->getHeight(),&_x,&_y)){
        this->setCustomVar(static_cast<float>(_x),"XPOS");
        this->setCustomVar(static_cast<float>(_y),"YPOS");
    }
    ImGui::Spacing();

    ImGuiEx::ObjectInfo(
                "FBO particle system effect",
                "https://mosaic.d3cod3.org/reference.php?r=fbo-particles", scaleFactor);
}

//--------------------------------------------------------------
void FboParticles::removeObjectContent(bool removeFileFromData){

}

//--------------------------------------------------------------
void FboParticles::initResolution(){
    output_width = static_cast<int>(floor(this->getCustomVar("OUTPUT_WIDTH")));
    output_height = static_cast<int>(floor(this->getCustomVar("OUTPUT_HEIGHT")));

    // init pingpong
    _outletParams[0] = new ofxPingPong();
    static_cast<ofxPingPong *>(_outletParams[0])->allocate(output_width,output_height);

}

OBJECT_REGISTER( FboParticles, "fbo particles", OFXVP_OBJECT_CAT_SURFACE)

#endif
