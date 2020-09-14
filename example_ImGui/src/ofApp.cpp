/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2018 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>
    Copyright (c) 2020 Daan DE LANGE - http://daandelange.com/

    Mosaic is distributed under the MIT License. This gives everyone the
    freedoms to use Mosaic in any context: commercial or non-commercial,
    public or private, open or closed source.

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

#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetWindowTitle("ofxVisualProgramming Example");
    ofSetDrawBitmapMode(OF_BITMAPMODE_SIMPLE);

    // Setup your GUI before ofxVP
    myAppGui.setup(nullptr, false);

    // Pass the variable to ofxVP if you're using your ImGui instance too
    visualProgramming = new ofxVisualProgramming();
    visualProgramming->setup( &myAppGui );
    visualProgramming->canvasViewport.set(glm::vec2(0,0), glm::vec2(ofGetWidth(), ofGetHeight()));

}

//--------------------------------------------------------------
void ofApp::update(){
    visualProgramming->update();
    visualProgramming->canvasViewport.set(glm::vec2(0,0), glm::vec2(ofGetWidth(), ofGetHeight()));
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(20);

    // Draw a box with available objects.
    myAppGui.begin();
    if(ImGui::Begin("Objects")){
        ofxVPObjects::factory::objectCategories& objectsMatrix = ofxVPObjects::factory::getCategories();
        for(ofxVPObjects::factory::objectCategories::iterator it = objectsMatrix.begin(); it != objectsMatrix.end(); ++it ){
            if(ImGui::BeginMenu(it->first.c_str())){
                std::sort(it->second.begin(), it->second.end());
                for(int j=0;j<static_cast<int>(it->second.size());j++){
                    if(it->second.at(j) != "audio device"){
                        if(ImGui::MenuItem(it->second.at(j).c_str())){
                            visualProgramming->addObject(it->second.at(j),ofVec2f(visualProgramming->canvas.getMovingPoint().x + 200,visualProgramming->canvas.getMovingPoint().y + 200));

                        }
                    }
                }
                ImGui::EndMenu();
            }
        }
    }
    ImGui::End();
    myAppGui.end();

    // Draw to vp Gui
    visualProgramming->draw();

    // Manually render ImGui once ofxVP rendered to it.
    myAppGui.draw();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
