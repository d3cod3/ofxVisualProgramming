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

#include "OutputWindow.h"

//--------------------------------------------------------------
OutputWindow::OutputWindow() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 0;

    _inletParams[0] = new ofTexture();  // projector
    _inletParams[1] = new ofxLua();     // script reference

    for(int i=0;i<this->numInlets;i++){
        this->inletsConnected.push_back(false);
    }

    isFullscreen            = false;
    scaleH                  = 0.0f;
    isNewScriptConnected    = false;
    inletScriptType         = 0;        // 0 -> ofxLua, 1 -> ofxPythonObject, .....

    output_width            = 320;
    output_height           = 240;

    window_actual_width     = STANDARD_PROJECTOR_WINDOW_WIDTH;
    window_actual_height    = STANDARD_PROJECTOR_WINDOW_HEIGHT;
}

//--------------------------------------------------------------
void OutputWindow::newObject(){
    this->setName("output window");
    this->addInlet(VP_LINK_TEXTURE,"projector");
    this->addInlet(VP_LINK_SCRIPT,"script");

    this->setCustomVar(static_cast<float>(output_width),"OUTPUT_WIDTH");
    this->setCustomVar(static_cast<float>(output_height),"OUTPUT_HEIGHT");
}

//--------------------------------------------------------------
void OutputWindow::setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow){
    loadWindowSettings();

    ofGLFWWindowSettings settings;
    settings.setGLVersion(2,1);
    settings.shareContextWith = mainWindow;
    settings.decorated = true;
    settings.resizable = true;
    settings.stencilBits = 0;
    // RETINA FIX
    if(ofGetScreenWidth() >= RETINA_MIN_WIDTH && ofGetScreenHeight() >= RETINA_MIN_HEIGHT){
        window_actual_width     = STANDARD_PROJECTOR_WINDOW_WIDTH*2;
        window_actual_height    = STANDARD_PROJECTOR_WINDOW_HEIGHT*2;
        settings.setPosition(ofDefaultVec2(mainWindow->getScreenSize().x-(window_actual_width+100),400));
    }else{
        settings.setPosition(ofDefaultVec2(mainWindow->getScreenSize().x-(STANDARD_PROJECTOR_WINDOW_WIDTH+50),200));
    }
    settings.setSize(window_actual_width, window_actual_height);


    window = dynamic_pointer_cast<ofAppGLFWWindow>(ofCreateWindow(settings));
    window->setWindowTitle("Projector"+ofToString(this->getId()));
    window->setVerticalSync(true);

    ofAddListener(window->events().draw,this,&OutputWindow::drawInWindow);
    ofAddListener(window->events().keyPressed,this,&OutputWindow::keyPressed);
    ofAddListener(window->events().keyReleased,this,&OutputWindow::keyReleased);
    ofAddListener(window->events().mouseMoved ,this,&OutputWindow::mouseMoved);
    ofAddListener(window->events().mouseDragged ,this,&OutputWindow::mouseDragged);
    ofAddListener(window->events().mousePressed ,this,&OutputWindow::mousePressed);
    ofAddListener(window->events().mouseReleased ,this,&OutputWindow::mouseReleased);
    ofAddListener(window->events().mouseScrolled ,this,&OutputWindow::mouseScrolled);
    ofAddListener(window->events().windowResized ,this,&OutputWindow::windowResized);

    static_cast<ofTexture *>(_inletParams[0])->allocate(output_width,output_height,GL_RGBA32F_ARB);

    if(static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        ofLog(OF_LOG_NOTICE,"%s: NEW PROJECTOR WINDOW CREATED WITH RESOLUTION %ix%i",this->name.c_str(),output_width,output_height);
    }

    // setup drawing  dimensions
    asRatio = reduceToAspectRatio(output_width,output_height);
    scaleTextureToWindow(window->getWidth(),window->getHeight());

}

//--------------------------------------------------------------
void OutputWindow::updateObjectContent(map<int,PatchObject*> &patchObjects){

    // Manage the different scripts reference available (ofxLua, ofxPython)
    if(!isNewScriptConnected && this->inletsConnected[1]){

        for(map<int,PatchObject*>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if(patchObjects[it->first] != nullptr && it->first != this->getId() && !patchObjects[it->first]->getWillErase()){
                for(int o=0;o<static_cast<int>(it->second->outPut.size());o++){
                    if(!it->second->outPut[o]->isDisabled && it->second->outPut[o]->toObjectID == this->getId()){
                        if(it->second->getName() == "lua script"){
                            _inletParams[1] = new ofxLua();
                            inletScriptType         = 0;
                            //ofLog(OF_LOG_NOTICE,"Init inlet as ofxLua pointer");
                        }else if(it->second->getName() == "python script"){
                            _inletParams[1] = new ofxPythonObject();
                            inletScriptType         = 1;
                            //ofLog(OF_LOG_NOTICE,"Init inlet as ofxPython pointer");
                        }else{
                            _inletParams[1] = nullptr;
                        }
                        break;
                    }
                }
            }
        }
    }

    isNewScriptConnected = this->inletsConnected[1];
}

//--------------------------------------------------------------
void OutputWindow::drawObjectContent(ofxFontStash *font){
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        scaleH = this->width*asRatio.y / asRatio.x;
        static_cast<ofTexture *>(_inletParams[0])->draw(0,this->height/2 - scaleH/2,this->width,scaleH);
    }else{
        ofSetColor(0);
        ofDrawRectangle(0,0,this->width,this->height);
    }
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void OutputWindow::removeObjectContent(){
    window->setWindowShouldClose();
}

//--------------------------------------------------------------
glm::vec2 OutputWindow::reduceToAspectRatio(int _w, int _h){
    glm::vec2 _res;
    int temp = _w*_h;
    if(temp>0){
        for(temp; temp>1; temp--){
            if((_w%temp==0) && (_h%temp==0)){
                _w/=temp;
                _h/=temp;
            }
        }
    }else if (temp<0){
        for (temp; temp<-1; temp++){
            if ((_w%temp==0) && (_h%temp==0)){
                _w/=temp;
                _h/=temp;
            }
        }
    }
    _res = glm::vec2(_w,_h);
    return _res;
}

//--------------------------------------------------------------
void OutputWindow::scaleTextureToWindow(int theScreenW, int theScreenH){
    if(output_width >= output_height){   // horizontal texture
        if(drawW >= theScreenW){
            drawW           = theScreenW;
            drawH           = drawW*asRatio.y / asRatio.x;
            posX            = 0;
            posY            = (theScreenH-drawH)/2.0f;
        }else{
            drawW           = (output_width*theScreenH)/output_height;
            drawH           = theScreenH;
            posX            = (theScreenW-drawW)/2.0f;
            posY            = 0;
        }

    }else{                              // vertical texture
        if(drawH >= theScreenH){
            drawW           = (output_width*theScreenH)/output_height;
            drawH           = theScreenH;
            posX            = (theScreenW-drawW)/2.0f;
            posY            = 0;
        }else{
            drawW           = theScreenW;
            drawH           = drawW*asRatio.y / asRatio.x;
            posX            = 0;
            posY            = (theScreenH-drawH)/2.0f;
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::toggleWindowFullscreen(){
    isFullscreen = !isFullscreen;
    window->toggleFullscreen();

    if(!isFullscreen){
        window->setWindowShape(window_actual_width, window_actual_height);
        scaleTextureToWindow(window->getWidth(),window->getHeight());
    }else{
        scaleTextureToWindow(window->getScreenSize().x,window->getScreenSize().y);
    }
}

//--------------------------------------------------------------
void OutputWindow::drawInWindow(ofEventArgs &e){
    ofBackground(0);
    ofPushStyle();
    ofSetColor(255);
    if(this->inletsConnected[0] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        static_cast<ofTexture *>(_inletParams[0])->draw(posX, posY, drawW, drawH);
    }else{
        ofSetColor(0);
        ofDrawRectangle(posX, posY, drawW, drawH);
    }
    ofPopStyle();
}

//--------------------------------------------------------------
bool OutputWindow::loadWindowSettings(){
    ofxXmlSettings XML;
    bool loaded = false;

    if (XML.loadFile(patchFile)){
        if (XML.pushTag("settings")){
            output_width = XML.getValue("output_width",0);
            output_height = XML.getValue("output_height",0);
            this->setCustomVar(static_cast<float>(output_width),"OUTPUT_WIDTH");
            this->setCustomVar(static_cast<float>(output_height),"OUTPUT_HEIGHT");
            XML.popTag();
        }

        loaded = true;
    }

    return loaded;
}

//--------------------------------------------------------------
void OutputWindow::keyPressed(ofKeyEventArgs &e){
    // OSX: CMD-F, WIN/LINUX: CTRL-F
    if(e.hasModifier(MOD_KEY) && e.keycode == 70){
        toggleWindowFullscreen();
    }

    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(inletScriptType == 0){
            if(static_cast<ofxLua *>(_inletParams[1])->isValid()){
                static_cast<ofxLua *>(_inletParams[1])->scriptKeyPressed(e.key);
            }
        }else if(inletScriptType == 1){
            if(static_cast<ofxPythonObject *>(_inletParams[1])){
                ofxPythonObject at = static_cast<ofxPythonObject *>(_inletParams[1])->attr("keyPressed");
                if(at){
                    at(ofxPythonObject::fromInt(e.key));
                }
            }
        }

    }
}

//--------------------------------------------------------------
void OutputWindow::keyReleased(ofKeyEventArgs &e){
    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(inletScriptType == 0){
            if(static_cast<ofxLua *>(_inletParams[1])->isValid()){
                static_cast<ofxLua *>(_inletParams[1])->scriptKeyReleased(e.key);
            }
        }else if(inletScriptType == 1){
            if(static_cast<ofxPythonObject *>(_inletParams[1])){
                ofxPythonObject at = static_cast<ofxPythonObject *>(_inletParams[1])->attr("keyReleased");
                if(at){
                    at(ofxPythonObject::fromInt(e.key));
                }
            }
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::mouseMoved(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        ofVec2f tm = ofVec2f(((window->events().getMouseX()-posX)/drawW * output_width),((window->events().getMouseY()-posY)/drawH * output_height));
        if(inletScriptType == 0){
            if(static_cast<ofxLua *>(_inletParams[1])->isValid()){
                static_cast<ofxLua *>(_inletParams[1])->scriptMouseMoved(static_cast<int>(tm.x),static_cast<int>(tm.y));
            }
        }else if(inletScriptType == 1){
            if(static_cast<ofxPythonObject *>(_inletParams[1])){
                ofxPythonObject at = static_cast<ofxPythonObject *>(_inletParams[1])->attr("mouseMoved");
                if(at){
                    at(ofxPythonObject::fromInt(static_cast<int>(tm.x)),ofxPythonObject::fromInt(static_cast<int>(tm.y)));
                }
            }
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::mouseDragged(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        ofVec2f tm = ofVec2f(((window->events().getMouseX()-posX)/drawW * output_width),((window->events().getMouseY()-posY)/drawH * output_height));
        if(inletScriptType == 0){
            if(static_cast<ofxLua *>(_inletParams[1])->isValid()){
                static_cast<ofxLua *>(_inletParams[1])->scriptMouseDragged(static_cast<int>(tm.x),static_cast<int>(tm.y), e.button);
            }
        }else if(inletScriptType == 1){
            if(static_cast<ofxPythonObject *>(_inletParams[1])){
                ofxPythonObject at = static_cast<ofxPythonObject *>(_inletParams[1])->attr("mouseDragged");
                if(at){
                    at(ofxPythonObject::fromInt(static_cast<int>(tm.x)),ofxPythonObject::fromInt(static_cast<int>(tm.y)),ofxPythonObject::fromInt(e.button));
                }
            }
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::mousePressed(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        ofVec2f tm = ofVec2f(((window->events().getMouseX()-posX)/drawW * output_width),((window->events().getMouseY()-posY)/drawH * output_height));
        if(inletScriptType == 0){
            if(static_cast<ofxLua *>(_inletParams[1])->isValid()){
                static_cast<ofxLua *>(_inletParams[1])->scriptMousePressed(static_cast<int>(tm.x),static_cast<int>(tm.y), e.button);
            }
        }else if(inletScriptType == 1){
            if(static_cast<ofxPythonObject *>(_inletParams[1])){
                ofxPythonObject at = static_cast<ofxPythonObject *>(_inletParams[1])->attr("mousePressed");
                if(at){
                    at(ofxPythonObject::fromInt(static_cast<int>(tm.x)),ofxPythonObject::fromInt(static_cast<int>(tm.y)),ofxPythonObject::fromInt(e.button));
                }
            }
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::mouseReleased(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        ofVec2f tm = ofVec2f(((window->events().getMouseX()-posX)/drawW * output_width),((window->events().getMouseY()-posY)/drawH * output_height));
        if(inletScriptType == 0){
            if(static_cast<ofxLua *>(_inletParams[1])->isValid()){
                static_cast<ofxLua *>(_inletParams[1])->scriptMouseReleased(static_cast<int>(tm.x),static_cast<int>(tm.y), e.button);
            }
        }else if(inletScriptType == 1){
            if(static_cast<ofxPythonObject *>(_inletParams[1])){
                ofxPythonObject at = static_cast<ofxPythonObject *>(_inletParams[1])->attr("mouseReleased");
                if(at){
                    at(ofxPythonObject::fromInt(static_cast<int>(tm.x)),ofxPythonObject::fromInt(static_cast<int>(tm.y)),ofxPythonObject::fromInt(e.button));
                }
            }
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::mouseScrolled(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && this->inletsConnected[1] && _inletParams[1] != nullptr && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        ofVec2f tm = ofVec2f(((window->events().getMouseX()-posX)/drawW * output_width),((window->events().getMouseY()-posY)/drawH * output_height));
        if(inletScriptType == 0){
            if(static_cast<ofxLua *>(_inletParams[1])->isValid()){
                static_cast<ofxLua *>(_inletParams[1])->scriptMouseScrolled(static_cast<int>(tm.x),static_cast<int>(tm.y), e.scrollX,e.scrollY);
            }
        }else if(inletScriptType == 1){
            if(static_cast<ofxPythonObject *>(_inletParams[1])){
                ofxPythonObject at = static_cast<ofxPythonObject *>(_inletParams[1])->attr("mouseScrolled");
                if(at){
                    at(ofxPythonObject::fromInt(static_cast<int>(tm.x)),ofxPythonObject::fromInt(static_cast<int>(tm.y)),ofxPythonObject::fromInt(static_cast<int>(e.scrollX)),ofxPythonObject::fromInt(static_cast<int>(e.scrollY)));
                }
            }
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::windowResized(ofResizeEventArgs &e){
    scaleTextureToWindow(window->getWidth(),window->getHeight());
}
