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

#include "Crossfader.h"

//--------------------------------------------------------------
Crossfader::Crossfader() : PatchObject(){

    this->numInlets  = 3;
    this->numOutlets = 1;

    _inletParams[0] = new ofSoundBuffer();  // audio in 1
    _inletParams[1] = new ofSoundBuffer();  // audio in 2
    _inletParams[2] = new float();          // fade
    *(float *)&_inletParams[2] = 0.0f;

    _outletParams[0] = new ofSoundBuffer(); // audio output L

    this->initInletsState();

    isGUIObject             = true;
    this->isOverGUI         = true;

    isAudioINObject         = true;
    isAudioOUTObject        = true;
    isPDSPPatchableObject   = true;

    loaded                  = false;

}

//--------------------------------------------------------------
void Crossfader::newObject(){
    this->setName(this->objectName);
    this->addInlet(VP_LINK_AUDIO,"in 1");
    this->addInlet(VP_LINK_AUDIO,"in 2");
    this->addInlet(VP_LINK_NUMERIC,"fade");
    this->addOutlet(VP_LINK_AUDIO,"out");

    this->setCustomVar(static_cast<float>(0),"FADE");
}

//--------------------------------------------------------------
void Crossfader::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadAudioSettings();

    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    gui->setAutoDraw(false);
    gui->setUseCustomMouse(true);
    gui->setWidth(this->width);
    gui->onSliderEvent(this, &Crossfader::onSliderEvent);

    header = gui->addHeader("CONFIG",false);
    header->setUseCustomMouse(true);
    header->setCollapsable(true);
    slider = gui->addSlider("Fade", 0,1,0);
    slider->setUseCustomMouse(true);

    gui->setPosition(0,this->height - header->getHeight());
    gui->collapse();
    header->setIsCollapsed(true);
}

//--------------------------------------------------------------
void Crossfader::setupAudioOutObjectContent(pdsp::Engine &engine){

    fade_ctrl >> crossfader.in_fade();
    fade_ctrl.set(0.0f);
    fade_ctrl.enableSmoothing(50.0f);

    this->pdspIn[0] >> crossfader.in_A();
    this->pdspIn[1] >> crossfader.in_B();

    crossfader >> this->pdspOut[0];

    crossfader >> scope >> engine.blackhole();
}

//--------------------------------------------------------------
void Crossfader::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){

    gui->update();
    header->update();
    slider->update();

    if(this->inletsConnected[2]){
        fade_ctrl.set(ofClamp(*(float *)&_inletParams[2],0.0f,1.0f));
        slider->setValue(fade_ctrl.get());
    }

    if(!loaded){
        loaded = true;
        slider->setValue(this->getCustomVar("FADE"));
        fade_ctrl.set(ofClamp(slider->getValue(),0.0f,1.0f));
    }

}

//--------------------------------------------------------------
void Crossfader::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();
    gui->draw();
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void Crossfader::removeObjectContent(bool removeFileFromData){
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspIn.begin(); it != this->pdspIn.end(); it++ ){
        it->second.disconnectAll();
    }
    for(map<int,pdsp::PatchNode>::iterator it = this->pdspOut.begin(); it != this->pdspOut.end(); it++ ){
        it->second.disconnectAll();
    }
}

//--------------------------------------------------------------
void Crossfader::loadAudioSettings(){
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
void Crossfader::audioInObject(ofSoundBuffer &inputBuffer){

}

//--------------------------------------------------------------
void Crossfader::audioOutObject(ofSoundBuffer &outputBuffer){
    // STEREO SIGNAL BUFFERS
    static_cast<ofSoundBuffer *>(_outletParams[0])->copyFrom(scope.getBuffer().data(), bufferSize, 1, sampleRate);
}

//--------------------------------------------------------------
void Crossfader::onSliderEvent(ofxDatGuiSliderEvent e){
    this->setCustomVar(static_cast<float>(e.value),"FADE");
    fade_ctrl.set(ofClamp(static_cast<float>(e.value),0.0f,1.0f));
}

OBJECT_REGISTER( Crossfader, "crossfader", OFXVP_OBJECT_CAT_SOUND)

#endif
