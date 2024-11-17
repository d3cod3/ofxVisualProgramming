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

#include "SimpleRandom.h"

//--------------------------------------------------------------
SimpleRandom::SimpleRandom() :
            PatchObject("simple random"),

            // define default values
            lastMinRange(0.f,"min"),
            lastMaxRange(1.f,"max")
{

    this->numInlets  = 3;
    this->numOutlets = 1;

    _inletParams[0] = new float();  // bang
    *(float *)&_inletParams[0] = 0.0f;

    *(float *)&_inletParams[1] = lastMinRange.get();
    *(float *)&_inletParams[2] = lastMaxRange.get();

    _outletParams[0] = new float(); // output
    *(float *)&_outletParams[0] = 0.0f;

    this->initInletsState();

    isAudioOUTObject        = true;

    bang            = false;

    forceInt        = false;

    loaded          = false;

}

//--------------------------------------------------------------
void SimpleRandom::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"min");
    this->addInlet(VP_LINK_NUMERIC,"max");
    this->addOutlet(VP_LINK_NUMERIC,"random");

    this->setCustomVar(static_cast<float>(lastMinRange.get()),"MIN");
    this->setCustomVar(static_cast<float>(lastMaxRange.get()),"MAX");
    this->setCustomVar(static_cast<float>(forceInt),"FORCE_INT");

}

//--------------------------------------------------------------
void SimpleRandom::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    unusedArgs(mainWindow);

    ofSetRandomSeed(ofGetElapsedTimeMillis());
}

//--------------------------------------------------------------
void SimpleRandom::setupAudioOutObjectContent(pdsp::Engine &engine){
    unusedArgs(engine);

}

//--------------------------------------------------------------
void SimpleRandom::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    unusedArgs(patchObjects);

    if(this->inletsConnected[1] || this->inletsConnected[2]){
        if(lastMinRange.get() != *(float *)&_inletParams[1] || lastMaxRange.get() != *(float *)&_inletParams[2]){
            lastMinRange.get()    = *(float *)&_inletParams[1];
            lastMaxRange.get()    = *(float *)&_inletParams[2];
        }
    }

    if(!loaded){
        loaded = true;

        lastMinRange.get()  = this->getCustomVar("MIN");
        lastMaxRange.get()  = this->getCustomVar("MAX");
        forceInt            = static_cast<bool>(floor(this->getCustomVar("FORCE_INT")));
    }
}

//--------------------------------------------------------------
void SimpleRandom::drawObjectContent(ofTrueTypeFont *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    unusedArgs(font,glRenderer);

    ofSetColor(255);
}

//--------------------------------------------------------------
void SimpleRandom::drawObjectNodeGui( ImGuiEx::NodeCanvas& _nodeCanvas ){

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

        ImGuiEx::plotValue(*(float *)&_outletParams[0], lastMinRange.get(), lastMaxRange.get(), IM_COL32(255,255,255,255), this->scaleFactor);

        _nodeCanvas.EndNodeContent();
    }
}

//--------------------------------------------------------------
void SimpleRandom::drawObjectNodeConfig(){
    ImGui::Spacing();
    ImGui::PushItemWidth(130*scaleFactor);
    if(ImGui::DragFloat("min", &lastMinRange.get())){
        this->setCustomVar(static_cast<float>(lastMinRange.get()),"MIN");
    }
    ImGui::Spacing();
    ImGui::PushItemWidth(130*scaleFactor);
    if(ImGui::DragFloat("max", &lastMaxRange.get())){
        this->setCustomVar(static_cast<float>(lastMaxRange.get()),"MAX");
    }
    ImGui::Spacing();
    if(ImGui::Checkbox("Force Integer",&forceInt)){
        this->setCustomVar(static_cast<float>(forceInt),"FORCE_INT");
    }

    ImGuiEx::ObjectInfo(
                "Standard Range controlled random number generator.",
                "https://mosaic.d3cod3.org/reference.php?r=simple-random", scaleFactor);
}

//--------------------------------------------------------------
void SimpleRandom::removeObjectContent(bool removeFileFromData){
    unusedArgs(removeFileFromData);
}

//--------------------------------------------------------------
void SimpleRandom::audioOutObject(ofSoundBuffer &outputBuffer){
    unusedArgs(outputBuffer);

    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] < 1.0){
            bang = false;
        }else if(!bang){
            bang = true;
            if(forceInt){
                *(float *)&_outletParams[0] = floor(ofRandom(lastMinRange.get(),lastMaxRange.get()));
            }else{
                *(float *)&_outletParams[0] = ofRandom(lastMinRange.get(),lastMaxRange.get());
            }
        }
    }

}

OBJECT_REGISTER( SimpleRandom, "simple random", OFXVP_OBJECT_CAT_MATH)

#endif
