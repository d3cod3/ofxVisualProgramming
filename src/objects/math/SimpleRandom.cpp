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

    bang            = false;

}

//--------------------------------------------------------------
void SimpleRandom::newObject(){
    PatchObject::setName( this->objectName );

    this->addInlet(VP_LINK_NUMERIC,"bang");
    this->addInlet(VP_LINK_NUMERIC,"min");
    this->addInlet(VP_LINK_NUMERIC,"max");
    this->addOutlet(VP_LINK_NUMERIC,"random");
}

//--------------------------------------------------------------
void SimpleRandom::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    ofSeedRandom(ofGetElapsedTimeMillis());
}

//--------------------------------------------------------------
void SimpleRandom::updateObjectContent(map<int,shared_ptr<PatchObject>> &patchObjects){
    if(this->inletsConnected[0]){
        if(*(float *)&_inletParams[0] < 1.0){
            bang = false;
        }else{
            bang = true;
        }
    }
    if(bang){
        *(float *)&_outletParams[0] = ofRandom(*(float *)&_inletParams[1],*(float *)&_inletParams[2]);
    }

    if(this->inletsConnected[1] || this->inletsConnected[2]){
        if(lastMinRange.get() != *(float *)&_inletParams[1] || lastMaxRange.get() != *(float *)&_inletParams[2]){
            lastMinRange.get()    = *(float *)&_inletParams[1];
            lastMaxRange.get()    = *(float *)&_inletParams[2];
        }
    }
}

//--------------------------------------------------------------
void SimpleRandom::drawObjectContent(ofxFontStash *font, shared_ptr<ofBaseGLRenderer>& glRenderer){
    ofSetColor(255);
    ofEnableAlphaBlending();

    ofDisableAlphaBlending();
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

            ImGui::Spacing();
            ImGui::PushItemWidth(130);
            ImGui::DragFloat("min", &lastMinRange.get());
            ImGui::Spacing();
            ImGui::PushItemWidth(130);
            ImGui::DragFloat("max", &lastMaxRange.get());

            ImGuiEx::ObjectInfo(
                        "Standard Range controlled random number generator.",
                        "https://mosaic.d3cod3.org/reference.php?r=simple-random");


            ImGui::EndMenu();
        }

        _nodeCanvas.EndNodeMenu();
    }

    // Visualize (Object main view)
    if( _nodeCanvas.BeginNodeContent(ImGuiExNodeView_Visualise) ){

        ImGuiEx::plotValue(*(float *)&_outletParams[0], lastMinRange.get(), lastMaxRange.get());

        _nodeCanvas.EndNodeContent();
    }
}

//--------------------------------------------------------------
void SimpleRandom::removeObjectContent(bool removeFileFromData){
    
}

OBJECT_REGISTER( SimpleRandom, "simple random", OFXVP_OBJECT_CAT_MATH)

#endif
