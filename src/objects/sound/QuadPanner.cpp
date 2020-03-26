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

#include "QuadPanner.h"

//--------------------------------------------------------------
QuadPanner::QuadPanner() : PatchObject(){

    this->numInlets  = 3;
    this->numOutlets = 4;

    _inletParams[0] = new ofSoundBuffer();  // audio input
    _inletParams[1] = new float();          // pan X
    _inletParams[2] = new float();          // pan Y
    *(float *)&_inletParams[1] = 0.0f;
    *(float *)&_inletParams[2] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output 1
    _outletParams[1] = new ofSoundBuffer(); // audio output 2
    _outletParams[2] = new ofSoundBuffer(); // audio output 3
    _outletParams[3] = new ofSoundBuffer(); // audio output 4

    this->initInletsState();

    padX                    = 0.5f;
    padY                    = 0.5f;

    isGUIObject             = true;
    this->isOverGUI         = true;

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    loaded                  = false;

}

//--------------------------------------------------------------
void QuadPanner::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_AUDIO,"signal");
    this->addInlet(VP_LINK_NUMERIC,"pan X");
    this->addInlet(VP_LINK_NUMERIC,"pan Y");
    this->addOutlet(VP_LINK_AUDIO,"channel1");
    this->addOutlet(VP_LINK_AUDIO,"channel2");
    this->addOutlet(VP_LINK_AUDIO,"channel3");
    this->addOutlet(VP_LINK_AUDIO,"channel4");

    this->setCustomVar(static_cast<float>(0),"XPOS");
    this->setCustomVar(static_cast<float>(0),"YPOS");
}

//--------------------------------------------------------------
void QuadPanner::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->on2dPadEvent(this, &QuadPanner::on2dPadEvent);

    pad = gui->add2dPad("");
    pad->setUseCustomMouse(true);
    pad->setPoint(ofPoint(this->getCustomVar("XPOS"),this->getCustomVar("YPOS"),0));

    gui->setPosition(0,this->box->getHeight()-pad->getHeight());
}

//--------------------------------------------------------------
void QuadPanner::setupAudioOutObjectContent(pdsp::Engine &engine){

    gain_ctrl1 >> gain1.in_mod();
    gain_ctrl1.set(0.0f);
    gain_ctrl1.enableSmoothing(50.0f);

    gain_ctrl2 >> gain2.in_mod();
    gain_ctrl2.set(0.0f);
    gain_ctrl2.enableSmoothing(50.0f);

    gain_ctrl3 >> gain3.in_mod();
    gain_ctrl3.set(0.0f);
    gain_ctrl3.enableSmoothing(50.0f);

    gain_ctrl4 >> gain4.in_mod();
    gain_ctrl4.set(0.0f);
    gain_ctrl4.enableSmoothing(50.0f);

    this->pdspIn[0] >> gain1 >> this->pdspOut[0];
    this->pdspIn[0] >> gain2 >> this->pdspOut[1];
    this->pdspIn[0] >> gain3 >> this->pdspOut[2];
    this->pdspIn[0] >> gain4 >> this->pdspOut[3];

    this->pdspIn[0] >> gain1 >> scope1 >> engine.blackhole();
    this->pdspIn[0] >> gain2 >> scope2 >> engine.blackhole();
    this->pdspIn[0] >> gain3 >> scope3 >> engine.blackhole();
    this->pdspIn[0] >> gain4 >> scope4 >> engine.blackhole();
}

//--------------------------------------------------------------
void QuadPanner::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects, ofxThreadedFileDialog &fd){

    gui->update();
    pad->update();

    if(this->inletsConnected[1]){
        if(*(float *)&_inletParams[1] == 0.0){
            pad->setPoint(ofPoint(0.000001,pad->getPoint().y,pad->getPoint().z));
        }else if(*(float *)&_inletParams[1] == 1.0){
            pad->setPoint(ofPoint(pad->getBounds().width*0.999999,pad->getPoint().y,pad->getPoint().z));
        }else{
            pad->setPoint(ofPoint(*(float *)&_inletParams[1]*pad->getBounds().width,pad->getPoint().y,pad->getPoint().z));
        }
        padX    = ofClamp(*(float *)&_inletParams[1],0.0f,1.0f);
    }
    if(this->inletsConnected[2]){
        if(*(float *)&_inletParams[2] == 0.0){
            pad->setPoint(ofPoint(pad->getPoint().x,0.000001,pad->getPoint().z));
        }else if(*(float *)&_inletParams[2] == 1.0){
            pad->setPoint(ofPoint(pad->getPoint().x,pad->getBounds().height*0.999999,pad->getPoint().z));
        }else{
            pad->setPoint(ofPoint(pad->getPoint().x,*(float *)&_inletParams[2]*pad->getBounds().height,pad->getPoint().z));
        }
        padY    = ofClamp(*(float *)&_inletParams[2],0.0f,1.0f);
    }

    gain_ctrl1.set(ofClamp(ofMap(padY,0.0,1.0,1.0,0.0),0.0f,1.0f) * ofClamp(ofMap(padX,0.0,1.0,1.0,0.0),0.0f,1.0f));
    gain_ctrl2.set(ofClamp(ofMap(padY,0.0,1.0,1.0,0.0),0.0f,1.0f) * padX);
    gain_ctrl3.set(ofClamp(padY,0.0f,1.0f) * ofClamp(ofMap(padX,0.0,1.0,1.0,0.0),0.0f,1.0f));
    gain_ctrl4.set(ofClamp(padY,0.0f,1.0f) * padX);

    if(!loaded){
        loaded = true;
        pad->setPoint(ofPoint(this->getCustomVar("XPOS"),this->getCustomVar("YPOS"),0));
        padX                    = this->getCustomVar("XPOS")/pad->getBounds().width;
        padY                    = this->getCustomVar("YPOS")/pad->getBounds().height;
        gain_ctrl1.set(ofClamp(ofMap(this->getCustomVar("YPOS")/pad->getBounds().height,0.0,1.0,1.0,0.0),0.0f,1.0f) * ofClamp(ofMap(this->getCustomVar("XPOS")/pad->getBounds().width,0.0,1.0,1.0,0.0),0.0f,1.0f));
        gain_ctrl2.set(ofClamp(ofMap(this->getCustomVar("YPOS")/pad->getBounds().height,0.0,1.0,1.0,0.0),0.0f,1.0f) * this->getCustomVar("XPOS")/pad->getBounds().width);
        gain_ctrl3.set(ofClamp(this->getCustomVar("YPOS")/pad->getBounds().height,0.0f,1.0f) * ofClamp(ofMap(this->getCustomVar("XPOS")/pad->getBounds().width,0.0,1.0,1.0,0.0),0.0f,1.0f));
        gain_ctrl4.set(ofClamp(this->getCustomVar("YPOS")/pad->getBounds().height,0.0f,1.0f) * this->getCustomVar("XPOS")/pad->getBounds().width);
    }

}

//--------------------------------------------------------------
void QuadPanner::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void QuadPanner::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void QuadPanner::loadAudioSettings(){
    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){
        if (XML.pushTag("settings")){
            sampleRate = XML.getValue("sample_rate_in",0);
            bufferSize = XML.getValue("buffer_size",0);
            XML.popTag();
        }
    }
}

//--------------------------------------------------------------
void QuadPanner::audioInObject(ofSoundBuffer &inputBuffer){

}

//--------------------------------------------------------------
void QuadPanner::audioOutObject(ofSoundBuffer &outputBuffer){
    // QUAD SIGNAL BUFFERS
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope1.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[1])->copyFrom(scope2.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[2])->copyFrom(scope3.getBuffer().data(), bufferSize, 1, sampleRate);
    static_cast<ofSoundBuffer *>(_outletParams[3])->copyFrom(scope4.getBuffer().data(), bufferSize, 1, sampleRate);
}

//--------------------------------------------------------------
void QuadPanner::mouseMovedObjectContent(ofVec3f _m){
    gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    pad->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));

    this->isOverGUI = pad->hitTest(_m-this->getPos());

}

//--------------------------------------------------------------
void QuadPanner::dragGUIObject(ofVec3f _m){
    if(this->isOverGUI){
        gui->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
        pad->setCustomMousePos(static_cast<int>(_m.x - this->getPos().x),static_cast<int>(_m.y - this->getPos().y));
    }else{
        ofNotifyEvent(dragEvent, nId);

        box->setFromCenter(_m.x, _m.y,box->getWidth(),box->getHeight());
        headerBox->set(box->getPosition().x,box->getPosition().y,box->getWidth(),headerHeight);

        x = box->getPosition().x;
        y = box->getPosition().y;

        for(int j=0;j<static_cast<int>(outPut.size());j++){
            outPut[j]->linkVertices[0].move(outPut[j]->posFrom.x,outPut[j]->posFrom.y);
            outPut[j]->linkVertices[1].move(outPut[j]->posFrom.x+20,outPut[j]->posFrom.y);
        }
    }
}

//--------------------------------------------------------------
void QuadPanner::on2dPadEvent(ofxDatGui2dPadEvent e){
    this->setCustomVar(static_cast<float>(e.x),"XPOS");
    this->setCustomVar(static_cast<float>(e.y),"YPOS");

    padX                    = static_cast<float>(e.x/pad->getBounds().width);
    padY                    = static_cast<float>(e.y/pad->getBounds().height);

    gain_ctrl1.set(ofClamp(ofMap(static_cast<float>(e.y/pad->getBounds().height),0.0,1.0,1.0,0.0),0.0f,1.0f) * ofClamp(ofMap(static_cast<float>(e.x/pad->getBounds().width),0.0,1.0,1.0,0.0),0.0f,1.0f));
    gain_ctrl2.set(ofClamp(ofMap(static_cast<float>(e.y/pad->getBounds().height),0.0,1.0,1.0,0.0),0.0f,1.0f) * static_cast<float>(e.x/pad->getBounds().width));
    gain_ctrl3.set(ofClamp(static_cast<float>(e.y/pad->getBounds().height),0.0f,1.0f) * ofClamp(ofMap(static_cast<float>(e.x/pad->getBounds().width),0.0,1.0,1.0,0.0),0.0f,1.0f));
    gain_ctrl4.set(ofClamp(static_cast<float>(e.y/pad->getBounds().height),0.0f,1.0f) * static_cast<float>(e.x/pad->getBounds().width));
}

OBJECT_REGISTER( QuadPanner, "quad panner", OFXVP_OBJECT_CAT_SOUND)
