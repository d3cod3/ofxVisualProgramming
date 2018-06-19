#include "OutputWindow.h"

//--------------------------------------------------------------
OutputWindow::OutputWindow() : PatchObject(){

    this->numInlets  = 2;
    this->numOutlets = 0;

    _inletParams[0] = new ofTexture();  // projector
    _inletParams[1] = new ofxLua();     // lua script reference

    for(int i=0;i<this->numInlets;i++){
        this->inletsConnected.push_back(false);
    }

    isFullscreen    = false;
    scaleH          = 0.0f;

    output_width    = 1280;
    output_height   = 720;

    window_actual_width = STANDARD_PROJECTOR_WINDOW_WIDTH;
    window_actual_height = STANDARD_PROJECTOR_WINDOW_HEIGHT;
}

//--------------------------------------------------------------
void OutputWindow::newObject(){
    this->setName("output window");
    this->addInlet(VP_LINK_TEXTURE,"projector");
    this->addInlet(VP_LINK_ARRAY,"lua script");
}

//--------------------------------------------------------------
void OutputWindow::setupObjectContent(shared_ptr<ofAppBaseWindow> &mainWindow){

    string GLMajor = ofToString(glGetString(GL_VERSION)).substr(0,1);
    string GLMinor = ofToString(glGetString(GL_VERSION)).substr(2,1);

    ofGLFWWindowSettings settings;
    settings.setGLVersion(ofToInt(GLMajor),ofToInt(GLMinor));
    settings.shareContextWith = mainWindow;
    settings.resizable = false;
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


    window = ofCreateWindow(settings);
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

    asRatio = reduceToAspectRatio(output_width,output_height);

    static_cast<ofTexture *>(_inletParams[0])->allocate(output_width,output_height,GL_RGBA);

    // setup drawing  dimensions
    if(output_width > window->getWidth()){
        drawW           = (output_width*window->getHeight())/output_height;
        drawH           = window->getHeight();
        posX            = (window->getWidth()-drawW)/2.0f;
        posY            = 0;
    }else{
        if(output_height < window->getHeight()){
            drawW           = window->getWidth();
            drawH           = (output_height*window->getWidth())/output_width;
            posX            = 0;
            posY            = (window->getHeight()-drawH)/2.0f;
        }else{
            drawW           = (output_width*window->getHeight())/output_height;
            drawH           = window->getHeight();
            posX            = (window->getWidth()-drawW)/2.0f;
            posY            = 0;
        }
    }

}

//--------------------------------------------------------------
void OutputWindow::updateObjectContent(){

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
void OutputWindow::toggleWindowFullscreen(){
    window->toggleFullscreen();
    isFullscreen = !isFullscreen;

    if(!isFullscreen){
        window->setWindowShape(window_actual_width, window_actual_height);
        if(width > window->getWidth()){
            drawW           = window->getWidth();
            drawH           = drawW*asRatio.y / asRatio.x;
            posX            = 0;
            posY            = (window->getHeight()-drawH)/2.0f;
        }else{
            if(height < window->getHeight()){
                drawW           = window->getWidth();
                drawH           = (output_height*window->getWidth())/output_width;
                posX            = 0;
                posY            = (window->getHeight()-drawH)/2.0f;
            }else{
                drawW           = (output_width*window->getHeight())/output_height;
                drawH           = window->getHeight();
                posX            = (window->getWidth()-drawW)/2.0f;
                posY            = 0;
            }
        }
    }else{
        int theScreenW = window->getScreenSize().x;
        int theScreenH = window->getScreenSize().y;

        if(output_width > theScreenW && output_height > theScreenH){ // width & height bigger than screen
            if(asRatio.x > asRatio.y){ // horizontal texture
                drawW       = theScreenW;
                drawH       = (asRatio.y*drawW)/asRatio.x;
                posX        = 0;
                posY        = (theScreenH-drawH)/2.0f;
            }else{ // vertical texture
                drawH       = theScreenH;
                drawW       = (asRatio.x*drawH)/asRatio.y;
                posX        = (theScreenW-drawW)/2.0f;
                posY        = 0;
            }
        }else if(output_width > theScreenW && output_height <= theScreenH){ // width bigger
            // horizontal texture only (due to landscape screens nature)
            drawW           = theScreenW;
            drawH           = (asRatio.y*drawW)/asRatio.x;
            posX            = 0;
            posY            = (theScreenH-drawH)/2.0f;
        }else if(output_width <= theScreenW && output_height > theScreenH){ // height bigger
            drawH           = theScreenH;
            drawW           = (asRatio.x*drawH)/asRatio.y;
            posX            = (theScreenW-drawW)/2.0f;
            posY            = 0;
        }else{ // smaller than screen
            if(asRatio.x > asRatio.y){ // horizontal texture
                if((asRatio.x/asRatio.y) < 3){
                    drawH       = theScreenH;
                    drawW       = (output_width*drawH)/output_height;
                    posX        = (theScreenW-drawW)/2.0f;
                    posY        = 0;
                }else{
                    drawW       = theScreenW;
                    drawH       = (output_height*drawW)/output_width;
                    posX        = 0;
                    posY        = (theScreenH-drawH)/2.0f;
                }
            }else{ // vertical texture
                drawW       = theScreenW;
                drawH       = (output_height*drawW)/output_width;
                posX        = 0;
                posY        = (theScreenH-drawH)/2.0f;
            }
        }
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
void OutputWindow::keyPressed(ofKeyEventArgs &e){
    // OSX: CMD-F, WIN/LINUX: CTRL-F
    if(e.hasModifier(MOD_KEY) && e.keycode == 70){
        toggleWindowFullscreen();
    }

    if(this->inletsConnected[0] && this->inletsConnected[1] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(static_cast<ofxLua *>(_inletParams[1])->isValid()){
            static_cast<ofxLua *>(_inletParams[1])->scriptKeyPressed(e.key);
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::keyReleased(ofKeyEventArgs &e){
    if(this->inletsConnected[0] && this->inletsConnected[1] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(static_cast<ofxLua *>(_inletParams[1])->isValid()){
            static_cast<ofxLua *>(_inletParams[1])->scriptKeyReleased(e.key);
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::mouseMoved(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && this->inletsConnected[1] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(static_cast<ofxLua *>(_inletParams[1])->isValid()){
            ofVec2f tm = ofVec2f(((window->events().getMouseX()-posX)/drawW * output_width),((window->events().getMouseY()-posY)/drawH * output_height));
            static_cast<ofxLua *>(_inletParams[1])->scriptMouseMoved(static_cast<int>(tm.x),static_cast<int>(tm.y));
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::mouseDragged(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && this->inletsConnected[1] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        ofVec2f tm = ofVec2f(((window->events().getMouseX()-posX)/drawW * output_width),((window->events().getMouseY()-posY)/drawH * output_height));
        static_cast<ofxLua *>(_inletParams[1])->scriptMouseDragged(static_cast<int>(tm.x),static_cast<int>(tm.y), e.button);
    }
}

//--------------------------------------------------------------
void OutputWindow::mousePressed(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && this->inletsConnected[1] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(static_cast<ofxLua *>(_inletParams[1])->isValid()){
            ofVec2f tm = ofVec2f(((window->events().getMouseX()-posX)/drawW * output_width),((window->events().getMouseY()-posY)/drawH * output_height));
            static_cast<ofxLua *>(_inletParams[1])->scriptMousePressed(static_cast<int>(tm.x),static_cast<int>(tm.y), e.button);
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::mouseReleased(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && this->inletsConnected[1] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(static_cast<ofxLua *>(_inletParams[1])->isValid()){
            ofVec2f tm = ofVec2f(((window->events().getMouseX()-posX)/drawW * output_width),((window->events().getMouseY()-posY)/drawH * output_height));
            static_cast<ofxLua *>(_inletParams[1])->scriptMouseReleased(static_cast<int>(tm.x),static_cast<int>(tm.y), e.button);
        }
    }
}

//--------------------------------------------------------------
void OutputWindow::mouseScrolled(ofMouseEventArgs &e){
    if(this->inletsConnected[0] && this->inletsConnected[1] && static_cast<ofTexture *>(_inletParams[0])->isAllocated()){
        if(static_cast<ofxLua *>(_inletParams[1])->isValid()){
            ofVec2f tm = ofVec2f(((window->events().getMouseX()-posX)/drawW * output_width),((window->events().getMouseY()-posY)/drawH * output_height));
            static_cast<ofxLua *>(_inletParams[1])->scriptMouseScrolled(static_cast<int>(tm.x),static_cast<int>(tm.y), e.scrollX,e.scrollY);
        }
    }
}
